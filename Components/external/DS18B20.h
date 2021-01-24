#ifndef __DS18B20__H__
#define __DS18B20__H__

#include "af.h"


void DS18B20_init(uint8 taskid);
void readTemperature(void);
void finalizeReadTemp(void);
void setReportDest(uint8 endpoint, afAddrType_t * dstAddr, uint8 * segNum);


extern int16 temp;

#endif