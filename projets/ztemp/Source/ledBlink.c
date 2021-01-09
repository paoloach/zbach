#include "onboard.h"
#include "OSAL_Timers.h"
#include "regs.h"
#include "ClusterOSALEvents.h"
#include "EventManager.h"

#include "ledBlink.h"

#define FAST_BLINK_TIME_ON 100	 
#define FAST_BLINK_TIME_OFF_LONG 2000	 
#define FAST_BLINK_TIME_OFF_SHORT 500	

static byte counter=0;
static byte blink=1;
static  byte stop=1;
static uint8 taskId;


static void blinkLedAction(uint16 event);


void setBlinkCounter(byte blinkCount) {
  blink = blinkCount;	
}

void blinkLedInit(uint8 deviceTaskId) {
  taskId = deviceTaskId;
  FUNCTION_SEL(LED_BLINK_PORT, LED_BLINK_PIN)=0;
  DIR(LED_BLINK_PORT, LED_BLINK_PIN)=1;
  PORT(LED_BLINK_PORT, LED_BLINK_PIN)=1;

  stop=0;
  addEventCB(FAST_BLINK_BIT, &blinkLedAction);
}


void blinkLedstart(byte taskid){
  osal_start_timerEx_bit( taskid, FAST_BLINK_BIT, FAST_BLINK_TIME_ON );
  PORT(LED_BLINK_PORT, LED_BLINK_PIN) = 1;
  stop=0;
}

void blinkLedEnd(byte taskid){
  osal_stop_timerEx_bit( taskid, FAST_BLINK_BIT );
  PORT(LED_BLINK_PORT, LED_BLINK_PIN) = 0;
  stop=1;
}


void blinkLedAction(uint16 event){
  if (stop)
    return;
  if (PORT(LED_BLINK_PORT, LED_BLINK_PIN)){
          PORT(LED_BLINK_PORT, LED_BLINK_PIN) = 0;
          if (counter > 0){
                  osal_start_timerEx_bit( taskId, FAST_BLINK_BIT, FAST_BLINK_TIME_OFF_SHORT );
                  counter--;
          } else {
                  osal_start_timerEx_bit( taskId, FAST_BLINK_BIT, FAST_BLINK_TIME_OFF_LONG );
                  counter=blink;
          }
  }else{
          PORT(LED_BLINK_PORT, LED_BLINK_PIN) = 1;
          osal_start_timerEx_bit( taskId, FAST_BLINK_BIT, FAST_BLINK_TIME_ON );
  }
}