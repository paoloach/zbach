
/**************************************************************************************************

 DESCRIPTION:
  --

 CREATED: 10/02/2015, by Paolo Achdjian

 FILE: OSAL_Timers2.c

***************************************************************************************************/


/*********************************************************************
 * INCLUDES
 */

#include "comdef.h"
#include "OnBoard.h"
#include "OSAL.h"
#include "OSAL_Timers.h"
#include "hal_timer.h"
#include "lcd-SSD1306.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

struct TimerRec {
  int32 timeout;
  uint16 eventFlag;
  uint8  taskId;
  uint32 reloadTimeout;
  void (* callback)(void);
};

/*********************************************************************
 * GLOBAL VARIABLES
 */

#define MAXELEMENTS 20
static struct TimerRec timerRec[MAXELEMENTS];
static struct TimerRec * end = timerRec+MAXELEMENTS;

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
// Milliseconds since last reboot
static uint32 osal_systemClock;

/*********************************************************************
 * LOCAL FUNCTION PROTOTYPES
 */
static struct TimerRec * getFreeElement(void);
static struct TimerRec * osalAddTimer( uint8 task_id, uint16 event_flag, uint32 timeout );
static struct TimerRec * osalAddTimer_cb(uint32 timeout_value, void (* callback)(void) );
static struct TimerRec * osalFindTimer( uint8 task_id, uint16 event_flag );
static struct TimerRec * osalFindTimerCb( void (* callback)(void) );


/*********************************************************************
 * FUNCTIONS
 *********************************************************************/


static struct TimerRec * getFreeElement(void) {
  struct TimerRec * iter = timerRec;

  for(;iter != end; iter++){
    if (iter->eventFlag == 0 && iter->callback == NULL){
      return iter;
    }
  }
  return NULL;
}

/*********************************************************************
 * @fn      osalTimerInit
 *
 * @brief   Initialization for the OSAL Timer System.
 *
 * @param   none
 *
 * @return
 */
void osalTimerInit( void ){
  osal_systemClock = 0;
  osal_memset(timerRec, 0, sizeof(struct TimerRec)*MAXELEMENTS);
}

/*********************************************************************
 * @fn      osalAddTimer
 *
 * @brief   Add a timer to the timer list.
 *          Ints must be disabled.
 *
 * @param   task_id
 * @param   event_flag
 * @param   timeout
 *
 * @return  osalTimerRec_t * - pointer to newly created timer
 */
struct TimerRec * osalAddTimer( uint8 task_id, uint16 event_flag, uint32 timeout ){
 	// Look for an existing timer first
	struct TimerRec * iter;
 	iter = osalFindTimer( task_id, event_flag );
 	if ( iter ) {
		iter->timeout = timeout;
		return iter;
	} else {
		iter = getFreeElement();
		if ( iter ) {
			iter->taskId = task_id;
			iter->eventFlag = event_flag;
			iter->timeout = timeout;
			iter->reloadTimeout = 0;
                        iter->callback=NULL;
			return iter;
		} else {
			return NULL;
		}
	}
}


struct TimerRec * osalAddTimer_cb(uint32 timeout_value, void (* callback)(void) ){
  struct TimerRec * iter;
  
  
      setCursor(50,63);
      clean(50,54,60 ,63);
      drawText("1");
      display();
 
  iter = osalFindTimerCb(callback);
  if(iter != NULL){
    iter->timeout = timeout_value;

      setCursor(50,63);
      clean(50,54,60 ,63);
      drawText("0");
      display();    

    return iter;
  } else {
    
    iter = getFreeElement();
      setCursor(50,63);
      clean(50,54,60 ,63);
      drawText("0");
      display();  
    if ( iter ) {
      iter->taskId = -1;
      iter->eventFlag = 0;
      iter->timeout = timeout_value;
      iter->callback = callback;
      iter->reloadTimeout = 0;
      return iter;
    } else {
      return NULL;
    }
  }
}

