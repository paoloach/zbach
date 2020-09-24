
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
static uint8  batteryAlarmMask=0x0;
extern NodePowerDescriptorFormat_t ZDO_Config_Power_Descriptor;

#define BATTERY_POLL_TIME_MS 60000


void powerClusterInit(byte appId) {
	#if !defined RTR_NWK
	osal_start_timerEx( appId, READ_BATTERY_LEVEL, BATTERY_POLL_TIME_MS );
#endif
}

#if !defined RTR_NWK
void powerClusterCheckBattery(byte appId) {
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
	osal_start_timerEx( appId, READ_BATTERY_LEVEL, BATTERY_POLL_TIME_MS );
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
		readBatteryVolt();
    #endif
		attribute->dataType = ZCL_DATATYPE_UINT8;
		attribute->dataPtr = (void *)&batteryVoltage;
		break;
	case ATTRID_POWER_CFG_BAT_ALARM_MASK:
		attribute->dataType = ZCL_DATATYPE_BITMAP8;
		attribute->dataPtr = (void *)&batteryAlarmMask;
		break;	
	default:
		attribute->status = ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
	}
}

#if !defined RTR_NWK
static void readBatteryVolt(void) {
	ADCCON3 = 0x2F;
	while (!(ADCCON1 & 0x80));
	uint32 adch = (uint32)ADCH ;
	uint32 adcl = (uint32)ADCL;
	uint32 v = (adch<< 6) + (adcl>> 2);
	v = (3*115*v)/256;
	batteryVoltage = v / 10;
}
#endif