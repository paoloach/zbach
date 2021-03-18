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
#include "report.h"
#include "EventManager.h"
 

int16 minTemperatureValue=-10;
int16 maxTemperatureValue=80;
uint16 toleranceTemperature=10;

extern byte deviceTaskId;
extern devStates_t devState;
extern int16 temp;
static uint16 prevTemp;
static uint8 reportCounter=0;

static void sendReport(uint16 event);

struct TemperatureReport {
  uint16 id;
  uint8  type;
  uint16 value;
};

struct TemperatureReport tempReport;

void clusterTemperatureMeasurementeInit(void) {
  tempReport.id = ATTRID_TEMPERATURE_MEASURE_VALUE;
  tempReport.type = ZCL_DATATYPE_INT16;
    
  addEventCB(NEW_TEMP_BIT, &sendReport);
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


void sendReport(uint16 event){
  if (reportCounter > 10 || prevTemp != temp){
    reportCounter=0;
    tempReport.value = temp;
  
    zcl_SendCommand( reportEndpoint,  &reportDstAddr, ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT, ZCL_CMD_REPORT, FALSE,
                              ZCL_FRAME_SERVER_CLIENT_DIR, TRUE, 0, reportSeqNum++, 5, (uint8*)&tempReport );
  }
  reportCounter++;
  prevTemp  = temp;
  

}