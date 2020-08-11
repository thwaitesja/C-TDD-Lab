/**
 * @file cmu.c
 * @author Justin_Thwaites
 * @date 2/13/2020
 * @brief This contains all of the clock tree setup
 *
 */
//***********************************************************************************
// Include files
//***********************************************************************************
#include "cmu.h"

//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// global variables
//***********************************************************************************


//***********************************************************************************
// prototypes
//***********************************************************************************
/***************************************************************************//**
 * @brief
 *	Sets up the clock tree for low energy function
 *
 * @details
 *	This is used to disable all oscillators except the ULFRCO oscillator, which is then routed to the
 *	low frequency clock tree, which is likewise enabled.
 *
 * @note
 *	This function only needs to be called once from main to allow for setup of the clock tree, it does not need to be called multiple times.
 *
 ******************************************************************************/
void cmu_open(void){

		CMU_ClockEnable(cmuClock_HFPER, true);

		// By default, LFRCO is enabled, disable the LFRCO oscillator

		CMU_OscillatorEnable(cmuOsc_LFRCO, false, false);	// using LFXO or ULFRCO
		// LEUART Enabling clock tree
		CMU_OscillatorEnable(cmuOsc_LFXO, true, true);
		CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO);
		// Route LF clock to the LF clock tree
		// No requirement to enable the ULFRCO oscillator.  It is always enabled in EM0-4H

//		CMU_OscillatorEnable(cmuOsc_LFXO, false, false);		// Disable LFXO
		CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_ULFRCO);	// route ULFRCO to proper Low Freq clock tree

		CMU_ClockEnable(cmuClock_CORELE, true);					// Enable the Low Freq clock tree

		// Enabling the High Frequency Peripheral Clock Tree
		// Enable the High Frequency Peripheral clock

		CMU_ClockEnable(cmuClock_HFPER, true);
		CMU_ClockPrescSet(cmuClock_HFPER, 0 );



}

