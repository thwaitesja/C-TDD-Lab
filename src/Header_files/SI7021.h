/*
 * SI7021.h
 *
 *  Created on: Feb 25, 2020
 *      Author: Justin Thwaites
 */
#ifndef SRC_HEADER_FILES_SI7021_H_
#define SRC_HEADER_FILES_SI7021_H_

//***********************************************************************************
// Include files
//***********************************************************************************

#include "gpio.h"
#include "i2c.h"

//***********************************************************************************
// defined files
//***********************************************************************************
#define 	Si7021_dev_addr				0x40
#define 	SI7021_TEMP_NO_HOLD			0xF3
#define 	SI7021_I2C_FREQ				I2C_FREQ_FAST_MAX//400000 //Hz
#define 	SI7021_I2C_CLK_RATIO		i2cClockHLRAsymetric
#define 	SI7021_SCL_LOC				I2C_ROUTELOC0_SCLLOC_LOC15
#define 	SI7021_SCL_EN				true
#define 	SI7021_SDA_LOC				I2C_ROUTELOC0_SDALOC_LOC15
#define 	SI7021_SDA_EN				true
#define		SI7021_I2C					I2C0
#define		SI7021_REFFREQ				0

#define		SI7021_BYTES				2





//***********************************************************************************
// global variables
//***********************************************************************************


//***********************************************************************************
// function prototypes
//***********************************************************************************

void si7021_i2c_open(uint32_t evt);
float si7021_i2c_data();
void si7021_read_temp();


#endif /* SRC_HEADER_FILES_SI7021_H_ */
