
#include <OnBoard.h>
#include "hal_mcu.h"
#include "OSAL_Timers.h"
#include "bl0937.h"
#include "ElectricityMeasureData.h"
#include "regs.h"
#include "ClusterOSALEvents.h"

#ifdef DISPLAY
#include "lcd.h"
#endif

static void readCFs(void);
static uint16 readCF(void);
static uint16 readCF1(void);
static uint8_t taskId;

#define COEF_POWER      5335660.0
#define COEF_CURRENT    13040.0
#define COEF_VOLT       1322.0*220
/*
#define COEF_POWER      1
#define COEF_CURRENT    1
#define COEF_VOLT       1
*/



void BL0937_init(uint8_t _taskId){
  taskId = _taskId;
  FUNCTION_SEL(IDENTIFY_PORT,IDENTIFY_PIN)=0;
  DIR(IDENTIFY_PORT,IDENTIFY_PIN)=1;
  PORT(IDENTIFY_PORT,IDENTIFY_PIN)=0; 
   
  FUNCTION_SEL(CF_PORT,CF_PIN)=0;
  FUNCTION_SEL(CF1_PORT,CF1_PIN)=0;
  FUNCTION_SEL(SEL_PORT,SEL_PIN)=0;
  DIR(SEL_PORT,SEL_PIN)=1;
  DIR(CF_PORT,CF_PIN)=0;
  DIR(CF1_PORT,CF1_PIN)=0;
  PORT(SEL_PORT,SEL_PIN)=0;
  
  T1_div=1;
  T1_mode=1;
  
  T1_CH1_MODE=0;
  T1_CH1_CAP=1;
  T1_CH1_IM=1;
  
  T1CCTL3=0x40;
  TIMIF=0;
 // T1IE=1;
  osal_start_reload_timer_cb(1000, &readCFs);
  osal_set_event_bit( taskId,  NEW_POWER_BIT );
}

                           
void readCFs(void){
  static uint8_t curOn=0;
  if (curOn == 0){
    apparentPower=readCF();
    if (apparentPower == 65535){
      activePower = 0;
    } else {
      activePower =(uint16)(COEF_POWER/apparentPower);
    }
  }
 #ifdef DISPLAY  
  char buffer[10];
  clean(0,9,DISPLAY_WIDTH, 18);
  setCursor(1,18);
  drawText("Power");
  _itoa(activePower, (uint8_t*)buffer, 10);
  drawText(buffer);
  drawText("W");
  
  clean(0,36,DISPLAY_WIDTH, 45);
  setCursor(1,45);
  drawText("apparentPower: ");
  _itoa(apparentPower, (uint8_t*)buffer, 10);
  drawText(buffer);
  if (curOn){
    drawText("-A");
  } else {
    drawText("-V");
  }
#endif
  uint16 volt;
  if (curOn){
    PORT(SEL_PORT,SEL_PIN)=0;
    curOn=0;
  } else {
    PORT(SEL_PORT,SEL_PIN)=1;
    curOn=1;
  }
  if (curOn==0){
    peakCurrent =  readCF1();
    if (peakCurrent == 65535){
      RMSCurrent = 0;
    } else {
      RMSCurrent = (uint16_t) (COEF_CURRENT/ peakCurrent);
    }
    
#ifdef DISPLAY
    clean(0,18,DISPLAY_WIDTH, 27);
    setCursor(1,27);
    drawText("current: ");
    _itoa(RMSCurrent, (uint8_t*)buffer, 10);
    drawText(buffer);
    drawText("A");
#endif
  } else {
    volt = readCF1();
    if (volt == 65535){
      RMSVolt = 0;
    } else {
      RMSVolt =(uint16_t)(COEF_VOLT/ volt);
    }
#ifdef DISPLAY
    clean(0,27,DISPLAY_WIDTH, 36);
    setCursor(1,36);
    drawText("volt: ");
    _itoa(RMSVolt, (uint8_t*)buffer, 10);
    drawText(buffer);
    drawText("V");
#endif
  }
#ifdef DISPLAY

  
  
#endif
  
}


uint16 readCF(void){
  uint16 cfDuration;
  T1CNTL=0;
  T1_OVFIF=0;
  while(PORT(CF_PORT,CF_PIN)==0 && T1_OVFIF==0);
  if (T1_OVFIF)
    return 65535;
  
  T1CNTL=0;
  T1_OVFIF=0;
  while(PORT(CF_PORT,CF_PIN)==1 && T1_OVFIF==0);
  while(PORT(CF_PORT,CF_PIN)==0 && T1_OVFIF==0);
  if (T1_OVFIF)
    return 65535;
  cfDuration = T1CNTL;
  cfDuration += (T1CNTH<<8);
  return cfDuration;
}    

uint16 readCF1(void){
  uint16 cf1Duration;
  T1CNTL=0;
  T1_OVFIF=0;
  while(PORT(CF1_PORT,CF1_PIN)==0 && T1_OVFIF==0);
  if (T1_OVFIF)
    return 65535;
  
  T1CNTL=0;
  T1_OVFIF=0;
  while(PORT(CF1_PORT,CF1_PIN)==1 && T1_OVFIF==0);
  while(PORT(CF1_PORT,CF1_PIN)==0 && T1_OVFIF==0);
  if (T1_OVFIF)
    return 65535;
  cf1Duration = T1CNTL;
  cf1Duration += (T1CNTH<<8);
  return cf1Duration;
}                             
                             
