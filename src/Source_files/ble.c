/**
 * @file ble.c
 * @author Justin Thwaites
 * @date 4/23/2020
 * @brief Contains all the functions to interface the application with the HM-18
 *   BLE module and the LEUART driver
 *
 */


//***********************************************************************************
// Include files
//***********************************************************************************
#include "ble.h"
#include <string.h>

//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// private variables
//***********************************************************************************
static CIRC_TEST_STRUCT test_struct;
static BLE_CIRCULAR_BUF ble_cbuf;
static bool is_celsius;
/***************************************************************************//**
 * @brief BLE module
 * @details
 *  This module contains all the functions to interface the application layer
 *  with the HM-18 Bluetooth module.  The application does not have the
 *  responsibility of knowing the physical resources required, how to
 *  configure, or interface to the Bluetooth resource including the LEUART
 *  driver that communicates with the HM-18 BLE module.
 *
 ******************************************************************************/

//***********************************************************************************
// Private functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 * This function returns the available space on the circular buffer.
 *
 * @details
 * This uses the property that the (read_ptr + buffer_size) - write_ptr will be equal to the empty space.
 *
 * @return
 * The available space on the buffer
 *
 * @note
 *	This function is a private helper function that only has local scope.
 ******************************************************************************/
static uint8_t ble_circ_space(void){
	return ble_cbuf.read_ptr + ble_cbuf.size - ble_cbuf.write_ptr;
}


/***************************************************************************//**
 * @brief
 * This function updates the write index by a certain specified amount
 *
 * @details
 * This adds a certain amount to the write pointer, and then uses modulo arithmetic
 * to ensure the value is under the size of the buffer array.
 *
 * @param[in] *index_struct
 * The address to the circular buffer struct.
 *
 * @param[in] update_by
 * The number that the write index is needing to be updated by.
 *
 * @note
 *	This function is a private helper function that only has local scope.
 ******************************************************************************/
static void update_circ_wrtindex(BLE_CIRCULAR_BUF *index_struct, uint32_t update_by){
	index_struct->write_ptr = (index_struct->write_ptr + update_by)%index_struct->size ;
}

/***************************************************************************//**
 * @brief
 * This function updates the read index by a certain specified amount
 *
 * @details
 * This adds a certain amount to the read pointer, and then uses modulo arithmetic
 * to ensure the value is under the size of the buffer array.
 *
 * @param[in] *index_struct
 * The address to the circular buffer struct.
 *
 * @param[in] update_by
 * The number that the read index is needing to be updated by.
 *
 * @note
 *	This function is a private helper function that only has local scope.
 ******************************************************************************/
static void update_circ_readindex(BLE_CIRCULAR_BUF *index_struct, uint32_t update_by){
	index_struct->read_ptr = (index_struct->read_ptr + update_by)%index_struct->size ;
}



//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *	This function sets up all necessary values for the bluetooth peripheral setup.
 *
 * @details
 * This passes in all of the correct values to the struct that then gets passed in to the leuart_open function
 *
 * @note
 * This function should only be called once in setup of the device.
 *
 *@param[in] tx_event
 * This passes in the event that should be flagged in the event handler when the bluetooth has finished transmitting
 *
 *@param[in] rx_event
 *This passes in the event that should be flagged in the event handler when the bluetooth has received a transmission.
 *
 ******************************************************************************/

void ble_open(uint32_t tx_event, uint32_t rx_event){

	LEUART_OPEN_STRUCT open_leuart;
	open_leuart.baudrate = HM10_BAUDRATE;
	open_leuart.databits = HM10_DATABITS;
	open_leuart.enable = HM10_ENABLE;
	open_leuart.parity = HM10_PARITY;
	open_leuart.ref_freq = HM10_REFFREQ;
	open_leuart.stopbits = HM10_STOPBITS;
	open_leuart.rx_done_evt = rx_event;
	open_leuart.tx_done_evt = tx_event;
	open_leuart.rx_loc = LEUART0_RX_ROUTE;
	open_leuart.tx_loc = LEUART0_TX_ROUTE;
	open_leuart.rx_en = true;
	open_leuart.tx_en = true;
	open_leuart.rx_pin_en = true;
	open_leuart.tx_pin_en = true;

	is_celsius = false;
	leuart_open(HM10_LEUART0, &open_leuart);
	ble_circ_init();
}

