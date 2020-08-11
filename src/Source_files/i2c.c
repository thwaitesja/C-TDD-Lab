/**
 * @file i2c.c
 * @author Justin_Thwaites
 * @date 2/13/2020
 * @brief This provides all of routing of outputs to various peripherals
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************

#include "i2c.h"
#include "em_cmu.h"

static I2C_PAYLOAD payload;



/***************************************************************************//**
 * @brief
 *   Driver to open an set an I2C peripheral.
 *
 * @details
 * 	 This routine is the low level setup of the i2c that sets all of the nessesary values in the i2c registers.
 * 	 This enables the correct clock tree and then will call the i2c init function with the proper input, and then
 * 	  this will enable the interrupts for the specific peripheral.
 *
 * @note
 *   This function is normally called once to initialize the peripheral and the
 *   function this does not start any transmission, it only will setup the peripheral.
 *
 * @param[in] i2c
 *   Pointer to the base peripheral address of the I2C peripheral being opened
 *
 * @param[in] i2c_setup
 *   Is the STRUCT that the calling routine will use to set the parameters for proper I2C
 *   operation
 *
 *   @param[in] i2c_io
 *   Is the STRUCT that the calling routine will use to reset the i2c and provides any gpio pins the i2c must use.
 *
 ******************************************************************************/


void i2c_open(I2C_TypeDef *i2c, I2C_OPEN_STRUCT *i2c_setup, I2C_IO_STRUCT *i2c_io){
	if(i2c == I2C0){
		CMU_ClockEnable(cmuClock_I2C0, true);
	}
	else if(i2c == I2C1){
		CMU_ClockEnable(cmuClock_I2C1, true);
	}
	else{
		EFM_ASSERT(false);
	}

	if ((i2c->IF & 0x01) == 0) {
		i2c->IFS = 0x01;
		EFM_ASSERT(i2c->IF & 0x01);
		i2c->IFC = 0x01;
	}
	else {
		i2c->IFC = 0x01;
		EFM_ASSERT(!(i2c->IF & 0x01));
	}

	payload.event = i2c_setup->event;

	I2C_Init_TypeDef i2c_start_values;

	i2c_start_values.clhr = i2c_setup->clhr;
	i2c_start_values.enable = i2c_setup->enable;
	i2c_start_values.freq = i2c_setup->freq;
	i2c_start_values.master = i2c_setup->master;
	i2c_start_values.refFreq = i2c_setup->refFreq;

	I2C_Init(i2c, &i2c_start_values);
	i2c->ROUTELOC0 = i2c_setup->SDA_RouteLoc0 | i2c_setup->SCL_RouteLoc0;

	i2c->ROUTEPEN = (i2c_setup->SCL_Enable *I2C_ROUTEPEN_SCLPEN )\
			|(i2c_setup->SDA_Enable *I2C_ROUTEPEN_SDAPEN );


	i2c_bus_reset(i2c, i2c_io);
	//Initializes the Interrupts
	I2C_IntClear(i2c, (I2C_IEN_ACK)|(I2C_IEN_NACK)|(I2C_IEN_MSTOP));
	i2c->RXDATA; //reads the RX data to clear the RX_Data interrupt
		//enables only the desired interrupts
	I2C_IntEnable(i2c, (I2C_IEN_ACK)|(I2C_IEN_NACK)|(I2C_IEN_MSTOP)|(I2C_IEN_RXDATAV));

//	Enable the correct IRQ
	if(i2c == I2C0){
		NVIC_EnableIRQ(I2C0_IRQn);
	}
	else if(i2c == I2C1){
		NVIC_EnableIRQ(I2C1_IRQn);
	}
	else{
		EFM_ASSERT(false);
	}


}
/***************************************************************************//**
 * @brief
 *   Resets the i2c and all peripherals that are using the i2c bus.
 *
 * @details
 * 	 This routine checks to makes sure that the bus is not being used, and then
 * 	  toggles the bus to reset any peripherals being used on it.
 *
 * @note
 *   This function is called from the i2c_open function.
 *
 * @param[in] i2c
 *   Pointer to the base peripheral address of the I2C peripheral being opened
 *
 * @param[in] io_ports
 * Is the STRUCT that contains the gpio pins that must be toggled to get a bus reset.
 *
 ******************************************************************************/
