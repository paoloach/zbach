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

static void newTemperature(uint16 );
static void sendReport(void);

void clusterTemperatureMeasurementeInit(void) {
  addEventCB(NEW_TEMP_BIT, &newTemperature);
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

void newTemperature(uint16 event) {
  sendReport();
}

void sendReport(void){
  zclReportCmd_t *pReportCmd;

  pReportCmd = osal_mem_alloc( sizeof(zclReportCmd_t) + sizeof(zclReport_t) );
  if ( pReportCmd != NULL ) {
    pReportCmd->numAttr = 1;
    pReportCmd->attrList[0].attrID = ATTRID_TEMPERATURE_MEASURE_VALUE;
    pReportCmd->attrList[0].dataType = ZCL_DATATYPE_INT16;
    pReportCmd->attrList[0].attrData = (void *)(&temp);

    zcl_SendReportCmd( reportEndpoint, &reportDstAddr,
                       ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT,
                       pReportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, TRUE, reportSeqNum++ );
    osal_mem_free( pReportCmd );
  }
}