
/**************************************************************************************************

 DESCRIPTION:
  --

 CREATED: 23/08/2016, by Paolo Achdjian

 FILE: zLightController.c

***************************************************************************************************/


/*********************************************************************
 * INCLUDES
 */
#include "ZComDef.h"
#include "OSAL.h"
#include "OSAL_Clock.h"
#include "hal_drivers.h"
#include "AF.h"
#include "nwk.h"
#include "ZDApp.h"
#include "DebugTrace.h"

#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ha.h"

#include "zLightController.h"

#include "onboard.h"

#include "clusters/ClusterIdentify.h"
#include "clusters/ClusterBasic.h"
#include "clusters/ClusterTemperatureMeasurement.h"
#include "clusters/ClusterPower.h"
#include "EventManager.h"


#ifdef DHT12
#include "clusters/ClusterHumidityRelativeMeasurement.h"
#endif
#include "ledBlink.h"
#ifdef DHT12
#include "dht112.h"
#endif
#ifdef DS18B20
#include "DS18B20.h"
#endif
	  

byte deviceTaskId;

extern SimpleDescriptionFormat_t temperatureSimpleDesc;

void User_Process_Pool(void);

// Functions to process ZCL Foundation incoming Command/Response messages 
static void processIncomingMsh( zclIncomingMsg_t *msg );
static uint8 processInReadRspCmd( zclIncomingMsg_t *pInMsg );
static uint8 processInWriteRspCmd( zclIncomingMsg_t *pInMsg );
static uint8 processInDefaultRspCmd( zclIncomingMsg_t *pInMsg );

			   
#ifdef ZCL_DISCOVER
static uint8 processInDiscRspCmd( zclIncomingMsg_t *pInMsg );
#endif
static ZStatus_t handleClusterCommands( zclIncoming_t *pInMsg );

static void initReport(void);
static void switch1Down(uint16 event);
static void switch1Up(uint16 event);


uint16 reportSecond = DEFAULT_REPORT_SEC;
uint16 reportSecondCounter;
uint8 reportSeqNum;
afAddrType_t reportDstAddr;
uint8  reportEndpoint;
uint8  connected=false;
uint32 switch1DownStart;

void zLightControllerInit( byte task_id ){
 	deviceTaskId = task_id;
   	zcl_registerPlugin( ZCL_CLUSTER_ID_GEN_BASIC,  ZCL_CLUSTER_ID_GEN_MULTISTATE_VALUE_BASIC,    handleClusterCommands );
  
	zclHA_Init( &temperatureSimpleDesc );
	addReadAttributeFn(ENDPOINT, ZCL_CLUSTER_ID_GEN_BASIC,basicClusterReadAttribute);
	addWriteAttributeFn(ENDPOINT, ZCL_CLUSTER_ID_GEN_BASIC,basicClusterWriteAttribute);
	addReadAttributeFn(ENDPOINT, ZCL_CLUSTER_ID_GEN_IDENTIFY,identifyClusterReadAttribute);
	addWriteAttributeFn(ENDPOINT, ZCL_CLUSTER_ID_GEN_IDENTIFY,identifyClusterWriteAttribute);
	addReadAttributeFn(ENDPOINT,ZCL_CLUSTER_ID_GEN_POWER_CFG,powerClusterReadAttribute);
	addReadAttributeFn(ENDPOINT,ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT,temperatureClusterReadAttribute);
#ifdef DHT12        
        addReadAttributeFn(ENDPOINT,ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY,humidityRelativeClusterReadAttribute);
#endif
  	zcl_registerForMsg( deviceTaskId );
        eventManagerInit();  
  	EA=1;
  	clusterTemperatureMeasurementeInit();
#ifdef DHT12        
        clusterHumidityMeasurementeInit();
#endif        
	powerClusterInit(deviceTaskId);
 	identifyInit(deviceTaskId);
	ZMacSetTransmitPower(TX_PWR_PLUS_19);
	//ZMacSetTransmitPower(POWER);
  blinkLedInit(deviceTaskId);
  blinkLedstart();
#ifdef DHT12
  dht112_init(deviceTaskId);
#endif
#ifdef DS18B20  
  DS18B20_init(deviceTaskId);
#endif
  setRegisteredKeysTaskID(deviceTaskId);
  addEventCB(SWITCH1_UP_EVT_BIT, switch1Up);
  addEventCB(SWITCH1_DOWN_EVT_BIT, switch1Down);
}

static void initReport(void){
  reportEndpoint = ENDPOINT;
  reportSecondCounter = reportSecond;
  reportDstAddr.addrMode = afAddr16Bit;
  reportDstAddr.endPoint = ENDPOINT;
  reportDstAddr.addr.shortAddr = 0;
  reportDstAddr.panId=_NIB.nodeDepth;
}


static void switch1Down(uint16 event){
  switch1DownStart = osal_getClock();
}

static void switch1Up(uint16 event){
  if (osal_getClock() -switch1DownStart >2){
    if (connected){
      ZDApp_LeaveReset(FALSE);
    }
  }
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
uint16 zLightControllerEventLoop( uint8 task_id, uint16 events ){
  if (handleEvent(&events)){
    return events;
  };
  
  
      afIncomingMSGPacket_t *MSGpkt;
	devStates_t zclSampleSw_NwkState;
  
	(void)task_id;  // Intentionally unreferenced parameter
	if ( events & SYS_EVENT_MSG ){
		while ( (MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( deviceTaskId )) )  {
			switch ( MSGpkt->hdr.event ) {
				case ZCL_INCOMING_MSG:
                                  // Incoming ZCL Foundation command/response messages
                                  processIncomingMsh( (zclIncomingMsg_t *)MSGpkt );
                                  break;
				case ZDO_STATE_CHANGE:
                                  zclSampleSw_NwkState = (devStates_t)(MSGpkt->hdr.status);
                                    
                                  switch(zclSampleSw_NwkState){
                                  case DEV_NWK_DISC:
                                    connected=false;
                                    setBlinkCounter(0);
                                    break;
                                  case DEV_NWK_JOINING:
                                    connected=false;
                                    setBlinkCounter(1);
                                    break;
                                  case DEV_NWK_REJOIN:
                                    connected=false;
                                    setBlinkCounter(2);
                                    break;
                                  case DEV_END_DEVICE_UNAUTH:
                                    connected=false;
                                    setBlinkCounter(3);
                                    break;
                                  case DEV_END_DEVICE:
                                    connected=true;
                                    blinkLedEnd();
                                    initReport();
                                    break;
                                  case DEV_ROUTER:
                                    connected=true;
                                    setBlinkCounter(5);
                                    break;
                                  case DEV_COORD_STARTING:
                                    connected=false;
                                    setBlinkCounter(6);
                                    break;
                                  case DEV_ZB_COORD:
                                    connected=true;
                                    setBlinkCounter(7);
                                    break;
                                  case DEV_NWK_ORPHAN:
                                    connected=false;
                                    setBlinkCounter(8);
                                    break;
                                  }
                                  break;
                                default:
                                  break;
      		}

        osal_msg_deallocate( (uint8 *)MSGpkt );
    	}

    	return (events ^ SYS_EVENT_MSG);
	}
	
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


void User_Process_Pool(void){
}


/****************************************************************************
****************************************************************************/


