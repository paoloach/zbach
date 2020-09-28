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


const int16 minHumidityValue=0;
const int16 maxHumidityValue=80;
const uint16 toleranceHumidity=10;

extern byte temperatureSensorTaskID;


void clusterHumidityMeasurementeInit(void) {
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

