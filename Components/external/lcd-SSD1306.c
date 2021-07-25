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


#define DISPLAY_HEIGHT 64
#define DISPLAY_WIDTH 128

#define BUFFER_SIZE DISPLAY_HEIGHT*DISPLAY_WIDTH/8

static void displayInit(void);
static void sendCmds(uint8 * cmds, uint8 len);

static uint8_t context=0;


static uint8_t   pow2[8] = {0x01, 0x02,0x04,0x08,0x10,0x20,0x40,0x80};
static uint8_t  cursorX;
static uint8_t  cursorY;

static uint8_t  lastMinX;
static uint8_t  lastMinY;
static uint8_t  lastMaxX;
static uint8_t  lastMaxY;

uint8 displayHeight=DISPLAY_HEIGHT;
uint8 displayWidth=DISPLAY_WIDTH;

static uint8  buffer[BUFFER_SIZE];
static const GFXfont * font;

static uint8 displayInitCmd[] ={
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


static uint8 displayOnCmd[] ={
  SSD1306_DISPLAYON
};

static uint8 displayOffCmd[] ={
  SSD1306_DISPLAYOFF
};

void initLcd() {
  initI2c();
  osal_memset(buffer, 0xff, BUFFER_SIZE);
  displayInit();
  setFont(&Font5x7Fixed);
  clean(0,0,DISPLAY_WIDTH, DISPLAY_HEIGHT);
  display();
 }

void displayOn(void) {
  sendCmds(displayOnCmd, sizeof(displayOnCmd));
}
void displayOff(void){
  sendCmds(displayOffCmd, sizeof(displayOffCmd));
}


static void displayInit(void){
  sendCmds(displayInitCmd, sizeof(displayInitCmd));
 
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


void setContext(uint8_t newContext) {
  context = newContext;
}


void display() {
  display_c(0);
}

void display_c(uint8_t usedContext){
  if (usedContext != context){
    return;
  }
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
  setCursor_c(x,y,0);
}
void setCursor_c(uint8_t x, uint8_t y, uint8_t usedContext) {
  if (usedContext != context){
    return;
  }
  cursorX = DISPLAY_WIDTH-x;
  cursorY = DISPLAY_HEIGHT-y;
}

uint8_t getXCursor(void){
  return cursorX;
}
uint8_t getYCursor(void){
  return cursorY;
}

void setFont(const GFXfont * newFont) {
  font = newFont;
}

void drawText(const char * text) {
  drawText_c(text, 0, 0);
}

void drawText_c(const char * text,uint8_t usedContext, uint8_t inverted){
  if (usedContext != context){
    return;
  }

  uint8_t i=0;
  char c;
  while ( (c=text[i]) != 0){
    drawChar_c(c, context, inverted);
    i++;
  }
}

void drawChar(char c) {
  drawChar_c(c,0,0);
}
void drawChar_c(char c, uint8_t usedContext, uint8_t inverted) {
  if (usedContext != context){
    return;
  }

  if (c < font->first || c > font->last){
    return;
  }
  GFXglyph * glyph = font->glyph+( c- (font->first));
  uint8_t * bitmap = font->bitmap+glyph->bitmapOffset;
  
  
  uint8_t  startX = cursorX-glyph->xOffset;
  uint8_t  startY = cursorY-glyph->yOffset;
  uint8_t  endXClean = startX -  glyph->xAdvance;
  uint8_t  endY = startY -  glyph->height;
  
  
  if (endXClean < lastMinX)
    lastMinX=endXClean;
  if (endY < lastMinY)
    lastMinY = endY;
  if (startX > lastMaxX)
    lastMaxX=startX;
  if (startY > lastMaxY)
    lastMaxY = startY;
  
  uint8_t * bufferStart;
  uint8_t  bitWrite;
  uint8_t  bitRead  =0;
  uint8_t  bitFont = *bitmap;
  uint8_t x;
  
  for(uint8_t y=cursorY+1; y < cursorY+1+font->yAdvance; y++){
    bufferStart = buffer + startX + DISPLAY_WIDTH*(y/8);
    bitWrite = pow2[y & 0x07];
    for (uint8_t x=0; x < glyph->xAdvance; x++){
      if (inverted){
        *bufferStart &= ~bitWrite;
      } else {
         *bufferStart |= bitWrite;
      }
      bufferStart--;
    }
  }

 
  for (uint8_t y=0; y < glyph->height; y++){
    bufferStart = buffer + startX + DISPLAY_WIDTH*(startY/8);
    bitWrite = pow2[startY & 0x07];
    for(x=0; x < glyph->width; x++){
      uint8_t bitMask = pow2[7-bitRead];
        if (bitFont & bitMask){
         if (inverted){
           *bufferStart |= bitWrite;
         } else {
           *bufferStart  &= ~bitWrite;
         }
        } else {
         if (inverted){
           *bufferStart  &= ~bitWrite;
         } else {
           *bufferStart |= bitWrite;
         }
        }
        bitRead++;
        if (bitRead == 8){
         bitmap++;
         bitFont=*bitmap;
         bitRead = 0;
        }
      
      bufferStart--;
    }
    startY--;
  }
  cursorX -= glyph->xAdvance;
}

void clean(uint8_t startX, uint8_t startY,uint8_t endX, uint8_t endY){
  clean_c(startX, startY, endX, endY, 0);
}

void clean_c(uint8_t startX, uint8_t startY,uint8_t endX, uint8_t endY, uint8_t usedContext){
  if (usedContext != context){
    return;
  }

  if (startX >= endX)
    return;
  if (startY >= endY)
    return;
  
  
  
  if (startX <= DISPLAY_WIDTH){
     startX = DISPLAY_WIDTH-startX-1;  
  } else {
    startX = DISPLAY_WIDTH-1;
  }
  if (startY <= DISPLAY_HEIGHT){
     startY = DISPLAY_HEIGHT-startY-1;  
  } else {
    startY = DISPLAY_HEIGHT-1;
  }

  
  if (endX <= DISPLAY_WIDTH){
     endX = DISPLAY_WIDTH-endX;  
  } else {
    endX = DISPLAY_WIDTH;
  }
  if (endY <= DISPLAY_HEIGHT){
     endY = DISPLAY_HEIGHT-endY;  
  } else {
    endY = DISPLAY_HEIGHT;
  }

  
  if (endX < lastMinX)
    lastMinX=endX;
  if (endY < lastMinY)
    lastMinY = endY;
  if (startX > lastMaxX)
    lastMaxX=startX;
  if (startY > lastMaxY)
    lastMaxY = startY;
  
  uint8_t * bufferIter;
  uint8_t bitWrite;
  
  uint8_t * bufferEnd = buffer + BUFFER_SIZE;
  
  for (int8_t y=startY; y >= endY; y--){
    bufferIter = buffer + startX + DISPLAY_WIDTH*(y/8);
    bitWrite = pow2[y & 0x07];
    for(int8_t x=startX; x >= endX; x--){
      *bufferIter |= bitWrite;
      bufferIter--;
      if (bufferIter >= bufferEnd){
        bufferIter = bufferEnd-1;
      }
    }
  }
}