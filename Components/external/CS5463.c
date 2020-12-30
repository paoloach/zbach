/**************************************************************************************************

 DESCRIPTION:
  --

 CREATED: 14/10/2015, by Paolo Achdjian

 FILE: CS2530.c

***************************************************************************************************/
#include "ZComDef.h"
#include "OSAL.h"
#include "ioCC2530.h"
#include "CS5463.h"
#include "regs.h"



union CS5463Reg {
	uint32	value;
	uint8	bytes[4];
};

void CS5463_Init(void ){
	U0_ACTIVE=0;
	U0BAUD=0;
	U0_BAUD_E=15;
	U0_CPOL=1;
	U0_CPHA=0;
	U0_ORDER=1;
	
	U0_MODE=0;
	U0_SLAVE=0;
	
	P0SEL_2=1; // Master In
	P0SEL_3=1; // Master Out
	P0SEL_5=1; // Clock
	
	
	CS5463_reset();
	for(uint8 i=0; i < 200; i++);
	CS5463_startConversion();
}

void CS5463_startConversion(void) {
	asm("NOP");
	asm("NOP");
	U0_TX_BYTE=0;
	U0DBUF=0xE8;
	asm("NOP");
	while(U0_TX_BYTE==0);
	asm("NOP");
	asm("NOP");
}

void CS5463_reset(void) {
	asm("NOP");
	asm("NOP");
	U0_TX_BYTE = 0;
	U0DBUF=0x80;
	while(U0_TX_BYTE==0);
	asm("NOP");
	asm("NOP");
}

int32 getCS5463RegisterValue(enum CS5463Register regIndex) {
	union CS5463Reg result;

	P0_7=0;
	asm("NOP");
	asm("NOP");
	uint8 index = (regIndex << 1);
	U0_TX_BYTE=0;
	U0DBUF=index;
	asm("NOP");
	
	while(U0_TX_BYTE==0);
	
	result.bytes[3] = 0;
	U0_TX_BYTE=0;
	U0DBUF=0xFF;
	asm("NOP");
	while(U0_TX_BYTE==0);
	
	result.bytes[2] = U0DBUF;
	U0_TX_BYTE=0;		
	U0DBUF=0xFF;
	asm("NOP");
	while(U0_TX_BYTE==0);
	result.bytes[1] = U0DBUF;

	U0_TX_BYTE=0;
	U0DBUF=0xFF;
	asm("NOP");
	while(U0_TX_BYTE==0);
	result.bytes[0] = U0DBUF;
	asm("NOP");
	asm("NOP");
	P0_7=1;
	asm("NOP");
	asm("NOP");
	asm("NOP");
	asm("NOP");
	
	U0_ACTIVE=0;
	return result.value;
}

void setCS5463RegisterValue(enum CS5463Register regIndex,unsigned char  byte1,unsigned  char byte2,unsigned  char byte3){
	union CS5463Reg result;

	P0_7=0;
	asm("NOP");
	asm("NOP");
	uint8 index = (regIndex << 1);
	index |= 0x40;
	U0_TX_BYTE=0;
	U0DBUF=index;
	asm("NOP");
	
	while(U0_TX_BYTE==0);
	
	result.bytes[3] = 0;
	U0_TX_BYTE=0;
	U0DBUF=byte1;
	asm("NOP");
	while(U0_TX_BYTE==0);
	
	result.bytes[2] = U0DBUF;
	U0_TX_BYTE=0;		
	U0DBUF=byte2;
	asm("NOP");
	while(U0_TX_BYTE==0);
	result.bytes[1] = U0DBUF;
	U0_TX_BYTE = 0;
	U0DBUF=byte3;
	asm("NOP");
	while(U0_TX_BYTE==0);
	result.bytes[0] = U0DBUF;
	asm("NOP");
	asm("NOP");
	P0_7=1;
	asm("NOP");
	asm("NOP");
	asm("NOP");
	asm("NOP");
	
	U0_ACTIVE=0;
}

