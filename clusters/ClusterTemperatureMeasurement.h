/**************************************************************************************************

 DESCRIPTION:
  Temperature Measuremente Cluster

 CREATED: 12/11/2014, by Paolo Achdjian

 FILE: ClusterTemperatureMeasurement.h

***************************************************************************************************/

#ifndef __CLUSTER_TEMPERATURE_MEASUREMENT__H__
#define __CLUSTER_TEMPERATURE_MEASUREMENT__H__

#include "zcl_general.h"
#include "zcl.h"
#include "ClusterOSALEvents.h"

extern int16 minTemperatureValue;
extern int16 maxTemperatureValue;
extern uint16 toleranceTemperature;

uint16 readTemperatureLoop(uint16 events);
void readTemperature(void);
void clusterTemperatureMeasurementeInit(void);
void temperatureClusterReadAttribute(zclAttrRec_t *);
#if !defined RTR_NWK
void temperatureClusterSendReport(uint8 endpoint, afAddrType_t * dstAddr, uint8 * segNum); 
#endif

#endif