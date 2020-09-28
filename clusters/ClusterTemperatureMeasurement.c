/**************************************************************************************************

 DESCRIPTION:
  Temperature Measurement Cluster

 CREATED: 12/11/2014, by Paolo Achdjian

 FILE: ClusterTemperatureMeasurement.c

RESOURCES:
 P1.2 Generaio I/O
 T3

***************************************************************************************************/
#include "ioCC2530.h"
#include "hal_mcu.h"
#include "ZDApp.h"
#include "hal_led.h"
#include "ClusterTemperatureMeasurement.h"
#include "OSAL_PwrMgr.h"
#include "regs.h"
#include "dht112.h"


#ifndef DHT12
#define WAIT_FOR_480us 	T3_start=0;		T3CC0=240;T3_clear=1; T3_start=1;
#define WAIT_FOR_2us T3_start=0;		T3CC0=8; T3_clear=1;T3_start=1;
#define WAIT_FOR_1us T3_start=0;		T3CC0=4; T3_clear=1;T3_start=1;
#define WAIT_FOR_12us T3_start=0;		T3CC0=48; T3_clear=1;T3_start=1;
#define WAIT_FOR_47us T3_start=0;	T3_clear=1;	T3CC0=188; T3_start=1;
#define WAIT_FOR_58us T3_start=0;	T3_clear=1;	T3CC0=232; T3_start=1;
#define WAIT_FOR_60us T3_start=0;	T3_clear=1;	T3CC0=240; T3_start=1;
#define WAIT_FOR_1ms T3_start=0;	T3_clear=1;	T3CC0=250; T3_start=1;
//#define WAIT_FOR_600us st(T3CTL &=0xE0;T3CTL |= 0x04;T3CC1=150;T3CTL &= 0xFB;)
#define WAIT_FOR_300us st(T3CTL &=0xE0; T3CTL |= 0x04;T3CC1=75;T3CTL &= 0xFB;)
#define WAIT_FOR_4us st(T3CTL &=0xE0; T3CTL |= 0x04;T3CC1=1;T3CTL &= 0xFB;)
#define WAIT_FOR_8us st(T3CTL &=0xE0; T3CTL |= 0x04;T3CC1=2;T3CTL &= 0xFB;)
#define WAIT_FOR_16us st(T3CTL &=0xE0; T3CTL |= 0x04;T3CC1=4;T3CTL &= 0xFB;)

#define ENABLE_P1EN st( IEN2 |= 0x10;)
#define DISABLE_P1EN st(IEN2 &= 0xEF;);

#define DISABLE_P1_5_INT  st(P1IEN &= 0xDF;)

#define STOP_T3 st(T3CNT &= 0xEF;);
#define START_T3 st(T3CNT |= 0x10;);
#define P1_LOW DIR1_0=1; P1_0 = 0;
#define P1_HIGH DIR1_0=0;

#define ENABLE_T3_CH0_INT  T3CCTL0_im = 1;
#define DISABLE_T3_CH0_INT T3CCTL0_im=0;
#define RESET_TM3   st(T3CNT=0;)
#define RESET_T3CH0_INT st(T3CH0IF=0;)
#define RESET_P1_5_INT st(P1IF=0; P1IFG =0;);


#define TIME_READ_ms 30*1000
#define MINUTES_BETWEEN 5
uint8 countMinutes=0;

int16 tempTemperatureValue;
int16 decTemperatureValue;
int16 temp=0;


static void write(unsigned char byte);
static uint8  read(void);
static void startReadSyncronus(void);
static uint8 reset(void);
static void finalizeReadTemp(void);
#endif


int16 minTemperatureValue=-10;
int16 maxTemperatureValue=80;
uint16 toleranceTemperature=10;

extern byte temperatureSensorTaskID;
extern devStates_t devState;



void clusterTemperatureMeasurementeInit(void) {
#ifdef DHT12
	dht112_init(temperatureSensorTaskID);
#else
	P1SEL &=0xC6;
	DIR1_0 = 0;
	DIR1_5 = 1;
	DIR1_3 = 1;
	P1_5=0;
	P1_3=1;
	P1_0 = 0;

	
	
	countMinutes = 2*MINUTES_BETWEEN;
	readTemperature();
	osal_start_timerEx( temperatureSensorTaskID, READ_TEMP_EVT, TIME_READ_ms );
#endif
}

