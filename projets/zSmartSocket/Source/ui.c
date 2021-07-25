#include "ui.h"
#include "lcd.h"
#include "Events.h"
#include "EventManager.h"
#include "bl0937.h"
#include "OSAL_Timers.h"
#include "OnBoard.h"


#define EXPECTED_POWER  220.0
#define EXPECTED_CURRENT 1.0

#define MAIN_MENU_CONTEXT 1
#define CALIBRATING_CONTEXT 2

void (* shortPress0)(uint16 event);

extern const GFXfont FreeMono9pt7b;
extern const GFXfont Font5x7Fixed;


static void startUI(void);
static void (* displayMenu0)(void);
static void displayMenu1(void);
static void BL0937_calibrate();
static void longPress0(uint16 events);

static void  BL0937_calibrate(void);
static void shortPressMenu1(uint16 events);
static void longPressMenu1(uint16 events);
static void calibrating(void);
static void calibratingWaitPress(uint16 events);

static uint8_t selectedRow;
static uint8_t calibratingCount=0;
static uint32_t calibratingCFTot=0;
static uint32_t calibratingCF1Tot=0;

void initUI(void (* menu0)(void), void (* shortPress)(uint16 event)){
  shortPress0 = shortPress;
  displayMenu0 = menu0;
  addEventCB(PRESS_BUTTON_BIT,shortPress0);  
  addEventCB(PRESS_BUTTON_LONG_BIT,longPress0);  
}

void startUI(void) {
  setContext(1);
  addEventCB(PRESS_BUTTON_BIT,shortPressMenu1);  
  addEventCB(PRESS_BUTTON_LONG_BIT,longPressMenu1);  
  displayMenu1();
}

static void displayMenu1(void){
  clean_c(0,0,displayWidth, displayHeight, 1);
  
  setFont(&Font5x7Fixed);
  setCursor_c(1,10,1);
  drawText_c("UN-JOIN DEVICE",1,selectedRow == 0); 
  setCursor_c(1,20,1);
  drawText_c("CALIBRATION",1,selectedRow == 1);  
  setCursor_c(1,30,1);
  drawText_c("EXIT",1,selectedRow == 2);  

  display_c(1);
}

void longPress0(uint16 events){
  startUI();
}

static void longPressMenu1(uint16 events){
  switch(selectedRow){
  case 1:
    setContext(2);
    BL0937_calibrate();
    break;
  case 2:
    setContext(0);
    clean_c(0,0,displayWidth, displayHeight, 0);
    displayMenu0();
    addEventCB(PRESS_BUTTON_BIT,shortPress0);  
    addEventCB(PRESS_BUTTON_LONG_BIT,longPress0);  
    break;
  }
}

static void shortPressMenu1(uint16 events){
  selectedRow++;
  if (selectedRow > 2)
    selectedRow=0;
  displayMenu1();
}

#define CALIBRATING_SAMPLES 10

void BL0937_calibrate() {
  clean_c(0,0,displayWidth, displayHeight, 2);
  setFont(&Font5x7Fixed);
  setCursor_c(1,10,2);
  drawText_c("Calibrating, please wait",2,0); 
  calibratingCFTot=getCFMean();
  calibratingCF1Tot=getCF1Mean();
  calibratingCount=1;
  setCursor_c(3,20,2);
  drawText_c(".",2,0);
  osal_start_timerEx_cb(2000, &calibrating);
  display_c(2);
}

void calibrating(void) {
  char buffer[30];
  if (calibratingCount < CALIBRATING_SAMPLES){
    calibratingCFTot += getCFMean();
    calibratingCF1Tot += getCF1Mean();
    calibratingCount++;
    setCursor_c(3+(calibratingCount-1)*10,20,CALIBRATING_CONTEXT);
    drawText_c(".",CALIBRATING_CONTEXT,0);
    osal_start_timerEx_cb(2000, &calibrating);
    display_c(CALIBRATING_CONTEXT);
    return;
  }
  
  setCursor_c(1,30,CALIBRATING_CONTEXT);
  drawText_c("Calibration finished",CALIBRATING_CONTEXT,0); 
  display_c(CALIBRATING_CONTEXT);
  double coefPower = calibratingCFTot/(double)CALIBRATING_SAMPLES * EXPECTED_POWER;
  double coefCurrent = calibratingCF1Tot/(double)CALIBRATING_SAMPLES * EXPECTED_CURRENT;
  setCursor_c(1,40,CALIBRATING_CONTEXT);
  drawText_c("Power counting: ", CALIBRATING_CONTEXT, 0);
  _itoa(calibratingCFTot/CALIBRATING_SAMPLES, (uint8_t*)buffer, 10);
  drawText_c(buffer, CALIBRATING_CONTEXT, 0);
  setCursor_c(1,47,CALIBRATING_CONTEXT);
  drawText_c("Current counting: ", CALIBRATING_CONTEXT, 0);
  _itoa(calibratingCF1Tot/CALIBRATING_SAMPLES, (uint8_t*)buffer, 10);
  drawText_c(buffer, CALIBRATING_CONTEXT, 0);
  display_c(2);
  saveConvertCoeff(coefPower, coefCurrent*1000);
  
  setCursor_c(1,60,2);
  drawText_c("Press to continue",2,0);
  display_c(2);
  addEventCB(PRESS_BUTTON_BIT,calibratingWaitPress);  
}

void calibratingWaitPress(uint16_t event) {
  startUI();
}

