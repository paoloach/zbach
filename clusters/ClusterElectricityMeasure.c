
/**************************************************************************************************

 DESCRIPTION:
  --

 CREATED: 05/11/2015, by Paolo Achdjian

 FILE: ClusterElectricityMeasure.c

***************************************************************************************************/


#include "zcl.h"
#include "ClusterElectricityMeasure.h"
#include "ElectricityMeasureData.h"
#include "report.h"
#include "ClusterOSALEvents.h"
#include "EventManager.h"

static void electricityMeasureClusterSendReport(void);	
static void newPowerValue(uint16_t events);

extern uint8 connected;

static uint32 measurementType=0x0107;
static uint16 frequency=220;
static uint16 frequencyMult=1;
static uint16 frequencyDiv=1;
static uint32 powerMult=1;
static uint32 powerDivisor=1;
uint16 prevRMSCurrent;
uint16 istantaneusCurrent;
uint16 RMSVolt;
uint16 RMSCurrent;
uint16 peakCurrent;
int16 prevActivePower;
int16 activePower;
int16 istantaneusReactivePower;
uint16 apparentPower;
int8 powerFactor;

/*
  From the schematic, the voltage is divided by 2000 first to send to cs5463
  So 220V ->110mv, that is 0.44 of the full scale voltage (250 mv)

  The value read by CS5463 is multiply for 0x1000, in order to obtaion a value from -0x1000 to 0x1000, where 0x1000 is 250mv, so to obtain the value read from cs5463
  
  val /(4*0x1000)

  Because the value read by cs5462 was divided by 2000, then it's need to multiplay for the same value.
  
*/
static uint16 acVoltMult=2000;
static uint16 acVoltDiv=0x4000;

/*
  From the schematic, the voltage is divided by 2000 first to send to cs5463
  So 220V ->110mv, that is 0.44 of the full scale voltage (250 mv)

  The value read by CS5463 is multiply for 0x1000, in order to obtaion a value from -0x1000 to 0x1000, where 0x1000 is 250mv, so to obtain the value read from cs5463
  
  val /(4*0x1000)

  Because the value read by cs5462 was divided by 2000, then it's need to multiplay for the same value.
  
*/
static uint16 acCurrentMult=1;
static uint16 acCurrentDiv=1;
static uint16 acPowerMult=1;
static uint16 acPowerDiv=1;

static uint16 averageRmsVolrPeriod;
static zclReportCmd8_t reportCmd;

#ifndef ELECTRICTY_MEASURE_REPORT_TIME
 #define ELECTRICTY_MEASURE_REPORT_TIME 60
#endif


void electricityMeasureClusterReadAttributeInit(void){
   osal_start_reload_timer_cb((uint32)ELECTRICTY_MEASURE_REPORT_TIME*1000, &electricityMeasureClusterSendReport); 
   addEventCB(NEW_POWER_BIT, &newPowerValue);
}


void electricityMeasureClusterReadAttribute(zclAttrRec_t * attribute){
	if (attribute == NULL){
		return;
	}
	attribute->accessControl = ACCESS_CONTROL_READ;
	attribute->status =  ZCL_STATUS_SUCCESS;
	switch(attribute->attrId){
	case ATTRID_ELECTRICITY_MEASURE_MEASUREMENT_TYPE:
		attribute->dataType = ZCL_DATATYPE_BITMAP32;
		attribute->dataPtr = (void *)&measurementType;
		break;
	case ATTRID_ELECTRICITY_MEASURE_AC_FREQ:
		attribute->dataType = ZCL_DATATYPE_UINT16;
		attribute->dataPtr = (void *)&frequency;
		break;		
	case ATTRID_ELECTRICITY_MEASURE_AC_FREQ_MULT:
		attribute->dataType = ZCL_DATATYPE_UINT16;
		attribute->dataPtr = (void *)&frequencyMult;
		break;
	case ATTRID_ELECTRICITY_MEASURE_AC_FREQ_DIV:
		attribute->dataType = ZCL_DATATYPE_UINT16;
		attribute->dataPtr = (void *)&frequencyDiv;
		break;
	case ATTRID_ELECTRICITY_MEASURE_POWER_MULT:
		attribute->dataType = ZCL_DATATYPE_UINT32;
		attribute->dataPtr = (void *)&powerMult;
		break;
	case ATTRID_ELECTRICITY_MEASURE_POWER_DIV:
		attribute->dataType = ZCL_DATATYPE_UINT32;
		attribute->dataPtr = (void *)&powerDivisor;
		break;
	case ATTRID_ELECTRICITY_MEASURE_AV_RMS_VOLT_PERIOD:
		attribute->dataType = ZCL_DATATYPE_UINT16;
		attribute->dataPtr = (void *)&averageRmsVolrPeriod;
		break;
	case ATTRID_ELECTRICITY_MEASURE_AC_VOLT_MULT:
		attribute->dataType = ZCL_DATATYPE_UINT16;
		attribute->dataPtr = (void *)&acVoltMult;
		break;
	case ATTRID_ELECTRICITY_MEASURE_AC_VOLT_DIV:
		attribute->dataType = ZCL_DATATYPE_UINT16;
		attribute->dataPtr = (void *)&acVoltDiv;
		break;
	case ATTRID_ELECTRICITY_MEASURE_AC_CURRENT_MULT:
		attribute->dataType = ZCL_DATATYPE_UINT16;
		attribute->dataPtr = (void *)&acCurrentMult;
		break;
	case ATTRID_ELECTRICITY_MEASURE_AC_CURRENT_DIV:
		attribute->dataType = ZCL_DATATYPE_UINT16;
		attribute->dataPtr = (void *)&acCurrentDiv;
		break;
	case ATTRID_ELECTRICITY_MEASURE_AC_POWER_MULT:
		attribute->dataType = ZCL_DATATYPE_UINT16;
		attribute->dataPtr = (void *)&acPowerMult;
		break;
	case ATTRID_ELECTRICITY_MEASURE_AC_POWER_DIV:
		attribute->dataType = ZCL_DATATYPE_UINT16;
		attribute->dataPtr = (void *)&acPowerDiv;
		break;
		
	case ATTRID_ELECTRICITY_MEASURE_LINE_CURRENT:
		attribute->dataType = ZCL_DATATYPE_UINT16;
		attribute->dataPtr = (void *)&istantaneusCurrent;
		break;
	case ATTRID_ELECTRICITY_MEASURE_RMS_VOLT:
		attribute->dataType = ZCL_DATATYPE_UINT16;
		attribute->dataPtr = (void *)&RMSVolt;
		break;
	case ATTRID_ELECTRICITY_MEASURE_RMS_CURRENT:
		attribute->dataType = ZCL_DATATYPE_UINT16;
		attribute->dataPtr = (void *)&RMSCurrent;
		break;
	case ATTRID_ELECTRICITY_MEASURE_RMS_CURRENT_MAX:
		attribute->dataType = ZCL_DATATYPE_UINT16;
		attribute->dataPtr = (void *)&peakCurrent;
		break;
	case ATTRID_ELECTRICITY_MEASURE_ACTIVE_POWER:
		attribute->dataType = ZCL_DATATYPE_INT16;
		attribute->dataPtr = (void *)&activePower;
		break;
	case ATTRID_ELECTRICITY_MEASURE_REACTIVE_POWER:
		attribute->dataType = ZCL_DATATYPE_INT16;
		attribute->dataPtr = (void *)&istantaneusReactivePower;
		break;
	case ATTRID_ELECTRICITY_MEASURE_APPARENT_POWER:
		attribute->dataType = ZCL_DATATYPE_UINT16;
		attribute->dataPtr = (void *)&apparentPower;
		break;
	case ATTRID_ELECTRICITY_MEASURE_POWER_FACTOR:
		attribute->dataType = ZCL_DATATYPE_INT8;
		attribute->dataPtr = (void *)&powerFactor;
		break;
	default:
		attribute->status = ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
	}
}