/*********************************************************************
 * @fn      osalFindTimer
 *
 * @brief   Find a timer in a timer list.
 *          Ints must be disabled.
 *
 * @param   task_id
 * @param   event_flag
 *
 * @return  index of elements or -1 if not found
 */
struct TimerRec *  osalFindTimer( uint8 task_id, uint16 event_flag ){
  struct TimerRec * iter;
  iter = timerRec;	
  for(;iter != end; iter++){
    if (iter->taskId == task_id && iter->eventFlag == event_flag){
      return iter;
    }
  }
  return NULL;
}


struct TimerRec *  osalFindTimerCb( void (* callback)(void) ){
  struct TimerRec *  iter;
  iter = timerRec;	
  for(;iter != end; iter++){
    if (iter->callback == callback){
       return iter;
    }
  }
  return NULL;
}

/*********************************************************************
 * @fn      osal_start_timerEx
 *
 * @brief
 *
 *   This function is called to start a timer to expire in n mSecs.
 *   When the timer expires, the calling task will get the specified event.
 *
 * @param   uint8 taskID - task id to set timer for
 * @param   uint16 event_id - event to be notified with
 * @param   uint32 timeout_value - in milliseconds.
 *
 * @return  SUCCESS, or NO_TIMER_AVAIL.
 */
uint8 osal_start_timerEx( uint8 taskID, uint16 event_id, uint32 timeout_value ){
  struct TimerRec * iter;
  halIntState_t intState;
  HAL_ENTER_CRITICAL_SECTION( intState );  // Hold off interrupts.

  iter = osalAddTimer( taskID, event_id, timeout_value );

  HAL_EXIT_CRITICAL_SECTION( intState );   // Re-enable interrupts.

  return ( (iter != NULL) ? SUCCESS : NO_TIMER_AVAIL );
}

/*********************************************************************
 * @fn      osal_start_timerEx
 *
 * @brief
 *
 *   This function is called to start a timer to expire in n mSecs.
 *   When the timer expires, the calling task will get the specified event.
 *
 * @param   uint8 taskID - task id to set timer for
 * @param   uint16 event_id - event to be notified with
 * @param   uint32 timeout_value - in milliseconds.
 *
 * @return  SUCCESS, or NO_TIMER_AVAIL.
 */
uint8 osal_start_timerEx_cb(uint32 timeout_value, void (* callback)(void) ){
      setCursor(40,63);
      clean(40,54,50 ,63);
      drawText("1");
      display();
      
  struct TimerRec * iter;
  halIntState_t intState;
  HAL_ENTER_CRITICAL_SECTION( intState );  // Hold off interrupts.
 
  iter = osalAddTimer_cb(timeout_value, callback);
 
  HAL_EXIT_CRITICAL_SECTION( intState );   // Re-enable interrupts.

  
      setCursor(40,63);
      clean(40,54,50 ,63);
      drawText("0");
      display();
  
 return ( (iter != NULL) ? SUCCESS : NO_TIMER_AVAIL );
}
/*********************************************************************
 * @fn      osal_start_reload_timer
 *
 * @brief
 *
 *   This function is called to start a timer to expire in n mSecs.
 *   When the timer expires, the calling task will get the specified event
 *   and the timer will be reloaded with the timeout value.
 *
 * @param   uint8 taskID - task id to set timer for
 * @param   uint16 event_id - event to be notified with
 * @param   UNINT16 timeout_value - in milliseconds.
 *
 * @return  SUCCESS, or NO_TIMER_AVAIL.
 */
uint8 osal_start_reload_timer( uint8 taskID, uint16 event_id, uint32 timeout_value ){
        halIntState_t intState;	
        struct TimerRec * iter;
	HAL_ENTER_CRITICAL_SECTION( intState );  // Hold off interrupts.

	// Add timer
 	iter = osalAddTimer( taskID, event_id, timeout_value );
 	if ( iter ){
 		iter->reloadTimeout = timeout_value;
 	}

	HAL_EXIT_CRITICAL_SECTION( intState );   // Re-enable interrupts.

	return ( (iter != NULL) ? SUCCESS : NO_TIMER_AVAIL );
}

