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

void clusterHumidityMeasurementeInit(void);
void humidityRelativeClusterReadAttribute(zclAttrRec_t *);


#endif