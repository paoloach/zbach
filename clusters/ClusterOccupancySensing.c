
#include "BindingTable.h"
#include "AddrMgr.h"
#include "ClusterOccupancySensing.h"

#include "regs.h"

static uint8 occupancy=0;
static uint8 type=OCCUPANCY_TYPE_PIR;
extern uint8 connected;

static void sendBindMessage(uint8 endpoint);

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

static zclReportCmd1_t reportCmd;

void occupancySensingClusterSendReport(uint8 endpoint, afAddrType_t * dstAddr, uint8 * segNum){
  if (connected){
      reportCmd.numAttr = 1;
      reportCmd.attrList[0].attrID = ATTRID_OCCUPANCY_OCCUPANCY;
      reportCmd.attrList[0].dataType = ZCL_DATATYPE_BITMAP8;
      reportCmd.attrList[0].attrData = (void *)(&occupancy);

      zcl_SendReportCmd( endpoint, dstAddr,
                         ZCL_CLUSTER_ID_MS_OCCUPANCY_SENSING,
                         (zclReportCmd_t *)&reportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, FALSE, (*segNum)++ );
      
  }

}

void static sendBindMessage(uint8 endpoint){
  BindingEntry_t * bindEntry;
  uint8 index=0;
  while(index < 10) {
    bindEntry = bindFind(endpoint, ZCL_CLUSTER_ID_GEN_ON_OFF,index);
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
      zcl_SendCommand(endpoint, &onOffSendaddr, ZCL_CLUSTER_ID_GEN_ON_OFF,cmd, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR, TRUE, 0, 0, 0,NULL);
    }
    index++;
  }
}

void clusterOccupancyInit(void) {
  P1SEL &=0xFB;
  DIR1_2 = 0;
}

void clusterOccupancySensingLoop(uint8 endpoint, afAddrType_t * dstAddr, uint8 * segNum) {
  uint8 oldOccupancy = occupancy;
  if (P1_2==0){
    occupancy=0;
  } else {
    occupancy=1;
  }
  if (oldOccupancy != occupancy){
    occupancySensingClusterSendReport(endpoint, dstAddr, segNum);
    sendBindMessage(endpoint);
  }
}