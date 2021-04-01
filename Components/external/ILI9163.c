#include <ioCC2530.h>
#include "comdef.h"
#include "hal_types.h"
#include "regs.h"
#include "lcd.h"
#include "fonts/Font5x7Fixed.h"


#define WAIT1us()  TIMIF = TIMIF & 0xE7; while ( (TIMIF & 0x10) == 0);
#define WAITms(ms) for(uint16_t i=0; i < ms; i++){\
    TIMIF = TIMIF & 0xE7; while ( (TIMIF & 0x10) == 0);\
  }


#define ST77XX_NOP 0x00
#define ST77XX_SWRESET 0x01
#define ST77XX_RDDID 0x04
#define ST77XX_RDDST 0x09

#define ST77XX_SLPIN 0x10
#define ST77XX_SLPOUT 0x11
#define ST77XX_PTLON 0x12
#define ST77XX_NORON 0x13

#define ST77XX_INVOFF 0x20
#define ST77XX_INVON 0x21
#define ST77XX_DISPOFF 0x28
#define ST77XX_DISPON 0x29
#define ST77XX_CASET 0x2A
#define ST77XX_RASET 0x2B
#define ST77XX_RAMWR 0x2C
#define ST77XX_RAMRD 0x2E

#define ST77XX_PTLAR 0x30
#define ST77XX_TEOFF 0x34
#define ST77XX_TEON 0x35
#define ST77XX_MADCTL 0x36
#define ST77XX_COLMOD 0x3A

#define ST77XX_MADCTL_MY 0x80
#define ST77XX_MADCTL_MX 0x40
#define ST77XX_MADCTL_MV 0x20
#define ST77XX_MADCTL_ML 0x10
#define ST77XX_MADCTL_RGB 0x00

#define ST77XX_RDID1 0xDA
#define ST77XX_RDID2 0xDB
#define ST77XX_RDID3 0xDC
#define ST77XX_RDID4 0xDD

#define ST7735_MADCTL_BGR 0x08
#define ST7735_MADCTL_MH 0x04

#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR 0xB4
#define ST7735_DISSET5 0xB6

#define ST7735_PWCTR1 0xC0
#define ST7735_PWCTR2 0xC1
#define ST7735_PWCTR3 0xC2
#define ST7735_PWCTR4 0xC3
#define ST7735_PWCTR5 0xC4
#define ST7735_VMCTR1 0xC5

#define ST7735_PWCTR6 0xFC

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

#define   BLACK   0x0000
#define   BLUE    0x001F
#define   RED     0xF800
#define   GREEN   0x07E0
#define   CYAN    0x07FF
#define   MAGENTA 0xF81F
#define   YELLOW  0xFFE0
#define   WHITE   0xFFFF

#define ST_CMD_DELAY      0x80    // special signifier for command lists

// Some ready-made 16-bit ('565') color settings:
#define ST77XX_BLACK 0x0000

#define RESET PORT(LCD_RESET_PORT, LCD_RESET_PIN)
#define CS PORT(LCD_CS_PORT, LCD_CS_PIN)
#define D_C PORT(LCD_D_C_PORT, LCD_D_C_PIN)
#define SCK PORT(LCD_CLK_PORT, LCD_CLK_PIN)
#define DATA PORT(LCD_DIN_PORT, LCD_DIN_PIN)

#define INITR_GREENTAB    0x00
#define INITR_REDTAB      0x01
#define INITR_BLACKTAB    0x02
#define INITR_18GREENTAB  INITR_GREENTAB
#define INITR_18REDTAB    INITR_REDTAB
#define INITR_18BLACKTAB  INITR_BLACKTAB
#define INITR_144GREENTAB 0x01
#define INITR_MINI160x80  0x04
#define INITR_HALLOWING   0x05

#define ST7735_TFTWIDTH_128   128 // for 1.44 and mini
#define ST7735_TFTWIDTH_80     80 // for mini
#define ST7735_TFTHEIGHT_128  128 // for 1.44" display
#define ST7735_TFTHEIGHT_160  160 // for 1.8" and mini display


static uint8_t   pow2[8] = {0x01, 0x02,0x04,0x08,0x10,0x20,0x40,0x80};

