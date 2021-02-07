/**************************************************************************************************

 DESCRIPTION:
  --

 CREATED: 23/08/2016, by Paolo Achdjian

 FILE: zLightController_data.c

***************************************************************************************************/


#include "zcl.h"
#include "AF.h"
#include "zcl_ha.h"

#include "zLightController.h"


#define DEVICE_VERSION     0
#define FLAGS              0


const cId_t temperatureInClusterList[] ={
	ZCL_CLUSTER_ID_GEN_BASIC,
	ZCL_CLUSTER_ID_GEN_IDENTIFY,
        ZCL_CLUSTER_ID_GEN_ON_OFF,
	ZCL_CLUSTER_ID_GEN_POWER_CFG         
};

const cId_t temperatureOutClusterList[] ={
	ZCL_CLUSTER_ID_GEN_BASIC,
    ZCL_CLUSTER_ID_GEN_IDENTIFY,
    ZCL_CLUSTER_ID_GEN_ON_OFF
};



SimpleDescriptionFormat_t zControlerLightSimpleDesc = {
	ENDPOINT,                  			//  int Endpoint;
	ZCL_HA_PROFILE_ID,                	//  uint16 AppProfId[2];
	ZCL_HA_DEVICEID_ON_OFF_LIGHT,     	//  uint16 AppDeviceId[2];
	DEVICE_VERSION,           			//  int   AppDevVer:4;
	FLAGS,                    			//  int   AppFlags:4;
	sizeof(temperatureInClusterList)/2,              //  byte  AppNumInClusters;
	(cId_t *)temperatureInClusterList, 	//  byte *pAppInClusterList;
	sizeof(temperatureOutClusterList)/2,           //  byte  AppNumInClusters;
	(cId_t *)temperatureOutClusterList 	//  byte *pAppInClusterList;
};