/***************************************************************************//**
 * @brief
 * This writes out a string of values that should be received by the bluetooth module.
 *
 * @details
 * This is a wrapper that will pass the values of the string along with the pointer to the registers into the
 * Leuart state machine
 *
 * @note
 * This function should be called each time a string needs to be sent over bluetooth.
 *
 *@param[in] *string
 * This is the pointer to the character array that is being sent out over bluetooth.
 *
 *******************************************************************************/

void ble_write(char* string){
	ble_circ_push(string);
	ble_circ_pop(CIRC_OPER);
//	leuart_start(HM10_LEUART0, string, strlen(string));
}

/***************************************************************************//**
 * @brief
 *   BLE Test performs two functions.  First, it is a Test Driven Development
 *   routine to verify that the LEUART is correctly configured to communicate
 *   with the BLE HM-18 module.  Second, the input argument passed to this
 *   function will be written into the BLE module and become the new name
 *   advertised by the module while it is looking to pair.
 *
 * @details
 * 	 This global function will use polling functions provided by the LEUART
 * 	 driver for both transmit and receive to validate communications with
 * 	 the HM-18 BLE module.  For the assignment, the communication with the
 * 	 BLE module must use low energy design principles of being an interrupt
 * 	 driven state machine.
 *
 * @note
 *   For this test to run to completion, the phone most not be paired with
 *   the BLE module.  In addition for the name to be stored into the module
 *   a breakpoint must be placed at the end of the test routine and stopped
 *   at this breakpoint while in the debugger for a minimum of 5 seconds.
 *
 * @param[in] *mod_name
 *   The name that will be written to the HM-18 BLE module to identify it
 *   while it is advertising over Bluetooth Low Energy.
 *
 * @return
 *   Returns bool true if successfully passed through the tests in this
 *   function.
 ******************************************************************************/

