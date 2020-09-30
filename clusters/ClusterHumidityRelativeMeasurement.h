/**************************************************************************************************

 DESCRIPTION:
  Temperature Measuremente Cluster

 CREATED: 12/11/2014, by Paolo Achdjian

 FILE: ClusterTemperatureMeasurement.h

***************************************************************************************************/

#ifndef __CLUSTER_HUMIDITY_RELATIVE_MEASUREMENT__H__
#define __CLUSTER_HUMIDITY_RELATIVE_MEASUREMENT__H__

#include "zcl_general.h"
#include "zcl.h"
#include "ClusterOSALEvents.h"


#define ATTRID_HUMIDITY_RELATIVE_MEASURE_VALUE		0
#define ATTRID_HUMIDITY_RELATIVE_MIN_MEASURE_VALUE	1
#define ATTRID_HUMIDITY_RELATIVE_MAX_MEASURE_VALUE	2
#define ATTRID_HUMIDITY_RELATIVE_TOLERANCE		3

uint16 readHumidityLoop(uint16 events);
void clusterHumidityMeasurementeInit(void);
void humidityRelativeClusterReadAttribute(zclAttrRec_t *);
void humidityRelativeClusterSendReport(uint8 endpoint, afAddrType_t * dstAddr, uint8 * segNum); 


#endif