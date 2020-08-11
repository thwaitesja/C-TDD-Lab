#ifndef __I2C_H__
#define __I2C_H__

//***********************************************************************************
// Include files
//***********************************************************************************
#include "em_i2c.h"
#include "em_gpio.h"
#include "sleep_routines.h"
#include "scheduler.h"

//***********************************************************************************
// defined files
//***********************************************************************************

// LED0 pin is
#define	a 0
#define READ 1
#define WRITE 0
#define I2C_EM_BLOCK EM2
//***********************************************************************************
// global variables
//***********************************************************************************
typedef struct {
	I2C_ClockHLR_TypeDef 	clhr;
	bool 					enable;
	uint32_t 				freq;
	bool 					master;
	uint32_t 				refFreq;
	bool      				SDA_Enable;
	bool      				SCL_Enable;
	uint32_t  				SDA_RouteLoc0;
	uint32_t  				SCL_RouteLoc0;
	uint32_t				event;

} I2C_OPEN_STRUCT;


typedef struct {

GPIO_Port_TypeDef 	SDA_port;
unsigned int 		SDA_pin;
GPIO_Port_TypeDef 	SCL_port;
unsigned int 		SCL_pin;

}I2C_IO_STRUCT;

typedef enum{
	INITIALIZE,
	MEASURE,
	RESTART,
	MSB_LISTEN,
	LSB_LISTEN,
	STOP
} I2C_States;

typedef struct {
	I2C_States			i2c_state;
	I2C_TypeDef			*i2c;
	uint32_t			cmd;
	bool 				read;
	uint32_t			*data;
	uint32_t			device_address;
	uint32_t			bytes;
	uint32_t			event;
}I2C_PAYLOAD;

typedef struct {
	I2C_TypeDef			*i2c;
	uint32_t			cmd;
	bool 				read;
	uint32_t			*data;
	uint32_t			device_address;
	uint32_t			bytes;
}I2C_PAYLOAD_INIT;
//***********************************************************************************
// function prototypes
//***********************************************************************************

void i2c_open(I2C_TypeDef *i2c, I2C_OPEN_STRUCT *i2c_setup, I2C_IO_STRUCT *i2c_io);
void i2c_bus_reset();
void I2C0_IRQHandler(void);
void I2C1_IRQHandler(void);
void i2c_start(I2C_PAYLOAD_INIT* param);



#endif
