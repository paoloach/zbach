#ifndef _CLUSTER_DIAGNOSTIC__H__
#define _CLUSTER_DIAGNOSTIC__H__


#include "zclReadAttributeFn.h"

void clusterDiagnosticReadAttribute(zclAttrRec_t *);
void clusterDiagnosticInit(byte appId);

#endif