const uint8_t
  Bcmd[] = {                        // Init commands for 7735B screens
    18,                             // 18 commands in list:
    ST77XX_SWRESET,   ST_CMD_DELAY, //  1: Software reset, no args, w/delay
      50,                           //     50 ms delay
    ST77XX_SLPOUT ,   ST_CMD_DELAY, //  2: Out of sleep mode, no args, w/delay
      255,                          //     255 = max (500 ms) delay
    ST77XX_COLMOD , 1+ST_CMD_DELAY, //  3: Set color mode, 1 arg + delay:
      0x05,                         //     16-bit color
      10,                           //     10 ms delay
    ST7735_FRMCTR1, 3+ST_CMD_DELAY, //  4: Frame rate control, 3 args + delay:
      0x00,                         //     fastest refresh
      0x06,                         //     6 lines front porch
      0x03,                         //     3 lines back porch
      10,                           //     10 ms delay
    ST77XX_MADCTL , 1,              //  5: Mem access ctl (directions), 1 arg:
      0x08,                         //     Row/col addr, bottom-top refresh
    ST7735_DISSET5, 2,              //  6: Display settings #5, 2 args:
      0x15,                         //     1 clk cycle nonoverlap, 2 cycle gate
                                    //     rise, 3 cycle osc equalize
      0x02,                         //     Fix on VTL
    ST7735_INVCTR , 1,              //  7: Display inversion control, 1 arg:
      0x0,                          //     Line inversion
    ST7735_PWCTR1 , 2+ST_CMD_DELAY, //  8: Power control, 2 args + delay:
      0x02,                         //     GVDD = 4.7V
      0x70,                         //     1.0uA
      10,                           //     10 ms delay
    ST7735_PWCTR2 , 1,              //  9: Power control, 1 arg, no delay:
      0x05,                         //     VGH = 14.7V, VGL = -7.35V
    ST7735_PWCTR3 , 2,              // 10: Power control, 2 args, no delay:
      0x01,                         //     Opamp current small
      0x02,                         //     Boost frequency
    ST7735_VMCTR1 , 2+ST_CMD_DELAY, // 11: Power control, 2 args + delay:
      0x3C,                         //     VCOMH = 4V
      0x38,                         //     VCOML = -1.1V
      10,                           //     10 ms delay
    ST7735_PWCTR6 , 2,              // 12: Power control, 2 args, no delay:
      0x11, 0x15,
    ST7735_GMCTRP1,16,              // 13: Gamma settings, I think?
      0x09, 0x16, 0x09, 0x20,
      0x21, 0x1B, 0x13, 0x19,
      0x17, 0x15, 0x1E, 0x2B,
      0x04, 0x05, 0x02, 0x0E,
    ST7735_GMCTRN1,16+ST_CMD_DELAY, // 14: More gamma settings?
      0x0B, 0x14, 0x08, 0x1E,
      0x22, 0x1D, 0x18, 0x1E,
      0x1B, 0x1A, 0x24, 0x2B,
      0x06, 0x06, 0x02, 0x0F,
      10,                           //     10 ms delay
    ST77XX_CASET  , 4,              // 15: Column addr set, 4 args, no delay:
      0x00, 0x02,                   //     XSTART = 2
      0x00, 0x81,                   //     XEND = 129
    ST77XX_RASET  , 4,              // 16: Row addr set, 4 args, no delay:
      0x00, 0x02,                   //     XSTART = 1
      0x00, 0x81,                   //     XEND = 160
    ST77XX_NORON  ,   ST_CMD_DELAY, // 17: Normal display on, no args, w/delay
      10,                           //     10 ms delay
    ST77XX_DISPON ,   ST_CMD_DELAY, // 18: Main screen turn on, no args, delay
      255 },                        //     255 = max (500 ms) delay

  Rcmd1[] = {                       // 7735R init, part 1 (red or green tab)
    15,                             // 15 commands in list:
    ST77XX_SWRESET,   ST_CMD_DELAY, //  1: Software reset, 0 args, w/delay
      150,                          //     150 ms delay
    ST77XX_SLPOUT ,   ST_CMD_DELAY, //  2: Out of sleep mode, 0 args, w/delay
      255,                          //     500 ms delay
    ST7735_FRMCTR1, 3,              //  3: Framerate ctrl - normal mode, 3 arg:
      0x01, 0x2C, 0x2D,             //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR2, 3,              //  4: Framerate ctrl - idle mode, 3 args:
      0x01, 0x2C, 0x2D,             //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR3, 6,              //  5: Framerate - partial mode, 6 args:
      0x01, 0x2C, 0x2D,             //     Dot inversion mode
      0x01, 0x2C, 0x2D,             //     Line inversion mode
    ST7735_INVCTR , 1,              //  6: Display inversion ctrl, 1 arg:
      0x07,                         //     No inversion
    ST7735_PWCTR1 , 3,              //  7: Power control, 3 args, no delay:
      0xA2,
      0x02,                         //     -4.6V
      0x84,                         //     AUTO mode
    ST7735_PWCTR2 , 1,              //  8: Power control, 1 arg, no delay:
      0xC5,                         //     VGH25=2.4C VGSEL=-10 VGH=3 * AVDD
    ST7735_PWCTR3 , 2,              //  9: Power control, 2 args, no delay:
      0x0A,                         //     Opamp current small
      0x00,                         //     Boost frequency
    ST7735_PWCTR4 , 2,              // 10: Power control, 2 args, no delay:
      0x8A,                         //     BCLK/2,
      0x2A,                         //     opamp current small & medium low
    ST7735_PWCTR5 , 2,              // 11: Power control, 2 args, no delay:
      0x8A, 0xEE,
    ST7735_VMCTR1 , 1,              // 12: Power control, 1 arg, no delay:
      0x0E,
    ST77XX_INVOFF , 0,              // 13: Don't invert display, no args
    ST77XX_MADCTL , 1,              // 14: Mem access ctl (directions), 1 arg:
      0xC8,                         //     row/col addr, bottom-top refresh
    ST77XX_COLMOD , 1,              // 15: set color mode, 1 arg, no delay:
      0x05 },                       //     16-bit color

  Rcmd2green[] = {                  // 7735R init, part 2 (green tab only)
    2,                              //  2 commands in list:
    ST77XX_CASET  , 4,              //  1: Column addr set, 4 args, no delay:
      0x00, 0x02,                   //     XSTART = 0
      0x00, 0x7F+0x02,              //     XEND = 127
    ST77XX_RASET  , 4,              //  2: Row addr set, 4 args, no delay:
      0x00, 0x01,                   //     XSTART = 0
      0x00, 0x9F+0x01 },            //     XEND = 159

  Rcmd2red[] = {                    // 7735R init, part 2 (red tab only)
    2,                              //  2 commands in list:
    ST77XX_CASET  , 4,              //  1: Column addr set, 4 args, no delay:
      0x00, 0x00,                   //     XSTART = 0
      0x00, 0x7F,                   //     XEND = 127
    ST77XX_RASET  , 4,              //  2: Row addr set, 4 args, no delay:
      0x00, 0x00,                   //     XSTART = 0
      0x00, 0x9F },                 //     XEND = 159

  Rcmd2green144[] = {               // 7735R init, part 2 (green 1.44 tab)
    2,                              //  2 commands in list:
    ST77XX_CASET  , 4,              //  1: Column addr set, 4 args, no delay:
      0x00, 0x00,                   //     XSTART = 0
      0x00, 0x7F,                   //     XEND = 127
    ST77XX_RASET  , 4,              //  2: Row addr set, 4 args, no delay:
      0x00, 0x00,                   //     XSTART = 0
      0x00, 0x7F },                 //     XEND = 127

  Rcmd2green160x80[] = {            // 7735R init, part 2 (mini 160x80)
    2,                              //  2 commands in list:
    ST77XX_CASET  , 4,              //  1: Column addr set, 4 args, no delay:
      0x00, 0x00,                   //     XSTART = 0
      0x00, 0x7F,                   //     XEND = 79
    ST77XX_RASET  , 4,              //  2: Row addr set, 4 args, no delay:
      0x00, 0x00,                   //     XSTART = 0
      0x00, 0x9F },                 //     XEND = 159

  Rcmd3[] = {                       // 7735R init, part 3 (red or green tab)
    4,                              //  4 commands in list:
    ST7735_GMCTRP1, 16      ,       //  1: Gamma setting, I think?
      0x02, 0x1c, 0x07, 0x12,
      0x37, 0x32, 0x29, 0x2d,
      0x29, 0x25, 0x2B, 0x39,
      0x00, 0x01, 0x03, 0x10,
    ST7735_GMCTRN1, 16      ,       //  2: More gamma?
      0x03, 0x1d, 0x07, 0x06,
      0x2E, 0x2C, 0x29, 0x2D,
      0x2E, 0x2E, 0x37, 0x3F,
      0x00, 0x00, 0x02, 0x10,
    ST77XX_NORON  ,   ST_CMD_DELAY, //  3: Normal display on, no args, w/delay
      10,                           //     10 ms delay
    ST77XX_DISPON ,   ST_CMD_DELAY, //  4: Main screen turn on, no args w/delay
      100 };                        //     100 ms delay


