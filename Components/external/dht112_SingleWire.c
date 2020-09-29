
#include <ioCC2530.h>
#include "comdef.h"
#include "hal_types.h"
#include "OSAL_Timers.h"
#include "regs.h"
#include "hal_i2c.h"

#include "ClusterOSALEvents.h"
#include "dht112.h"

#define TIME_READ_ms 30*1000
#define DEFAULT_READ_PERIOD_MINUTES 0

#ifndef READ_PERIOD_MINUTES
#define READ_PERIOD_MINUTES DEFAULT_READ_PERIOD_MINUTES
#endif

#define READ_PERIOD_MAX_COUNTER 2*READ_PERIOD_MINUTES
#define START_READ_PERIOD_ms 200
#define STARTING_DELAY_ms 5000
#define STABILING_DELAY_ms 3000

#define SDA P1_5
#define POWER P1_4
#define SDA_OFF  DIR1_5=0
#define SDA_ON   DIR1_5=1

int16 temp;
uint16 humidity;


enum Status {
        START,
	WAIT,
	READ_START,
        READ,
	ERROR
};

static enum Status status=START; 
static uint8 readPeriodCounter;

static enum Status waitAction(uint8 taskid);
static enum Status readStartAction(uint8 taskid);
static enum Status readAction(uint8 taskid);
static enum Status resetAction(uint8 taskid);
static enum Status startAction(uint8 taskid);
static bool read8Bit(uint8 * data);
static enum Status internalReadAction(void);


void dht112_init(uint8 taskid){
    P1SEL &=0xCF; // 11001111
    P1INP &= 0xCF;
    P2INP &= 0xBF;
    DIR1_4=1;
    POWER=0;
    P0_1 = 0;
    SDA_OFF;
    SDA = 0;
    
    osal_start_timerEx(taskid, READ_TEMP_EVT, STARTING_DELAY_ms );
}

void dht112_loop(uint8 taskid) {
  do {
    switch(status){
      case START:
        status = startAction(taskid);
        break;
      case WAIT:
        status = waitAction(taskid);
        break;
      case READ_START:
        status = readStartAction(taskid);
        break;
      case READ:
        status = readAction(taskid);
        break;
      case ERROR:
        status = resetAction(taskid);
        break;
      }
  } while (status == READ_START || status == ERROR);
}

static enum Status startAction(uint8 taskid) {
  POWER=1;
  P0_1 = 1;
  readPeriodCounter=0;  
  osal_start_timerEx(taskid, READ_TEMP_EVT, 10 ); 
  return READ_START;
}

static enum Status waitAction(uint8 taskid) {
  readPeriodCounter++;
  if (readPeriodCounter >= READ_PERIOD_MAX_COUNTER){
    readPeriodCounter=0;
     osal_start_timerEx(taskid, READ_TEMP_EVT, STARTING_DELAY_ms );
    return START;
  } else {
    osal_start_timerEx(taskid, READ_TEMP_EVT, TIME_READ_ms );
    return WAIT;
  }
	
}

static enum Status readStartAction(uint8 taskid) {
  SDA_ON;
  osal_start_timerEx(taskid, READ_TEMP_EVT, START_READ_PERIOD_ms );
  return READ;
}



static enum Status readAction(uint8 taskid) {
  halIntState_t intState;
  HAL_ENTER_CRITICAL_SECTION( intState );
    enum Status ret = internalReadAction();
  HAL_EXIT_CRITICAL_SECTION( intState );
  if (status == WAIT){
     osal_start_timerEx(taskid, READ_TEMP_EVT, TIME_READ_ms );
  }
  return ret;
}


static enum Status internalReadAction(void){
  uint8 data[5];
  T3CTL=0;
  T3_mode = 0x00;
  T3_div = 4; 
  SDA_OFF;
  T3CCTL0 = 0;
  T3CCTL1 = 0;
  T3IF=0;
  T3CH0IF=0;
  T3IE=0;
  T3OVFIF=0;      
  
  T3_clear=1;
  T3_start=1;
  while(SDA == 1 && T3OVFIF==0);
  T3_start=0;
  if (T3OVFIF || T3CNT < 10)
    return ERROR;
  T3_start=0;
  
  T3_clear=1;
  T3_start=1;
  while(SDA == 0 && T3OVFIF==0);
  T3_start=0;
  if (T3OVFIF || T3CNT < 30)
    return ERROR;

  T3_clear=1;
  T3_start=1; 
  while(SDA == 1 && T3OVFIF==0);
  T3_start=0;
  if (T3OVFIF || T3CNT < 30)
    return ERROR;

  if (read8Bit(&data[0]) == false)
    return ERROR;
  if (read8Bit(&data[1]) == false)
    return ERROR;
  if (read8Bit(&data[2]) == false)
    return ERROR;
  if (read8Bit(&data[3]) == false)
    return ERROR;
  if (read8Bit(&data[4]) == false)
    return ERROR;
  
  humidity = (uint16)data[0]*100 + data[1];
  temp = (uint16)data[2]*100 + data[3];
  POWER=0;
  P0_1 = 0;
  return WAIT;
}

static bool read8Bit(uint8 * data) {
  for(uint8 i=0; i < 8; i++){
    T3_clear=1;
    T3_start=1;  
    while(SDA == 0 && T3OVFIF==0);
    T3_start=0;
    if (T3OVFIF || T3CNT < 30)
      return false;
    
    T3_clear=1;
    T3_start=1;
    while(SDA == 1 && T3OVFIF==0);
    T3_start=0;
    if (T3OVFIF || T3CNT < 30)
      return false;
    *data = *data << 1;
    if (T3CNT > 90){
      *data |= 1;
    }
  }
  return true;
}

static enum Status resetAction(uint8 taskid) {
  SDA_OFF; 
  osal_start_timerEx(taskid, READ_TEMP_EVT, TIME_READ_ms );
  temp=0xFFFF;
  humidity=0xFFFF;
  POWER=0;
    P0_1 = 0;
  return WAIT;
}

