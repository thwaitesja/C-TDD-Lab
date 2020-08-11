//***********************************************************************************
// Include files
//***********************************************************************************
#ifndef GPIO_H
#define	GPIO_H

#include "em_gpio.h"

//***********************************************************************************
// defined files
//***********************************************************************************

// LED0 pin is
#define	LED0_port		gpioPortF
#define LED0_pin		04u
#define LED0_default	false 	// off
// LED1 pin is
#define LED1_port		gpioPortF
#define LED1_pin		05u
#define LED1_default	false	// off
// I2C Temp
#define SI7021_SCL_PORT 		gpioPortC
#define SI7021_SCL_PIN  		11u
#define SI7021_SCL_DEFAULT 		true
// I2C0_SCL #15 configuration
#define SI7021_SDA_PORT 		gpioPortC
#define SI7021_SDA_PIN			10u
#define SI7021_SDA_DEFAULT 		true
// I2C0_SDA #15 configuration
#define SI7021_SENSOR_EN_PORT 	gpioPortB
#define SI7021_SENSOR_EN_PIN 	10u
#define SI7021_EN_DEFAULT  		true
// LEUART # 18 Configuration
#define LEUART_TX_PORT 			gpioPortD
#define LEUART_TX_PIN 			10u
#define LEUART_TX_DEFAULT  		true
#define LEUART_RX_PORT 			gpioPortD
#define LEUART_RX_PIN 			11u
#define LEUART_RX_DEFAULT  		true
//***********************************************************************************
// global variables
//***********************************************************************************


//***********************************************************************************
// function prototypes
//***********************************************************************************
void gpio_open(void);

#endif