bool ble_test(char *mod_name){
	uint32_t	str_len;

	__disable_irq();
	// This test will limit the test to the proper setup of the LEUART
	// peripheral, routing of the signals to the proper pins, pin
	// configuration, and transmit/reception verification.  The test
	// will communicate with the BLE module using polling routines
	// instead of interrupts.
	// How is polling different than using interrupts?

	// ANSWER: polling requires the processor to constantly check a resource
	//taking power and removing time from other processes, while an interrupt wakes up the processor when a resource is available

	// How does interrupts benefit the system for low energy operation?

	// ANSWER: They allow the processor the ability to go completely to sleep. This turns off all the required power
	// of the processor. Therefore the process can operate for a longer time with less energy.

	// How does interrupts benefit the system that has multiple tasks?

	// ANSWER: The interrupts benefit the system if it has multiple tasks because this allows for multiple interrupts
	// which can wake up the processor at different times to react quickly to various events without having to wait for
	// other events to finish or the need to poll to check for resources.

	// First, you will need to review the DSD HM10 datasheet to determine
	// what the default strings to write data to the BLE module and the
	// expected return statement from the BLE module to test / verify the
	// correct response

	// The break_str is used to tell the BLE module to end a Bluetooth connection
	// such as with your phone.  The ok_str is the result sent from the BLE module
	// to the micro-controller if there was not active BLE connection at the time
	// the break command was sent to the BLE module.
	// Replace the break_str "" with the command to break or end a BLE connection
	// Replace the ok_str "" with the result that will be returned from the BLE
	//   module if there was no BLE connection
	char		break_str[80] = "AT";
	char		ok_str[80] = "OK";

	// output_str will be the string that will program a name to the BLE module.
	// From the DSD HM10 datasheet, what is the command to program a name into
	// the BLE module?
	// The  output_str will be a string concatenation of the DSD HM10 command
	// and the input argument sent to the ble_test() function
	// Replace the otput_str "" with the command to change the program name
	// Replace the result_str "" with the first part of the expected result
	//  the backend of the expected response will be concatenated with the
	//  input argument
	char		output_str[80] = "AT+Name";
	char		result_str[80] = "OK+Set:";

	// To program the name into your module, you must reset the module after you
	// have sent the command to update the modules name.  What is the DSD HM10
	// name to reset the module?
	// Replace the reset_str "" with the command to reset the module
	// Replace the reset_result_str "" with the expected BLE module response to
	//  to the reset command
	char		reset_str[80] = "AT+RESET";
	char		reset_result_str[80] = "OK+RESET";
	char		return_str[80];

	bool		success;
	bool		rx_disabled, rx_en, tx_en;
	uint32_t	status;

	// These are the routines that will build up the entire command and response
	// of programming the name into the BLE module.  Concatenating the command or
	// response with the input argument name
	strcat(output_str, mod_name);
	strcat(result_str, mod_name);

	// The test routine must not alter the function of the configuration of the
	// LEUART driver, but requires certain functionality to insure the logical test
	// of writing and reading to the DSD HM10 module.  The following c-lines of code
	// save the current state of the LEUART driver that will be used later to
	// re-instate the LEUART configuration

	status = leuart_status(HM10_LEUART0);
	if (status & LEUART_STATUS_RXBLOCK) {
		rx_disabled = true;
		// Enabling, unblocking, the receiving of data from the LEUART RX port
		leuart_cmd_write(HM10_LEUART0, LEUART_CMD_RXBLOCKDIS);
	}
	else rx_disabled = false;
	if (status & LEUART_STATUS_RXENS) {
		rx_en = true;
	} else {
		rx_en = false;
		// Enabling the receiving of data from the RX port
		leuart_cmd_write(HM10_LEUART0, LEUART_CMD_RXEN);
		while (!(leuart_status(HM10_LEUART0) & LEUART_STATUS_RXENS));
	}

	if (status & LEUART_STATUS_TXENS){
		tx_en = true;
	} else {
		// Enabling the transmission of data to the TX port
		leuart_cmd_write(HM10_LEUART0, LEUART_CMD_TXEN);
		while (!(leuart_status(HM10_LEUART0) & LEUART_STATUS_TXENS));
		tx_en = false;
	}
//    leuart_cmd_write(HM10_LEUART0, (LEUART_CMD_CLEARRX | LEUART_CMD_CLEARTX));

	// This sequence of instructions is sending the break ble connection
	// to the DSD HM10 module.

	// Why is this command required if you want to change the name of the
	// DSD HM10 module?

	// ANSWER: It is required because if you want to change the name you must send the command
	// to correctly tell the bluetooth chip to change the name using the proper protocol.
	str_len = strlen(break_str);
	for (int i = 0; i < str_len; i++){
		leuart_app_transmit_byte(HM10_LEUART0, break_str[i]);
	}

	// What will the ble module response back to this command if there is
	// a current ble connection?

	// ANSWER: There will be no response if there is already a connection between the ports.

	str_len = strlen(ok_str);
	for (int i = 0; i < str_len; i++){
		return_str[i] = leuart_app_receive_byte(HM10_LEUART0);
		if (ok_str[i] != return_str[i]) {
				EFM_ASSERT(false);;
		}
	}

	// This sequence of code will be writing or programming the name of
	// the module to the DSD HM10
	str_len = strlen(output_str);
	for (int i = 0; i < str_len; i++){
		leuart_app_transmit_byte(HM10_LEUART0, output_str[i]);
	}

	// Here will be the check on the response back from the DSD HM10 on the
	// programming of its name
	str_len = strlen(result_str);
	for (int i = 0; i < str_len; i++){
		return_str[i] = leuart_app_receive_byte(HM10_LEUART0);
		if (result_str[i] != return_str[i]) {
			EFM_ASSERT(false);
		}
	}

	// It is now time to send the command to RESET the DSD HM10 module
	str_len = strlen(reset_str);
	for (int i = 0; i < str_len; i++){
		leuart_app_transmit_byte(HM10_LEUART0, reset_str[i]);
	}

	// After sending the command to RESET, the DSD HM10 will send a response
	// back to the micro-controller
	str_len = strlen(reset_result_str);
	for (int i = 0; i < str_len; i++){
		return_str[i] = leuart_app_receive_byte(HM10_LEUART0);
		if (reset_result_str[i] != return_str[i]) {
				EFM_ASSERT(false);;
		}
	}

	// After the test and programming have been completed, the original
	// state of the LEUART must be restored
	if (!rx_en) leuart_cmd_write(HM10_LEUART0, LEUART_CMD_RXDIS);
	if (rx_disabled) leuart_cmd_write(HM10_LEUART0, LEUART_CMD_RXBLOCKEN);
	if (!tx_en) leuart_cmd_write(HM10_LEUART0, LEUART_CMD_TXDIS);
	leuart_if_reset(HM10_LEUART0);

	success = true;

	__enable_irq();
	return success;
}


/***************************************************************************//**
 * @brief
 * This function tests to makes sure that the circular buffer has been set up properly.
 *
 * @details
 * This function does this by generating three strings of different lengths, then the function
 * pushes on the first of the strings and pops it off to make sure that the functions work properly.
 * Once this happens the function makes sure of the ability of the looping around of the buffer, and the ability
 * of the buffer to store more than one piece of information at a time.
 *
 * @note
 *	This function is a test and should only be called once.
 ******************************************************************************/