uint8 osal_start_reload_timer_cb(uint32 timeout_value, void (* callback)(void) ){
  halIntState_t intState;
  struct TimerRec * iter;
  HAL_ENTER_CRITICAL_SECTION( intState );  // Hold off interrupts.
 
  iter = osalAddTimer_cb(timeout_value, callback);
  if ( iter ){
          iter->reloadTimeout = timeout_value;
  }
 
  HAL_EXIT_CRITICAL_SECTION( intState );   // Re-enable interrupts.

  return ( (iter != NULL) ? SUCCESS : NO_TIMER_AVAIL );
}


/*********************************************************************
 * @fn      osal_stop_timerEx
 *
 * @brief
 *
 *   This function is called to stop a timer that has already been started.
 *   If ZSUCCESS, the function will cancel the timer and prevent the event
 *   associated with the timer from being set for the calling task.
 *
 * @param   uint8 task_id - task id of timer to stop
 * @param   uint16 event_id - identifier of the timer that is to be stopped
 *
 * @return  SUCCESS or INVALID_EVENT_ID
 */
uint8 osal_stop_timerEx( uint8 task_id, uint16 event_id ){
        halIntState_t intState;
	struct TimerRec * iter;
 	HAL_ENTER_CRITICAL_SECTION( intState );  // Hold off interrupts.

 	iter = osalFindTimer( task_id, event_id );
 	if ( iter ){
 		iter->eventFlag=0;
	}

 	HAL_EXIT_CRITICAL_SECTION( intState );   // Re-enable interrupts.

	return ( (iter != NULL) ? SUCCESS : INVALID_EVENT_ID );
}




uint8 osal_stop_timerEx_cb(  void (* callback)(void)  ){
  struct TimerRec * iter;
  halIntState_t intState;
  HAL_ENTER_CRITICAL_SECTION( intState );  // Hold off interrupts.

  iter = timerRec;	
  for(;iter != end; iter++){
    if (iter->callback == callback){
      iter->callback = NULL;
      iter->eventFlag=0;
    }
  }

  HAL_EXIT_CRITICAL_SECTION( intState );   // Re-enable interrupts.

  return SUCCESS;
}

/*********************************************************************
 * @fn      osal_get_timeoutEx
 *
 * @brief
 *
 * @param   uint8 task_id - task id of timer to check
 * @param   uint16 event_id - identifier of timer to be checked
 *
 * @return  Return the timer's tick count if found, zero otherwise.
 */
uint32 osal_get_timeoutEx( uint8 task_id, uint16 event_id ){
	struct TimerRec * iter;
 	uint32 rtrn = 0;
        halIntState_t intState;
 	
        HAL_ENTER_CRITICAL_SECTION( intState );  // Hold off interrupts.

 	iter = osalFindTimer( task_id, event_id );

 	if ( iter ){
		rtrn = iter->timeout;
 	}

 	HAL_EXIT_CRITICAL_SECTION( intState );   // Re-enable interrupts.

	return rtrn;
}

/*********************************************************************
 * @fn      osal_timer_num_active
 *
 * @brief
 *
 *   This function counts the number of active timers.
 *
 * @return  uint8 - number of timers
 */
uint8 osal_timer_num_active( void ){
	struct TimerRec * iter;
	uint8 numTimers = 0;
        halIntState_t intState;
 	HAL_ENTER_CRITICAL_SECTION( intState );  // Hold off interrupts.

	iter = timerRec;	
	for(;iter != end; iter++){
		if (iter->eventFlag || iter->callback){
			numTimers++;
		}
	}

	HAL_EXIT_CRITICAL_SECTION( intState );   // Re-enable interrupts.

	return numTimers;
}

