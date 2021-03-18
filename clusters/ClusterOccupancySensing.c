
#include "BindingTable.h"
#include "AddrMgr.h"
#include "ClusterOccupancySensing.h"

#include "regs.h"
#include "report.h"

static uint8 occupancy=0;
static uint8 type=OCCUPANCY_TYPE_PIR;
static zclReportCmd1_t reportCmd;
extern uint8 connected;

static void sendBindMessage(void);
static void occupancySensingClusterSendReport(void); 

void occupancySensingReadAttribute(zclAttrRec_t * statusRec) {
  if (statusRec == NULL){
    return;
  }
  statusRec->accessControl = ACCESS_CONTROL_READ;
  statusRec->status=ZCL_STATUS_SUCCESS;
  switch(statusRec->attrId){
  case ATTRID_OCCUPANCY_OCCUPANCY:
    statusRec->dataType = ZCL_DATATYPE_BITMAP8;
    statusRec->dataPtr = (void *)&occupancy;
    break;
   case ATTRID_OCCUPANCY_SENSOR_TYPE:
    statusRec->dataType = ZCL_DATATYPE_ENUM8;
    statusRec->dataPtr = (void *)&type;
    break;   
  default:
    statusRec->status = ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;  
  }
  
}


void occupancySensingClusterSendReport(void){
  if (connected){
        zcl_SendReportCmd( reportEndpoint, &reportDstAddr,
                       ZCL_CLUSTER_ID_MS_OCCUPANCY_SENSING,
                       (zclReportCmd_t *)&reportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, TRUE, reportSeqNum++ );
      
  }

}

void static sendBindMessage(){
  BindingEntry_t * bindEntry;
  uint8 index=0;
  while(index < 10) {
    bindEntry = bindFind(reportEndpoint, ZCL_CLUSTER_ID_GEN_ON_OFF,index);
    if (bindEntry == NULL){
      break;
    }
    AddrMgrEntry_t entry;
    entry.user = ADDRMGR_USER_BINDING;
    entry.index = bindEntry->dstIdx;
    uint8 stat = AddrMgrEntryGet( &entry );
    if (stat){
      afAddrType_t onOffSendaddr;
      onOffSendaddr.addrMode = afAddr16Bit;
      onOffSendaddr.addr.shortAddr = entry.nwkAddr;
      onOffSendaddr.endPoint=bindEntry->dstEP;
      uint8 cmd;
      if (occupancy){
        cmd = COMMAND_ON;
      } else {
        cmd = COMMAND_OFF;
      }
      zcl_SendCommand(reportEndpoint, &onOffSendaddr, ZCL_CLUSTER_ID_GEN_ON_OFF,cmd, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR, TRUE, 0, 0, 0,NULL);
    }
    index++;
  }
}

void clusterOccupancyInit(void) {
  DIR(PIR_PORT, PIR_PIN)=0;
  FUNCTION_SEL(PIR_PORT, PIR_PIN)=0;
  reportCmd.numAttr = 1;
  reportCmd.attrList[0].attrID = ATTRID_OCCUPANCY_OCCUPANCY;
  reportCmd.attrList[0].dataType = ZCL_DATATYPE_BITMAP8;
  reportCmd.attrList[0].attrData = (void *)(&occupancy);
  
  osal_start_reload_timer_cb((uint32)DEFAULT_REPORT_SEC*1000, &occupancySensingClusterSendReport );  
}

void clusterOccupancySensingLoop() {
  uint8 oldOccupancy = occupancy;
  if (PORT(PIR_PORT, PIR_PIN)==0){
    occupancy=0;
  } else {
    occupancy=1;
  }
  if (oldOccupancy != occupancy){
    occupancySensingClusterSendReport();
    sendBindMessage();
  }
}