/**
 * @file leuart.c
 * @author Justin Thwaites
 * @date April 23, 2020
 * @brief Contains all the functions of the LEUART peripheral
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************

//** Standard Library includes
#include <string.h>

//** Silicon Labs include files
#include "em_gpio.h"
#include "em_cmu.h"

//** Developer/user include files
#include "leuart.h"

//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// private variables
//***********************************************************************************
static uint32_t	rx_done_evt;
static uint32_t	tx_done_evt;


static LEUART_PAYLOAD payload;

/***************************************************************************//**
 * @brief LEUART driver
 * @details
 *  This module contains all the functions to support the driver's state
 *  machine to transmit a string of data across the LEUART bus.  There are
 *  additional functions to support the Test Driven Development test that
 *  is used to validate the basic set up of the LEUART peripheral.  The
 *  TDD test for this class assumes that the LEUART is connected to the HM-18
 *  BLE module.  These TDD support functions could be used for any TDD test
 *  to validate the correct setup of the LEUART.
 *
 ******************************************************************************/

//***********************************************************************************
// Private functions
//***********************************************************************************



//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 * 	Called as an IRQ handler for the RXDATAV interrupt.
 *
 * @details
 *	This function only operates if in receive data state or on reset state. If in the receive data
 *	state, the character is written and index increased. If in the Reset state, this means the piece
 *	of data is the signal frame: ";", so there is a full reset of the state machine and RX Block is enabled again.
 *
 * @note
 * 	The scheduled rx_done_event is added as well if in the reset state.
 *
 *******************************************************************************/
void leuart_rxdatav(){

	switch(payload.rx_state){
		case 	WAIT:
			EFM_ASSERT(false);
			break;

		case RECIEVE_DATA:
			payload.rx_message[payload.rx_index++] = payload.leuart->RXDATA;
			break;

		case RESET: // called after receiving a signal frame. This is to clear any remaining data and reset the state machine.
			payload.rxbusy = false;
			payload.leuart->RXDATA; // clears excess data (sig frame character)
			payload.leuart->CMD = LEUART_CMD_RXBLOCKEN;
			payload.rx_state = WAIT;
			while(payload.leuart->SYNCBUSY);

			add_scheduled_event(rx_done_evt);
			break;

		default:
			EFM_ASSERT(false);
			break;
	}

}
/***************************************************************************//**
 * @brief
 *	Called as an IRQ handler for the Sigframe interrupt.
 *
 * @details
 *	If there is a sig frame interrupt this makes sure that it is only in the Receive data state, and when it is
 *	in the receive data state and it receives the interrupt it shifts the message over to get rid of the start character,
 *	and it adds a null character.
 *
 * @note
 *	This interrupt should only happen in the receive data state.
 *******************************************************************************/
void leuart_sigf(){
	LEUART_IntClear(payload.leuart, LEUART_IFC_SIGF);
	switch(payload.rx_state){
			case WAIT:
				EFM_ASSERT(false);
				break;

			case RECIEVE_DATA:
				payload.rx_state = RESET;
				payload.rx_message[payload.rx_index] = 0;
				strcpy(payload.rx_message, payload.rx_message+1); //removes the first character of the string ">message" to "message"
				LEUART_IntClear(payload.leuart, LEUART_IFC_STARTF);
				LEUART_IntEnable(payload.leuart, LEUART_IEN_STARTF);
				LEUART_IntDisable(payload.leuart, LEUART_IEN_SIGF);
				break;

			case RESET:
				EFM_ASSERT(false);
				break;

			default:
				EFM_ASSERT(false);
				break;
		}
}

/***************************************************************************//**
 * @brief
 *	Called as an IRQ handler for the Start frame interrupt.
 *
 * @details
 *	If there is a start frame interrupt, this ensures it is in the waiting state,
 *	This turns on the rx busy, switches to the receive data state and then enables the sig frame interrupt
 *	and disables the start Frame interrupt.
 *
 * @note
 *	This will not prevent the start frame from being in the RX Buffer, this is taken care of when the sig frame interrupt is received.
 *******************************************************************************/