void circular_buff_test(void){
	 bool buff_empty;
	 int test1_len = 50;
	 int test2_len = 25;
	 int test3_len = 5;

	 // Why this 0 initialize of read and write pointer?
	 // Student Response:
	 // This makes it so that the circular buffer starts empty, both starting at the 0th index of the array.
	 ble_cbuf.read_ptr = 0;
	 ble_cbuf.write_ptr = 0;

	 // Why do none of these test strings contain a 0?
	 // Student Response:
	 //	0 marks the end of a string, which would immediately stop the string,
	 //	 therefore we want the string to contain elements that are not the ascii null character

	 for (int i = 0;i < test1_len; i++){
		 test_struct.test_str[0][i] = i+1;
	 }
	 for (int i = 0;i < test2_len; i++){
		 test_struct.test_str[1][i] = i + 20;
	 }
	 for (int i = 0;i < test3_len; i++){
		 test_struct.test_str[2][i] = i +  35;
	 }

	 // Why is there only one push to the circular buffer at this stage of the test
	 // Student Response:
	 //The string is 50 characters in length and if the next string was pushed on as well, there would be an overflow
	 //the second test string is 25 characters in length, 50 + 25 is greater than what the 64 element circular buffer can hold.
	 ble_circ_push(&test_struct.test_str[0][0]);

	 // Why is the expected buff_empty test = false?
	 // Student Response:
	 //The expected buff empty test is false because after pushing the string there should be some value
	 // to pop from the circular buffer. True is only returned if nothing is popped out of the buffer,
	 // but something was popped out of the buffer, therefore false.
	 buff_empty = ble_circ_pop(CIRC_TEST);
	 EFM_ASSERT(buff_empty == false);
	 for (int i = 0; i < test1_len; i++){
		 EFM_ASSERT(test_struct.test_str[0][i] == test_struct.result_str[i]);
	 }

	 // What does this next push on the circular buffer test?
	 // Student Response: This next push onto the circular buffer will test the looping capability of the circular buffer.

	 ble_circ_push(&test_struct.test_str[1][0]);

	 // What does this next push on the circular buffer test?
	 // Student Response: This next push onto the circular buffer tests the ability of the buffer to have multiple strings
	 //in the circular buffer at one time. If it overwrites the other value in the buffer, we know the write pointer is
	 //not aligned properly
	 ble_circ_push(&test_struct.test_str[2][0]);


	 // Why is the expected buff_empty test = false?
	 // Student Response: There are currently two strings that should be on the circular buffer at this time.
	 // One of them gets popped off, which should return false since true is only returned if there is nothing that gets popped off.
	 buff_empty = ble_circ_pop(CIRC_TEST);
	 EFM_ASSERT(buff_empty == false);
	 for (int i = 0; i < test2_len; i++){
		 EFM_ASSERT(test_struct.test_str[1][i] == test_struct.result_str[i]);
	 }

	 // Why is the expected buff_empty test = false?
	 // Student Response: There should be still one string that is still on the buffer. This string gets popped off this time.
	 //since data is removed from the buffer, that means the function should return false.
	 buff_empty = ble_circ_pop(CIRC_TEST);
	 EFM_ASSERT(buff_empty == false);
	 for (int i = 0; i < test3_len; i++){
		 EFM_ASSERT(test_struct.test_str[2][i] == test_struct.result_str[i]);
	 }

	 // Using these three writes and pops to the circular buffer, what other test
	 // could we develop to better test out the circular buffer?
	 // Student Response: We could attempt to have a test for the overflow of the buffer. This would make sure that the buffer catches when
	 // there is too much written from the buffer. See if there is a size handling by inputting a string of a greater length than the buffer.


	 // Why is the expected buff_empty test = true?
	 // Student Response: There were 3 writes and three pops so the fourth pop should have no values that can be popped of the buffer.
	 // Because of this nothing should be popped from the buffer, and as a result the function should return true signifying that there was no pop done.
	 buff_empty = ble_circ_pop(CIRC_TEST);
	 EFM_ASSERT(buff_empty == true);
	 ble_write("\nPassed Circular Buffer Test\n");

 }

/***************************************************************************//**
 * @brief
 * This function initializes the circular buffer to have the read and write pointer
 * values equal to 0, as well as the size of the buffer equal to the defined macro.
 *
 * @details
 * This function only has to initialize the values of the circular buffer private struct.
 *
 * @note
 *	This function is called once from the ble setup function.
 ******************************************************************************/

