
/**************************************************************************************************

 DESCRIPTION:
  --

 CREATED: 05/11/2015, by Paolo Achdjian

 FILE: ClusterElectricityMeasure.c

***************************************************************************************************/


#include "zcl.h"
#include "ClusterElectricityMeasure.h"
#include "CS5463.h"


extern uint8 connected;

static uint32 measurementType=0x0107;
static uint16 frequency=220;
static uint16 frequencyMult=1;
static uint16 frequencyDiv=1;
static uint32 powerMult=1;
static uint32 powerDivisor=1;

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

static uint16 tmp;

static uint16 averageRmsVolrPeriod;


void electricityMeasureClusterReadAttributeInit(void){
  CS5463_Init();
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
		tmp = getCS5463RegisterValue(IstantaneusCurrent) >> 8;
		attribute->dataType = ZCL_DATATYPE_UINT16;
		attribute->dataPtr = (void *)&tmp;
		break;
	case ATTRID_ELECTRICITY_MEASURE_RMS_VOLT:
		tmp = getCS5463RegisterValue(RMSVolt) >> 8;
		attribute->dataType = ZCL_DATATYPE_UINT16;
		attribute->dataPtr = (void *)&tmp;
		break;
	case ATTRID_ELECTRICITY_MEASURE_RMS_CURRENT:
		tmp = getCS5463RegisterValue(RMSCurrent) >> 8;
		attribute->dataType = ZCL_DATATYPE_UINT16;
		attribute->dataPtr = (void *)&tmp;
		break;
	case ATTRID_ELECTRICITY_MEASURE_RMS_CURRENT_MAX:
		tmp = getCS5463RegisterValue(PeakCurrent) >> 8;
		attribute->dataType = ZCL_DATATYPE_UINT16;
		attribute->dataPtr = (void *)&tmp;
		break;
	case ATTRID_ELECTRICITY_MEASURE_ACTIVE_POWER:
		tmp = getCS5463RegisterValue(ActivePower) >> 8;
		attribute->dataType = ZCL_DATATYPE_INT16;
		attribute->dataPtr = (void *)&tmp;
		break;
	case ATTRID_ELECTRICITY_MEASURE_REACTIVE_POWER:
		tmp = getCS5463RegisterValue(IstantaneusReactivePower) >> 8;
		attribute->dataType = ZCL_DATATYPE_INT16;
		attribute->dataPtr = (void *)&tmp;
		break;
	case ATTRID_ELECTRICITY_MEASURE_APPARENT_POWER:
		tmp = getCS5463RegisterValue(ApparentPower) >> 8;
		attribute->dataType = ZCL_DATATYPE_UINT16;
		attribute->dataPtr = (void *)&tmp;
		break;
	case ATTRID_ELECTRICITY_MEASURE_POWER_FACTOR:
		tmp = getCS5463RegisterValue(PowerFactor) >> 8;
		attribute->dataType = ZCL_DATATYPE_INT8;
		attribute->dataPtr = (void *)&tmp;
		break;
	default:
		attribute->status = ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
	}
}

void electricityMeasureClusterSendReport(uint8 endpoint, afAddrType_t * dstAddr, uint8 * segNum) {
  if (connected){
    uint16 lineCurrent;
    uint16 rmsVolt;
    uint16 rmsCurrent;
    uint16 rmsCurrentMax;
    uint16 activePower;
    uint16 reactivePower;
    uint16 apparentPower;
    uint16 powerFactor; 
    
    zclReportCmd_t * pReportCmd = osal_mem_alloc( sizeof(zclReportCmd_t) + 8*sizeof(zclReport_t) );
    if ( pReportCmd != NULL ) {
      pReportCmd->numAttr = 8;
      pReportCmd->attrList[0].attrID = ATTRID_ELECTRICITY_MEASURE_LINE_CURRENT;
      lineCurrent = getCS5463RegisterValue(IstantaneusCurrent) >> 8;
      pReportCmd->attrList[0].dataType = ZCL_DATATYPE_UINT16;
      pReportCmd->attrList[0].attrData = (void *)&lineCurrent;
      pReportCmd->attrList[1].attrID = ATTRID_ELECTRICITY_MEASURE_RMS_VOLT;
      rmsVolt = getCS5463RegisterValue(RMSVolt) >> 8;
      pReportCmd->attrList[1].dataType = ZCL_DATATYPE_UINT16;
      pReportCmd->attrList[1].attrData = (void *)&rmsVolt;
      pReportCmd->attrList[2].attrID = ATTRID_ELECTRICITY_MEASURE_RMS_CURRENT;
      rmsCurrent = getCS5463RegisterValue(RMSCurrent) >> 8;
      pReportCmd->attrList[2].dataType = ZCL_DATATYPE_UINT16;
      pReportCmd->attrList[2].attrData = (void *)&rmsCurrent;
      pReportCmd->attrList[3].attrID = ATTRID_ELECTRICITY_MEASURE_RMS_CURRENT_MAX;
      rmsCurrentMax = getCS5463RegisterValue(PeakCurrent) >> 8;
      pReportCmd->attrList[3].dataType = ZCL_DATATYPE_UINT16;
      pReportCmd->attrList[3].attrData = (void *)&rmsCurrentMax;
      pReportCmd->attrList[4].attrID = ATTRID_ELECTRICITY_MEASURE_ACTIVE_POWER;
      activePower = getCS5463RegisterValue(ActivePower) >> 8;
      if (activePower == 0xFFFF)
          activePower=0;
      pReportCmd->attrList[4].dataType = ZCL_DATATYPE_INT16;
      pReportCmd->attrList[4].attrData = (void *)&activePower;
      pReportCmd->attrList[5].attrID = ATTRID_ELECTRICITY_MEASURE_REACTIVE_POWER;
      reactivePower = getCS5463RegisterValue(IstantaneusReactivePower) >> 8;
      pReportCmd->attrList[5].dataType = ZCL_DATATYPE_INT16;
      pReportCmd->attrList[5].attrData = (void *)&reactivePower;
      pReportCmd->attrList[6].attrID = ATTRID_ELECTRICITY_MEASURE_APPARENT_POWER;
      apparentPower = getCS5463RegisterValue(ApparentPower) >> 8;
      pReportCmd->attrList[6].dataType = ZCL_DATATYPE_UINT16;
      pReportCmd->attrList[6].attrData = (void *)&apparentPower;
      pReportCmd->attrList[7].attrID = ATTRID_ELECTRICITY_MEASURE_POWER_FACTOR;
      powerFactor = getCS5463RegisterValue(PowerFactor) >> 8;
      pReportCmd->attrList[7].dataType = ZCL_DATATYPE_INT8;
      pReportCmd->attrList[7].attrData = (void *)&powerFactor;
      

      zcl_SendReportCmd( endpoint, dstAddr,
                         ZCL_CLUSTER_ID_HA_ELECTRICAL_MEASUREMENT,
                         pReportCmd, ZCL_FRAME_SERVER_CLIENT_DIR, TRUE, (*segNum)++ );
      
      osal_mem_free( pReportCmd );
    }
  }

}