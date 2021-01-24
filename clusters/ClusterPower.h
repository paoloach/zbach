/**************************************************************************************************
  Filename:       ClusterPower.h

  Autorh:  Paolo Achdjian
  Created: 12/11/2014

**************************************************************************************************/

#ifndef __CLUSTER_POWER__H__
#define __CLUSTER_POWER__H__

#include "zclReadAttributeFn.h"
	
void powerClusterReadAttribute(zclAttrRec_t *);
void powerClusterInit(byte appId);
void powerClusterCheckBattery(void);

extern const uint8 batterySize;
extern const uint8 batteryQuantity;

#endif