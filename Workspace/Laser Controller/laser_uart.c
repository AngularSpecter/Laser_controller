/*
 * uart.c
 *
 *  Created on: May 21, 2013
 *      Author: Collin
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "inc/hw_memmap.h"
#include "ucs.h"
#include "usci_a_uart.h"
#include "gpio.h"

#include "laser_uart.h"
#include "buffer.h"

static circular_buffer rx_buffer; //incoming data
static circular_buffer tx_buffer; //outgoing data

/* _uart_transmitCharacter()
 *
 * Transmit a single character, taken from the transmit buffer
 * over the UART.
 *
 * Arguments:
 * NONE
 *
 * Returns:
 * NONE
 *
 */
static void _uart_transmitCharacter()
{
	char data = 0x00;

	/* Try and get a character from the transmit buffer.
	 * If data is available, transmit it.
	 */
	if (buffer_read(&tx_buffer, &data, 1) == 1)
	{
		USCI_A_UART_transmitData(USCI_A0_BASE, data);
	}
}

/* uart_init()
 *
 * Setup the UART.
 *
 * Arguments:
 * NONE
 *
 * Returns:
 * NONE
 *
 */
void uart_init()
{
	/* Setup the UART hardware. */
	//P1.1,P1.2 = USCI_A0 TXD/RXD
	GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1, (GPIO_PIN1 | GPIO_PIN2));

	//Initialize USCI UART module
	USCI_A_UART_init(
			USCI_A0_BASE,
			USCI_A_UART_CLOCKSOURCE_SMCLK,
			UCS_getSMCLK(UCS_BASE),
			BAUD_RATE,
			USCI_A_UART_NO_PARITY,
			USCI_A_UART_LSB_FIRST,
			USCI_A_UART_ONE_STOP_BIT,
			USCI_A_UART_MODE,
			USCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION);

	//Enable UART module for operation
	USCI_A_UART_enable(USCI_A0_BASE);

	//Enable Receive Interrupt
	USCI_A_UART_clearInterruptFlag(USCI_A0_BASE, USCI_A_UART_RECEIVE_INTERRUPT);
	USCI_A_UART_clearInterruptFlag(USCI_A0_BASE, USCI_A_UART_TRANSMIT_INTERRUPT);
	USCI_A_UART_enableInterrupt(USCI_A0_BASE, USCI_A_UART_RECEIVE_INTERRUPT);
	USCI_A_UART_enableInterrupt(USCI_A0_BASE, USCI_A_UART_TRANSMIT_INTERRUPT);

	/* Setup the tx/rx buffers. */
	buffer_init(&rx_buffer);
	buffer_init(&tx_buffer);
}

/* uart_puts()
 *
 * Put a string out the uart. This function is asynchronous and returns
 * immediately, on success or failure. Characters will be written out
 * onto the bus automatically.
 *
 * Arguments:
 * str:		The string to write over the UART.
 * len:		The length of the string to write.
 *
 * Returns:
 * The total number of characters actually written.
 *
 */
uint8_t uart_puts(char* str, uint8_t len)
{
	uint8_t len_written = 0;

	/* Write the new data to the buffer. */
	len_written = buffer_write(&tx_buffer, str, len);

	/* If the write succeeded and data is now available, and the uart is idle, transmit a character. */
	if (len_written > 0 && !USCI_A_UART_queryStatusFlags(USCI_A0_BASE, USCI_A_UART_BUSY))
	{
		_uart_transmitCharacter();
	}

	return len_written;
}

/* uart_gets()
 *
 * Retrieve characters already received on the UART.
 *
 * Arguments:
 * str:		The string to read data into.
 * max_len:	The number of maximum bytes to read into the buffer.
 *
 * Returns:
 * The total number of characters actually written.
 *
 */
uint8_t uart_gets(char* str, uint8_t max_len)
{
	/* Pull data from the RX buffer and return it to the user. */
	return buffer_read(&rx_buffer, str, max_len);
}

#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
	char data = 0x00;

	switch (__even_in_range(UCA0IV, 4))
	{
		case USCI_UCRXIFG:
			/* Receive data and place it into the RX buffer. */
			data = USCI_A_UART_receiveData(USCI_A0_BASE);
			buffer_write(&rx_buffer, &data, 1);
			break;

		case USCI_UCTXIFG:
			/* The TX buffer is empty so transmit the next character.
			 * If none exists, this function does nothing and the chain of
			 * interrupts stops until the next string is entered into the buffer. */
			_uart_transmitCharacter();
			break;

		default:
			break;
	}
}
