/***************************************************************************//**
 * @file main.c
 * @author Justin_Thwaites
 * @date 2/13/2020
 * @brief Simple LED Blink Demo for SLSTK3402A
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

/* System include statements */
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

/* Silicon Labs include statements */
#include "em_device.h"
#include "em_chip.h"
#include "em_emu.h"
#include "em_letimer.h"
#include "bsp.h"
#include "letimer.h"

/* The developer's include statements */
#include "main.h"
#include "app.h"


int main(void)
{
  EMU_DCDCInit_TypeDef dcdcInit = EMU_DCDCINIT_DEFAULT;
  CMU_HFXOInit_TypeDef hfxoInit = CMU_HFXOINIT_DEFAULT;

  /* Chip errata */
  CHIP_Init();

  /* Init DCDC regulator and HFXO with kit specific parameters */
  /* Init DCDC regulator and HFXO with kit specific parameters */
  /* Initialize DCDC. Always start in low-noise mode. */
  EMU_EM23Init_TypeDef em23Init = EMU_EM23INIT_DEFAULT;
  EMU_DCDCInit(&dcdcInit);
  em23Init.vScaleEM23Voltage = emuVScaleEM23_LowPower;
  EMU_EM23Init(&em23Init);
  CMU_HFXOInit(&hfxoInit);

  /* Switch HFCLK to HFRCO and disable HFXO */
  CMU_HFRCOBandSet(cmuHFRCOFreq_26M0Hz);// Set main CPU osc. to 26MHz
  CMU_OscillatorEnable(cmuOsc_HFRCO, true, true);
  CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFRCO);
  CMU_OscillatorEnable(cmuOsc_HFXO, false, false);

  /* Call application program to open / initialize all required peripheral */
  app_peripheral_setup();



  //make sure event is correct
  EFM_ASSERT(get_scheduled_events() & BOOT_UP_EVT);
  while (1) {
//	  EMU_EnterEM1();
//	  EMU_EnterEM2(true);
	  if (!(get_scheduled_events()&(LETIMER0_UF_EVT | LETIMER0_COMP0_EVT | LETIMER0_COMP1_EVT|BOOT_UP_EVT))) enter_sleep();
	  if(get_scheduled_events() & BOOT_UP_EVT){
		  scheduled_boot_up_evt();
	  }
	  if(get_scheduled_events() & LETIMER0_UF_EVT){
		  scheduled_letimer0_uf_evt();
	  }
	  if(get_scheduled_events() & LETIMER0_COMP0_EVT){
		  scheduled_letimer0_comp0_evt();
	  }
	  if(get_scheduled_events() & LETIMER0_COMP1_EVT){
		  scheduled_letimer0_comp1_evt();
	  }
	  if(get_scheduled_events() & SI7021_READ_EVT){
		  scheduled_si7021_done_evt();
	  }
	  if(get_scheduled_events() & LEUART0_TX_DONE_EVT){
		  leuart0_tx_done_evt();
	  }
	  if(get_scheduled_events() & LEUART0_RX_DONE_EVT){
		  leuart0_rx_done_evt();
	  }

  }
}
