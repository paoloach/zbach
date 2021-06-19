#include <ioCC2530.h>
#include "comdef.h"
#include "hal_types.h"
#include "regs.h"
#include "i2c.h"


#define SCK PORT(SCK_PORT, SCK_PIN)
#define SDA PORT(SDA_PORT, SDA_PIN)


#define START PORT(SDA_PORT, SDA_PORT)=0 
              

static void startI2c(void);
static uint8 write(uint8 byte);

#define WAIT1us()  TIMIF = TIMIF & 0xE7; while ( (TIMIF & 0x10) == 0);

void initI2c(void){
  FUNCTION_SEL(SCK_PORT, SCK_PIN)=0;
  FUNCTION_SEL(SDA_PORT, SDA_PORT)=0;

  //PULLUP_DOWN(SCK_PORT, SCK_PIN)=0;
  PULL_UP(SCK_PORT);
  PULLUP_DOWN(SDA_PORT, SDA_PIN)=0;
  PULL_UP(SDA_PORT);

  DIR(SDA_PORT, SDA_PIN)=1;
  DIR(SCK_PORT, SCK_PIN)=1;
  
  SCK=1;
  SDA=1;
}


static void startI2c(void) {
  SDA=0;
  T4CNT=0;
  T4CTL = 2;
  T4CC0=31;
  T4CC1=31;
  T4CCTL0=0x14;
  TIMIF = TIMIF & 0xE7;
  T4CTL |= 0x10;
  while ( (TIMIF & 0x10) == 0);
  SCK=0;
}


void sendI2c(uint8 * data, uint8  dataLen) {
  startI2c();
  for(uint8 i=0; i < dataLen; i++){
    if (write(data[i]) != 0){
      break;
    };
  }
  SDA = 0;
  WAIT1us();
  SCK=1;
  WAIT1us();
  SDA=1;
}


void fillI2cData(uint8 address, uint8 cmd,  uint8 * data, uint16 dataLen){
  startI2c();
  if (write(address) != 0){
    SDA = 0;
    WAIT1us();
    SCK=1;
    WAIT1us();
    SDA=1;
    return;
  }
  if (write(cmd) != 0){
    SDA = 0;
    WAIT1us();
    SCK=1;
    WAIT1us();
    SDA=1;
    return;
  }
  for(uint16 i=0; i < dataLen; i++){
    if (write(data[i]) != 0){
      break;
    };
  }
  SDA = 0;
  WAIT1us();
  SCK=1;
  WAIT1us();
  SDA=1;
}

static uint8 write(uint8 data){
  uint8 ack;
  for(uint8 i=0; i < 8; i++){
    WAIT1us();
    if (data & 0x80){
      SDA=1;
    } else {
      SDA=0;
    }
    WAIT1us();
    SCK=1;
    WAIT1us();
    SCK=0;
    data = data << 1;
  }
  WAIT1us();
  DIR(SDA_PORT, SDA_PIN)=0;
  WAIT1us();
  SCK=1;
  WAIT1us();
  ack = SDA;
  SCK=0;
  WAIT1us();
  DIR(SDA_PORT, SDA_PIN)=1;
  return ack;
}

void writeByte(uint8 data) {
  uint8 i=0;
  startI2c();
  
  for(i=0; i < 8; i++){
    WAIT1us();
    if (data & 0x80){
      SDA=1;
    } else {
      SDA=0;
    }
    WAIT1us();
    SCK=1;
    WAIT1us();
    SCK=0;
    data = data << 1;
  }
  WAIT1us();
  SDA=0;
  SCK=1;
  WAIT1us();
  SDA=1;
}