void leuart_startf(){
	LEUART_IntClear(payload.leuart, LEUART_IFC_STARTF);
	switch(payload.rx_state){
		case 	WAIT:
			payload.rx_state = RECIEVE_DATA;
			payload.rxbusy = true;
			LEUART_IntClear(payload.leuart, LEUART_IFC_SIGF);
			LEUART_IntDisable(payload.leuart, LEUART_IEN_STARTF);
			LEUART_IntEnable(payload.leuart, LEUART_IEN_SIGF);
			payload.rx_index = 0;
			break;

		case RECIEVE_DATA:
			EFM_ASSERT(false);
			break;

		case RESET:
			EFM_ASSERT(false);
			break;

		default:
			EFM_ASSERT(false);
			break;
	}

}
/***************************************************************************//**
 * @brief
 *	This returns the rx_message
 *
 * @details
 *	This returns the character pointer that is the start of the rx_message array.
 *
 * @note
 * 	This function is meant to allow outside functions to access private data
 *
 *******************************************************************************/
char* leuart_rxmessage(void){
	return payload.rx_message;
}
/***************************************************************************//**
 * @brief	This function acts as the test driven development to make sure that the RX buffer is setup
 * in a proper fashion to make the code function properly. This code is made to help insure that the
 * functions governing the rx of the Pearl Gecko's LEUART will work as expected and reach the full scope
 * of receiving messages from the bluetooth connection.
 *
 * @details	These functions build off of the already built LEUART start driver, and use polling to insure
 * that the full message has been transmitted before checking to see if the proper transmission has occurred.
 * This function tests to make sure that the start frame and end frames are being registered properly, as well
 * as that the null character is written after the message
 *
 * The defined start frame is ">"
 * The defined signal frame is ";"
 *
 * This function tests these capabilities in this order:
 * - The ability of the LEUART to send characters to itself and not have the characters be read by the state machine without start or sig frames
 * - The ability of the LEUART to send a message to itself with proper sig and start characters and have the message be copied over
 * - The ability of the LEUART to send a message to itself with proper sig and start characters and have the null character be properly placed at the end of the message.
 * - The ability of the LEUART to receive a signal frame without a start frame and have the pearl gecko ignore the input.
 * - The ability to receive a start frame and part of a message and test to make sure the rx is still busy/ waiting for an signal frame.
 * - The ability to receive a start frame and after a small delay receive a signal frame and have the rx no longer busy.
 * - The ability to receive 2 start characters in a row and have the second be treated as a normal character.
 * - The ability to receive a message with sig and start frames and only keep the data within the start and sig frames.
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to test, this gives the test the ability to test different peripherals if more that one LEUART is being used.
 *
 *
 * @note	This function should only be called once and is meant to check to make sure that all desired parts of the
 * RX driver have been properly setup. This is called at the end of LEUART_OPEN() to make sure the LEUART RX has been setup
 * properly after being initialized.
 *
 *******************************************************************************/

