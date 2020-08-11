/**
 * @file sleep_routines.c
 * @author Justin Thwaites
 * @date 2/13/2020
 * @brief Contains all the Sleep Routines functions
 *
 */
/*************************************************************************
*
* @file sleep.c
**************************************************************************
*
* @section License
* <b>(C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
**************************************************************************
*
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*
* DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Silicon Labs has no
* obligation to support this Software. Silicon Labs is providing the
* Software "AS IS", with no express or implied warranties of any kind,
* including, but not limited to, any implied warranties of merchantability
* or fitness for any particular purpose or warranties against infringement
* of any proprietary rights of a third party.
*
* Silicon Labs will not be liable for any consequential, incidental, or
* special damages, or any other relief, or for any claim by any third party,
* arising from your use of this Software.
*
*************************************************************************/
#include "sleep_routines.h"

//***********************************************************************************
// private variables
//***********************************************************************************
static unsigned int lowest_energy_mode[MAX_ENERGY_MODES];

//***********************************************************************************
// functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *   Initializes the sleep settings
 *
 * @details
 * 	 Initializes the Private array values all to 0
 *
 * @note
 *   This function is normally called once to initialize the values and then it should remain balanced where each new peripheral used adds one
 *   to the index of the energy mode below where it can go, and each peripherial when turned off, subtracts one from the energy mode index below where it can go.
 *
 ******************************************************************************/
void sleep_open(void){
	lowest_energy_mode[EM0] =0;
	lowest_energy_mode[EM1] =0;
	lowest_energy_mode[EM2] =0;
	lowest_energy_mode[EM3] =0;
	lowest_energy_mode[EM4] =0;
}


/***************************************************************************//**
 * @brief
 *   Used to block an energy mode.
 *
 * @details
 * adds one to the value at the specific index of the array.
 *
 * @note
 *   The inputed value must be a valid index below the length of the array.
 *
 * @param[in] EM
 *   Energy mode that needs to be blocked.
 *
 *
 ******************************************************************************/
void sleep_block_mode(uint32_t EM){
	__disable_irq();
	if(EM < MAX_ENERGY_MODES){
		lowest_energy_mode[EM]++;
	}
	EFM_ASSERT (lowest_energy_mode[EM] < 10);
	__enable_irq();
}

/***************************************************************************//**
 * @brief
 *   Used to unblock an energy mode.
 *
 * @details
 * subtracts one to the value at the specific index of the array.
 *
 * @note
 *   The inputed value must be a valid index below the length of the array. Just because it is being unblocked does not mean this mode can be used
 *   There may be a second peripheral that also has this energy mode blocked.
 *
 * @param[in] EM
 *   Energy mode that needs to be released from being blocked.
 *
 *
 ******************************************************************************/
void sleep_unblock_mode(uint32_t EM){
	__disable_irq();
	if(EM < MAX_ENERGY_MODES){
		lowest_energy_mode[EM]--;
	}
	EFM_ASSERT (lowest_energy_mode[EM] >= 0);
	__enable_irq();
}

/***************************************************************************//**
 * @brief
 *   based on the blocked sleep levels, this function chooses the proper energy mode and enters it.
 *
 * @details
 * 	 starting at the 0th element, the function checks each element of the array and only sleeps once it finds a filled element, or if there are no values
 * 	 past EM3 then the processor enters EM3
 *
 * @note
 *   Once entered into an energy mode, the processor will wake with an interrupt and return out of this function.
 *
 ******************************************************************************/

void enter_sleep(void){
if (lowest_energy_mode[EM0] > 0){
	return;
}
if (lowest_energy_mode[EM1] > 0){
	return;
}
if (lowest_energy_mode[EM2] > 0){
	EMU_EnterEM1();
	return;
}
if (lowest_energy_mode[EM3] > 0){
	EMU_EnterEM2(true);
	return;
}

EMU_EnterEM3(true);
return;

}

/***************************************************************************//**
 * @brief
 *   Returns the current blocked energy mode.
 *
 * @details
 * 	 Loops through the energy modes from 0 up and if it find a non zero element, it returns it, otherwise it returns the highest possible energy mode.
 *
 * @note
 *   While this occurs with an infinite while loop, the iterator i can only increase and will eventually lead to the exit of the loop, for the pearl gecko
 *   this will happen in under 6 loops, where at the 6th loop, i is at the top value possible.
 *
 * @return
 * 	This function will return a unsigned int with the current blocked energy mode.
 *
 ******************************************************************************/
uint32_t current_block_energy_mode(void){
	uint32_t i=0;
	while(true){
		if(i< MAX_ENERGY_MODES){
			if(lowest_energy_mode[i]!=0){
				return i;
			}
			else{
				i++;
			}
		}
		else{
			return MAX_ENERGY_MODES -1;
		}
	}

}
