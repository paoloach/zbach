
/**************************************************************************************************

 DESCRIPTION:
  --

 CREATED: 23/08/2016, by Paolo Achdjian

 FILE: TemperatureSensor.c

***************************************************************************************************/


/*********************************************************************
 * INCLUDES
 */
#include "ZComDef.h"
#include "OSAL.h"
#include "AF.h"
#include "ZDApp.h"
#include "DebugTrace.h"

#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ha.h"

#include "TemperatureSensor.h"

#include "onboard.h"

/* HAL */
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"

#include "clusters/ClusterIdentify.h"
#include "clusters/ClusterBasic.h"
#include "clusters/ClusterTemperatureMeasurement.h"
#include "clusters/ClusterPower.h"

#define FAST_BLINK_TIME_ON 100	 
#define FAST_BLINK_TIME_OFF 1000	 
	 
byte temperatureSensorTaskID;
extern SimpleDescriptionFormat_t temperatureSimpleDesc;
	
// Functions to process ZCL Foundation incoming Command/Response messages 
static void processIncomingMsh( zclIncomingMsg_t *msg );
static uint8 processInReadRspCmd( zclIncomingMsg_t *pInMsg );
static uint8 processInWriteRspCmd( zclIncomingMsg_t *pInMsg );
static uint8 processInDefaultRspCmd( zclIncomingMsg_t *pInMsg );
static void fastBlinkOn(void);
			   
static void fastBlinkOff(void);
#ifdef ZCL_DISCOVER
static uint8 processInDiscRspCmd( zclIncomingMsg_t *pInMsg );
#endif
static ZStatus_t handleClusterCommands( zclIncoming_t *pInMsg );

__sfr __no_init volatile struct  {
	unsigned char DIR0_0: 1;
	unsigned char DIR0_1: 1;
	unsigned char DIR0_2: 1;
	unsigned char DIR0_3: 1;
	unsigned char DIR0_4: 1;
	unsigned char DIR0_5: 1;
	unsigned char DIR0_6: 1;
	unsigned char DIR0_7: 1;
} @ 0xFD;

__sfr __no_init volatile struct  {
	unsigned char P0SEL_0: 1;
	unsigned char P0SEL_1: 1;
	unsigned char P0SEL_2: 1;
	unsigned char P0SEL_3: 1;
	unsigned char P0SEL_4: 1;
	unsigned char P0SEL_5: 1;
	unsigned char P0SEL_6: 1;
	unsigned char P0SEL_7: 1;
} @ 0xF3;


void temperatureSensorInit( byte task_id ){
 	temperatureSensorTaskID = task_id;

   	zcl_registerPlugin( ZCL_CLUSTER_ID_GEN_BASIC,  ZCL_CLUSTER_ID_GEN_MULTISTATE_VALUE_BASIC,    handleClusterCommands );
  
	zclHA_Init( &temperatureSimpleDesc );
	addReadAttributeFn(ENDPOINT, ZCL_CLUSTER_ID_GEN_BASIC,basicClusterReadAttribute);
	addWriteAttributeFn(ENDPOINT, ZCL_CLUSTER_ID_GEN_BASIC,basicClusterWriteAttribute);
	addReadAttributeFn(ENDPOINT, ZCL_CLUSTER_ID_GEN_IDENTIFY,identifyClusterReadAttribute);
	addWriteAttributeFn(ENDPOINT, ZCL_CLUSTER_ID_GEN_IDENTIFY,identifyClusterWriteAttribute);
	addReadAttributeFn(ENDPOINT,ZCL_CLUSTER_ID_GEN_POWER_CFG,powerClusterReadAttribute);
	addReadAttributeFn(ENDPOINT,ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT,temperatureClusterReadAttribute);

  	zcl_registerForMsg( temperatureSensorTaskID );
  
  	EA=1;
  	clusterTemperatureMeasurementeInit();
	powerClusterInit(temperatureSensorTaskID);
 	identifyInit(temperatureSensorTaskID);
	ZMacSetTransmitPower(TX_PWR_PLUS_19);
	//ZMacSetTransmitPower(POWER);
	DIR0_1 = 1;
 	P0SEL_1 = 0;
 	P0_1 = 0;
	fastBlinkOn();
}

