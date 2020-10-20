#include "nwk.h"
#include "OSAL.h"
#include "OSAL_Timers.h"

#define NETWORK_EVENT_POOL 1

void nwk_set_poll_evt(uint16 time);

void nwk_set_poll_evt(uint16 time){
  if (_NIB.beaconOrder == 0)
    return;
  if (_NIB.nwkState == NWK_ROUTER){
    return;
  }
  if (_NIB.nwkState != NWK_ENDDEVICE){
    return;
  }
  osal_start_timerEx(NWK_TaskID, NETWORK_EVENT_POOL, (uint32)time);
}