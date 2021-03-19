#include <OSAL.h>
#include <OSAL_Memory.h>

#include "lcd.h"

#include "i2c.h"
#include "font.h"
#include "fonts/Font5x7Fixed.h"






#define CMD_WRITE 0x78
#define CMD_READ 0x79
#define DATA_WRITE 0x7A
#define DATA_READ  0x7B


#define SSD1306_BLACK 0   ///< Draw 'off' pixels
#define SSD1306_WHITE 1   ///< Draw 'on' pixels
#define SSD1306_INVERSE 2 ///< Invert pixels

#define SSD1306_MEMORYMODE 0x20          ///< See datasheet
#define SSD1306_COLUMNADDR 0x21          ///< See datasheet
#define SSD1306_PAGEADDR 0x22            ///< See datasheet
#define SSD1306_SETCONTRAST 0x81         ///< See datasheet
#define SSD1306_CHARGEPUMP 0x8D          ///< See datasheet
#define SSD1306_SEGREMAP 0xA0            ///< See datasheet
#define SSD1306_DISPLAYALLON_RESUME 0xA4 ///< See datasheet
#define SSD1306_DISPLAYALLON 0xA5        ///< Not currently used
#define SSD1306_NORMALDISPLAY 0xA6       ///< See datasheet
#define SSD1306_INVERTDISPLAY 0xA7       ///< See datasheet
#define SSD1306_SETMULTIPLEX 0xA8        ///< See datasheet
#define SSD1306_DISPLAYOFF 0xAE          ///< See datasheet
#define SSD1306_DISPLAYON 0xAF           ///< See datasheet
#define SSD1306_COMSCANINC 0xC0          ///< Not currently used
#define SSD1306_COMSCANDEC 0xC8          ///< See datasheet
#define SSD1306_SETDISPLAYOFFSET 0xD3    ///< See datasheet
#define SSD1306_SETDISPLAYCLOCKDIV 0xD5  ///< See datasheet
#define SSD1306_SETPRECHARGE 0xD9        ///< See datasheet
#define SSD1306_SETCOMPINS 0xDA          ///< See datasheet
#define SSD1306_SETVCOMDETECT 0xDB       ///< See datasheet

#define SSD1306_SETLOWCOLUMN 0x00  ///< Not currently used
#define SSD1306_SETHIGHCOLUMN 0x10 ///< Not currently used
#define SSD1306_SETSTARTLINE 0x40  ///< See datasheet

#define SSD1306_SETLOWCOLUMN 0x00  ///< Not currently used
#define SSD1306_SETHIGHCOLUMN 0x10 ///< Not currently used
#define SSD1306_SETSTARTLINE 0x40  ///< See datasheet

#define SSD1306_EXTERNALVCC 0x01  ///< External display voltage source
#define SSD1306_SWITCHCAPVCC 0x02 ///< Gen. display voltage from 3.3V

#define SSD1306_RIGHT_HORIZONTAL_SCROLL 0x26              ///< Init rt scroll
#define SSD1306_LEFT_HORIZONTAL_SCROLL 0x27               ///< Init left scroll
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29 ///< Init diag scroll
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 0x2A  ///< Init diag scroll
#define SSD1306_DEACTIVATE_SCROLL 0x2E                    ///< Stop scroll
#define SSD1306_ACTIVATE_SCROLL 0x2F                      ///< Start scroll
#define SSD1306_SET_VERTICAL_SCROLL_AREA 0xA3             ///< Set scroll range




#define BUFFER_SIZE DISPLAY_HEIGHT*DISPLAY_WIDTH/8

static void displayOn(void);
static void sendCmds(uint8 * cmds, uint8 len);

static uint8_t   pow2[8] = {0x01, 0x02,0x04,0x08,0x10,0x20,0x40,0x80};
static uint8_t  cursorX;
static uint8_t  cursorY;

static uint8_t  lastMinX;
static uint8_t  lastMinY;
static uint8_t  lastMaxX;
static uint8_t  lastMaxY;



static uint8 * buffer;
static const GFXfont * font;

static uint8 displayOnCmd[] ={
SSD1306_DISPLAYOFF,
SSD1306_SETDISPLAYCLOCKDIV, 0xF0,
SSD1306_SETMULTIPLEX, DISPLAY_HEIGHT-1,
SSD1306_SETDISPLAYOFFSET, 0,
SSD1306_SETSTARTLINE,
SSD1306_CHARGEPUMP, 0x14,
SSD1306_MEMORYMODE, 0,
SSD1306_SEGREMAP,
SSD1306_COMSCANINC,
SSD1306_SETCOMPINS, 0x12,
SSD1306_SETCONTRAST, 0xCF,
SSD1306_SETPRECHARGE, 0xF1,
SSD1306_SETVCOMDETECT, 0x40,
SSD1306_DISPLAYALLON_RESUME,
SSD1306_NORMALDISPLAY,
SSD1306_DEACTIVATE_SCROLL,
SSD1306_DISPLAYON,
SSD1306_COLUMNADDR, 0, DISPLAY_WIDTH-1,
SSD1306_PAGEADDR, 0, 7,

};




