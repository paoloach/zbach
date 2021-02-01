#ifndef __ELECTRICITY_MEASURE_DATA__H__
#define __ELECTRICITY_MEASURE_DATA__H__

#include "hal_types.h"

extern uint16 istantaneusCurrent;
extern uint16 RMSVolt;
extern uint16 RMSCurrent;
extern uint16 peakCurrent;
extern int16 activePower;
extern int16 istantaneusReactivePower;
extern uint16 apparentPower;
extern int8 powerFactor;

#endif