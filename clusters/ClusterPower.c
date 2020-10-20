
/**************************************************************************************************

 DESCRIPTION:
  Metering Cluster

 CREATED: 08/09/2015, by Paolo Achdjian

 FILE: ClusterMetering.c

***************************************************************************************************/

#include "zcl.h"
#include "zcl_general.h"
#include "ClusterPower.h"
#include "ioCC2530.h"
#include "ClusterOSALEvents.h"

#if !defined RTR_NWK
 static void readBatteryVolt(void);
#endif

static uint16 mainVoltage=0;
static uint16 batteryVoltage=0xFF;
static uint16 batteryPercRemaining=100;
static uint8  batteryAlarmMask=0x0;
extern NodePowerDescriptorFormat_t ZDO_Config_Power_Descriptor;

#define BATTERY_POLL_TIME_MS 60000


void powerClusterInit(byte appId) {
}

#if !defined RTR_NWK
void powerClusterCheckBattery(void) {
	readBatteryVolt();
	if (batteryVoltage > 0x30){
		ZDO_Config_Power_Descriptor.CurrentPowerSourceLevel = NODEPOWER_LEVEL_100;
	} else if (batteryVoltage > 29){
		ZDO_Config_Power_Descriptor.CurrentPowerSourceLevel = NODEPOWER_LEVEL_66;
	}else if (batteryVoltage > 29){
		ZDO_Config_Power_Descriptor.CurrentPowerSourceLevel = NODEPOWER_LEVEL_33;
	} else {
		ZDO_Config_Power_Descriptor.CurrentPowerSourceLevel = NODEPOWER_LEVEL_CRITICAL;
	}
}
#endif

void powerClusterReadAttribute(zclAttrRec_t * attribute) {
	if (attribute == NULL){
		return;
	}
	attribute->accessControl = ACCESS_CONTROL_READ;
	attribute->status = ZCL_STATUS_SUCCESS;
	switch(attribute->attrId){
	case ATTRID_POWER_CFG_MAINS_VOLTAGE:
		attribute->dataType = ZCL_DATATYPE_UINT16;
		attribute->dataPtr = (void *)&mainVoltage;
		break;
	case ATTRID_POWER_CFG_BATTERY_VOLTAGE:
	#if !defined RTR_NWK		
		powerClusterCheckBattery();
    #endif
		attribute->dataType = ZCL_DATATYPE_UINT8;
		attribute->dataPtr = (void *)&batteryVoltage;
		break;
        case ATTRID_POWER_CFG_BATTERY_PERCENTAGE_REMAINING:
		attribute->dataType = ZCL_DATATYPE_UINT8;
		attribute->dataPtr = (void *)&batteryPercRemaining;
                break;
	case ATTRID_POWER_CFG_BAT_ALARM_MASK:
		attribute->dataType = ZCL_DATATYPE_BITMAP8;
		attribute->dataPtr = (void *)&batteryAlarmMask;
		break;	
        case ATTRID_POWER_CFG_BAT_SIZE:
          	attribute->dataType = ZCL_DATATYPE_ENUM8;
		attribute->dataPtr = (void *)&batterySize;
		break;
        case ATTRID_POWER_CFG_BAT_QUANTITY:
          	attribute->dataType = ZCL_DATATYPE_UINT8;
		attribute->dataPtr = (void *)&batteryQuantity;
		break;        
	default:
		attribute->status = ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
	}
}

#if !defined RTR_NWK
static void readBatteryVolt(void) {
	ADCCON3 = 0x0F;
	while (!(ADCCON1 & 0x80));
	uint32 adch = (uint32)ADCH ;
	uint32 adcl = (uint32)ADCL;
	uint32 v = (adch<< 6) + (adcl>> 2);
	v = (3*115*v)/0x2000;
	batteryVoltage = v / 10; 
        if (batteryVoltage >= 33){
          batteryPercRemaining=200; 
        } else if (batteryVoltage <23){
          batteryPercRemaining=0;
        } else {
          batteryPercRemaining = (batteryVoltage-23)*20;
        }

}

void powerClusterSendReport(uint8 endpoint, afAddrType_t * dstAddr, uint8 * segNum){
  powerClusterCheckBattery();
  zclReportCmd_t *pReportCmd;
  
  pReportCmd = osal_mem_alloc( sizeof(zclReportCmd_t) + 2*sizeof(zclReport_t) );
  if ( pReportCmd != NULL ) {
    pReportCmd->numAttr = 2;
    pReportCmd->attrList[0].attrID = ATTRID_POWER_CFG_BATTERY_VOLTAGE;
    pReportCmd->attrList[0].dataType = ZCL_DATATYPE_UINT8;
    pReportCmd->attrList[0].attrData = (void *)(&batteryVoltage);
    pReportCmd->attrList[1].attrID = ATTRID_POWER_CFG_BATTERY_PERCENTAGE_REMAINING;
    pReportCmd->attrList[1].dataType = ZCL_DATATYPE_UINT8;
    pReportCmd->attrList[1].attrData = (void *)(&batteryPercRemaining);    

    zcl_SendReportCmd( endpoint, dstAddr,
                       ZCL_CLUSTER_ID_GEN_POWER_CFG,
                       pReportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, TRUE, (*segNum)++ );
  }

  osal_mem_free( pReportCmd );
}

#endif