static void fastBlinkOn(void){
	osal_start_timerEx( temperatureSensorTaskID, FAST_BLINK, FAST_BLINK_TIME_ON );
	P0_1 = 1;
}
			   
static void fastBlinkOff(void){
	osal_stop_timerEx( temperatureSensorTaskID, FAST_BLINK );
	P0_1 = 0;
}

/*********************************************************************
 * @fn          zclSample_event_loop
 *
 * @brief       Event Loop Processor for zclGeneral.
 *
 * @param       none
 *
 * @return      none
 */
uint16 temperatureSensorEventLoop( uint8 task_id, uint16 events ){
	afIncomingMSGPacket_t *MSGpkt;
	devStates_t zclSampleSw_NwkState;
  
	(void)task_id;  // Intentionally unreferenced parameter
	if ( events & SYS_EVENT_MSG ){
		while ( (MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( temperatureSensorTaskID )) )  {
			switch ( MSGpkt->hdr.event ) {
				case ZCL_INCOMING_MSG:
          			// Incoming ZCL Foundation command/response messages
          			processIncomingMsh( (zclIncomingMsg_t *)MSGpkt );
          			break;
				case ZDO_STATE_CHANGE:
          			zclSampleSw_NwkState = (devStates_t)(MSGpkt->hdr.status);
					if (zclSampleSw_NwkState == DEV_NWK_DISC)
						break;
					if (zclSampleSw_NwkState == DEV_END_DEVICE || zclSampleSw_NwkState == DEV_ROUTER){
						fastBlinkOff();
					}
					break;
		       default:
        		  break;
      		}

        osal_msg_deallocate( (uint8 *)MSGpkt );
    	}

    	return (events ^ SYS_EVENT_MSG);
	}
	
	if ( events & IDENTIFY_TIMEOUT_EVT ) {
		return identifyLoop(events);
	}
	
	if ( events & FAST_BLINK ) {
		if (P0_1){
			P0_1 = 0;
			osal_start_timerEx( temperatureSensorTaskID, FAST_BLINK, FAST_BLINK_TIME_OFF );
		}else{
			P0_1 = 1;
			osal_start_timerEx( temperatureSensorTaskID, FAST_BLINK, FAST_BLINK_TIME_ON );
		}
		
		return events ^ FAST_BLINK;
	}

#if !defined RTR_NWK
	if ( events & READ_BATTERY_LEVEL ) {
		powerClusterCheckBattery(task_id);
		events = events ^ READ_BATTERY_LEVEL;
	}
#endif	
	
//	if ( events & READ_TEMP_MASK ) {
//		return readTemperatureLoop(events);
//	}

 	return 0;
}

/*********************************************************************
 * @fn      processIncomingMsh
 *
 * @brief   Process ZCL Foundation incoming message
 *
 * @param   pInMsg - pointer to the received message
 *
 * @return  none
 */
static void processIncomingMsh( zclIncomingMsg_t *pInMsg)
{
  switch ( pInMsg->zclHdr.commandID )
  {
    case ZCL_CMD_READ_RSP:
      processInReadRspCmd( pInMsg );
      break;
    case ZCL_CMD_WRITE_RSP:
      processInWriteRspCmd( pInMsg );
      break;
#ifdef ZCL_REPORT
    // See ZCL Test Applicaiton (zcl_testapp.c) for sample code on Attribute Reporting
    case ZCL_CMD_CONFIG_REPORT:
      //zclSampleLight_ProcessInConfigReportCmd( pInMsg );
      break;
    
    case ZCL_CMD_CONFIG_REPORT_RSP:
      //zclSampleLight_ProcessInConfigReportRspCmd( pInMsg );
      break;
    
    case ZCL_CMD_READ_REPORT_CFG:
      //zclSampleLight_ProcessInReadReportCfgCmd( pInMsg );
      break;
    
    case ZCL_CMD_READ_REPORT_CFG_RSP:
      //zclSampleLight_ProcessInReadReportCfgRspCmd( pInMsg );
      break;
    
    case ZCL_CMD_REPORT:
      //zclSampleLight_ProcessInReportCmd( pInMsg );
      break;
#endif   
    case ZCL_CMD_DEFAULT_RSP:
      processInDefaultRspCmd( pInMsg );
      break;
#ifdef ZCL_DISCOVER     
    case ZCL_CMD_DISCOVER_RSP:
      processInDiscRspCmd( pInMsg );
      break;
#endif  
    default:
      break;
  }
  
  if ( pInMsg->attrCmd )
    osal_mem_free( pInMsg->attrCmd );
}

