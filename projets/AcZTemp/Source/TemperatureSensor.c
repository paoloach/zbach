
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
#include "nwk.h"
#include "ZDApp.h"
#include "DebugTrace.h"

#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ha.h"

#include "TemperatureSensor.h"

#include "onboard.h"

#include "clusters/ClusterIdentify.h"
#include "clusters/ClusterBasic.h"
#include "clusters/ClusterTemperatureMeasurement.h"
#include "clusters/ClusterPower.h"
#include "EventManager.h"
#include "Report.h"

#ifdef DHT12
#include "clusters/ClusterHumidityRelativeMeasurement.h"
#endif
#include "ledBlink.h"
#include "dht112.h"

#ifdef DISPLAY
#include "lcd.h"
#endif
	  

void User_Process_Pool(void);
	 
byte temperatureSensorTaskID;
extern SimpleDescriptionFormat_t temperatureSimpleDesc;
	
// Functions to process ZCL Foundation incoming Command/Response messages 
static void processIncomingMsh( zclIncomingMsg_t *msg );
static uint8 processInReadRspCmd( zclIncomingMsg_t *pInMsg );
static uint8 processInWriteRspCmd( zclIncomingMsg_t *pInMsg );
static uint8 processInDefaultRspCmd( zclIncomingMsg_t *pInMsg );
static void sysEvent(uint16 events);
static void ZDOStateChange(devStates_t newState);

#ifdef ZCL_DISCOVER
static uint8 processInDiscRspCmd( zclIncomingMsg_t *pInMsg );
#endif
static ZStatus_t handleClusterCommands( zclIncoming_t *pInMsg );

static void initReport(void);
uint16 reportSecond = DEFAULT_REPORT_SEC;
uint16 reportSecondCounter;
uint8 reportSeqNum;
afAddrType_t reportDstAddr;
uint8  reportEndpoint;
devStates_t prevState;


void temperatureSensorInit( byte task_id ){
    temperatureSensorTaskID = task_id;
  eventManagerInit();
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
    zcl_registerForMsg( temperatureSensorTaskID );

    EA=1;
    clusterTemperatureMeasurementeInit();
#ifdef DHT12        
    clusterHumidityMeasurementeInit();
#endif        
    powerClusterInit(temperatureSensorTaskID);
    identifyInit(temperatureSensorTaskID);
    ZMacSetTransmitPower(POWER);
  blinkLedInit(temperatureSensorTaskID);
  blinkLedstart();

#ifdef DHT12
  dht112_init(temperatureSensorTaskID);
#endif
#ifdef DS18B20  
  DS18B20_init(temperatureSensorTaskID);
#endif
  addEventCB(SYS_EVENT_MSG_BIT,sysEvent);

}


static void initReport(void){
  reportEndpoint = ENDPOINT;
  reportSecondCounter = reportSecond;
  reportDstAddr.addrMode = afAddr16Bit;
  reportDstAddr.endPoint = ENDPOINT;
  reportDstAddr.addr.shortAddr = 0;
  reportDstAddr.panId=_NIB.nodeDepth;
}


void sysEvent(uint16 events){
  afIncomingMSGPacket_t *MSGpkt;

  while (true)  {
    MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( temperatureSensorTaskID ); 
    if (MSGpkt == NULL){
      return;
    }
    switch ( MSGpkt->hdr.event ) {
      case ZCL_INCOMING_MSG:
        // Incoming ZCL Foundation command/response messages
        processIncomingMsh( (zclIncomingMsg_t *)MSGpkt );
      break;
      case ZDO_STATE_CHANGE:
        ZDOStateChange((devStates_t)(MSGpkt->hdr.status));
        break;
      default:
      break;
    }
    osal_msg_deallocate( (uint8 *)MSGpkt );
  }

}

void ZDOStateChange(devStates_t newState){
  if(prevState !=   newState){
#ifdef DISPLAY
    char buffer[10];
    setCursor(0, 54);
    clean(0,45, DISPLAY_WIDTH, 54);
    drawText("ID: ");
    _itoa(_NIB.nwkDevAddress, (uint8_t*)buffer, 16);
    drawText(buffer);
    setCursor(0,9);
    clean(0,0, DISPLAY_WIDTH, 9);
#endif
    switch(newState){
      case DEV_NWK_DISC:
#ifdef DISPLAY
        drawText("DISCOVERING");
#endif
        setBlinkCounter(0);
        break;
      case DEV_NWK_JOINING:
#ifdef DISPLAY
        drawText("JOINING");
#endif
        setBlinkCounter(1);
        break;
      case DEV_NWK_REJOIN:
#ifdef DISPLAY
        drawText("REJOIN");
#endif
        setBlinkCounter(2);
        break;
      case DEV_END_DEVICE_UNAUTH:
#ifdef DISPLAY
        drawText("END_DEVICE_UNAUTH");
#endif        
        setBlinkCounter(3);
        break;
      case DEV_END_DEVICE:
#ifdef DISPLAY        
        drawText("CONNECTED AS END DEVICE");
#endif
        blinkLedEnd();
        initReport();
        break;
      case DEV_ROUTER:
#ifdef DISPLAY        
        drawText("CONNECTED AS ROUTER");
#endif
        blinkLedEnd();
        initReport();
        break;
      case DEV_COORD_STARTING:
#ifdef DISPLAY
        drawText("STARTING AS COORDINATOR");
#endif        
        setBlinkCounter(6);
        break;
      case DEV_ZB_COORD:
#ifdef DISPLAY        
        drawText("COORDINATOR");
#endif        
        setBlinkCounter(7);
        break;
      case DEV_NWK_ORPHAN:
#ifdef DISPLAY        
        drawText("ORPHAN");
#endif
        setBlinkCounter(8);
        break;
      }
#ifdef DISPLAY    
  display();  
#endif  
  prevState = newState;
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
uint16 temperatureSensorEventLoop( uint8 task_id, uint16 events ){
  handleEvent(&events);
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


void User_Process_Pool(void) {
}

/****************************************************************************
****************************************************************************/


