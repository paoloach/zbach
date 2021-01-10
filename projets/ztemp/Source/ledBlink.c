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

static void blinkLedAction(void);


void setBlinkCounter(byte blinkCount) {
  blink = blinkCount;	
}

void blinkLedInit(uint8 deviceTaskId) {
  FUNCTION_SEL(LED_BLINK_PORT, LED_BLINK_PIN)=0;
  DIR(LED_BLINK_PORT, LED_BLINK_PIN)=1;
  PORT(LED_BLINK_PORT, LED_BLINK_PIN)=1;

  stop=0;
}


void blinkLedstart(byte taskid){
  osal_start_timerEx_cb(FAST_BLINK_TIME_ON, &blinkLedAction );      
  PORT(LED_BLINK_PORT, LED_BLINK_PIN) = 1;
  stop=0;
}

void blinkLedEnd(byte taskid){
  PORT(LED_BLINK_PORT, LED_BLINK_PIN) = 0;
  stop=1;
}


void blinkLedAction(void){
  if (stop)
    return;
  if (PORT(LED_BLINK_PORT, LED_BLINK_PIN)){
          PORT(LED_BLINK_PORT, LED_BLINK_PIN) = 0;
          if (counter > 0){
            osal_start_timerEx_cb(FAST_BLINK_TIME_OFF_SHORT, &blinkLedAction );      
            counter--;
          } else {
            osal_start_timerEx_cb(FAST_BLINK_TIME_OFF_LONG, &blinkLedAction );      
            counter=blink;
          }
  }else{
          PORT(LED_BLINK_PORT, LED_BLINK_PIN) = 1;
          osal_start_timerEx_cb(FAST_BLINK_TIME_ON, &blinkLedAction );      
          
  }
}