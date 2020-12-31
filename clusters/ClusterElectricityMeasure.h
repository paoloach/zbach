#ifndef __CLUSTER_ELECTRICITY_MEASURE__H__
#define __CLUSTER_ELECTRICITY_MEASURE__H__

#include "ClusterElectricityMeasure_constant.h"
#include <hal_types.h>
#include "zclReadAttributeFn.h"


void electricityMeasureClusterReadAttribute(zclAttrRec_t *);
void electricityMeasureClusterReadAttributeInit(void);
void electricityMeasureClusterSendReport(uint8 endpoint, afAddrType_t * dstAddr, uint8 * segNum);		

#endif
