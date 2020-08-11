/**
 * @file app.c
 * @author Justin_Thwaites
 * @date 4/23/2020
 * @brief This provides all of the helper functions that main.c will call
 *
 */


//***********************************************************************************
// Include files
//***********************************************************************************
#include "app.h"
#include "letimer.h"
#include "SI7021.h"


//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// global variables
//***********************************************************************************


//***********************************************************************************
// function
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *	Used to set up the Letimer through the clock tree and Letimer configurations
 *
 * @details
 *	This is a high-end function used to call the low level functions that set up the clock tree,
 *	 the pin routing and configures the Letimer for a PWM function
 *
 * @note
 *	This function only needs to be called once from main to allow for the setup of the Letimer.
 *
 ******************************************************************************/
void app_peripheral_setup(void){
	cmu_open();
	gpio_open();
	scheduler_open();
	sleep_open();
	app_letimer_pwm_open(PWM_PER, PWM_ACT_PER);
	si7021_i2c_open(SI7021_READ_EVT);
	add_scheduled_event(BOOT_UP_EVT);
}
/***************************************************************************//**
 * @brief
 *	This is the Letimer setup of gathering all of the macro defines into a struct.
 *
 * @details
 *	This function calls an instance of APP_LETIMER_PWM_TypeDef, which then all the values are inputed into the struct,
 *	and then the letimer_pwm_open function is called with this struct pointer. In essence this function confugres the
 *	data into a usable function.
 *
 * @note
 * 	This function is the proper way to call the initialization of the letimer.
 *
 *
 * @param[in] period The duration of each PWM wave in seconds.
 *
 *
 * @param[in] act_period The duration of the time where the PWM wave is high.
 *
 ******************************************************************************/
void app_letimer_pwm_open(float period, float act_period){
	// Initializing LETIMER0 for PWM operation by creating the
	// letimer_pwm_struct and initializing all of its elements
	APP_LETIMER_PWM_TypeDef letimer_pwm_struct;


	letimer_pwm_struct.active_period = act_period;
	letimer_pwm_struct.period = period;
	letimer_pwm_struct.enable = false;
	letimer_pwm_struct.out_pin_0_en = LETIMER0_OUT0_EN;
	letimer_pwm_struct.out_pin_1_en = LETIMER0_OUT1_EN;
	letimer_pwm_struct.out_pin_route0 = LETIMER0_ROUTE_OUT0;
	letimer_pwm_struct.out_pin_route1 = LETIMER0_ROUTE_OUT1;
	letimer_pwm_struct.debugRun = false;
	letimer_pwm_struct.comp0_evt = LETIMER0_COMP0_EVT;
	letimer_pwm_struct.comp0_irq_enable = DISABLE_IRQ;
	letimer_pwm_struct.comp1_evt = LETIMER0_COMP1_EVT;
	letimer_pwm_struct.comp1_irq_enable = DISABLE_IRQ;
	letimer_pwm_struct.uf_evt = LETIMER0_UF_EVT;
	letimer_pwm_struct.uf_irq_enable = ENABLE_IRQ;
	letimer_pwm_open(LETIMER0, &letimer_pwm_struct);
}

/***************************************************************************//**
 * @brief
 * This is the routine called by the scheduler when the uf event is triggered
 *
 *
 * @details
 * Removes the LETIMER_IF_UF event from the scheduler, then based on what the current blocked energy mode is,
 * it either unblocks the current mode and blocks the next lower energy mode, or if it is in EM4 blocking, it
 * will remove one of the two blockings and block EM0
 *
 * @note
 * This function is intended to be called by the scheduler
 *
 *
 ******************************************************************************/


void scheduled_letimer0_uf_evt(void){
	EFM_ASSERT(get_scheduled_events() & LETIMER0_UF_EVT);
	remove_scheduled_event(LETIMER0_UF_EVT);

	si7021_read_temp();
//	uint32_t current_mode = current_block_energy_mode();
//	sleep_unblock_mode(current_mode);
//	if(current_mode >= EM4){
//		sleep_block_mode(EM0);
//	}
//	else{
//		sleep_block_mode(++current_mode);
//	}

}

/***************************************************************************//**
 * @brief
 * This is the routine called by the scheduler when the comp0 event is triggered
 *
 *
 * @details
 * Removes the LETIMER_IF_COMP0 event from the scheduler.
 *
 * @note
 * This function should never be reached based on the current setup of the Pearl Gecko
 *
 *
 ******************************************************************************/