void temperatureClusterReadAttribute(zclAttrRec_t * statusRec) {
	if (statusRec == NULL){
		return;
	}
	statusRec->accessControl = ACCESS_CONTROL_READ;
	statusRec->status=ZCL_STATUS_SUCCESS;
	switch(statusRec->attrId){
		case ATTRID_TEMPERATURE_MEASURE_VALUE:
			statusRec->dataType = ZCL_DATATYPE_INT16;
			statusRec->dataPtr = (void *)&temp;
		break;
		case ATTRID_TEMPERATURE_MIN_MEASURE_VALUE:
			statusRec->dataType = ZCL_DATATYPE_INT16;
			statusRec->dataPtr = (void *)&minTemperatureValue;
		break;
		case ATTRID_TEMPERATURE_MAX_MEASURE_VALUE:
			statusRec->dataType = ZCL_DATATYPE_INT16;
			statusRec->dataPtr = (void *)&maxTemperatureValue;
		break;
		case ATTRID_TEMPERATURE_TOLERANCE:
			statusRec->dataType = ZCL_DATATYPE_UINT16;
			statusRec->dataPtr = (void *)&toleranceTemperature;
		break;
		default:
			statusRec->status = ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
	}
}

uint16 readTemperatureLoop(uint16 events) {
#ifdef DHT12
	if (events & READ_TEMP_EVT){
		dht112_loop(temperatureSensorTaskID);
		return ( events ^ READ_TEMP_EVT );
	}
#else
	if (events & READ_TEMP_EVT){
		readTemperature();
		return ( events ^ READ_TEMP_EVT );
	};
	if (events & START_READ_TEMP){
		startReadSyncronus();
		return ( events ^ START_READ_TEMP );
	};
	if (events & END_READ_TEMP_EVT){
		finalizeReadTemp();
		return ( events ^ END_READ_TEMP_EVT );
	}
#endif
	return events;
}

#ifndef DHT12
void readTemperature(void) {
	countMinutes++;
	if (countMinutes >= 2*MINUTES_BETWEEN  || 1){
		P1_5=1;
		P1_4=1;
		osal_pwrmgr_task_state(temperatureSensorTaskID, PWRMGR_HOLD);
		osal_start_timerEx( temperatureSensorTaskID, START_READ_TEMP, 100 );
		countMinutes=0;
	}
	osal_start_timerEx( temperatureSensorTaskID, READ_TEMP_EVT, TIME_READ_ms );
}


void startReadSyncronus(void) {
	P1_5 = 1;
	P1_3=0;
	T3CTL = 0x04 | 0xA0; //Clear counter. interrupt disable. Compare mode. 4us at cycle
	T3CCTL0 = 0x4; // compare mode
	T3CCTL1 = 0;
	P0DIR=0xFF;
		
	DISABLE_P1_5_INT;
	T3IF=0;
	T3CH0IF=0;
	st(T3IE=0;);
		
	if (reset()==0){
		P1_5=0; 
		P1_3=1;
		osal_pwrmgr_task_state(temperatureSensorTaskID, PWRMGR_CONSERVE);
		return;
	}
	
	write(0xCC);
	write(0x44);
	
	osal_start_timerEx( temperatureSensorTaskID, END_READ_TEMP_EVT, 1000 );
}

void finalizeReadTemp(void){
	uint8 low;
	uint8 heigh;
	reset();
	write(0xCC);
	write(0xBE);
	low = read();
	heigh = read();
	
	tempTemperatureValue = BUILD_UINT16(low,heigh);
	temp = (tempTemperatureValue >> 4)*100;
	decTemperatureValue = (tempTemperatureValue & 0x0F)*100;
	
	temp += decTemperatureValue >> 4;
	P1_5=0;  
	P1_3=1;
	osal_pwrmgr_task_state(temperatureSensorTaskID, PWRMGR_CONSERVE);
}


uint8 reset() {
	
	P1_LOW;
	P1_3=1;
	T3_div=6;
	T3_clear=1;
	T3CNT=0;
	T3_start=1;
	while(T3CNT <250 );
	T3_start=0;
	P1_3=0;
	P1_HIGH;
	T3_clear=1;
	T3_start=1;
	while(T3CNT < 30);
	P1_3=1;
	T3_clear=1;
	while(T3CNT < 240  && P1_0 == 1);
	
	if (P1_0 == 1){
		return 0;
	}
	if (P1_0 == 0){
		P1_3=0;
	}
	while(P1_0==0){
		P1_3=0;
	}
	return 1;
}

void write(unsigned char byte){
	uint8 bit=8;
	T3_start = 0;
	T3_div=5;
	T3_start=1;
	T3_clear=1;
	while(T3CNT < 2);

	while(bit > 0){
		P1_LOW;
		T3_clear=1;
		if (byte & 0x1){
			while(T3CNT < 10);
		} else {
			while(T3CNT < 60);
		}
		
		P1_HIGH;
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
		P1_LOW;
		T3_clear=1;
		while(T3CNT < 2);
		P1_HIGH;
		T3_clear=1;
		while(T3CNT < 10);
		result >>= 1;
		if (P1_0){
			result |= 0x80;
		}
		T3_clear=1;
		while(T3CNT < 30);
		bit--;
	}
	return result;
}

#endif

