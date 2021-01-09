/**************************************************************************************************
  Filename:       ClusterOSALEvents.h

  Autorh:  Paolo Achdjia
  Created: 03/11/2014

***************************************************************************************************/

#ifndef __CLUSTER_OSAL_EVENTS__H__
#define __CLUSTER_OSAL_EVENTS__H__

#define CLUSTER_EVENT

#define FAST_BLINK_BIT          6

#define IDENTIFY_TIMEOUT_EVT     0x0001
#define LEVEL_MOVEMENT           0x0002
#define READ_TEMP_EVT	         0x0004
#define END_READ_TEMP_EVT        0x0008
#define START_READ_TEMP		 0x0010
#define READ_TEMP_MASK	         0x001C
#define READ_BATTERY_LEVEL	 0x0020
//#define FAST_BLINK		 0x0040
#define REPORT_EVT               0x0080


  
#endif