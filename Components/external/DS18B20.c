
#include <ioCC2530.h>

#include "comdef.h"
#include "ClusterOSALEvents.h"
#include "hal_types.h"
#include "zcl.h"
#include "zcl_general.h"
#include "OSAL_PwrMgr.h"
#include "OSAL_Timers.h"
#include "regs.h"

#include "DS18B20.h"


#define DEFAULT_READ_PERIOD_MINUTES 5

#ifndef READ_PERIOD_MINUTES
#define READ_PERIOD_MINUTES DEFAULT_READ_PERIOD_MINUTES
#endif

#define READ_PERIOD_MAX_COUNTER 2*READ_PERIOD_MINUTES


#define DS18B20_POWER  PORT(DS18B20_POWER_PORT, DS18B20_POWER_PIN)
#define DATA_LOW  DIR(DS18B20_DATA_PORT, DS18B20_DATA_PIN)=1;PORT(DS18B20_DATA_PORT, DS18B20_DATA_PIN)=0;
#define DATA_HIGH DIR(DS18B20_DATA_PORT, DS18B20_DATA_PIN)=0;PORT(DS18B20_DATA_PORT, DS18B20_DATA_PIN)=1;
#define DATA PORT(DS18B20_DATA_PORT, DS18B20_DATA_PIN)

int16 temp=0;

static uint8 readWaitCounter=0;
static uint8 taskId;

static uint8 reset(void);
static void write(unsigned char byte);
static uint8  read(void);
static void waitRead(void);


void DS18B20_init(uint8 deviceTaskId) {
  taskId = deviceTaskId;
  FUNCTION_SEL(DS18B20_POWER_PORT, DS18B20_POWER_PIN)=0;
  FUNCTION_SEL(DS18B20_DATA_PORT, DS18B20_DATA_PIN)=0;

  DIR(DS18B20_POWER_PORT, DS18B20_POWER_PIN)=1;
  DIR(DS18B20_DATA_PORT, DS18B20_DATA_PIN)=1;

  DS18B20_POWER=0;
  PORT(DS18B20_DATA_PORT, DS18B20_DATA_PIN)=0;
  readTemperature();
}


void waitRead(void) {
  if (readWaitCounter==0){
    readTemperature();
    readWaitCounter = READ_PERIOD_MAX_COUNTER;
  } else {
    readWaitCounter--;
     osal_start_timerEx_cb(30000,&waitRead );
  }
}

void readTemperature(void) {
  DS18B20_POWER=1;
  DATA_HIGH;
  osal_pwrmgr_task_state(taskId, PWRMGR_HOLD);

  T3CTL = 0x04 | 0xA0; //Clear counter. interrupt disable. Compare mode. 4us at cycle
  T3CCTL0 = 0x4; // compare mode
  T3CCTL1 = 0;
          
  T3IF=0;
  T3CH0IF=0;
  st(T3IE=0;);
          
  if (reset()==0){
    DS18B20_POWER=0; 
    waitRead();
    osal_pwrmgr_task_state(taskId, PWRMGR_CONSERVE);
    return;
  }

  write(0xCC);
  write(0x44);
  // convertion time:
  // 9bits  ->93.75ms
  // 10bits -> 187.5ms
  // 11bits -> 375ms
  // 12bits -> 750ms

  osal_start_timerEx_cb( 750, &finalizeReadTemp );
}

void finalizeReadTemp(void){
  uint8 low;
  uint8 heigh;
  int16 tempTemperatureValue;
  int16 decTemperatureValue;
  reset();
  write(0xCC);
  write(0xBE);
  low = read();
  heigh = read();

  tempTemperatureValue = BUILD_UINT16(low,heigh);
  temp = (tempTemperatureValue >> 4)*100;
  decTemperatureValue = (tempTemperatureValue & 0x0F)*100;

  temp += decTemperatureValue >> 4;
  DS18B20_POWER=0;  
  
  osal_set_event_bit( taskId,  NEW_TEMP_BIT );
  waitRead();
  osal_pwrmgr_task_state(taskId, PWRMGR_CONSERVE);
  
}


uint8 reset() {

  DATA_LOW;
  T3_div=6;
  T3_clear=1;
  T3CNT=0;
  T3_start=1;
  while(T3CNT <240 ); // 2uS*240 = 480us
  T3_start=0;
  DATA_HIGH;
  T3_clear=1;
  T3_start=1;
  while(T3CNT < 30);
  T3_clear=1;
  while(T3CNT < 240  &&  DATA == 1);

  if (DATA== 1){ // If after 480us DS18B20 doesn't send presence, it fails
    return 0;
  }
  while(T3CNT < 240);
  return 1;
}

void write(unsigned char byte){
	uint8 bit=8;
	T3_start = 0;
	T3_div=5; /// 1us
	T3_start=1;
	T3_clear=1;
	while(T3CNT < 2);

	while(bit > 0){
		DATA_LOW;
		T3_clear=1;
		if (byte & 0x1){
			while(T3CNT < 10);
		} else {
			while(T3CNT < 60);
		}
		
		DATA_HIGH;
		while(T3CNT < 62);
		byte = byte >> 1;
		bit--;
	}
	
}

uint8  read(void) {
	uint8 bit=8;
	uint8 result=0;
	
	T3_start = 0;
	T3_div=5;
	T3_start=1;
	
	T3_clear=1;
	while(T3CNT < 2);
	while(bit > 0){
		DATA_LOW;
		T3_clear=1;
		while(T3CNT < 2);
		DATA_HIGH;
		while(T3CNT < 12);
		result >>= 1;
		if (DATA){
		  result |= 0x80;
		}
		while(T3CNT < 60);
		bit--;
	}
	return result;
}
