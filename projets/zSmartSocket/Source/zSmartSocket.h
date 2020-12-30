/**************************************************************************************************

 DESCRIPTION:
  --

 
 FILE: zSmartSocket.h

***************************************************************************************************/


#ifndef ZPROX_SENSOR_H__
#define ZPROX_SENSOR__H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "zcl.h"

/*********************************************************************
 * CONSTANTS
 */
#define ENDPOINT            13

extern SimpleDescriptionFormat_t zclSampleLight_SimpleDesc;
extern CONST zclAttrRec_t lightAchdjianAttrs[];
extern uint16 zclSampleLight_IdentifyTime;

extern void zSmartSocketInit( byte task_id );
extern UINT16 zSmartSocketEventLoop( byte task_id, UINT16 events );



#ifdef __cplusplus
}
#endif

#endif /* TEMPERATURE_SENSOR__H__ */
