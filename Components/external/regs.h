
#ifndef __REGS__H__
#define __REGS__H__

#include "ioCC2530.h"
__sfr __no_init volatile union {
	struct {
		unsigned char T3_mode: 2;
		unsigned char T3_clear: 1;
		unsigned char T3_OVFIM: 1;
		unsigned char T3_start: 1;
		unsigned char T3_div: 3;
	};
} @ 0xCB;

__sfr __no_init volatile union {
	struct {
		unsigned char T3CCTL0_cap: 2;
		unsigned char T3CCTL0_mode: 1;
		unsigned char T3CCTL0_cmp: 3;
		unsigned char T3CCTL0_im: 1;
	};
} @ 0xCC;

__sfr __no_init volatile union {
	struct {
		unsigned char RFIE: 1;
		unsigned char P2IE: 1;
		unsigned char UTX0IE: 1;
		unsigned char UTX11E: 1;
		unsigned char P11E: 1;
		unsigned char WDTIE: 1;
	};
} @ 0x9A

__sfr __no_init volatile struct  {
	unsigned char DIR1_0: 1;
	unsigned char DIR1_1: 1;
	unsigned char DIR1_2: 1;
	unsigned char DIR1_3: 1;
	unsigned char DIR1_4: 1;
	unsigned char DIR1_5: 1;
	unsigned char DIR1_6: 1;
	unsigned char DIR1_7: 1;
} @ 0xFE;


#endif