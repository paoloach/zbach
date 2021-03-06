
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
#include "report.h"


struct PowerReport {
  uint16 id1;
  uint8  type1;
  uint8 batteryVoltage;
  uint16 id2;
  uint8  type2;
  uint8 batteryRemain;
};

#if !defined RTR_NWK
 static void readBatteryVolt(void);
#endif

#define BATTERY_POLL_TIME_SEC 300

static uint16 mainVoltage=0;
static uint16 batteryVoltage=0xFF;
static uint16 batteryPercRemaining=100;
static uint8  batteryAlarmMask=0x0;

extern NodePowerDescriptorFormat_t ZDO_Config_Power_Descriptor;

#if !defined RTR_NWK
static void powerClusterSendReport(void); 
static struct PowerReport reportCmd;
#endif

void powerClusterInit(byte appId) {
#if !defined RTR_NWK
  reportCmd.id1 = ATTRID_POWER_CFG_BATTERY_VOLTAGE;
  reportCmd.id2 = ATTRID_POWER_CFG_BATTERY_PERCENTAGE_REMAINING;
  reportCmd.type1=ZCL_DATATYPE_UINT8;
  reportCmd.type2=ZCL_DATATYPE_UINT8;

  osal_start_reload_timer_cb((uint32)BATTERY_POLL_TIME_SEC*1000, &powerClusterSendReport );  
#endif
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
          batteryPercRemaining = (v-230)*2;
        }

}



void powerClusterSendReport(){
    powerClusterCheckBattery();

    reportCmd.batteryVoltage = batteryVoltage;
    reportCmd.batteryRemain = batteryPercRemaining;

   
    zcl_SendCommand( reportEndpoint,  &reportDstAddr, ZCL_CLUSTER_ID_GEN_POWER_CFG, ZCL_CMD_REPORT, FALSE,
                              ZCL_FRAME_SERVER_CLIENT_DIR, TRUE, 0, reportSeqNum++, 8, (uint8*)&reportCmd );
}

#endif