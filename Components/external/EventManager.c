#include <hal_mcu.h>
#include "EventManager.h"

static void (* callbacks[16])(uint16 event);

void eventManagerInit(void) {
    for(uint8 i=0; i < 16; i++){
      callbacks[i] = NULL;
    }
}

void addEventCB(uint8 eventBit, void (* callback)(uint16 event) ){
  if (eventBit >= 16)
    return;
  callbacks[eventBit] = callback;
}


uint8 handleEvent(uint16 * event){
  uint16 bit=1;
  uint16 tmpEvent = *event;

  for(uint8 i=0; i < 16; i++){
    if ((tmpEvent & bit) && (callbacks[i] != NULL)){
      callbacks[i](tmpEvent);
      tmpEvent = tmpEvent  ^ bit;
    }
    bit = bit << 1;
  }
  *event = tmpEvent;
  return tmpEvent;
  
}
