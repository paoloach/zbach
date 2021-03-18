
#include "zcl.h"
#include "zcl_general.h"
#include "ClusterOSALEvents.h"
#include "report.h"
#include "OSAL_Memory.h"

#include "ClusterDiagnostic.h"

#define MAX_TIMEOUT_SEC 30
#define REPORT_TIME 300

static uint16 remainingSec=REPORT_TIME;
static void setTimer(void);
static void eventReport(void);
static void sendReport(void);
static zclReportCmd2_t reportCmd;

void clusterDiagnosticReadAttribute(zclAttrRec_t *attribute) {
  
} 
  
void clusterDiagnosticInit(byte appId) {
  setTimer();

}


static void setTimer(void) {
  return;
  uint16 nextReportEventSec;
  if (remainingSec==0){
    remainingSec = REPORT_TIME;
  }
  if (remainingSec > MAX_TIMEOUT_SEC){
    nextReportEventSec = MAX_TIMEOUT_SEC;
  } else {
    nextReportEventSec = remainingSec;
  }
  remainingSec -= nextReportEventSec;
  osal_start_timerEx_cb(nextReportEventSec*1000, &eventReport );
}

static void eventReport(void) {
  if (remainingSec == 0){
    sendReport();
  }
  setTimer();
}


void sendReport(){
  
  uint16 memoryUsed = osal_heap_mem_used();
  uint16 heapBlockFree = osal_heap_block_free();  
  
  reportCmd.numAttr = 2;
  reportCmd.attrList[0].attrID = ATTRID_DIAGNOSTIC_NUMBER_OF_RESET;
  reportCmd.attrList[0].dataType = ZCL_DATATYPE_UINT16;
  reportCmd.attrList[0].attrData = (void *)(&memoryUsed);
  reportCmd.attrList[1].attrID = ATTRID_DIAGNOSTIC_PERSISTEN_MEMORY_WRITE;
  reportCmd.attrList[1].dataType = ZCL_DATATYPE_UINT16;
  reportCmd.attrList[1].attrData = (void *)(&heapBlockFree);    

  zcl_SendReportCmd( reportEndpoint, &reportDstAddr,
                     ZCL_CLUSTER_ID_HA_DIAGNOSTIC,
                     (zclReportCmd_t *)(&reportCmd),
                     ZCL_FRAME_SERVER_CLIENT_DIR, 
                     TRUE, 
                     reportSeqNum++ );
 
 
}
