
#include "ClusterBasic.h"

const uint8 HWRevision = 2;
const uint8 ZCLVersion = 1;
const uint8 manufacturerName[] = { 14, 'A','c','h','d','j','i','a','n',' ','E','l','e','c','t',' ',' ',' ',' ',' ',' ' };

#ifdef HAL_PA_LNA_CC2592
const uint8 modelId[] = { 15,'z', 'P','r','o','x','y','S','e','n','s','o','r','-','P','A',' ' };
const uint8 dateCode[] = { 8, '2','0','2','1','0','5','3','0',' ',' ',' ',' ',' ',' ',' ',' ' };
#else
const uint8 modelId[] = { 12,'z', 'P','r','o','x','y','S','e','n','s','o','r',' ',' ',' ',' ' };
const uint8 dateCode[] = { 8, '2','0','2','0','1','2','2','1',' ',' ',' ',' ',' ',' ',' ',' ' };
#endif
const uint8 batterySize = 10;
const uint8 batteryQuantity = 1;