void leuart_rxtest(LEUART_TypeDef *leuart){

	char myarr1[16] = "hello world";
	char myarr2[10] = "asdfg";
	char sendarr2[10] = ">asdfg;";
	char myarr3[10] = "123";
	char sendarr3[10] = ">123;";
	char myarr4[10] = ">123";
	char sendarr4[10] = ">>123;";
	char myarr5[10] = "123";
	char sendarr5[10] = ">123;4567";

	strcpy(payload.rx_message, myarr1);
	leuart->CTRL |= LEUART_CTRL_LOOPBK;
	while(leuart->SYNCBUSY);

	//This test makes sure that the message is not overwritten without the start frame
	//two bytes are sent to ensure that the rx starts receiving before the tx is done transmitting.
	leuart_start(leuart, "aa", 1);
	while(payload.txbusy);
	while(payload.rxbusy);
	for(int i = 0; i < strlen(myarr1); i++ ){
		EFM_ASSERT(payload.rx_message[i] == myarr1[i]);
	}

	//This test ensures that the message is properly received
	// ">asdfg;" is sent, and then the test makes sure that rx message contains "asdfg"
	leuart_start(leuart, sendarr2, strlen(sendarr2));
	while(payload.txbusy);
	while(payload.rxbusy);
	for(int i = 0; i < strlen(myarr2); i++ ){
		EFM_ASSERT(payload.rx_message[i] == myarr2[i]);
	}

	//This test ensures that the message has a null character at the end
	//This test sends ">123;" and makes sure the character array has rx_message[3] = "\0"
	//testing the array is ['1','2','3','\0','g','\0', ...] not ['1','2','3','f','g','\0', ...]
	leuart_start(leuart, sendarr3, strlen(sendarr3));
	while(payload.txbusy);
	while(payload.rxbusy);
	EFM_ASSERT(payload.rx_message[strlen(myarr3)] == 0);

	//This test ensures that the signal frame only impacts the state machine if a start frame occurs first.
	//Since no Start Frame has been sent before this the message should still be what was sent before: "123"
	leuart_start(leuart, ";a", 1);
	while(payload.txbusy);
	while(payload.rxbusy);
	for(int i = 0; i < strlen(myarr3); i++ ){
		EFM_ASSERT(payload.rx_message[i] == myarr3[i]);
	}


	//This test ensures that the message RX will wait until it receives a signal frame before stopping
	//There is a timer delay of 2 milliseconds to ensure that the rx has received the '0' character and is still waiting/ busy.
	leuart_start(leuart, ">123456789", 10);
	while(payload.txbusy);
	timer_delay(2); // gives enough time to ensure that the final character of the above sent message is received
	EFM_ASSERT(payload.rxbusy);


	//This test ensures that the message stops after the signal frame
	// Two characters are sent to make sure that the rx starts receiving the message before the tx completes.
	leuart_start(leuart, "0;", 2);
	while(payload.txbusy);
	while(payload.rxbusy);
	EFM_ASSERT(!payload.rxbusy);

	//This test ensures that the start frame character after a start frame already being received is treated like a normal character
	//this test sends the string ">>123;" and makes sure that ">123" is in the rx message location.
	leuart_start(leuart, sendarr4, strlen(sendarr4));
	while(payload.txbusy);
	while(payload.rxbusy);
	for(int i = 0; i < strlen(myarr4); i++ ){
		EFM_ASSERT(payload.rx_message[i] == myarr4[i]);
	}

	//This test ensures that the message in the array is only "123\0" and not continuing past the sigframe ie "123;4567"
	//this test sends the message ">123;4567" tests the array is ['1','2','3','\0', ...] not ['1','2','3', ';', '4', '5', '6', '7' ...]
	//or any similar array.
	leuart_start(leuart, sendarr5, strlen(sendarr5));
	while(payload.txbusy);
	while(payload.rxbusy);
	for(int i = 0; i < strlen(myarr5); i++ ){
		EFM_ASSERT(payload.rx_message[i] == myarr5[i]);
	}
	EFM_ASSERT(payload.rx_message[strlen(myarr5)] == 0);

	remove_scheduled_event(rx_done_evt);
	leuart->CTRL &= ~LEUART_CTRL_LOOPBK;
	while(leuart->SYNCBUSY);
}

/***************************************************************************//**
 * @brief
 *	This function allows for the setup of the rx of the LEUART.
 *
 * @details
 *	This function enables the start frame buffer unblock, blocks the RX buffer,
 *	sets the start frame to '>' and the sig frame to ';'. Then This function enables the RXDATAV and STARTF interrupt.
 *
 * @note
 * This function should be called once from the LEUART open function, and does not need to be called again.
 *
 *******************************************************************************/