void ble_circ_init(void){
ble_cbuf.read_ptr = 0;
ble_cbuf.write_ptr = 0;
ble_cbuf.size = CSIZE;
ble_cbuf.size_mask = 0; // unknown yet
}

/***************************************************************************//**
 * @brief
 * This function pushes a string onto the circular buffer.
 *
 * @details
 * This calculates the packet size and then after checking if the packet will fit,
 * it loops through putting the packet on the buffer, and then it updates the write
 * pointer to the new start of the empty space on the circular buffer.
 *
 * @param[in] *string
 * The character array of the message that is being sent to the circular buffer.
 *
 * @note
 *	This function is generally called from ble_write as a way of storing information
 *	until the transmission has finished on the leuart.
 ******************************************************************************/
void ble_circ_push(char *string){
uint8_t packet_size = strlen(string)+1;
if(packet_size > ble_circ_space()) EFM_ASSERT(false);

ble_cbuf.cbuf[ble_cbuf.write_ptr] = packet_size;

for(int i= 1; i < packet_size; i++){
	ble_cbuf.cbuf[(ble_cbuf.write_ptr+i)%ble_cbuf.size] = string[i-1];
}
update_circ_wrtindex(&ble_cbuf, packet_size);
}

/***************************************************************************//**
 * @brief
 * This function pops a string off the circular buffer and depending on the input, sends it
 * to a struct, or sends it to the LEUART.
 *
 * @details
 * This function loops through and pulls off information from the buffer, after making sure there
 * is something to remove and the LEUART isn't busy. Once this happens it sends the string to the LEUART to be written,
 * and it updates the read pointer.
 *
 * @param[in] test
 * Specifies if this is a test of the function, or if it is the function in operation. If false the popped string will be
 * sent to the LEUART to be transmitted
 *
 * @return
 * returns true if nothing is popped from the buffer and false if something is removed from the buffer.
 *
 * @note
 *	This function is generally called from ble_write as a way of popping information and giving the information to the LEUART,
 *	as well as it is called to check if there is information for the LEUART to send once the LEUART finishes a transmission.
 ******************************************************************************/
bool ble_circ_pop(bool test){
uint8_t filled = ble_cbuf.size - ble_circ_space() ;
if(filled == 0)return true; //Empty circular buffer
if(leuart_tx_busy(HM10_LEUART0))return true;
uint8_t string_length = ble_cbuf.cbuf[ble_cbuf.read_ptr] - 1;
if(string_length + 1 > filled) EFM_ASSERT(false); //Guaranteed string mismatch
char print_str[ble_cbuf.size];
for(int i = 0; i < string_length; i++){
	print_str[i] = ble_cbuf.cbuf[(ble_cbuf.read_ptr+1+i)% ble_cbuf.size];
}
print_str[string_length] = 0;//adds null to signify end of string
update_circ_readindex(&ble_cbuf, string_length + 1);
if(test){
	for (int i = 0; i < string_length; i++){
		test_struct.result_str[i] = print_str[i];
	}
	return false;
}
else{
	leuart_start(HM10_LEUART0, print_str, strlen(print_str));
//	ble_write(print_str);
	return false;
}

}

/***************************************************************************//**
 * @brief
 * Returns if the operating mode of the ble is currently set to celsuis.
 *
 *
 * @details
 * This function just returns the mode of operation which is stored in a private variable.
 *
 *
 * @note
 * This function is unable to modify the value, it is only able to return it.
 *
 ******************************************************************************/
bool ble_mode_celsius(void){
	return is_celsius;
}
/***************************************************************************//**
 * @brief
 *	This function is meant to update a private variable storing the ble settings for Celsius of Fahrenheit.
 *
 * @details
 *	This function uses string compare to determine if the value stored in the rxmessage array is the same as
 *	either "Celsius" or "Fahrenheit". If neither, it does not change the current mode.
 *
 * @note
 *	This function doesn't return anything, it just updates if the mode is Celsius or not. This data is returned
 *	in the helper function ble_mode_celsuis.
 *
 ******************************************************************************/
void ble_update_mode(void){
	char * message;
	char celsius[16]= CELSIUS_MESSAGE;
	char farenheit[16] = FAHRENHEIT_MESSAGE;
	message = leuart_rxmessage();
	if(strcmp(celsius, message) == 0){
		is_celsius = true;
	}
	else if(strcmp(farenheit, message) == 0) {
		is_celsius = false;
	}
}


