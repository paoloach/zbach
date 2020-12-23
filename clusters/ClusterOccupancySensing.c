

#include "ClusterOccupancySensing.h"
#include "regs.h"

static uint8 occupancy=0;
static uint8 type=OCCUPANCY_TYPE_PIR;
extern uint8 connected;


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


void occupancySensingClusterSendReport(uint8 endpoint, afAddrType_t * dstAddr, uint8 * segNum){
  if (connected){
    zclReportCmd_t * pReportCmd = osal_mem_alloc( sizeof(zclReportCmd_t) + sizeof(zclReport_t) );
    if ( pReportCmd != NULL ) {
      pReportCmd->numAttr = 1;
      pReportCmd->attrList[0].attrID = ATTRID_OCCUPANCY_OCCUPANCY;
      pReportCmd->attrList[0].dataType = ZCL_DATATYPE_BITMAP8;
      pReportCmd->attrList[0].attrData = (void *)(&occupancy);

      zcl_SendReportCmd( endpoint, dstAddr,
                         ZCL_CLUSTER_ID_MS_OCCUPANCY_SENSING,
                         pReportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, TRUE, (*segNum)++ );
      
      osal_mem_free( pReportCmd );
    }
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
  }
}