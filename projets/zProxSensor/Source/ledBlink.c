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
static byte lifeSignal=0;

static void blinkLedAction(void);


void setBlinkCounter(byte blinkCount) {
  osal_stop_timerEx_cb( &blinkLedAction);
  blink = blinkCount;	
  blinkLedstart();
}

void blinkLedInit(uint8 deviceTaskId) {
  FUNCTION_SEL(LED_BLINK_PORT, LED_BLINK_PIN)=0;
  DIR(LED_BLINK_PORT, LED_BLINK_PIN)=1;
  PORT(LED_BLINK_PORT, LED_BLINK_PIN)=1;
  lifeSignal=0;
  stop=0;
}


void blinkLedstart(){
  osal_start_timerEx_cb(FAST_BLINK_TIME_ON, &blinkLedAction );      
  PORT(LED_BLINK_PORT, LED_BLINK_PIN) = 1;
  stop=0;
  lifeSignal=0;
}

void blinkLedEnd(){
  PORT(LED_BLINK_PORT, LED_BLINK_PIN) = 0;
  osal_stop_timerEx_cb( &blinkLedAction);
  lifeSignal=1;
  osal_start_reload_timer_cb(2000, &blinkLedAction );      
  PORT(LED_BLINK_PORT, LED_BLINK_PIN) = 1;
  stop=0;
  
}


void blinkLedAction(void){
  if (stop)
    return;
  if (PORT(LED_BLINK_PORT, LED_BLINK_PIN)){
          PORT(LED_BLINK_PORT, LED_BLINK_PIN) = 0;
          if (!lifeSignal){
            if (counter > 0){
              osal_start_timerEx_cb(FAST_BLINK_TIME_OFF_SHORT, &blinkLedAction );      
              counter--;
            } else {
              osal_start_timerEx_cb(FAST_BLINK_TIME_OFF_LONG, &blinkLedAction );      
              counter=blink;
            }
          }
  }else{
          PORT(LED_BLINK_PORT, LED_BLINK_PIN) = 1;
          if (!lifeSignal){
            osal_start_timerEx_cb(FAST_BLINK_TIME_ON, &blinkLedAction );      
          }
  }
}