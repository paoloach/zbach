
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
	unsigned char DIR0_0: 1;
	unsigned char DIR0_1: 1;
	unsigned char DIR0_2: 1;
	unsigned char DIR0_3: 1;
	unsigned char DIR0_4: 1;
	unsigned char DIR0_5: 1;
	unsigned char DIR0_6: 1;
	unsigned char DIR0_7: 1;
} @ 0xFD;

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




__sfr __no_init volatile struct  {
	unsigned char P0SEL_0: 1;
	unsigned char P0SEL_1: 1;
	unsigned char P0SEL_2: 1;
	unsigned char P0SEL_3: 1;
	unsigned char P0SEL_4: 1;
	unsigned char P0SEL_5: 1;
	unsigned char P0SEL_6: 1;
	unsigned char P0SEL_7: 1;
} @ 0xF3;


// U0GCR
__sfr __no_init volatile struct  {
	unsigned char U0_BAUD_E: 5;
	unsigned char U0_ORDER: 1;
	unsigned char U0_CPHA: 1;
	unsigned char U0_CPOL: 1;
} @ 0xC5;

// U0CSR
__sfr __no_init volatile struct  {
	unsigned char U0_ACTIVE: 1;
	unsigned char U0_TX_BYTE: 1;
	unsigned char U0_RX_BYTE: 1;
	unsigned char U0_ERR: 1;
	unsigned char U0_FE: 1;
	unsigned char U0_SLAVE: 1;
	unsigned char U0_RE: 1;
	unsigned char U0_MODE: 1;
} @ 0x86;

#define DIR_PORT_IMPL(A,B) DIR ## A ## _ ## B
#define DIR(A,B) DIR_PORT_IMPL( A,B)


#define FUNCTION_SEL_IMPL(A,B) P ## A ## SEL_ ## B
#define FUNCTION_SEL(A,B) FUNCTION_SEL_IMPL( A,B)

#define PORT_IMPL(A,B) P ## A ## _ ## B
#define PORT(A,B) PORT_IMPL( A,B)



#endif