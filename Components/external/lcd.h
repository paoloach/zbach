#ifndef __LCD_SSD1306__H__
#define __LCD_SSD1306__H__

#include "font.h"

void initLcd(void);
void displayOn(void);
void displayOff(void);

void display(void);

void setCursor(uint8_t x, uint8_t y);
void setFont(const GFXfont * font);
void drawText(const char * text);
void drawChar(char c);
void clean(uint8_t startX, uint8_t startY,uint8_t endX, uint8_t endY);
void setRotation(uint8_t m);
uint8_t getXCursor(void);
uint8_t getYCursor(void);

extern uint8 displayHeight;
extern uint8 displayWidth;

#endif