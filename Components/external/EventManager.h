#ifndef __EVENT_MANAGER__H__
#define __EVENT_MANAGER__H__


#include "hal_types.h"


void eventManagerInit(void);
void addEventCB(uint8 eventBit, void (* callback)(uint16 event) ); 
void handleEvent(uint16 * event);


#endif