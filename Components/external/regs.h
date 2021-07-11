
#ifndef __REGS__H__
#define __REGS__H__

#include "ioCC2530.h"


struct Byte2reg {
  uint8_t low;
  uint8_t high;
};

union Reg16 {
  uint16_t value;
  struct Byte2reg reg;
};


__sfr __no_init volatile union {
	struct {
		unsigned char T1_mode: 2;
		unsigned char T1_div: 2;
		unsigned char T1_CTL_RESERVED: 4;
	};
} @ 0xE4;

__sfr __no_init volatile union {
	struct {
		unsigned char T1_CH0IF: 1;
		unsigned char T1_CH1IF: 1;
                unsigned char T1_CH2IF: 1;
                unsigned char T1_CH3IF: 1;
                unsigned char T1_CH4IF: 1;
                unsigned char T1_OVFIF: 1;
	};
} @ 0xAF;

__sfr __no_init volatile union {
	struct {
		unsigned char T1_CH0_CAP: 2;
		unsigned char T1_CH0_MODE: 1;
                unsigned char T1_CH0_CMP: 3;
                unsigned char T1_CH0_IM: 1;
                unsigned char T1_CH0_RFIRQ: 1;
	};
} @ 0xE5;



__sfr __no_init volatile union {
	struct {
		unsigned char T1_CH1_CAP: 2;
		unsigned char T1_CH1_MODE: 1;
                unsigned char T1_CH1_CMP: 3;
                unsigned char T1_CH1_IM: 1;
                unsigned char T1_CH1_RFIRQ: 1;
	};
} @ 0xE6;

__sfr __no_init volatile union {
	struct {
		unsigned char T1_CH2_CAP: 2;
		unsigned char T1_CH2_MODE: 1;
                unsigned char T1_CH2_CMP: 3;
                unsigned char T1_CH2_IM: 1;
                unsigned char T1_CH2_RFIRQ: 1;
	};
} @ 0xE7;

__sfr __no_init volatile union {
	struct {
		unsigned char T1_CH3_CAP: 2;
		unsigned char T1_CH3_MODE: 1;
                unsigned char T1_CH3_CMP: 3;
                unsigned char T1_CH3_IM: 1;
                unsigned char T1_CH3_RFIRQ: 1;
	};
} @ 0xE8;
                
__sfr __no_init volatile union {
	struct {
		unsigned char T1_CH4_CAP: 2;
		unsigned char T1_CH4_MODE: 1;
                unsigned char T1_CH4_CMP: 3;
                unsigned char T1_CH4_IM: 1;
                unsigned char T1_CH4_RFIRQ: 1;
	};
} @ 0xE9;
  
          
__sfr __no_init volatile union {
	struct {
		unsigned char T2_CH2_CAP: 2;
		unsigned char T2_CH2_MODE: 1;
                unsigned char T2_CH2_CMP: 3;
                unsigned char T2_CH2_IM: 1;
                unsigned char T2_CH2_RFIRQ: 1;
	};
} @ 0xE7;





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
	unsigned char DIR2_0: 1;
	unsigned char DIR2_1: 1;
	unsigned char DIR2_2: 1;
	unsigned char DIR2_3: 1;
	unsigned char DIR2_4: 1;
	unsigned char DIR2_5: 1;
	unsigned char DIR2_6: 1;
	unsigned char DIR2_7: 1;
} @ 0xFF;



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


__sfr __no_init volatile struct  {
	unsigned char P1SEL_0: 1;
	unsigned char P1SEL_1: 1;
	unsigned char P1SEL_2: 1;
	unsigned char P1SEL_3: 1;
	unsigned char P1SEL_4: 1;
	unsigned char P1SEL_5: 1;
	unsigned char P1SEL_6: 1;
	unsigned char P1SEL_7: 1;
} @ 0xF4;

__sfr __no_init volatile struct  {
	unsigned char P0ICON: 1;
	unsigned char P1ICONL: 1;
	unsigned char P1ICONH: 1;
	unsigned char P2ICON: 1;
	unsigned char PICTL_RESERVED3: 1;
	unsigned char PICTL_RESERVED2: 1;
	unsigned char PICTL_RESERVED1: 1;
	unsigned char PADSC: 1;
} @ 0x8C;





__sfr __no_init volatile struct  {
	unsigned char MDP0_0: 1;
	unsigned char MDP0_1: 1;
	unsigned char MDP0_2: 1;
	unsigned char MDP0_3: 1;
	unsigned char MDP0_4: 1;
	unsigned char MDP0_5: 1;
	unsigned char MDP0_6: 1;
	unsigned char MDP0_7: 1;
} @ 0x8F;

__sfr __no_init volatile struct  {
	unsigned char NOVALID__MDP1_0: 1;
	unsigned char NOVALID__MDP1_1: 1;
	unsigned char MDP1_2: 1;
	unsigned char MDP1_3: 1;
	unsigned char MDP1_4: 1;
	unsigned char MDP1_5: 1;
	unsigned char MDP1_6: 1;
	unsigned char MDP1_7: 1;
} @ 0xF6;



__sfr __no_init volatile struct  {
	unsigned char MDP2_0: 1;
	unsigned char MDP2_1: 1;
	unsigned char MDP2_2: 1;
	unsigned char MDP2_3: 1;
	unsigned char MDP2_4: 1;
	unsigned char PDUP0: 1;
	unsigned char PDUP1: 1;
	unsigned char PDUP2: 1;
} @ 0x8F;



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

#define PULLUP_DOWN_IMPL(A,B) MDP ## A ## _ ## B
#define PULLUP_DOWN(A,B) PULLUP_DOWN_IMPL( A,B)

#define PULL_UP_IMPL(A) PDUP ## A
#define PULL_UP(A) PULL_UP_IMPL(A) = 0
#define PULL_DOWN(A) PULL_UP_IMPL(A) = 1

#endif