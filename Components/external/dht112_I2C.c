
#include <ioCC2530.h>
#include "hal_types.h"
#include "OSAL_Timers.h"
#include "regs.h"
#include "hal_i2c.h"

#include "ClusterOSALEvents.h"
#include "dht112.h"

#define TIME_READ_ms 2000
#define DEFAULT_READ_PERIOD_MINUTES 0

#ifndef READ_PERIOD_MINUTES
#define READ_PERIOD_MINUTES DEFAULT_READ_PERIOD_MINUTES
#endif

#define READ_PERIOD_MAX_COUNTER 2*READ_PERIOD_MINUTES

#define SDA PORT(DHT112_SDA_PORT, DHT112_SDA_PIN)
#define SCL PORT(DHT112_SCL_PORT, DHT112_SCL_PIN)
#define SDA_OFF  DIR(DHT112_SDA_PORT, DHT112_SDA_PIN)=0
#define SDA_ON   DIR(DHT112_SDA_PORT, DHT112_SDA_PIN)=1
#define SCL_ON   DIR(DHT112_SCL_PORT, DHT112_SCL_PIN)=1
#define SCL_OFF  DIR(DHT112_SCL_PORT, DHT112_SCL_PIN)=0

enum Status {
	WAIT,
	READ,
	RESET
};

static enum Status status=WAIT; 
static uint8 readPeriodCounter;


static enum Status waitAction(void);
static enum Status readAction(void);
static enum Status resetAction(void);
static void startI2c(void);
static uint8 writeI2c(uint8 data);
static uint8 readI2c(uint8 * data);
static void stopI2c(void);


#define WAITCLK 		while(T3OVFIF == 0);T3OVFIF=0;

void dht112_init(uint8 taskid){
   FUNCTION_SEL(DHT112_SCL_PORT, DHT112_SCL_PIN)=0;
    FUNCTION_SEL(DHT112_SDA_PORT, DHT112_SDA_PIN)=0;
    
    PULLUP_DOWN(DHT112_SDA_PORT, DHT112_SDA_PIN)=0;
    PULL_UP(DHT112_SDA_PORT);
    SDA_OFF;
    SCL_OFF;  
    SDA = 0;
    SCL = 0;
    
    osal_start_timerEx(taskid, READ_TEMP_EVT, TIME_READ_ms );
}

void dht112_loop(uint8 taskid) {
	do {
		switch(status){
		   case WAIT:
			  status = waitAction();
			  break;
		   case READ:
			  status = readAction();
			  break;
		  case RESET:
			  status = resetAction();
			  break;
		}
	} while (status != WAIT);
	
	osal_start_timerEx(taskid, READ_TEMP_EVT, TIME_READ_ms );
}


static enum Status waitAction(void) {
	readPeriodCounter++;
	if (readPeriodCounter >= READ_PERIOD_MAX_COUNTER){
		return READ;
	} else {
		return WAIT;
	}
	
}

static enum Status readAction(void) {
	uint8 data[5];
	T3CTL=0;
	T3_mode = 0x00;
	T3_div = 0;
	T3CCTL0 = 0;
	T3CCTL1 = 0;
	T3IF=0;
	T3CH0IF=0;
	T3IE=0;
	T3OVFIF=0;
	
	T3_start=1;
	startI2c();
	if(writeI2c(0xB8)==0){
		return RESET;
	};
	if(writeI2c(0) == 0){
		return RESET;
	}
	
	stopI2c();
	startI2c();
	if(writeI2c(0xB9)==0){
		return RESET;
	};
	if (readI2c(data)==0){
		return RESET;
	}
	if (readI2c(data+1)==0){
		return RESET;
	}
	if (readI2c(data+2)==0){
		return RESET;
	}
	if (readI2c(data+3)==0){
		return RESET;
	}
	if (readI2c(data+5)==0){
		return RESET;
	}
	return WAIT;
}

static enum Status resetAction(void) {
	stopI2c();
	return WAIT;
}

static void startI2c(void){
  SCL_OFF;
  SDA_OFF;
  WAITCLK;
  SDA_ON;
  SCL=1;
  SCL_ON;  
  WAITCLK;
  SCL=0;
  WAITCLK;  
}

static void stopI2c(void){
  SCL_ON;
  SDA_ON;
  WAITCLK;
  SCL_OFF;
  WAITCLK;
  SDA_OFF; 
}


static uint8 writeI2c(uint8 data) {
	uint8 ack;
	for (uint8 i=0; i < 8; i++){
                SCL=0;
		WAITCLK
		if ( (data & 0x80) == 0)
			SDA=0;
		else
			SDA=1;
		WAITCLK;
		SCL=1;
		WAITCLK;
		data = data << 1;
	}
	SCL=0;
	WAITCLK;
	SDA_OFF;
	WAITCLK;
	SCL=1;
	WAITCLK;
	if (SDA==0)
		ack =  1;
	else
		ack =  0;
	WAITCLK;
	SCL=0;
	WAITCLK;
	return ack;
}

static uint8 readI2c(uint8 * data) {
	uint8 ack;
	SDA_OFF;
	*data=0;
	for (uint8 i=0; i <= 8; i++){
		SCL_ON;
		WAITCLK
		SCL_OFF;
		WAITCLK
 
		*data= *data << 1;
                if (SDA != 0){
                  *data |= 1; 
                }
	}
        SCL_ON;
	WAITCLK
        SDA_ON; 
	SCL_OFF;
	WAITCLK
	SCL_ON;
	WAITCLK
	return ack;
}
