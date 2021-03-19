
#include "bl0937.h"
#include "ElectricityMeasureData.h"
#include "regs.h"
#include "hal_mcu.h"
#include "OSAL_Timers.h"

static void readCFs(void);
static uint16 readCF(void);
static uint16 readCF1(void);


#define COEF_POWER      48.54
#define COEF_CURRENT    1990.41
#define COEF_VOLT       86.82


void BL0937_init(void){
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
  
  T1_div=0;
  T1_mode=1;
  
  T1_CH1_MODE=0;
  T1_CH1_CAP=1;
  T1_CH1_IM=1;
  
  T1CCTL3=0x40;
  TIMIF=0;
 // T1IE=1;
  osal_start_reload_timer_cb(1000, &readCFs);
  
}

                           
void readCFs(void){
  apparentPower=readCF();
  activePower =(uint16)(apparentPower/COEF_POWER);
  if (PORT(SEL_PORT,SEL_PIN)==0){
    peakCurrent =  readCF1();
    RMSCurrent = (uint16)(peakCurrent/COEF_CURRENT );
    PORT(SEL_PORT,SEL_PIN)=1;
  } else {
    RMSVolt = (uint16)( readCF1()/COEF_VOLT );
    PORT(SEL_PORT,SEL_PIN)=0;
  }
}


uint16 readCF(void){
  uint16 cfDuration;
  T1CNTL=0;
  T1_OVFIF=0;
  while(PORT(CF_PORT,CF_PIN)==0 && T1_OVFIF==0);
  if (T1_OVFIF)
    return 0;
  
  T1CNTL=0;
  T1_OVFIF=0;
  while(PORT(CF_PORT,CF_PIN)==1 && T1_OVFIF==0);
  while(PORT(CF_PORT,CF_PIN)==0 && T1_OVFIF==0);
  if (T1_OVFIF)
    return 0;
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
    return 0;
  
  T1CNTL=0;
  T1_OVFIF=0;
  while(PORT(CF1_PORT,CF1_PIN)==1 && T1_OVFIF==0);
  while(PORT(CF1_PORT,CF1_PIN)==0 && T1_OVFIF==0);
  if (T1_OVFIF)
    return 0;
  cf1Duration = T1CNTL;
  cf1Duration += (T1CNTH<<8);
  return cf1Duration;
}                             
                             