void initLcd() {
  initI2c();
  buffer = osal_mem_alloc(BUFFER_SIZE);
  osal_memset(buffer, 0xff, BUFFER_SIZE);
  displayOn();
  setFont(&Font5x7Fixed);
  clean(0,0,DISPLAY_WIDTH, DISPLAY_HEIGHT);
  display();
 }

static void displayOn(void){
  sendCmds(displayOnCmd, sizeof(displayOnCmd));
 
}

static void sendCmds(uint8 * cmds, uint8 len) {
  uint8 buffer[4];
  buffer[0] = CMD_WRITE;
  buffer[1] = 0x80;
  for(uint8 i=0; i < len; i++){
    buffer[2] = cmds[i];
    sendI2c(buffer, 3);
  }
  
}


void display(void) {
  if (lastMinX > lastMaxX || lastMinY > lastMaxY)
    return;
  if (lastMinX == 0 &&   lastMaxX==DISPLAY_WIDTH && lastMinY == 0 && lastMaxY==DISPLAY_HEIGHT){
    uint8_t cmd[] = {SSD1306_COLUMNADDR, 0, DISPLAY_WIDTH-1,SSD1306_PAGEADDR, 0, 7};
    sendCmds(cmd, sizeof(cmd));
    fillI2cData(CMD_WRITE, 0x40, buffer, BUFFER_SIZE);
  } else {
    uint8_t firstPage = lastMinY/8;
    uint8_t lastPage = (lastMaxY+7)/8;
    uint8_t numCol = lastMaxX-lastMinX;
    uint8_t cmd[] = {SSD1306_COLUMNADDR, lastMinX, lastMaxX-1,SSD1306_PAGEADDR, firstPage, lastPage};
    sendCmds(cmd, sizeof(cmd));
    for (uint8_t page = firstPage; page < lastPage; page++){
      uint8_t * bufferStart = buffer + lastMinX + DISPLAY_WIDTH*(page);
      fillI2cData(CMD_WRITE, 0x40, bufferStart, numCol);
    }
  }
  lastMinX = DISPLAY_WIDTH;
  lastMaxX = 0;
  lastMinY = DISPLAY_HEIGHT;
  lastMaxY = 0;
}

void setCursor(uint8_t x, uint8_t y) {
  cursorX = x;
  cursorY = y;
}

void setFont(const GFXfont * newFont) {
  font = newFont;
}

void drawText(char * text) {
  uint8_t i=0;
  char c;
  while ( (c=text[i]) != 0){
    drawChar(c);
    i++;
  }
}

void drawChar(char c) {
  if (c < font->first || c > font->last){
    return;
  }
  GFXglyph * glyph = font->glyph+( c- (font->first));
  uint8_t * bitmap = font->bitmap+glyph->bitmapOffset;
  
  
  uint8_t  startX = cursorX+glyph->xOffset;
  uint8_t  startY = cursorY+glyph->yOffset;
  uint8_t  endX = startX +  glyph->width;
  uint8_t  endY = startY +  glyph->height;
  
  
  if (startX < lastMinX)
    lastMinX=startX;
  if (startY < lastMinY)
    lastMinY = startY;
  if (endX > lastMaxX)
    lastMaxX=endX;
  if (endY > lastMaxY)
    lastMaxY = endY;
  
  uint8_t * bufferStart;
  uint8_t  bitWrite;
  uint8_t  bitRead  =0;
  uint8_t  bitFont = *bitmap;
  uint8_t x;

  for (uint8_t y=0; y < glyph->height; y++){
    bufferStart = buffer + startX + DISPLAY_WIDTH*(startY/8);
    bitWrite = pow2[startY & 0x07];
    for(x=0; x < glyph->width; x++){
       uint8_t bitMask = pow2[7-bitRead];
       if (bitFont & bitMask){
         *bufferStart  &= ~bitWrite;
       } else {
         *bufferStart |= bitWrite;
       }
       bitRead++;
       if (bitRead == 8){
         bitmap++;
         bitFont=*bitmap;
         bitRead = 0;
       }
       bufferStart++;
    }
    for(; x < glyph->xAdvance; x++){
      *bufferStart |= bitWrite;
    }
    startY++;
  }
  cursorX += glyph->xAdvance;
}

void clean(uint8_t startX, uint8_t startY,uint8_t endX, uint8_t endY){
  
  if (startX < lastMinX)
    lastMinX=startX;
  if (startY < lastMinY)
    lastMinY = startY;
  if (endX > lastMaxX)
    lastMaxX=endX;
  if (endY > lastMaxY)
    lastMaxY = endY;
  
    uint8_t * bufferIter;
    uint8_t bitWrite;
    for (uint16_t y=startY; y < endY; y++){
      bufferIter = buffer + startX + DISPLAY_WIDTH*(y/8);
      bitWrite = pow2[y & 0x07];
      for(uint16_t x=startX; x < endX; x++){
        *bufferIter |= bitWrite;
        bufferIter++;
      }
    }
}