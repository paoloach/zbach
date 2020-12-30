/**************************************************************************************************

 DESCRIPTION:
  --

  FILE: zSmartSocket_data.c

***************************************************************************************************/


#include "zcl.h"
#include "AF.h"
#include "zcl_ha.h"

#include "zSmartSocket.h"


#define DEVICE_VERSION     0
#define FLAGS              0


const cId_t zSmartSocketInClusterList[] ={
	ZCL_CLUSTER_ID_GEN_BASIC,
	ZCL_CLUSTER_ID_GEN_IDENTIFY,
        ZCL_CLUSTER_ID_GEN_ON_OFF,
	ZCL_CLUSTER_ID_GEN_POWER_CFG
        
       
};

const cId_t zSmartSocketOutClusterList[] ={
	ZCL_CLUSTER_ID_GEN_BASIC,
    ZCL_CLUSTER_ID_GEN_IDENTIFY,
    ZCL_CLUSTER_ID_GEN_ON_OFF
};



SimpleDescriptionFormat_t temperatureSimpleDesc = {
	ENDPOINT,                  			//  int Endpoint;
	ZCL_HA_PROFILE_ID,                	//  uint16 AppProfId[2];
	ZCL_HA_DEVICEID_ON_OFF_LIGHT,     	//  uint16 AppDeviceId[2];
	DEVICE_VERSION,           			//  int   AppDevVer:4;
	FLAGS,                    			//  int   AppFlags:4;
	sizeof(zSmartSocketInClusterList)/2,              //  byte  AppNumInClusters;
	(cId_t *)zSmartSocketInClusterList, 	//  byte *pAppInClusterList;
	sizeof(zSmartSocketOutClusterList)/2,           //  byte  AppNumInClusters;
	(cId_t *)zSmartSocketOutClusterList 	//  byte *pAppInClusterList;
};

