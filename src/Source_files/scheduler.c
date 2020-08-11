/**
 * @file scheduler.c
 * @author Justin Thwaites
 * @date 2/13/2020
 * @brief Contains all the Scheduler functions
 *
 */

#include "scheduler.h"
#include "em_emu.h"
#include "em_assert.h"

//***********************************************************************************
// private variables
//***********************************************************************************
static unsigned int event_scheduled;

//***********************************************************************************
// Functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *   Sets there to be no events scheduled, in essence initializing the scheduler
 *
 * @details
 * 	Sets the private variable event_scheduled equal to 0
 *
 * @note
 *   This is called to fully clear the scheduler, or to initialize the scheduler.
 *
 ******************************************************************************/
void scheduler_open(void){
	__disable_irq();
	event_scheduled = EVENT_RESET;
	__enable_irq();
}

/***************************************************************************//**
 * @brief
 *   Used to add an event to the scheduler.
 *
 * @details
 *  Bitwise ORs any flag bits passed through add_scheduled_event
 *
 * @note
 *   For this current code, only the first three bits are used as specified events
 *
 * @param[in] event
 *   an unsigned int where each of the 32 bits represents one event that if the bit is true, needs to be scheduled
 *
 *
 ******************************************************************************/
void add_scheduled_event(uint32_t event){
	__disable_irq();
	event_scheduled |= event;
	__enable_irq();
}

/***************************************************************************//**
 * @brief
 *   Used to remove an event to the scheduler.
 *
 * @details
 *  Bitwise ANDs with the negation of the flag bits passed through remove_scheduled_event, which causes the flagged positions to become 0.
 *
 * @note
 *   For this current code, only the first three bits are used as specified events.
 *
 * @param[in] event
 *   an unsigned int where each of the 32 bits represents one event that if the bit is true, needs to be scheduled
 *
 *
 ******************************************************************************/

void remove_scheduled_event(uint32_t event){
	__disable_irq();
	event_scheduled &= ~event;
	__enable_irq();
}
/***************************************************************************//**
 * @brief
 *   Returns the events needing to occur.
 *
 * @details
 * 	 Returns the private variable event_scheduled
 *
 * @note
 *   For this current code, only the first three bits are used as specified events.
 *
 * @return
 * 	This function will return a unsigned int with the current scheduled events.
 *
 ******************************************************************************/

uint32_t get_scheduled_events(void){
	return event_scheduled;
}