void leuart_rxsetup(LEUART_TypeDef *leuart){
	sleep_block_mode(LEUART_RX_EM);
	leuart->CTRL |= LEUART_CTRL_SFUBRX;
	while(leuart->SYNCBUSY);
	leuart->CMD = LEUART_CMD_RXBLOCKEN;
	while(leuart->SYNCBUSY);
	leuart->STARTFRAME = RX_STARTFRAME;
	while(leuart->SYNCBUSY);
	leuart->SIGFRAME = RX_SIGFRAME;
	while(leuart->SYNCBUSY);
	LEUART_IntClear(leuart, LEUART_IFC_STARTF);
	LEUART_IntEnable(leuart, LEUART_IEN_RXDATAV|LEUART_IEN_STARTF);
	//	LEUART_IntClear(leuart, LEUART_IFC_SIGF|LEUART_IFC_STARTF);
	//	LEUART_IntEnable(leuart, LEUART_IEN_RXDATAV|LEUART_IEN_SIGF|LEUART_IEN_STARTF);
	payload.rxbusy = false;
	payload.leuart = leuart;
}
/***************************************************************************//**
 * @brief
 * This function sets up the leuart peripheral in order to have all of the proper values and defaults setup
 *
 * @details
 * This function enables the clock for the LEUART, then sets up the init-struct and calls the leuart-init for this struct.
 * It then routes the LEUART and enables the correct interrupts
 *
 * @note
 * This function should only be called once in setup of the device.
 *
 *@param[in] *leuart
 * This is the pointer to the registers of the LEUART
 *
 *@param[in] *leuart_settings
 *	This struct holds all of the parameter information necessary for the operation of the LEUART.
 *
 * ******************************************************************************/

void leuart_open(LEUART_TypeDef *leuart, LEUART_OPEN_STRUCT *leuart_settings){

	if(leuart == LEUART0){
		CMU_ClockEnable(cmuClock_LEUART0, true);
	}
	else {
		EFM_ASSERT(false);
	}

	leuart->STARTFRAME = 0x01;
	while(leuart->SYNCBUSY);
	EFM_ASSERT(leuart->STARTFRAME & 0x01);
	while(leuart->SYNCBUSY);
	leuart->STARTFRAME = 0x00;
	while(leuart->SYNCBUSY);

	LEUART_Init_TypeDef start_leuart;
	start_leuart.baudrate = leuart_settings->baudrate;
	start_leuart.databits = leuart_settings->databits;
	start_leuart.enable = false;
	start_leuart.parity = leuart_settings->parity;
	start_leuart.refFreq = leuart_settings->ref_freq;
	start_leuart.stopbits = leuart_settings->stopbits;

	rx_done_evt = leuart_settings->rx_done_evt;
	tx_done_evt = leuart_settings->tx_done_evt;

	payload.txbusy = false;

	LEUART_Init(leuart, &start_leuart);
	while(leuart->SYNCBUSY);

	leuart->ROUTELOC0 = leuart_settings->rx_loc | leuart_settings->tx_loc;
// LEUART_ROUTELOC0_RXLOC_LOC18
	leuart->ROUTEPEN = (leuart_settings->rx_pin_en * LEUART_ROUTEPEN_RXPEN  | leuart_settings->tx_pin_en * LEUART_ROUTEPEN_TXPEN );

	while(leuart->SYNCBUSY);
	if(leuart_settings->enable){
		LEUART_Enable(leuart, leuart_settings->enable);
	}
	while(leuart->SYNCBUSY);

	//clears the receive and transmission buffer
	leuart->CMD = LEUART_CMD_CLEARRX | LEUART_CMD_CLEARTX ;


	while(leuart->SYNCBUSY);
	while((leuart->STATUS & LEUART_STATUS_TXENS) != LEUART_STATUS_TXENS);
	while((leuart->STATUS & LEUART_STATUS_RXENS) != LEUART_STATUS_RXENS);
	EFM_ASSERT(leuart->STATUS & (LEUART_STATUS_TXENS|LEUART_STATUS_RXENS));


	while(leuart->SYNCBUSY);
	LEUART_IntClear(leuart, LEUART_IFC_TXC);





//	Enable the correct IRQ
	if(leuart == LEUART0){
		NVIC_EnableIRQ(LEUART0_IRQn);
	}

	leuart_rxsetup(leuart);
	leuart_rxtest(leuart);
}

/***************************************************************************//**
 * @brief
 * This is a private function to handle if a txbl interrupt is received
 *
 * @details
 * By using a switch statement the function checks to make sure the transmission is in the correct
 * state and then it performs the correct actions for that state.
 *
 * @note
 *	This private function should only be called from the interrupt handler for LEUART0
 *******************************************************************************/

