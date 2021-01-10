/**************************************************************************************************
  Filename:       ClusterIdentify.cpp

  Autorh:  Paolo Achdjia
  Created: 28/10/2014

**************************************************************************************************/

#include "OSAL.h"
#include "OSAL_PwrMgr.h"
#include "hal_led.h"

#include "ClusterIdentify.h"
#include "zcl_general.h"
#include "ClusterOSALEvents.h"
#include "regs.h"
#include "EventManager.h"

#define ON_TIME 600
#define OFF_TIME 400

static uint16 identifyTime=0;

static byte mainAppTaskId;
static void identifyLoop(uint16 events);


#define OFF 0
#define  ON 1

static uint8  onOff=OFF;


void identifyClusterReadAttribute(zclAttrRec_t * attribute){
	if (attribute == NULL){
		return;
	}
	
	if (attribute->attrId == ATTRID_IDENTIFY_TIME){
		attribute->dataType = ZCL_DATATYPE_UINT16;
		attribute->dataPtr = (void *)&identifyTime;
		attribute->status = ZCL_STATUS_SUCCESS;
	} else {
		attribute->status = ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
	}
}
void identifyClusterWriteAttribute(ZclWriteAttribute_t * writeAttribute){
	if (writeAttribute == NULL){
		return;
	}
	
	if(writeAttribute->attrId == ATTRID_IDENTIFY_TIME){
		identifyTime = *(uint16 *)writeAttribute;
		writeAttribute->status=ZCL_STATUS_SUCCESS;
	} else {
		writeAttribute->status = ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
	}
}

void identifyInit(byte taskId){
  DIR(IDENTIFY_PORT, IDENTIFY_PIN)=1;
  FUNCTION_SEL(IDENTIFY_PORT,   IDENTIFY_PIN)=0;
  PORT(IDENTIFY_PORT, IDENTIFY_PIN)=0;
  mainAppTaskId = taskId;
  
  addEventCB(IDENTIFY_TIMEOUT_EVT_BIT, &identifyLoop);
}

void identifyLoop(uint16 events){
  if (onOff==ON){
          osal_start_timerEx( mainAppTaskId, IDENTIFY_TIMEOUT_EVT_BIT, OFF_TIME );
          onOff=OFF;
          PORT(IDENTIFY_PORT, IDENTIFY_PIN) = 1;
  } else if (onOff==OFF ){
          onOff=ON;
          if ( identifyTime > 0 ){
          identifyTime--;
          }
  if (identifyTime>0){
                  osal_start_timerEx( mainAppTaskId, IDENTIFY_TIMEOUT_EVT_BIT, ON_TIME );
                  osal_pwrmgr_task_state(mainAppTaskId, PWRMGR_HOLD);
                  PORT(IDENTIFY_PORT, IDENTIFY_PIN) = 0;
          } else{
                  osal_pwrmgr_task_state(mainAppTaskId, PWRMGR_CONSERVE);
                  PORT(IDENTIFY_PORT, IDENTIFY_PIN) = 0;
          }
  } 
}




void processIdentifyTimeChange( void ){
	if ( identifyTime > 0 ) {
		osal_start_timerEx( mainAppTaskId, IDENTIFY_TIMEOUT_EVT_BIT, ON_TIME );
		onOff=ON;
		PORT(IDENTIFY_PORT, IDENTIFY_PIN) = 1;
		osal_pwrmgr_task_state(mainAppTaskId, PWRMGR_HOLD);
	}  else {
		osal_stop_timerEx( mainAppTaskId, IDENTIFY_TIMEOUT_EVT_BIT );
		osal_pwrmgr_task_state(mainAppTaskId, PWRMGR_CONSERVE);
	}
}

ZStatus_t processIdentifyClusterServerCommands( zclIncoming_t *pInMsg ){

	switch(pInMsg->hdr.commandID){
	case COMMAND_IDENTIFY:
		identifyTime = BUILD_UINT16( pInMsg->pData[0], pInMsg->pData[1] );
		processIdentifyTimeChange();
		return ZSuccess;
	case COMMAND_IDENTIFY_QUERY:
		if ( identifyTime > 0 ) {
			zcl_SendCommand( pInMsg->msg->endPoint, &pInMsg->msg->srcAddr, ZCL_CLUSTER_ID_GEN_IDENTIFY,
                         COMMAND_IDENTIFY_QUERY_RSP, TRUE,
                         ZCL_FRAME_SERVER_CLIENT_DIR, TRUE, 0, pInMsg->hdr.transSeqNum, 2, (uint8 *) &identifyTime );
			return  ZCL_STATUS_CMD_HAS_RSP;
	    } else {
			return ZSuccess;
		}
	default:
    	return ZFailure;   // Error ignore the command
	}
}

ZStatus_t processIdentifyClusterClientCommands( zclIncoming_t *pInMsg ){
	if ( pInMsg->hdr.commandID != COMMAND_IDENTIFY_QUERY_RSP )
		return ZFailure;

	return ZFailure;
}

