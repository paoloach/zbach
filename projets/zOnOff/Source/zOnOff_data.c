/**************************************************************************************************

 DESCRIPTION:
  --

  FILE: zOnOff_data.c

***************************************************************************************************/


#include "zcl.h"
#include "AF.h"
#include "zcl_ha.h"

#include "zOnOff.h"


#define DEVICE_VERSION     0
#define FLAGS              0


const cId_t zOnOffInClusterList[] ={
	ZCL_CLUSTER_ID_GEN_BASIC,
	ZCL_CLUSTER_ID_GEN_IDENTIFY,
	ZCL_CLUSTER_ID_GEN_POWER_CFG
        
       
};

const cId_t zOnOffOutClusterList[] ={
	ZCL_CLUSTER_ID_GEN_BASIC,
    ZCL_CLUSTER_ID_GEN_IDENTIFY
};



SimpleDescriptionFormat_t temperatureSimpleDesc = {
	ENDPOINT,                  			//  int Endpoint;
	ZCL_HA_PROFILE_ID,                	//  uint16 AppProfId[2];
	ZCL_HA_DEVICEID_OCCUPANCY_SENSOR,     	//  uint16 AppDeviceId[2];
	DEVICE_VERSION,           			//  int   AppDevVer:4;
	FLAGS,                    			//  int   AppFlags:4;
	sizeof(zOnOffInClusterList)/2,              //  byte  AppNumInClusters;
	(cId_t *)zOnOffInClusterList, 	//  byte *pAppInClusterList;
	sizeof(zOnOffOutClusterList)/2,           //  byte  AppNumInClusters;
	(cId_t *)zOnOffOutClusterList 	//  byte *pAppInClusterList;
};

