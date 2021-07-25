#ifndef __LCD_SSD1306__H__
#define __LCD_SSD1306__H__

#include "font.h"

void initLcd(void);
void displayOn(void);
void displayOff(void);


void setContext(uint8_t newContext);

void display(void);
void display_c(uint8_t context);

void setCursor(uint8_t x, uint8_t y);
void setCursor_c(uint8_t x, uint8_t y, uint8_t context);
void setFont(const GFXfont * font);
void drawText(const char * text);
void drawText_c(const char * text,uint8_t context, uint8_t inverted);
void drawChar(char c);
void drawChar_c(char c, uint8_t context, uint8_t inverted);
void clean(uint8_t startX, uint8_t startY,uint8_t endX, uint8_t endY);
void clean_c(uint8_t startX, uint8_t startY,uint8_t endX, uint8_t endY, uint8_t context);
void setRotation(uint8_t m);
void setRotation_c(uint8_t m, uint8_t context );

uint8_t getXCursor(void);
uint8_t getYCursor(void);



extern uint8 displayHeight;
extern uint8 displayWidth;

#endif