void scheduled_letimer0_comp0_evt(void){
	EFM_ASSERT(false);
	remove_scheduled_event(LETIMER0_COMP0_EVT);
}

/***************************************************************************//**
 * @brief
 * This is the routine called by the scheduler when the comp1 event is triggered
 *
 *
 * @details
 * Removes the LETIMER_IF_COMP1 event from the scheduler.
 *
 * @note
 * This function should never be reached based on the current setup of the Pearl Gecko
 *
 *
 ******************************************************************************/

void scheduled_letimer0_comp1_evt(void){
	EFM_ASSERT(false);
	remove_scheduled_event(LETIMER0_COMP1_EVT);
}

/***************************************************************************//**
 * @brief
 * This is the routine called by the scheduler when the si7021 event is triggered,
 * this then turns on the led if the read temperature is above 80 degrees Fahrenheit. This also will
 * Transmit the values to a connected bluetooth device.
 *
 *
 * @details
 * Removes the SI7021_READ_EVT event from the scheduler. And controls the LED
 *
 * @note
 * this function occurs every time a measurement is made
 *
 *
 ******************************************************************************/

void scheduled_si7021_done_evt(void){
	EFM_ASSERT(get_scheduled_events() & SI7021_READ_EVT);
	remove_scheduled_event(SI7021_READ_EVT);
	float temp;
	temp = si7021_i2c_data();
	if(temp >= 80){
		GPIO_PinOutSet(LED1_port, LED1_pin);
	}
	else{
		GPIO_PinOutClear(LED1_port, LED1_pin);
	}

	char temp_arr[16];
	if(ble_mode_celsius()){
		temp = (temp-32)*5/9;
		sprintf(temp_arr,"Temp = %4.1f C\n", temp);
	}
	else{
		//	gcvt(temp, 3, temp_val);
		//	sprintf(temp_arr,"Temp = %s F\n", temp_val);
		sprintf(temp_arr,"Temp = %4.1f F\n", temp);
	}


	ble_write(temp_arr);

}


/***************************************************************************//**
 * @brief
 *This function is setup up to contain values necessary for the boot up of the Pearl Gecko
 *
 * @details
 * This tests the circular buffer, then writes "\nHello World\n", "Circular Buffer Lab\n" and
 *  "Justin Thwaites\n" to the bluetooth device and if the test is enabled, it changes the
 *  name of the device to "JTBLE"
 *
 * @note
 * This function should only be called once from the while loop in main.c. If BLE_TEST_ENABLED then optimization settings must be -O0
 *
 ******************************************************************************/

void scheduled_boot_up_evt(void){
	EFM_ASSERT(get_scheduled_events() & BOOT_UP_EVT);
	remove_scheduled_event(BOOT_UP_EVT);
	ble_open(LEUART0_TX_DONE_EVT,LEUART0_RX_DONE_EVT);
	/* Call to start the LETIMER operation */
	#ifdef BLE_TEST_ENABLED
	EFM_ASSERT(ble_test("JTBLE"));
	for (int i = 0; i < 20000000; i++);
	#endif
	circular_buff_test();
	ble_write("\nHello World\n");
	ble_write("Circular Buffer Lab\n");
	ble_write("Justin Thwaites\n");

}

/***************************************************************************//**
 * @brief
 * This event occurs when the leuart0 has finished transmitting a message.
 *
 * @details
 * This function first makes sure it is called because of the event, then removes the event and turns on LETIMER0
 *
 * @note
 * If LETIMER0 is already turned on, the call to letimer start will rewrite to the command register, but have no overall effect
 * since the letimer0 is already on.
 *
 ******************************************************************************/
void leuart0_tx_done_evt(void){
	EFM_ASSERT(get_scheduled_events() & LEUART0_TX_DONE_EVT);
	remove_scheduled_event(LEUART0_TX_DONE_EVT);
	letimer_start(LETIMER0, true);
	ble_circ_pop(CIRC_OPER);
}

/***************************************************************************//**
 * @brief
 * This event occurs when the leuart0 has finished recieving a message.
 *
 * @details
 * This function first makes sure it is called because of the event, then removes the event and then calls a function to update the ble sending mode.
 *
 * @note
 * This will only change the mode if the message is the correct string, otherwise the update will not change the sending mode.
 *
 ******************************************************************************/
void leuart0_rx_done_evt(void){
	EFM_ASSERT(get_scheduled_events() & LEUART0_RX_DONE_EVT);
	remove_scheduled_event(LEUART0_RX_DONE_EVT);
	ble_update_mode();
}