uint8_t
  _width,         ///< Display width as modified by current rotation
  _height,        ///< Display height as modified by current rotation
  _xstart   = 0,  ///< Internal framebuffer X offset
  _ystart   = 0,  ///< Internal framebuffer Y offset
  _colstart = 0,  ///< Some displays need this changed to offset
  _rowstart = 0,  ///< Some displays need this changed to offset
  tabcolor,
  rotation  = 0;  ///< Display rotation (0 thru 3)
uint8_t xCursor;
uint8_t yCursor;
const GFXfont * font;

static void   writeSPI(uint8_t data);
static uint32 readSPI24(void);
static uint32 readSPI32(void);

static void initB(void);
static void tft_initR(uint8_t options);
static void setAddrWindow(uint8_t x, uint8_t y, uint8_t w, uint8_t h) ;
static void drawPixel(uint8_t x, uint8_t y, uint16_t color);
static void fillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color);
static void displayInit(const uint8_t *addr);
static void writeCmd(uint8_t data);
static void writeData(uint8_t data);
static void writeCmd1Arg(uint8_t data, uint8_t arg1);
static void writeCmd2Arg(uint8_t data, uint8_t arg1, uint8_t arg2);
static void writeCmd3Arg(uint8_t data, uint8_t arg1, uint8_t arg2, uint8_t arg3);
static void writeCmd4Arg(uint8_t data, uint8_t arg1, uint8_t arg2, uint8_t arg3, uint8_t arg4);

