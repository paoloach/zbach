#ifndef __LED_BLINK__H__
#define __LED_BLINK__H__

#include "ZComDef.h"

void blinkLedInit(void);

void blinkLedstart(byte taskid);

void blinkLedEnd(byte taskid);

void blinkLedAction(byte taskid);

void setBlinkCounter(byte blinkCount);




#endif