/*********************************************************************
 * @fn      osalTimerUpdate
 *
 * @brief   Update the timer structures for a timer tick.
 *
 * @param   none
 *
 * @return  none
 *********************************************************************/
void osalTimerUpdate( uint32 updateTime ){
  halIntState_t intState;
  struct TimerRec * iter;
  iter = timerRec;
      setCursor(10,63);
      clean(10,54,20 ,63);
      drawText("1");
      display();
  HAL_ENTER_CRITICAL_SECTION( intState );  
  osal_systemClock += updateTime;
  HAL_EXIT_CRITICAL_SECTION( intState );   
  void (*callback)(void);

 	// Look for open timer slot
  for (;iter != end; iter++){
    if (iter->eventFlag == 0 && iter->callback == NULL)
            continue;
    HAL_ENTER_CRITICAL_SECTION( intState );  
    iter->timeout -= updateTime;
    if (iter->timeout <= 0){
      iter->timeout=0;
      callback = NULL;
      if (iter->callback){
        callback = iter->callback;
      } else {
        setCursor(20,63);
        clean(20,54,30 ,63);
        drawText("1");
        display();
        osal_set_event( iter->taskId, iter->eventFlag );
        setCursor(20,63);
        clean(20,54,30 ,63);
        drawText("0");
        display();
      }
      if (iter->reloadTimeout){
        iter->timeout = iter->reloadTimeout;
      } else {
        iter->eventFlag = 0;
        iter->callback=NULL;
      }
      if (callback != NULL){
          setCursor(30,63);
          clean(30,54,40 ,63);
          drawText("1");
          display();
        HAL_EXIT_CRITICAL_SECTION( intState );   
        callback();
        HAL_ENTER_CRITICAL_SECTION( intState ); 
          setCursor(30,63);
          clean(30,54,40 ,63);
          drawText("0");
          display();
      }
    }
    HAL_EXIT_CRITICAL_SECTION( intState );   
  }
      setCursor(10,63);
      clean(10,54,20 ,63);
      drawText("0");
      display();
}

#ifdef POWER_SAVING
/*********************************************************************
 * @fn      osal_adjust_timers
 *
 * @brief   Update the timer structures for elapsed ticks.
 *
 * @param   none
 *
 * @return  none
 *********************************************************************/
void osal_adjust_timers( void ){
	 //Deprecated for CC2530 and CC2430 SoC.
	/*
	uint32 eTime;

 	if ( timerHead != NULL ) {
		// Compute elapsed time (msec)
 		eTime = TimerElapsed() / TICK_COUNT;

		if ( eTime ){
			osalTimerUpdate( eTime );
		}
	}
	*/
}

/*********************************************************************
 * @fn      osal_next_timeout
 *
 * @brief
 *
 *   Search timer table to return the lowest timeout value. If the
 *   timer list is empty, then the returned timeout will be zero.
 *
 * @param   none
 *
 * @return  none
 *********************************************************************/
uint32 osal_next_timeout( void ){
	uint32 nextTimeout;
	struct TimerRec * iter = timerRec;
	iter = timerRec;	
	
	nextTimeout=OSAL_TIMERS_MAX_TIMEOUT;
	for(;iter != end; iter++){
		if (iter->eventFlag==0 && iter->callback == NULL)
			continue;
		if (iter->timeout < nextTimeout){
			nextTimeout = iter->timeout;
		}
	}

  	return nextTimeout;
}
#endif // POWER_SAVING

/*********************************************************************
 * @fn      osal_GetSystemClock()
 *
 * @brief   Read the local system clock.
 *
 * @param   none
 *
 * @return  local clock in milliseconds
 */
uint32 osal_GetSystemClock( void ){
	return ( osal_systemClock );
}

/*********************************************************************
*********************************************************************/
