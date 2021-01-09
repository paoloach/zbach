#ifndef __LED_BLINK__H__
#define __LED_BLINK__H__

#include "ZComDef.h"

void blinkLedInit(uint8 deviceTaskId);

void blinkLedstart(byte taskid);

void blinkLedEnd(byte taskid);


void setBlinkCounter(byte blinkCount);




#endif