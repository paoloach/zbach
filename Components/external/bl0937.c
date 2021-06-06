
#include <math.h>
#include <OnBoard.h>
#include "hal_mcu.h"
#include "OSAL_Timers.h"
#include "bl0937.h"
#include "ElectricityMeasureData.h"
#include "regs.h"
#include "ClusterOSALEvents.h"

#ifdef DISPLAY
#include "lcd.h"
#include "fonts/FreeMono9pt7b.h"
extern const GFXfont Font5x7Fixed;
#endif



static void readCFs(void);


static uint16_t CFprevValue=0;
static uint32_t CFaccCount=0;
static uint16_t CFCount=0;
static uint16_t CFMean=0;

static uint16_t CF1prevValue=0;
static uint32_t CF1accCount=0;
static uint16_t CF1Count=0;
static uint16_t CF1Mean=0;


static uint16_t T1CC;
static uint8_t taskId;

#define COEF_POWER      522250.0
#define COEF_CURRENT    1270000.0
#define COEF_VOLT       1322.0*220



void BL0937_init(uint8_t _taskId){
  taskId = _taskId;
 
  FUNCTION_SEL(CF_PORT,CF_PIN)=1;
  FUNCTION_SEL(CF1_PORT,CF1_PIN)=0;
  FUNCTION_SEL(SEL_PORT,SEL_PIN)=0;
  
  DIR(SEL_PORT,SEL_PIN)=1;
  DIR(CF_PORT,CF_PIN)=0;
  DIR(CF1_PORT,CF1_PIN)=0;
 
  PORT(SEL_PORT,SEL_PIN)=0;
  
  P2DIR &= 0x3F;
  P2DIR |= 0x80;
  P2SEL &= 0xE0;
  P2SEL |= 0x08;
  T1_div=1;
  T1_mode=1;
  
  T1_CH1_MODE=0;
  T1_CH1_CAP=1;
  T1_CH1_IM=1;
  
  T1_CH2_MODE=0;
  T1_CH2_CAP=1;
  T1_CH2_IM=1;
  
  
  T1_CH0_MODE=0;
  T1_CH0_CAP=1;
  T1_CH0_IM=1;
  
  T1_CH3_MODE=0;
  T1_CH3_CAP=1;
  T1_CH3_IM=1;
  
  T1_CH4_MODE=0;
  T1_CH4_CAP=1;
  T1_CH4_IM=1;
  
  
  
  TIMIF=0;
 
  osal_start_reload_timer_cb(1000, &readCFs);
  osal_set_event_bit( taskId,  NEW_POWER_BIT );
  
  T1IE=1;
  
  P0IFG=0;
  P0IF=0;
  P0IE=1;
  P0IEN=0x10;
}

void readCFs(void) {
  halIntState_t intState;

 
  HAL_ENTER_CRITICAL_SECTION( intState );  // Hold off interrupts.
  uint16_t tempCFMean = CFMean;
  CFMean=0;
  HAL_EXIT_CRITICAL_SECTION( intState );   // Re-enable interrupts.

  if (tempCFMean == 0){
    activePower = 0;
  } else {
     activePower =(uint16)(COEF_POWER/tempCFMean);
  }
#ifdef DISPLAY  
  setFont(&FreeMono9pt7b);
  char buffer[10];
  clean(0,9,displayWidth, 27);
  setCursor(1,27);
  _itoa(activePower, (uint8_t*)buffer, 10);
  drawText(buffer);
  setFont(&Font5x7Fixed); 
  drawText(" W");
#endif

  HAL_ENTER_CRITICAL_SECTION( intState );  // Hold off interrupts.
  
  uint16_t tempCF1Mean = CF1Mean;
  CF1Mean=0;
   
  HAL_EXIT_CRITICAL_SECTION( intState );   // Re-enable interrupts.
  
  if (tempCF1Mean == 0){
    RMSCurrent=0;
  } else {
    RMSCurrent = (uint16_t) (COEF_CURRENT/ tempCF1Mean);
  }
  RMSVolt = 0;
#ifdef DISPLAY  
  setFont(&FreeMono9pt7b);
  clean(0,27,displayWidth, 45);
  setCursor(1,45);
  _itoa(RMSCurrent/1000, (uint8_t*)buffer, 10);
  drawText(buffer);
  drawText(".");
  uint16_t mils = RMSCurrent%1000;
  if (mils < 100){
    drawText("0");
  }
  if (mils < 10){
    drawText("0");
  }
  _itoa(mils, (uint8_t*)buffer, 10);
  drawText(buffer);
  setFont(&Font5x7Fixed); 
  drawText(" A");
  display();
#endif
  
  osal_set_event_bit( taskId,  NEW_POWER_BIT );

}



HAL_ISR_FUNCTION( IOisr, P0INT_VECTOR )
{
   HAL_ENTER_ISR();
   if (P0IFG & 0x10){
     static union Reg16 end;
     end.reg.low = T1CNTL;
     end.reg.high = T1CNTH;
   
     uint16_t period=end.value;
    
    if (end.value > CF1prevValue){
      period -= CF1prevValue;
    } else {
      period += (0xFFFF-CF1prevValue)+1;
    }
    CF1accCount += period;
    CF1Count++;
    if (CF1Count >= 1000){
      CF1Mean = CF1accCount / CF1Count;
      CF1accCount = 0;
      CF1Count = 0;
    }
    CF1prevValue = end.value;
   }
   
   P0IFG=0;
   P0IF=0;
    CLEAR_SLEEP_MODE();
  HAL_EXIT_ISR();
}

HAL_ISR_FUNCTION( Timer1Isr, T1_VECTOR )
{
  HAL_ENTER_ISR();

  
  if (T1_CH1IF){
    T1CC=T1CC1L;
    T1CC += ((uint16_t)T1CC1H <<8);

    uint16_t period=T1CC;
    if (T1CC > CFprevValue){
      period -= CFprevValue;
    } else {
      period += (0xFFFF-CFprevValue)+1;
    }
    CFaccCount += period;
    CFCount++;
    if (CFCount >= 1000){
      CFMean = CFaccCount / CFCount;
      CFaccCount = 0;
      CFCount = 0;
    }
    
    CFprevValue = T1CC;
    T1_CH1IF=0;
  }
  
  T1_CH0IF=0;
T1_CH3IF=0;
T1_CH4IF=0;
  T1IF=0;
  CLEAR_SLEEP_MODE();
  HAL_EXIT_ISR();
}
                             
