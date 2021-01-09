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
#ifdef DHT12
#include "dht112.h"
#endif
#ifdef DS18B20
#include "DS18B20.h"
#endif
  

int16 minTemperatureValue=-10;
int16 maxTemperatureValue=80;
uint16 toleranceTemperature=10;

extern byte deviceTaskId;
extern devStates_t devState;

void clusterTemperatureMeasurementeInit(void) {
#ifdef DHT12
  dht112_init(deviceTaskId);
#endif
#ifdef DS18B20  
  DS18B20_init(deviceTaskId);
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
    dht112_loop(deviceTaskId);
    return ( events ^ READ_TEMP_EVT );
  }
#endif
#ifdef DS18B20
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


void temperatureClusterSendReport(uint8 endpoint, afAddrType_t * dstAddr, uint8 * segNum){
#ifdef DS18B20
  setReportDest(endpoint, dstAddr, segNum);
  readTemperature();
#endif
    
}

