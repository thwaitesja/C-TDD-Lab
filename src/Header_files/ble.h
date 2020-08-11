/*
 * ble.h
 *
 *  Created on: March 12, 2019
 *      Author: Justin Thwaites
 */

#ifndef BLE_H
#define	BLE_H
//***********************************************************************************
// Include files
//***********************************************************************************

//** Standard Libraries
#include <stdbool.h>
#include <stdint.h>

#include "leuart.h"

//***********************************************************************************
// defined files
//***********************************************************************************

#define HM10_LEUART0		LEUART0
#define HM10_BAUDRATE		9600
#define	HM10_DATABITS		leuartDatabits8
#define HM10_ENABLE			leuartEnable
#define HM10_PARITY			leuartNoParity
#define HM10_REFFREQ		0		// use reference clock
#define HM10_STOPBITS		leuartStopbits1

#define LEUART0_TX_ROUTE	LEUART_ROUTELOC0_TXLOC_LOC18
#define LEUART0_RX_ROUTE	LEUART_ROUTELOC0_RXLOC_LOC18

#define CIRC_TEST_SIZE		3
#define CIRC_TEST 			true
#define CIRC_OPER 			false
#define CSIZE 				64

#define CELSIUS_MESSAGE		"Celsius"
#define FAHRENHEIT_MESSAGE	"Fahrenheit"

typedef struct {
	char test_str[CIRC_TEST_SIZE][64];
	char result_str[64];
}CIRC_TEST_STRUCT;


typedef struct {
char cbuf[64];
uint8_t size_mask;
uint32_t size;
uint32_t read_ptr;
uint32_t write_ptr;
} BLE_CIRCULAR_BUF;


//***********************************************************************************
// function prototypes
//***********************************************************************************
void ble_open(uint32_t tx_event, uint32_t rx_event);
void ble_write(char *string);
void circular_buff_test(void);
bool ble_test(char *mod_name);
void ble_circ_init(void);
void ble_circ_push(char *string);
void circular_buff_test(void);
bool ble_circ_pop(bool test);
bool ble_mode_celsius(void);
void ble_update_mode(void);
#endif
