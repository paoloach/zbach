	/**************************************************************************************************
  Filename:       ClusterOnOff.c

  Autorh:  Paolo Achdjia
  Created: 27/10/2014

**************************************************************************************************/

#include "zcl_general.h"

#include "ClusterOnOff.h"
#include "ClusterOSALEvents.h"
#include "onboard.h"
#include "regs.h"    
#include "report.h"

#define MAX_TIMEOUT_SEC 30
#define REPORT_POLL_TIME_SEC 300

extern uint8 connected;

static uint8  onOffValue = LIGHT_ON;
static uint16 onTime=0;
static zclReportCmd2_t reportCmd;



static void setIOStatus(void);
void onOffClusterSendReport(void);


void onOffInit(void) {
  DIR(ON_OFF_PORT, ON_OFF_PIN)=1;
  FUNCTION_SEL(ON_OFF_PORT,   ON_OFF_PIN)=0;
  PORT(ON_OFF_PORT, ON_OFF_PIN)=0;
  osal_start_reload_timer_cb(REPORT_POLL_TIME_SEC*1000, &onOffClusterSendReport );
  addEventCB(NEW_POWER_BIT, &onOffClusterSendReport);
}

void onOffClusterReadAttribute(zclAttrRec_t * attribute) {
	if (attribute == NULL){
		return;
	}
	attribute->accessControl = ACCESS_CONTROL_R_W;
	attribute->status =  ZCL_STATUS_SUCCESS;
	switch(attribute->attrId){
	case ATTRID_ON_OFF:
		attribute->dataType = ZCL_DATATYPE_BOOLEAN;
		attribute->dataPtr = (void *)&onOffValue;
		break;
        case ATTRID_ON_OFF_ON_TIME:
                attribute->dataType = ZCL_DATATYPE_UINT16;
		attribute->dataPtr = (void *)&onTime;
		break;
	default:
		attribute->status = ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
	}
}

void onOffClusterWriteAttribute(ZclWriteAttribute_t * writeAttribute) {
	if (writeAttribute == NULL){
		return;
	}
	writeAttribute->status=ZCL_STATUS_SUCCESS;
	switch(writeAttribute->attrId){
		case ATTRID_ON_OFF:
			if (writeAttribute->dataType == ZCL_DATATYPE_BOOLEAN){
				onOffValue = *(uint8 *)writeAttribute->dataPtr;
				setIOStatus();
			}else
				writeAttribute->status = ZCL_STATUS_INVALID_DATA_TYPE;
                case ATTRID_ON_OFF_ON_TIME:
                        if (writeAttribute->dataType == ZCL_DATATYPE_UINT16){
				onTime = *(uint16 *)writeAttribute->dataPtr;
			}else
				writeAttribute->status = ZCL_STATUS_INVALID_DATA_TYPE;        
		break;
	}
}

void setIOStatus(void){
  if ( onOffValue  == LIGHT_ON ){
      PORT(ON_OFF_PORT, ON_OFF_PIN)=0;
      PORT(LED_BLINK_PORT, LED_BLINK_PIN)=0;
  } else {
      PORT(ON_OFF_PORT, ON_OFF_PIN)=1;
      PORT(LED_BLINK_PORT, LED_BLINK_PIN)=1;
  }
  
  if (connected ){
    onOffClusterSendReport();
  }
}

void setLightStatus(uint8 status){
	onOffValue = status;
	setIOStatus();
}



ZStatus_t processOnOffClusterServerCommands(zclIncoming_t *pInMsg) {
	switch(pInMsg->hdr.commandID){
	case COMMAND_ON:
		onOffValue = LIGHT_ON;
		break;
	case COMMAND_OFF:
		onOffValue = LIGHT_OFF;
		break;
	case COMMAND_TOGGLE:
		if ( onOffValue == LIGHT_OFF )
      		onOffValue = LIGHT_ON;
    	else
      		onOffValue = LIGHT_OFF;
		break;
	default:
		return ZFailure;
	}
	setIOStatus();
	return ZSuccess;
}


void onOffClusterSendReport(void) {
  if (connected){
      reportCmd.numAttr = 1;
      reportCmd.attrList[0].attrID = ATTRID_ON_OFF;
      reportCmd.attrList[0].dataType = ZCL_DATATYPE_BOOLEAN;
      reportCmd.attrList[0].attrData = (void *)&onOffValue;

      zcl_SendReportCmd( reportEndpoint, &reportDstAddr,
                       ZCL_CLUSTER_ID_GEN_ON_OFF,
                       (zclReportCmd_t *)&reportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, TRUE, reportSeqNum++ );
  }

}