static void leuart_txbl(void){
	switch(payload.state){
		case LEUART_INITIALIZE:
			EFM_ASSERT(false);
			break;

		case SEND_DATA:
			if(payload.index < payload.message_len){
				LEUART0->TXDATA = payload.message[payload.index++];
				if(payload.index >= payload.message_len){
					payload.state = FINISH_WAIT;
					LEUART_IntDisable(payload.leuart, LEUART_IEN_TXBL);
					LEUART_IntEnable(payload.leuart, LEUART_IEN_TXC);
				}

			}
			else{
				EFM_ASSERT(false);
				// should not be in this state that would mean it is at the end of the string.
			}

			break;

		case FINISH_WAIT:
			EFM_ASSERT(false);
			break;


		default:
				EFM_ASSERT(false);
			break;
	}
}



/*****************************************************************************
 * @brief
 * This is a private function to handle if a txc interrupt is received
 *
 * @details
 * By using a switch statement the function checks to make sure the transmission is in the correct
 * state and then it performs the correct actions for that state.
 *
 * @note
 *	This private function should only be called from the interrupt handler for LEUART0
 *	This function should only occur once per message transmission signaling the end of
 *	the transmission, and scheduling the tx_done event.
 *******************************************************************************/

static void leuart_txc(void){
	switch(payload.state){
		case LEUART_INITIALIZE:
			EFM_ASSERT(false);
			break;

		case SEND_DATA:
			EFM_ASSERT(false);
			break;

		case FINISH_WAIT:
			//LEUART_IntDisable(payload.leuart, LEUART_IEN_TXC);
			//NVIC_DisableIRQ(LEUART0_IRQn);
			LEUART_IntClear(payload.leuart, LEUART_IEN_TXC);
			add_scheduled_event(tx_done_evt);
			sleep_unblock_mode(LEUART_RX_EM);
			payload.txbusy = false;
			payload.state = LEUART_INITIALIZE;
			break;

		default:
			EFM_ASSERT(false);
			break;
	}
}

/*****************************************************************************
 * @brief
 * This is The interrupt handler for the LEUART0
 *
 * @details
 * This function checks if the interrupt was caused by the txbl interrupt or the txc interrupt,
 *  or the start frame sig frame or RXDATAV interrupt and then calls the function of whichever
 *  of the two will handle the function properly.
 *
 * @note
 *	This function should only be triggered from txbl or txc, all other interrupts should never be enabled.
 *******************************************************************************/

void LEUART0_IRQHandler(void){
	__disable_irq();
	uint32_t int_flag;
	int_flag = LEUART0->IF & LEUART0->IEN;
	LEUART0->IFC = int_flag;
	if(int_flag & LEUART_IF_TXBL){
		leuart_txbl();
	}
	if(int_flag & LEUART_IF_TXC){
		leuart_txc();
	}
	if(int_flag & LEUART_IF_SIGF){
		leuart_sigf();
	}
	if(int_flag & LEUART_IEN_STARTF){
		leuart_startf();
	}
	if(int_flag & LEUART_IF_RXDATAV){
		leuart_rxdatav();
	}
	__enable_irq();
}



/*****************************************************************************
 * @brief
 * This function starts the software state machine for the leuart communication.
 *
 * @details
 * This function copies the inputed character array, and then initializes the state machine and all other state
 * information, then switches to the second state, and enables the TXBL interrupt.
 *
 * @note
 *	This function should be called every time it is necessary to write a message over the UART communication.
 *******************************************************************************/

void leuart_start(LEUART_TypeDef *leuart, char *string, uint32_t string_len){
	sleep_block_mode(LEUART_TX_EM);
//	EFM_ASSERT(leuart_tx_busy(leuart));
	payload.state = LEUART_INITIALIZE;
	payload.txbusy = true;
	payload.message_len = string_len;
	strcpy(payload.message, string);
	payload.index = 0;
	payload.leuart = leuart;
	payload.state = SEND_DATA;
//	payload.event = done_event;
//	LEUART_IntClear(leuart, LEUART_IEN_TXBL);

	LEUART_IntEnable(leuart, LEUART_IEN_TXBL);

}

