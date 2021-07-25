#ifndef __BL0937__H__
#define __BL0937__H__

void BL0937_init(uint8_t taskId);
uint16_t getCFMean(void);
uint16_t getCF1Mean(void);
void saveConvertCoeff(double coefPower, double coefCurrent);


#endif