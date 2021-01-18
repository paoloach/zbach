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
#include "ClusterHumidityRelativeMeasurement.h"
#include "dht112.h"
#include "report.h"
#include "EventManager.h"


const int16 minHumidityValue=0;
const int16 maxHumidityValue=80;
const uint16 toleranceHumidity=10;

extern byte zProxSensorTaskID;

static void sendReport(void);
static void newHumidity(uint16 );

void clusterHumidityMeasurementeInit(void) {
  addEventCB(NEW_HUMIDITY_BIT, &newHumidity);
}

void humidityRelativeClusterReadAttribute(zclAttrRec_t * statusRec) {
	if (statusRec == NULL){
		return;
	}
	statusRec->accessControl = ACCESS_CONTROL_READ;
	statusRec->status=ZCL_STATUS_SUCCESS;
	switch(statusRec->attrId){
		case ATTRID_HUMIDITY_RELATIVE_MEASURE_VALUE:
			statusRec->dataType = ZCL_DATATYPE_UINT16;
			statusRec->dataPtr = (void *)&humidity;
		break;
		case ATTRID_HUMIDITY_RELATIVE_MIN_MEASURE_VALUE:
			statusRec->dataType = ZCL_DATATYPE_UINT16;
			statusRec->dataPtr = (void *)&minHumidityValue;
		break;
		case ATTRID_HUMIDITY_RELATIVE_MAX_MEASURE_VALUE:
			statusRec->dataType = ZCL_DATATYPE_UINT16;
			statusRec->dataPtr = (void *)&maxHumidityValue;
		break;
		case ATTRID_HUMIDITY_RELATIVE_TOLERANCE:
			statusRec->dataType = ZCL_DATATYPE_UINT16;
			statusRec->dataPtr = (void *)&toleranceHumidity;
		break;
		default:
			statusRec->status = ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
	}
}

uint16 readHumidityLoop(uint16 events) {
  return events;
}


void newHumidity(uint16 events ){
  sendReport();
}


void sendReport(void){
  zclReportCmd_t *pReportCmd;
  if (humidity == 0xFFFF)
    return;
  pReportCmd = osal_mem_alloc( sizeof(zclReportCmd_t) + sizeof(zclReport_t) );
  if ( pReportCmd != NULL ) {
    pReportCmd->numAttr = 1;
    pReportCmd->attrList[0].attrID = ATTRID_HUMIDITY_RELATIVE_MEASURE_VALUE;
    pReportCmd->attrList[0].dataType = ZCL_DATATYPE_UINT16;
    pReportCmd->attrList[0].attrData = (void *)(&humidity);

    zcl_SendReportCmd( reportEndpoint, &reportDstAddr,
                       ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY,
                       pReportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, TRUE, reportSeqNum++ );
    
    osal_mem_free( pReportCmd );
  }

}