/***************************************************************************//**
 * @brief
 *   Returns the private variable that marks if the LEUART is in the middle of a transmission,
 *   or if the LEUART is free to use.
 *
 * @details
 * 	 This function returns the value of payload.busy which is marked true at the begining of the LEUART transmission,
 * 	 and marked false at the occurrence of the TXC interrupt.
 *
 ******************************************************************************/

bool leuart_tx_busy(LEUART_TypeDef *leuart){
//return (leuart->STATUS & LEUART_STATUS_TXIDLE) != LEUART_STATUS_TXIDLE;
	return payload.txbusy; //private variable to know if it is busy or not.
}

/***************************************************************************//**
 * @brief
 *   LEUART STATUS function returns the STATUS of the peripheral for the
 *   TDD test
 *
 * @details
 * 	 This function enables the LEUART STATUS register to be provided to
 * 	 a function outside this .c module.
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 * @return
 * 	 Returns the STATUS register value as an uint32_t value
 *
 ******************************************************************************/

uint32_t leuart_status(LEUART_TypeDef *leuart){
	uint32_t	status_reg;
	status_reg = leuart->STATUS;
	return status_reg;
}

/***************************************************************************//**
 * @brief
 *   LEUART CMD Write sends a command to the CMD register
 *
 * @details
 * 	 This function is used by the TDD test function to program the LEUART
 * 	 for the TDD tests.
 *
 * @note
 *   Before exiting this function to update  the CMD register, it must
 *   perform a SYNCBUSY while loop to ensure that the CMD has by synchronized
 *   to the lower frequency LEUART domain.
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 * @param[in] cmd_update
 * 	 The value to write into the CMD register
 *
 ******************************************************************************/

void leuart_cmd_write(LEUART_TypeDef *leuart, uint32_t cmd_update){

	leuart->CMD = cmd_update;
	while(leuart->SYNCBUSY);
}

/***************************************************************************//**
 * @brief
 *   LEUART IF Reset resets all interrupt flag bits that can be cleared
 *   through the Interrupt Flag Clear register
 *
 * @details
 * 	 This function is used by the TDD test program to clear interrupts before
 * 	 the TDD tests and to reset the LEUART interrupts before the TDD
 * 	 exits
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 ******************************************************************************/

void leuart_if_reset(LEUART_TypeDef *leuart){
	leuart->IFC = 0xffffffff;
}

/***************************************************************************//**
 * @brief
 *   LEUART App Transmit Byte transmits a byte for the LEUART TDD test
 *
 * @details
 * 	 The BLE module will respond to AT commands if the BLE module is not
 * 	 connected to the phone app.  To validate the minimal functionality
 * 	 of the LEUART peripheral, write and reads to the LEUART will be
 * 	 performed by polling and not interrupts.
 *
 * @note
 *   In polling a transmit byte, a while statement checking for the TXBL
 *   bit in the Interrupt Flag register is required before writing the
 *   TXDATA register.
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 * @param[in] data_out
 *   Byte to be transmitted by the LEUART peripheral
 *
 ******************************************************************************/

void leuart_app_transmit_byte(LEUART_TypeDef *leuart, uint8_t data_out){
	while (!(leuart->IF & LEUART_IF_TXBL));
	leuart->TXDATA = data_out;
}


/***************************************************************************//**
 * @brief
 *   LEUART App Receive Byte polls a receive byte for the LEUART TDD test
 *
 * @details
 * 	 The BLE module will respond to AT commands if the BLE module is not
 * 	 connected to the phone app.  To validate the minimal functionality
 * 	 of the LEUART peripheral, write and reads to the LEUART will be
 * 	 performed by polling and not interrupts.
 *
 * @note
 *   In polling a receive byte, a while statement checking for the RXDATAV
 *   bit in the Interrupt Flag register is required before reading the
 *   RXDATA register.
 *
 * @param[in] leuart
 *   Defines the LEUART peripheral to access.
 *
 * @return
 * 	 Returns the byte read from the LEUART peripheral
 *
 ******************************************************************************/

uint8_t leuart_app_receive_byte(LEUART_TypeDef *leuart){
	uint8_t leuart_data;
	while (!(leuart->IF & LEUART_IF_RXDATAV));
	leuart_data = leuart->RXDATA;
	return leuart_data;
}
