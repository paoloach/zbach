
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

#define DS18B20_POWER  PORT(DS18B20_POWER_PORT, DS18B20_POWER_PIN)


#define DATA_LOW  DIR(DS18B20_DATA_PORT, DS18B20_DATA_PIN)=1;PORT(DS18B20_DATA_PORT, DS18B20_DATA_PIN)=0;
#define DATA_HIGH DIR(DS18B20_DATA_PORT, DS18B20_DATA_PIN)=1
#define DATA DIR(DS18B20_DATA_PORT, DS18B20_DATA_PIN)

int16 temp=0;


static uint8 taskId;
static uint8 reportEndpoint;
static afAddrType_t * reportDstAddr;
static uint8 * reportSegNum;

static uint8 reset(void);
static void write(unsigned char byte);
static uint8  read(void);

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


void readTemperature(void) {
  DS18B20_POWER=1;
  PORT(DS18B20_DATA_PORT, DS18B20_DATA_PIN)=1;
  osal_pwrmgr_task_state(taskId, PWRMGR_HOLD);
  osal_start_timerEx(taskId, START_READ_TEMP, 100 );
}


void startReadSyncronus(void) {
  DS18B20_POWER = 1;
  T3CTL = 0x04 | 0xA0; //Clear counter. interrupt disable. Compare mode. 4us at cycle
  T3CCTL0 = 0x4; // compare mode
  T3CCTL1 = 0;
          
  T3IF=0;
  T3CH0IF=0;
  st(T3IE=0;);
          
  if (reset()==0){
    DS18B20_POWER=0; 
    osal_pwrmgr_task_state(taskId, PWRMGR_CONSERVE);
    return;
  }

  write(0xCC);
  write(0x44);

  osal_start_timerEx( taskId, END_READ_TEMP_EVT, 1000 );
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
  
  
  zclReportCmd_t *pReportCmd;

  pReportCmd = osal_mem_alloc( sizeof(zclReportCmd_t) + sizeof(zclReport_t) );
  if ( pReportCmd != NULL ) {
    pReportCmd->numAttr = 1;
    pReportCmd->attrList[0].attrID = ATTRID_TEMPERATURE_MEASURE_VALUE;
    pReportCmd->attrList[0].dataType = ZCL_DATATYPE_INT16;
    pReportCmd->attrList[0].attrData = (void *)(&temp);

    zcl_SendReportCmd( reportEndpoint, reportDstAddr,
                       ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT,
                       pReportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, TRUE, (*reportSegNum)++ );
    osal_mem_free( pReportCmd );
  }
  
  osal_pwrmgr_task_state(taskId, PWRMGR_CONSERVE);
}


uint8 reset() {

  DATA_LOW;
  T3_div=6;
  T3_clear=1;
  T3CNT=0;
  T3_start=1;
  while(T3CNT <250 );
  T3_start=0;
  DATA_HIGH;
  T3_clear=1;
  T3_start=1;
  while(T3CNT < 30);
  T3_clear=1;
  while(T3CNT < 240  && P1_0 == 1);

  if (DATA== 1){
    return 0;
  }
  T3_clear=1;
  while(T3CNT < 240){
    if (DATA == 1)
      return 1;
  }
  return 0;
}

void write(unsigned char byte){
	uint8 bit=8;
	T3_start = 0;
	T3_div=5;
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
		T3_clear=1;
		while(T3CNT < 10);
		result >>= 1;
		if (DATA){
			result |= 0x80;
		}
		T3_clear=1;
		while(T3CNT < 30);
		bit--;
	}
	return result;
}


void setReportDest(uint8 endpoint, afAddrType_t * dstAddr, uint8 * segNum) {
  reportEndpoint=endpoint;
  reportDstAddr=dstAddr;
  reportSegNum=segNum;
}