void electricityMeasureClusterSendReport(void) {
  if (0){
      reportCmd.numAttr = 8;
      reportCmd.attrList[0].attrID = ATTRID_ELECTRICITY_MEASURE_LINE_CURRENT;
      reportCmd.attrList[0].dataType = ZCL_DATATYPE_UINT16;
      reportCmd.attrList[0].attrData = (void *)&istantaneusCurrent;
      reportCmd.attrList[1].attrID = ATTRID_ELECTRICITY_MEASURE_RMS_VOLT;
      reportCmd.attrList[1].dataType = ZCL_DATATYPE_UINT16;
      reportCmd.attrList[1].attrData = (void *)&RMSVolt;
      reportCmd.attrList[2].attrID = ATTRID_ELECTRICITY_MEASURE_RMS_CURRENT;
      reportCmd.attrList[2].dataType = ZCL_DATATYPE_UINT16;
      reportCmd.attrList[2].attrData = (void *)&RMSCurrent;
      reportCmd.attrList[3].attrID = ATTRID_ELECTRICITY_MEASURE_RMS_CURRENT_MAX;
      reportCmd.attrList[3].dataType = ZCL_DATATYPE_UINT16;
      reportCmd.attrList[3].attrData = (void *)&peakCurrent;
      reportCmd.attrList[4].attrID = ATTRID_ELECTRICITY_MEASURE_ACTIVE_POWER;
      reportCmd.attrList[4].dataType = ZCL_DATATYPE_INT16;
      reportCmd.attrList[4].attrData = (void *)&activePower;
      reportCmd.attrList[5].attrID = ATTRID_ELECTRICITY_MEASURE_REACTIVE_POWER;
      reportCmd.attrList[5].dataType = ZCL_DATATYPE_INT16;
      reportCmd.attrList[5].attrData = (void *)&istantaneusReactivePower;
      reportCmd.attrList[6].attrID = ATTRID_ELECTRICITY_MEASURE_APPARENT_POWER;
      reportCmd.attrList[6].dataType = ZCL_DATATYPE_UINT16;
      reportCmd.attrList[6].attrData = (void *)&apparentPower;
      reportCmd.attrList[7].attrID = ATTRID_ELECTRICITY_MEASURE_POWER_FACTOR;
      reportCmd.attrList[7].dataType = ZCL_DATATYPE_INT8;
      reportCmd.attrList[7].attrData = (void *)&powerFactor;
      

      zcl_SendReportCmd( reportEndpoint, &reportDstAddr,
                         ZCL_CLUSTER_ID_HA_ELECTRICAL_MEASUREMENT,
                        (zclReportCmd_t *)&reportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, TRUE, reportSeqNum++ );
      
    
  }

}

static void newPowerValue(uint16_t events) {
  if (prevActivePower != activePower || prevRMSCurrent != RMSCurrent){
    prevActivePower = activePower;
    prevRMSCurrent = RMSCurrent;
    electricityMeasureClusterSendReport();
      
  }
}