void initLcd(void){
  FUNCTION_SEL(LCD_RESET_PORT, LCD_RESET_PIN)=0;
  FUNCTION_SEL(LCD_CS_PORT, LCD_CS_PIN)=0;
  FUNCTION_SEL(LCD_D_C_PORT, LCD_D_C_PIN)=0;
  FUNCTION_SEL(LCD_DIN_PORT, LCD_DIN_PIN)=0;
  FUNCTION_SEL(LCD_CLK_PORT, LCD_CLK_PIN)=0;

  PULLUP_DOWN(LCD_RESET_PORT, LCD_RESET_PIN)=0;
  PULLUP_DOWN(LCD_CS_PORT, LCD_CS_PIN)=0;
  PULLUP_DOWN(LCD_D_C_PORT, LCD_D_C_PIN)=0;
  PULLUP_DOWN(LCD_DIN_PORT, LCD_DIN_PIN)=0;
  PULLUP_DOWN(LCD_CLK_PORT, LCD_CLK_PIN)=0;
  
  PULL_UP(LCD_RESET_PORT);
  PULL_UP(LCD_CS_PORT);
  PULL_UP(LCD_D_C_PORT);
  PULL_UP(LCD_DIN_PORT);
  PULL_UP(LCD_CLK_PORT);
  
  DIR(LCD_RESET_PORT, LCD_RESET_PIN)=1;
  DIR(LCD_CS_PORT, LCD_CS_PIN)=1;
  DIR(LCD_D_C_PORT, LCD_D_C_PIN)=1;
  DIR(LCD_DIN_PORT, LCD_DIN_PIN)=1;
  DIR(LCD_CLK_PORT, LCD_CLK_PIN)=1;
  
  RESET=1;
  CS=0;
  D_C=1;
  DATA=0;
  SCK=1;

  T4CNT=0;
  T4CTL = 0xE2; // Tick Frequency/128, 250 cycle for mS
  T4CTL |= 0x10;
  T4CC0=250;
  T4CC1=250;
  T4CCTL0=0x14;
  TIMIF = TIMIF & 0xE7;
  T4CTL |= 0x10;
  
  tft_initR(INITR_BLACKTAB);  
  fillRect(0,0,_width,_height,WHITE);
  xCursor=0;
  yCursor=0;
  setFont(&Font5x7Fixed);
}

