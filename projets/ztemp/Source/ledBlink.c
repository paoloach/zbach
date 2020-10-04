#include "onboard.h"
#include "OSAL_Timers.h"

#include "ClusterOSALEvents.h"

#include "ledBlink.h"



__sfr __no_init volatile struct  {
	unsigned char DIR0_0: 1;
	unsigned char DIR0_1: 1;
	unsigned char DIR0_2: 1;
	unsigned char DIR0_3: 1;
	unsigned char DIR0_4: 1;
	unsigned char DIR0_5: 1;
	unsigned char DIR0_6: 1;
	unsigned char DIR0_7: 1;
} @ 0xFD;

__sfr __no_init volatile struct  {
	unsigned char P0SEL_0: 1;
	unsigned char P0SEL_1: 1;
	unsigned char P0SEL_2: 1;
	unsigned char P0SEL_3: 1;
	unsigned char P0SEL_4: 1;
	unsigned char P0SEL_5: 1;
	unsigned char P0SEL_6: 1;
	unsigned char P0SEL_7: 1;
} @ 0xF3;


#define FAST_BLINK_TIME_ON 100	 
#define FAST_BLINK_TIME_OFF_LONG 2000	 
#define FAST_BLINK_TIME_OFF_SHORT 500	

static byte counter=0;
static byte blink=1;
static  byte stop=1;


void setBlinkCounter(byte blinkCount) {
	blink = blinkCount;	
}

void blinkLedInit(void) {
	DIR0_1 = 1;
 	P0SEL_1 = 0;
 	P0_1 = 0;
        stop=0;
}


void blinkLedstart(byte taskid){
	osal_start_timerEx( taskid, FAST_BLINK, FAST_BLINK_TIME_ON );
	P0_1 = 1;
        stop=0;
}

void blinkLedEnd(byte taskid){
	osal_stop_timerEx( taskid, FAST_BLINK );
	P0_1 = 0;
        stop=1;
}


void blinkLedAction(byte taskid){
  if (stop)
    return;
  if (P0_1){
          P0_1 = 0;
          if (counter > 0){
                  osal_start_timerEx( taskid, FAST_BLINK, FAST_BLINK_TIME_OFF_SHORT );
                  counter--;
          } else {
                  osal_start_timerEx( taskid, FAST_BLINK, FAST_BLINK_TIME_OFF_LONG );
                  counter=blink;
          }
  }else{
          P0_1 = 1;
          osal_start_timerEx( taskid, FAST_BLINK, FAST_BLINK_TIME_ON );
  }
}