/**************************************************************************************************
  Filename:       ClusterBasic.h

  Autorh:  Paolo Achdjia
  Created: 28/10/2014

**************************************************************************************************/

#ifndef __CLUSTER_BASIC__H__
#define __CLUSTER_BASIC__H__

#include "zclReadAttributeFn.h"
#include "zclWriteAttributeFn.h"
#include "zcl.h"
#include "osal.h"
#include "zcl_general.h"


void basicResetCB( void );
void basicClusterReadAttribute(zclAttrRec_t *);
void basicClusterWriteAttribute(ZclWriteAttribute_t *);
ZStatus_t processBasicClusterCommands( zclIncoming_t *pInMsg );

extern const uint8 HWRevision;
extern const uint8 ZCLVersion;
extern const uint8 manufacturerName[];
extern const uint8 modelId[];
extern const uint8 dateCode[];
extern const uint8 powerSource;
#endif