/*********************************************************************
 * @fn      processInReadRspCmd
 *
 * @brief   Process the "Profile" Read Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 processInReadRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclReadRspCmd_t *readRspCmd;
  uint8 i;

  readRspCmd = (zclReadRspCmd_t *)pInMsg->attrCmd;
  for (i = 0; i < readRspCmd->numAttr; i++)
  {
    // Notify the originator of the results of the original read attributes 
    // attempt and, for each successfull request, the value of the requested 
    // attribute
  }

  return TRUE; 
}

/*********************************************************************
 * @fn      processInWriteRspCmd
 *
 * @brief   Process the "Profile" Write Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 processInWriteRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclWriteRspCmd_t *writeRspCmd;
  uint8 i;

  writeRspCmd = (zclWriteRspCmd_t *)pInMsg->attrCmd;
  for (i = 0; i < writeRspCmd->numAttr; i++)
  {
    // Notify the device of the results of the its original write attributes
    // command.
  }

  return TRUE; 
}

/*********************************************************************
 * @fn      processInDefaultRspCmd
 *
 * @brief   Process the "Profile" Default Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 processInDefaultRspCmd( zclIncomingMsg_t *pInMsg )
{
  // zclDefaultRspCmd_t *defaultRspCmd = (zclDefaultRspCmd_t *)pInMsg->attrCmd;
   
  // Device is notified of the Default Response command.
  (void)pInMsg;
  
  return TRUE; 
}

#ifdef ZCL_DISCOVER
/*********************************************************************
 * @fn      processInDiscRspCmd
 *
 * @brief   Process the "Profile" Discover Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 processInDiscRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclDiscoverRspCmd_t *discoverRspCmd;
  uint8 i;
  
  discoverRspCmd = (zclDiscoverRspCmd_t *)pInMsg->attrCmd;
  for ( i = 0; i < discoverRspCmd->numAttr; i++ )
  {
    // Device is notified of the result of its attribute discovery command.
  }
  
  return TRUE;
}
#endif // ZCL_DISCOVER

/*********************************************************************
 * @brief      dispatch the cluster command to the right callback function
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static ZStatus_t handleClusterCommands( zclIncoming_t *pInMsg ){
	ZStatus_t stat = ZFailure;

	if (zcl_ServerCmd( pInMsg->hdr.fc.direction ) ) {
		switch ( pInMsg->msg->clusterId ){
		    case ZCL_CLUSTER_ID_GEN_BASIC:
    		return processBasicClusterCommands(pInMsg);
	    case ZCL_CLUSTER_ID_GEN_IDENTIFY:
			return processIdentifyClusterServerCommands( pInMsg );
	    case ZCL_CLUSTER_ID_GEN_GROUPS:
    	case ZCL_CLUSTER_ID_GEN_SCENES:
	    case ZCL_CLUSTER_ID_GEN_ON_OFF:
    	case ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL:
	    case ZCL_CLUSTER_ID_GEN_ALARMS:
    	case ZCL_CLUSTER_ID_GEN_LOCATION:
	    case ZCL_CLUSTER_ID_GEN_POWER_CFG:
    	case ZCL_CLUSTER_ID_GEN_DEVICE_TEMP_CONFIG:
	    case ZCL_CLUSTER_ID_GEN_ON_OFF_SWITCH_CONFIG:
    	case ZCL_CLUSTER_ID_GEN_TIME:
	    default:
    	  stat = ZFailure;
	      break;
  		}
	} else {
		switch ( pInMsg->msg->clusterId ){
	
  	    case ZCL_CLUSTER_ID_GEN_IDENTIFY:
			return processIdentifyClusterClientCommands( pInMsg );
	    default:
    	  stat = ZFailure;
	      break;
  		}
	}

  return ( stat );
}



/****************************************************************************
****************************************************************************/