void i2c_bus_reset(I2C_TypeDef* i2c, I2C_IO_STRUCT* io_ports){
	EFM_ASSERT(GPIO_PinInGet(io_ports->SCL_port, io_ports->SCL_pin));
	EFM_ASSERT(GPIO_PinInGet(io_ports->SDA_port, io_ports->SDA_pin));
//	holds SDA High
//  each loop toggles SCL first high to low, then low to high
	for(int i = 0; i < 18; i++){
		GPIO_PinOutToggle(io_ports->SCL_port, io_ports->SCL_pin);
	}
	GPIO_PinOutSet(io_ports->SCL_port, io_ports->SCL_pin);
	i2c->CMD = I2C_CMD_ABORT;
}

/***************************************************************************//**
 * @brief
 *   ACK handling for the I2C.
 *
 * @details
 * 	 Utilizes a case statement in order to makes sure the i2c is in the correct
 * 	 state and then progresses through the ladder flow chart or calls an EFM assert if in the wrong state.
 *
 * @note
 *   This function is called by the irq handler.
 *
 ******************************************************************************/
static void i2c_ack(){

switch(payload.i2c_state){
	case INITIALIZE:
//		EFM_ASSERT(payload.i2c_state == INITIALIZE);
		payload.i2c_state = MEASURE;
		payload.i2c->TXDATA = payload.cmd;
		break;

	case MEASURE:
//		EFM_ASSERT(payload.i2c_state == MEASURE);
		payload.i2c_state = RESTART;
		payload.i2c->CMD = I2C_CMD_START;
		payload.i2c->TXDATA = (payload.device_address << 1) | READ;
		break;

	case RESTART:
//		EFM_ASSERT(payload.i2c_state == RESTART);
		payload.i2c_state = MSB_LISTEN;
		break;

	case MSB_LISTEN:
		EFM_ASSERT(false);
		break;

	case LSB_LISTEN:
		EFM_ASSERT(false);
		break;

	case STOP:
		EFM_ASSERT(false);
		break;
	default:
			EFM_ASSERT(false);
		break;
}
}

/***************************************************************************//**
 * @brief
 *   NACK handling for the I2C.
 *
 * @details
 * 	 Utilizes a case statement in order to makes sure the i2c is in the correct
 * 	 state and then progresses through the ladder flow chart or calls an EFM assert if in the wrong state.
 *
 * @note
 *   This function is called by the irq handler.
 *
 ******************************************************************************/
static void i2c_nack(){
	switch(payload.i2c_state){
		case INITIALIZE:
			EFM_ASSERT(false);
			break;

		case MEASURE:
			EFM_ASSERT(false);
			break;

		case RESTART:
//			EFM_ASSERT(payload.i2c_state == RESTART);
			payload.i2c_state = RESTART;
			payload.i2c->CMD = I2C_CMD_START;
			payload.i2c->TXDATA = (payload.device_address << 1) | READ;
			break;

		case LSB_LISTEN:
			EFM_ASSERT(false);
			break;

		case MSB_LISTEN:
			EFM_ASSERT(false);
			break;

		case STOP:
			EFM_ASSERT(false);
			break;
		default:
			EFM_ASSERT(false);
			break;
	}
}

/***************************************************************************//**
 * @brief
 *   RXDATAV handling for the I2C.
 *
 * @details
 * 	 Utilizes a case statement in order to makes sure the i2c is in the correct
 * 	 state and then progresses through the ladder flow chart or calls an EFM assert if in the wrong state.
 * 	  This pulls the data from the rxdata register into a private variable.
 *
 * @note
 *   This function is called by the irq handler.
 *
 ******************************************************************************/
static void i2c_rxdatav(){
	switch(payload.i2c_state){
		case INITIALIZE:
			EFM_ASSERT(false);
			break;

		case MEASURE:
			EFM_ASSERT(false);
			break;

		case RESTART:
			EFM_ASSERT(false);
			break;

		case MSB_LISTEN:
//			EFM_ASSERT(payload.i2c_state == MSB_LISTEN);
			*(payload.data) = payload.i2c->RXDATA << 8;
			payload.i2c->CMD = I2C_CMD_ACK;
			payload.i2c_state = LSB_LISTEN;
			break;

		case LSB_LISTEN:
//			EFM_ASSERT(payload.i2c_state == LSB_LISTEN);
			*(payload.data) |= payload.i2c->RXDATA;// keeps only one byte
			payload.i2c_state = STOP;
			payload.i2c->CMD = I2C_CMD_NACK;
			payload.i2c->CMD = I2C_CMD_STOP;
			break;

		case STOP:
			EFM_ASSERT(false);
			break;
		default:
			EFM_ASSERT(false);
			break;
	}
}

/***************************************************************************//**
 * @brief
 *   MSTOP handling for the I2C.
 *
 * @details
 * 	 Utilizes a case statement in order to makes sure the i2c is in the final
 * 	 state and then progresses through the ladder flow chart or calls an EFM assert if in the wrong state.
 * 	  This unblocks the sleep mode for the I2C and sets an event flag.
 *
 * @note
 *   This function is called by the irq handler.
 *
 ******************************************************************************/
