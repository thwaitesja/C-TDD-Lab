/**
 * @file gpio.c
 * @author Justin_Thwaites
 * @date 2/13/2020
 * @brief This provides all of routing of outputs to various peripherals
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************
#include "gpio.h"
#include "em_cmu.h"

//***********************************************************************************
// defined files
//***********************************************************************************



//***********************************************************************************
// global variables
//***********************************************************************************


//***********************************************************************************
// function prototypes
//***********************************************************************************


//***********************************************************************************
// functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *   Driver to open an set the gpio pins for the pearl gecko
 *
 * @details
 * 	 This routine is a low enabling function that sets the paths on the board inorder to
 * 	 drive the outputs.
 *
 * @note
 *   This function is normally called once to initialize the peripheral and then is no longer needed besides for this setup.
 *
 *
 ******************************************************************************/
void gpio_open(void){

	CMU_ClockEnable(cmuClock_GPIO, true);

	// Set LED ports to be standard output drive with default off (cleared)
	GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthStrongAlternateStrong);
//	GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthWeakAlternateWeak);
	GPIO_PinModeSet(LED0_port, LED0_pin, gpioModePushPull, LED0_default);
	GPIO_DriveStrengthSet(LED1_port, gpioDriveStrengthStrongAlternateStrong);
//	GPIO_DriveStrengthSet(LED1_port, gpioDriveStrengthWeakAlternateWeak);
	GPIO_PinModeSet(LED1_port, LED1_pin, gpioModePushPull, LED1_default);


// Enable the GPIO for Temp I2C
	GPIO_DriveStrengthSet(SI7021_SENSOR_EN_PORT, gpioDriveStrengthWeakAlternateWeak);
	GPIO_PinModeSet(SI7021_SENSOR_EN_PORT, SI7021_SENSOR_EN_PIN, gpioModePushPull, SI7021_EN_DEFAULT);
//	GPIO_DriveStrengthSet(SI7021_SCL_PORT, gpioDriveStrengthWeakAlternateWeak);
	GPIO_PinModeSet(SI7021_SCL_PORT, SI7021_SCL_PIN, gpioModeWiredAnd, SI7021_SCL_DEFAULT);
//	GPIO_DriveStrengthSet(SI7021_SDA_PORT, gpioDriveStrengthWeakAlternateWeak);
	GPIO_PinModeSet(SI7021_SDA_PORT, SI7021_SDA_PIN, gpioModeWiredAnd, SI7021_SDA_DEFAULT);

// Enable the GPIO for UART COMMUNICATION
	GPIO_DriveStrengthSet(LEUART_TX_PORT, gpioDriveStrengthStrongAlternateWeak);
	GPIO_PinModeSet(LEUART_TX_PORT, LEUART_TX_PIN, gpioModePushPull, LEUART_TX_DEFAULT);
	GPIO_PinModeSet(LEUART_RX_PORT, LEUART_RX_PIN, gpioModeInput, LEUART_RX_DEFAULT);



}
