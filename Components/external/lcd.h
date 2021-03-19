#ifndef __LCD_SSD1306__H__
#define __LCD_SSD1306__H__

#include "font.h"

#define DISPLAY_HEIGHT 64
#define DISPLAY_WIDTH 128

void initLcd(void);
void display(void);

void setCursor(uint8_t x, uint8_t y);
void setFont(const GFXfont * font);
void drawText(char * text);
void drawChar(char c);
void clean(uint8_t startX, uint8_t startY,uint8_t endX, uint8_t endY);

#endif