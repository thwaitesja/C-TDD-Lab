/**
 * @file SI7021.c
 * @author Justin_Thwaites
 * @date 3/2/2020
 * @brief This provides all of routing of outputs to various peripherals
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************
#include "SI7021.h"


static uint32_t si7021_data;

/***************************************************************************//**
 * @brief
 *	Sets up the default values of the i2c for using the si7021.
 *
 * @details
 *	Uses values that are predefined in SI7021.h to correctly pass in the all structs to initialize the i2c
 *
 * @note
 *	This function only needs to be called once the call happens from the app_peripherial setup.
 *
 * @param[in] evt
 * The specification of what event should be scheduled by the si7021 completing its i2c flowchart. This makes sure that the
 * scope of the si7021 is not including functions from app.c
 ******************************************************************************/
void si7021_i2c_open(uint32_t evt){
	I2C_IO_STRUCT i2c_io;
	I2C_OPEN_STRUCT i2c_open_values;

//	si7021_data = 0;

	i2c_io.SCL_pin 	= SI7021_SCL_PIN;
	i2c_io.SCL_port	= SI7021_SCL_PORT;
	i2c_io.SDA_pin	= SI7021_SDA_PIN;
	i2c_io.SDA_port	= SI7021_SDA_PORT;

	i2c_open_values.SCL_Enable = SI7021_SCL_EN;
	i2c_open_values.SCL_RouteLoc0 = SI7021_SCL_LOC;
	i2c_open_values.SDA_Enable = SI7021_SDA_EN;
	i2c_open_values.SDA_RouteLoc0 = SI7021_SDA_LOC;
	i2c_open_values.clhr = SI7021_I2C_CLK_RATIO;
	i2c_open_values.enable = true;
	i2c_open_values.freq = SI7021_I2C_FREQ;
	i2c_open_values.master = true;
	i2c_open_values.refFreq = SI7021_REFFREQ;

	i2c_open_values.event = evt;

	i2c_open(SI7021_I2C, &i2c_open_values, &i2c_io);

}

/***************************************************************************//**
 * @brief
 * Returns the temperature data from the private variable stored in si7021 in Fahrenheit
 *
 * @details
 *	Uses the documentation from the SI7021 to read the temperature into a Celsius float, and then returns this value as a Fahrenheit float value
 *
 * @note
 *	This call must happen after the I2C communication from the sensor has already received the data and put it in the private variable.
 *
 ******************************************************************************/

float si7021_i2c_data(){
float Celsius;
Celsius = (175.72 * si7021_data)/65536.0 - 46.85;
return (9.0/5.0)*Celsius + 32;
}

/***************************************************************************//**
 * @brief
 * Pulls temperature data from the SI7021 into a private variable for use.
 *
 * @details
 *	Sets values for I2C protocol in a software state machine in order to use I2C to get 2 bytes from the SI7021
 *
 * @note
 *	This call must happen before any information can be pulled from the private variable to be used in application code.
 *
 ******************************************************************************/

void si7021_read_temp(){
	I2C_PAYLOAD_INIT temp_read;
	temp_read.data = &si7021_data;
	temp_read.bytes = SI7021_BYTES;
	temp_read.i2c = SI7021_I2C;
	temp_read.device_address = Si7021_dev_addr;
	temp_read.cmd = SI7021_TEMP_NO_HOLD;
//	temp_read.event = SI7021_READ_EVT;
	i2c_start(&temp_read);
}

