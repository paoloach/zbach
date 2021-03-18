#include <hal_mcu.h>
#include "EventManager.h"
#include "lcd-SSD1306.h"

static void (* callbacks[16])(uint16 event);

void eventManagerInit(void) {
    for(uint8 i=0; i < 16; i++){
      callbacks[i] = NULL;
    }
}

void addEventCB(uint8 eventBit, void (* callback)(uint16 event) ){
  halIntState_t intState;

  if (eventBit >= 16)
    return;
  callbacks[eventBit] = callback;
}


uint8 handleEvent(uint16 * event){
  halIntState_t intState;
  uint16 bit=1;
  uint16 tmpEvent = *event;

  for(uint8 i=0; i < 16; i++){
    if ((tmpEvent & bit) && (callbacks[i] != NULL)){
      setCursor(0,63);
      clean(0,54,10 ,63);
      drawText("1");
      display();
      callbacks[i](tmpEvent);
      tmpEvent = tmpEvent  ^ bit;
      setCursor(0,63);
      clean(0,54,10 ,63);
      drawText("0");
      display();
    }
    bit = bit << 1;
  }
  *event = tmpEvent;
  return tmpEvent;
  
}
