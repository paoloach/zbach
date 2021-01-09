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


void handleEvent(uint16 * event){
  uint8 bit=1;
  for(uint8 i=0; i < 16; i++){
    if (callbacks[i] != NULL){
      callbacks[i](*event);
      *event = *event  ^ bit;
    }
    bit = bit << 1;
  }
  
  
}
