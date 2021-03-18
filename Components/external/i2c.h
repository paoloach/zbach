
#ifndef __I2C__H__
#define __I2C__H__

#include <hal_types.h>

void initI2c(void);
void writeByte(uint8 data);
void sendI2c(uint8 * data, uint8 dataLen);
void fillI2cData(uint8 addreass, uint8 cmd, uint8 * data, uint16 dataLen);

#endif