static void i2c_mstop(){
	switch(payload.i2c_state){
		case INITIALIZE:
			EFM_ASSERT(false);
			break;

		case MEASURE:
			EFM_ASSERT(false);
			break;

		case RESTART:
			EFM_ASSERT(false);
			break;

		case LSB_LISTEN:
			EFM_ASSERT(false);
			break;

		case MSB_LISTEN:
			EFM_ASSERT(false);
			break;

		case STOP:
			payload.i2c_state = INITIALIZE;
//			while(!((payload.i2c->STATE & _I2C_STATE_STATE_MASK) == I2C_STATE_STATE_IDLE));
			add_scheduled_event(payload.event);
			sleep_unblock_mode(I2C_EM_BLOCK);
			break;
		default:
			EFM_ASSERT(false);
			break;
	}
}

/***************************************************************************//**
 * @brief
 *   ISR handler for the I2C0
 *
 * @details
 * 	 Sets the interrupt flag and then clears all interrupts. Then based on what interrupts are triggered, it
 * 	 will call the designated function.
 *
 * @note
 *   This function is called through an I2C0 interrupt.
 *
 ******************************************************************************/
void I2C0_IRQHandler(void){
	uint32_t int_flag;
	int_flag = I2C0->IF & I2C0->IEN;
	I2C0->IFC = int_flag;
	if(int_flag & I2C_IF_RXDATAV){
//		EFM_ASSERT(!(I2C0->IF & I2C_IF_RXDATAV));
		i2c_rxdatav();
	}
	if(int_flag & I2C_IF_ACK){
//		ACK ISR
		EFM_ASSERT(!(I2C0->IF & I2C_IF_ACK));
		i2c_ack();
	}
	if(int_flag & I2C_IF_NACK){
//		NACK ISR
		EFM_ASSERT(!(I2C0->IF & I2C_IF_NACK));
		i2c_nack();
	}
	if(int_flag & I2C_IF_MSTOP){
//		MSTOP ISR
		EFM_ASSERT(!(I2C0->IF & I2C_IF_MSTOP));
		i2c_mstop();
	}
}

/***************************************************************************//**
 * @brief
 *   ISR handler for the I2C1
 *
 * @details
 * 	 Sets the interrupt flag and then clears all interrupts. Then based on what interrupts are triggered, it
 * 	 will call the designated function.
 *
 * @note
 *   This function is called through an I2C1 interrupt.
 *
 ******************************************************************************/
void I2C1_IRQHandler(void){
	uint32_t int_flag;
	int_flag = I2C1->IF & I2C1->IEN;
	I2C1->IFC = int_flag;
	if(int_flag & I2C_IF_RXDATAV){
//		EFM_ASSERT(!(I2C1->IF & I2C_IF_RXDATAV));
		i2c_rxdatav();
	}
	if(int_flag & I2C_IF_ACK){
//		ACK ISR
		EFM_ASSERT(!(I2C1->IF & I2C_IF_ACK));
		i2c_ack();
	}
	if(int_flag & I2C_IF_NACK){
//		NACK ISR
		EFM_ASSERT(!(I2C1->IF & I2C_IF_NACK));
		i2c_nack();
	}
	if(int_flag & I2C_IF_MSTOP){
//		MSTOP ISR
		EFM_ASSERT(!(I2C1->IF & I2C_IF_MSTOP));
		i2c_mstop();
	}
}


/***************************************************************************//**
 * @brief
 *   Starts the I2C transmission to a peripheral.
 *
 * @details
 * 	 Blocks the sleep mode of the I2C, and then sends the start command and the slave
 * 	 address shifted and including a write bit.
 *
 * @note
 *   This function is called once each time the software ladder flow chart occurs to take a measure.
 *
 ******************************************************************************/

void i2c_start(I2C_PAYLOAD_INIT* param){
	EFM_ASSERT((param->i2c->STATE & _I2C_STATE_STATE_MASK) == I2C_STATE_STATE_IDLE);
	sleep_block_mode(I2C_EM_BLOCK);
	payload.device_address = param->device_address;
	payload.data = param->data;
	payload.i2c_state = INITIALIZE;
	payload.read = param->read;
	payload.bytes = param->bytes;
	payload.cmd = param->cmd;
	payload.i2c = param->i2c;



	payload.i2c->CMD = I2C_CMD_START;
	payload.i2c->TXDATA = payload.device_address << 1 | WRITE;



}

