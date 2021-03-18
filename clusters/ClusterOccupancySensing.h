#ifndef _CLUSTER_OCCUPANCY_SENSING__H__
#define _CLUSTER_OCCUPANCY_SENSING__H__

#include "zclReadAttributeFn.h"
#include "zclWriteAttributeFn.h"
#include "zcl.h"
#include "osal.h"
#include "zcl_general.h"

#define ATTRID_OCCUPANCY_OCCUPANCY                                      0x00
#define ATTRID_OCCUPANCY_SENSOR_TYPE                                    0x01
#define ATTRID_OCCUPANCY_PIR_OCCUPIED_TO_UNOCCUPIED_DELAY               0x10
#define ATTRID_OCCUPANCY_PIR_UNOCCUPIED_TO_OCCUPIED_DELAY               0x11
#define ATTRID_OCCUPANCY_PIR_UNOCCUPIED_TO_OCCUPIED_THRESHOLD           0x12
#define ATTRID_OCCUPANCY_ULTRASONIC_OCCUPIED_TO_UNOCCUPIED_DELAY        0x20
#define ATTRID_OCCUPANCY_ULTRASONIC_UNOCCUPIED_TO_OCCUPIED_DELAY        0x21
#define ATTRID_OCCUPANCY_ULTRASONIC_UNOCCUPIED_TO_OCCUPIED_THRESHOLD    0x22

#define OCCUPANCY_TYPE_PIR                      0
#define OCCUPANCY_TYPE_ULTRASONIC               1
#define OCCUPANCY_TYPE_PIR_AND_ULTRASONIC       2

void clusterOccupancyInit(void);
void clusterOccupancySensingLoop(void);
void occupancySensingResetCB( void );
void occupancySensingReadAttribute(zclAttrRec_t *);

#endif