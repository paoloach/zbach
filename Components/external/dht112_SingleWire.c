
#include <ioCC2530.h>
#include <OnBoard.h>
#include "comdef.h"
#include "hal_types.h"
#include "OSAL_Timers.h"
#include "regs.h"
#include "hal_i2c.h"
#include "OSAL.h"

#include "ClusterOSALEvents.h"
#include "EventManager.h"
#include "dht112.h"
#ifdef DISPLAY
#include "lcd.h"
#include "fonts/FreeMono9pt7b.h"

#endif

#define DEFAULT_READ_PERIOD_MINUTES 5

#ifndef READ_PERIOD_MINUTES
#define READ_PERIOD_MINUTES DEFAULT_READ_PERIOD_MINUTES
#endif

#define DHT_INIT_TIME_ms 1500
#define START_READ_PERIOD_ms 200
#define STARTING_DELAY_ms 5000
#define STABILING_DELAY_ms 3000

#define SDA PORT(DHT112_SDA_PORT, DHT112_SDA_PIN)
#define DHT112_POWER PORT(DHT112_POWER_PORT, DHT112_POWER_PIN)
#define SDA_OFF  DIR(DHT112_SDA_PORT, DHT112_SDA_PIN)=0
#define SDA_ON   DIR(DHT112_SDA_PORT, DHT112_SDA_PIN)=1

#ifdef DISPLAY
extern const GFXfont Font5x7Fixed;
#endif

int16 temp;
uint16 humidity;
uint8 taskId;

enum Status {
        START,
	WAIT,
	READ_START,
        DHT11_START,
        READ,
	ERROR
};

static enum Status status=START; 

static enum Status waitAction(void);
static enum Status readStartAction(void);
static enum Status readAction(void);
static enum Status resetAction(void);
static enum Status startAction(void);
static enum Status startDHT12(void);
static bool read8Bit(uint8 * data);
static enum Status internalReadAction(void);
static void dht112_loop(void);

void dht112_init(uint8 deviceTaskId){
    taskId = deviceTaskId;
    FUNCTION_SEL(DHT112_POWER_PORT, DHT112_POWER_PIN)=0;
    FUNCTION_SEL(DHT112_SDA_PORT, DHT112_SDA_PIN)=0;
  
    PULLUP_DOWN(DHT112_SDA_PORT, DHT112_SDA_PIN)=0;
    PULL_UP(DHT112_SDA_PORT);
      
    DIR(DHT112_POWER_PORT, DHT112_POWER_PIN)=1;
    DHT112_POWER=0;
    SDA_OFF;
    SDA = 0;
    
    osal_start_timerEx_cb(STARTING_DELAY_ms, &dht112_loop );
}

void dht112_loop(void) {
  do {
    switch(status){
      case START:
        status = startAction();
        break;
      case WAIT:
        status = waitAction();
        break;
      case READ_START:
        status = readStartAction();
        break;
      case DHT11_START:
        status = startDHT12();
        break;  
      case READ:
        status = readAction();
        break;
      case ERROR:
        status = resetAction();
        break;
      }
  } while (status == ERROR);
}

static enum Status startAction() {
  DHT112_POWER=1;
  osal_start_timerEx_cb(DHT_INIT_TIME_ms, &dht112_loop );
  return READ_START;
}

static enum Status waitAction() {
  //osal_start_timerEx_cb(READ_PERIOD_MINUTES*6000, &dht112_loop );
  osal_start_timerEx_cb(10000, &dht112_loop );
  return START;
}

static enum Status readStartAction() {
  SDA_ON;
  osal_start_timerEx_cb(STABILING_DELAY_ms, &dht112_loop );
  return DHT11_START;
}


static enum Status startDHT12() {
  SDA_ON;
  SDA=0;
  osal_start_timerEx_cb(START_READ_PERIOD_ms, &dht112_loop );
  return READ;
}


static enum Status readAction() {
  enum Status ret = internalReadAction();
#ifdef DISPLAY  
  uint8_t buffer[20];
  setFont(&FreeMono9pt7b);
  
  setCursor(0,27);
  clean(0,9,displayWidth, 50);
    
  if (ret == ERROR){
    drawText("ERROR");
  } else {
    drawText("Temp: ");
    _itoa(temp/100, buffer, 10);
    drawText((char*)buffer);
    drawText(".");
    _itoa(temp%100, buffer, 10);
    drawText((char*)buffer);
    setCursor(0,50);
    drawText("Hum: ");
    _itoa(humidity/100, buffer, 10);
    drawText((char *)buffer);
    drawText(".");
    _itoa(temp%100, buffer, 10);
    drawText((char*)buffer);
    drawText("%");

 
  }
  display();
  setFont(&Font5x7Fixed);
#endif
  return ret;
}


static enum Status internalReadAction(void){
  uint8 data[5];
  T3CTL=0;
  T3_mode = 0x00;
  T3_div = 4; // 1 cycle 0.5 uS
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
  DHT112_POWER=1;
  osal_start_timerEx_cb(STARTING_DELAY_ms, &dht112_loop );
  osal_set_event_bit( taskId,  NEW_TEMP_BIT );
  osal_set_event_bit( taskId,  NEW_HUMIDITY_BIT );
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

static enum Status resetAction() {
  SDA_OFF; 
  osal_start_timerEx_cb(1000, &dht112_loop );
  temp=0xFFFF;
  humidity=0xFFFF;
  DHT112_POWER=1;
   return WAIT;
}

