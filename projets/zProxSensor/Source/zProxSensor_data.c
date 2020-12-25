/**************************************************************************************************

 DESCRIPTION:
  --

  FILE: zProxSensor_data.c

***************************************************************************************************/


#include "zcl.h"
#include "AF.h"
#include "zcl_ha.h"

#include "zProxSensor.h"


#define DEVICE_VERSION     0
#define FLAGS              0


const cId_t zProxSensorInClusterList[] ={
	ZCL_CLUSTER_ID_GEN_BASIC,
	ZCL_CLUSTER_ID_GEN_IDENTIFY,
	ZCL_CLUSTER_ID_GEN_POWER_CFG,
        ZCL_CLUSTER_ID_MS_OCCUPANCY_SENSING
        
       
};

const cId_t zProxSensorOutClusterList[] ={
	ZCL_CLUSTER_ID_GEN_BASIC,
    ZCL_CLUSTER_ID_GEN_IDENTIFY,
    ZCL_CLUSTER_ID_GEN_ON_OFF
};



SimpleDescriptionFormat_t temperatureSimpleDesc = {
	ENDPOINT,                  			//  int Endpoint;
	ZCL_HA_PROFILE_ID,                	//  uint16 AppProfId[2];
	ZCL_HA_DEVICEID_OCCUPANCY_SENSOR,     	//  uint16 AppDeviceId[2];
	DEVICE_VERSION,           			//  int   AppDevVer:4;
	FLAGS,                    			//  int   AppFlags:4;
	sizeof(zProxSensorInClusterList)/2,              //  byte  AppNumInClusters;
	(cId_t *)zProxSensorInClusterList, 	//  byte *pAppInClusterList;
	sizeof(zProxSensorOutClusterList)/2,           //  byte  AppNumInClusters;
	(cId_t *)zProxSensorOutClusterList 	//  byte *pAppInClusterList;
};