void tft_initR(uint8_t options) {
    RESET = 1;
    WAITms(100);
    RESET = 0;
    WAITms(100);
    RESET=1;
    WAITms(200);
  CS = 1;
  displayInit(Rcmd1);
  if(options == INITR_GREENTAB) {
    displayInit(Rcmd2green);
    _colstart = 2;
    _rowstart = 1;
  } else if((options == INITR_144GREENTAB) || (options == INITR_HALLOWING)) {
    _height   = ST7735_TFTHEIGHT_128;
    _width    = ST7735_TFTWIDTH_128;
    displayInit(Rcmd2green144);
    _colstart = 2;
    _rowstart = 3; // For default rotation 0
  } else if(options == INITR_MINI160x80) {
    _height   = ST7735_TFTHEIGHT_160;
    _width    = ST7735_TFTWIDTH_80;
    displayInit(Rcmd2green160x80);
    _colstart = 24;
    _rowstart = 0;
  } else {
    // colstart, rowstart left at default '0' values
    displayInit(Rcmd2red);
  }
  displayInit(Rcmd3);

  // Black tab, change MADCTL color filter
  if((options == INITR_BLACKTAB) || (options == INITR_MINI160x80)) {
    CS=0;
    writeCmd(ST77XX_MADCTL);
    writeData(0xC0);
    CS=1;
  }

  if(options == INITR_HALLOWING) {
    // Hallowing is simply a 1.44" green tab upside-down:
    tabcolor = INITR_144GREENTAB;
    setRotation(2);
  } else {
    tabcolor = options;
    setRotation(0);
  }
}


void display(void){
  
}

void setCursor(uint8_t x, uint8_t y){
  xCursor=x;
  yCursor=y;
}
void setFont(const GFXfont * newFont){
  font = newFont;
}
void drawText(char * text){
  uint8_t i=0;
  char c;
  while ( (c=text[i]) != 0){
    drawChar(c);
    i++;
  }
}
void drawChar(char c){
   if (c < font->first || c > font->last){
    return;
  }
  GFXglyph * glyph = font->glyph+( c- (font->first));
  uint8_t * bitmap = font->bitmap+glyph->bitmapOffset;
  
  uint8_t  startX = xCursor+glyph->xOffset;
  uint8_t  startY = yCursor+glyph->yOffset;
 
  uint8_t  xIter;
  uint8_t  yIter;
  uint8_t  bitRead  =0;
  uint8_t  bitFont = *bitmap;
  uint8_t x;

  yIter = startY;
  for (uint8_t y=0; y < glyph->height; y++){
    xIter = startX;
    for(x=0; x < glyph->width; x++){
       uint8_t bitMask = pow2[7-bitRead];
       if (bitFont & bitMask){
         drawPixel(xIter, yIter, BLACK); 
       } else {
         drawPixel(xIter, yIter, WHITE); 
       }
       bitRead++;
       if (bitRead == 8){
         bitmap++;
         bitFont=*bitmap;
         bitRead = 0;
       }
       xIter++;
    }
    for(; x < glyph->xAdvance; x++){
      drawPixel(xIter, yIter, WHITE); 
      xIter++;
    }
    yIter++;
  }
  xCursor += glyph->xAdvance;
}


void clean(uint8_t startX, uint8_t startY,uint8_t endX, uint8_t endY){
  fillRect(startX,startY, endX-startX,endY-startY,WHITE);
  
}


static void initB(void) {
  RESET=1;
  WAITms(100);
  RESET=0;
  WAITms(100);
  RESET=1;
  WAITms(200);
  CS=1;
  D_C=0;  
  displayInit(Bcmd);
  setRotation(0);
}

