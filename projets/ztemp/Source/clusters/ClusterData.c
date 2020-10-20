
#include "ClusterBasic.h"

const uint8 HWRevision = 2;
const uint8 ZCLVersion = 1;
const uint8 manufacturerName[] = { 8, 'A','c','h','d','j','i','a','n',' ','E','l','e','c','t',' ',' ',' ',' ',' ',' ' };
#ifdef DHT12 
const uint8 modelId[] = { 16,'z', 'T','e','m','p','e','r','a','t','u','r','e','H',' ',' ',' ' };
#else
const uint8 modelId[] = { 16,'z', 'T','e','m','p','e','r','a','t','u','r','e',' ',' ',' ',' ' };

#endif
const uint8 dateCode[] = { 16, '2','0','2','0','0','9','2','2',' ',' ',' ',' ',' ',' ',' ',' ' };

const uint8 batterySize = 10;
const uint8 batteryQuantity = 1;