/**************************************************************************/
/*!
    @brief  Companion code to the initiliazation tables. Reads and issues
            a series of LCD commands stored in ROM byte array.
    @param  addr  Flash memory array with commands and data to send
*/
/**************************************************************************/
void displayInit(const uint8_t *addr){
  uint8_t  numCommands, numArgs;
  uint16_t ms;
  
  numCommands = *addr++;   // Number of commands to follow
  CS=0;
  while(numCommands--) {       
    writeCmd(*addr++);
    numArgs  = *addr++;    // Number of args to follow
    ms       = numArgs & ST_CMD_DELAY;   // If hibit set, delay follows args
    numArgs &= ~ST_CMD_DELAY;            // Mask out delay bit
    while(numArgs--) {                   // For each argument...
     writeData(*addr++);   // Read, issue argument
    }

    if(ms) {
      ms = *addr++; // Read post-command delay time (ms)
      if(ms == 255){
        ms=500;
      }
      WAITms(ms);
    }
  }
  CS=1;
}

static uint32 readSPI24(void){
  uint32 result=0;
  DIR(LCD_DIN_PORT, LCD_DIN_PIN)=0;
 
  SCK=1;
  for(uint8_t i=0; i < 24; i++){
      if (DATA){
        result |= 1;
      }
      SCK=0;
      result = result << 1;
      SCK=1;
  }
  
  if (DATA){
    result |= 1;
  }
  DIR(LCD_DIN_PORT, LCD_DIN_PIN)=1;
  return result &0xFFFFFF;
}

static uint32 readSPI32(void){
  uint32 result=0;
  DIR(LCD_DIN_PORT, LCD_DIN_PIN)=0;
 
  SCK=1;
  for(uint8_t i=0; i < 32; i++){
      if (DATA){
        result |= 1;
      }
      SCK=0;
      result = result << 1;
      SCK=1;
  }
  if (DATA){
    result |= 1;
  }
  
  DIR(LCD_DIN_PORT, LCD_DIN_PIN)=1;
  return result;
}

static void writeCmd(uint8_t cmd) {
  D_C=0;
  writeSPI(cmd);
}

static void writeData(uint8_t data) {
  D_C=1;
  writeSPI(data);
}

void writeCmd1Arg(uint8_t cmd, uint8_t arg1){
  D_C=0;
  writeSPI(cmd);
  D_C=1;
  writeSPI(arg1);
  
}

static void writeCmd2Arg(uint8_t cmd, uint8_t arg1, uint8_t arg2){
  writeCmd1Arg(cmd,arg1);
  writeSPI(arg2);
}

static void writeCmd3Arg(uint8_t cmd, uint8_t arg1, uint8_t arg2, uint8_t arg3){
  writeCmd2Arg(cmd,arg1,arg2);
  writeSPI(arg3);
}

static void writeCmd4Arg(uint8_t cmd, uint8_t arg1, uint8_t arg2, uint8_t arg3, uint8_t arg4){
  writeCmd3Arg(cmd,arg1,arg2,arg3);
  writeSPI(arg4);
}


static void writeSPI(uint8_t data) {
  CS=0;
  SCK=0;
  uint8_t mask=0x80;
  for(uint8_t i=0; i< 8; i++){
    
    if (data & mask){
      DATA=1;
    } else {
      DATA=0;
    }
    SCK=1;
    data = data << 1;
    SCK=0;
  }
}

void setRotation(uint8_t m) {
  uint8_t madctl = 0;

  rotation = m & 3; // can't be higher than 3

  // For ST7735 with GREEN TAB (including HalloWing)...
  if((tabcolor == INITR_144GREENTAB) || (tabcolor == INITR_HALLOWING)) {
    // ..._rowstart is 3 for rotations 0&1, 1 for rotations 2&3
    _rowstart = (rotation < 2) ? 3 : 1;
  }

  switch (rotation) {
   case 0:
     if ((tabcolor == INITR_BLACKTAB) || (tabcolor == INITR_MINI160x80)) {
       madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MY | ST77XX_MADCTL_RGB;
     } else {
       madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MY | ST7735_MADCTL_BGR;
     }

     if (tabcolor == INITR_144GREENTAB) {
       _height = ST7735_TFTHEIGHT_128;
       _width  = ST7735_TFTWIDTH_128;
     } else if (tabcolor == INITR_MINI160x80)  {
       _height = ST7735_TFTHEIGHT_160;
       _width  = ST7735_TFTWIDTH_80;
     } else {
       _height = ST7735_TFTHEIGHT_160;
       _width  = ST7735_TFTWIDTH_128;
     }
     _xstart   = _colstart;
     _ystart   = _rowstart;
     break;
   case 1:
     if ((tabcolor == INITR_BLACKTAB) || (tabcolor == INITR_MINI160x80)) {
       madctl = ST77XX_MADCTL_MY | ST77XX_MADCTL_MV | ST77XX_MADCTL_RGB;
     } else {
       madctl = ST77XX_MADCTL_MY | ST77XX_MADCTL_MV | ST7735_MADCTL_BGR;
     }

     if (tabcolor == INITR_144GREENTAB)  {
       _width  = ST7735_TFTHEIGHT_128;
       _height = ST7735_TFTWIDTH_128;
     } else if (tabcolor == INITR_MINI160x80)  {
       _width  = ST7735_TFTHEIGHT_160;
       _height = ST7735_TFTWIDTH_80;
     } else {
       _width  = ST7735_TFTHEIGHT_160;
       _height = ST7735_TFTWIDTH_128;
     }
     _ystart   = _colstart;
     _xstart   = _rowstart;
     break;
  case 2:
     if ((tabcolor == INITR_BLACKTAB) || (tabcolor == INITR_MINI160x80)) {
       madctl = ST77XX_MADCTL_RGB;
     } else {
       madctl = ST7735_MADCTL_BGR;
     }

     if (tabcolor == INITR_144GREENTAB) {
       _height = ST7735_TFTHEIGHT_128;
       _width  = ST7735_TFTWIDTH_128;
     } else if (tabcolor == INITR_MINI160x80)  {
       _height = ST7735_TFTHEIGHT_160;
       _width  = ST7735_TFTWIDTH_80;
     } else {
       _height = ST7735_TFTHEIGHT_160;
       _width  = ST7735_TFTWIDTH_128;
     }
     _xstart   = _colstart;
     _ystart   = _rowstart;
     break;
   case 3:
     if ((tabcolor == INITR_BLACKTAB) || (tabcolor == INITR_MINI160x80)) {
       madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MV | ST77XX_MADCTL_RGB;
     } else {
       madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MV | ST7735_MADCTL_BGR;
     }

     if (tabcolor == INITR_144GREENTAB)  {
       _width  = ST7735_TFTHEIGHT_128;
       _height = ST7735_TFTWIDTH_128;
     } else if (tabcolor == INITR_MINI160x80)  {
       _width  = ST7735_TFTHEIGHT_160;
       _height = ST7735_TFTWIDTH_80;
     } else {
       _width  = ST7735_TFTHEIGHT_160;
       _height = ST7735_TFTWIDTH_128;
     }
     _ystart   = _colstart;
     _xstart   = _rowstart;
     break;
  }

  CS=0;
  writeCmd(ST77XX_MADCTL);
  writeData(madctl);
  CS=1;
}

void setAddrWindow(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
  x += _xstart;
  y += _ystart;

  writeCmd(ST77XX_CASET); // Column addr set
  writeData(0);
  writeData(x);
  writeData(0);
  writeData(x+w-1);

  writeCmd(ST77XX_RASET); // Row addr set
  writeData(0);
  writeData(y);
  writeData(0);
  writeData(y+h-1);

  writeCmd(ST77XX_RAMWR); // write to RAM
}


void drawPixel(uint8_t x, uint8_t y, uint16_t color) {
  if((x < _width) && (y < _height)) {
    CS=0;
    setAddrWindow(x, y, 1, 1);
    writeData(color >> 8);
    writeData(color & 0xFF);
    CS=1;
  }
}


void fillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color) {
  uint16_t px;
  if(w && h) {                            // Nonzero width and height?  
    uint8_t hi = color >> 8, lo = color;
    if((x >= _width) || (y >= _height))
      return;
    if((x + w - 1) >= _width)  
      w = _width  - x;
    if((y + h - 1) >= _height) 
      h = _height - y;
    CS=0;
    setAddrWindow(x, y, w, h);
    px = (uint16_t)w * h;
    while (px--) {
      writeData(hi);
      writeData(lo);
    }
    CS=1;
  }
}