/*
 * tca6416A.c
 *
 *  Created on: May 19, 2013
 *      Author: Collin
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "inc/hw_memmap.h"
#include "timer_d.h"
#include "wdt_a.h"
#include "gpio.h"
#include "ucs.h"
#include "pmm.h"
#include "pmap.h"

#include "usci_b_i2c.h"
#include "ucs.h"

#include "hardware.h"
#include "tca6416A.h"

#define EXP_BUFFER_SIZE 12

static uint8_t exp_buf[EXP_BUFFER_SIZE];
static uint8_t buf_index = 0;	//index into the current transmission.
static uint8_t msg_len = 0;		//total length of the transmission.

#define TRANSMISSION_COMPLETE() (buf_index == msg_len)

/* _start_I2C_tx()
 *
 * Start an I2C (transmit) transaction.
 *
 * Arguments:
 * data:	The data to transmit
 * len:		The length of the data buffer to transmit.
 *
 * Returns:
 * The actual number of bytes that were transmitted.
 *
 */
uint8_t _start_I2C_tx(uint8_t* data, uint8_t len)
{
	if (TRANSMISSION_COMPLETE() && !(USCI_B_I2C_isBusBusy(USCI_B0_BASE)))
	{
		/* Limit the size of the outgoing transmission. */
		if (len > EXP_BUFFER_SIZE)
		{
			len = EXP_BUFFER_SIZE;
		}

		memcpy(exp_buf, data, len);		//copy the data
		buf_index = 0;					//reset the buffer state machine.
		msg_len = len;

		/* Start the I2C transaction. */
		USCI_B_I2C_masterSendStart(USCI_B0_BASE);

		return len;
	}

	return 0;		//a current transaction is not complete, nothing new can be transmitted.
}

/* ioexp_init()
 *
 * Initialize the I2C I/O expander chip.
 *
 * Arguments:
 * address:		The address of the I/O Expander chip:
 * 		-ADDRRESS_H
 * 		-ADDRRESS_L
 *
 * Returns:
 * NONE
 *
 */
void ioexp_init(uint8_t address)
{
	/* Setup buffers. */
	memset(exp_buf, 0, sizeof(exp_buf));
	buf_index = 0;
	msg_len = 0;

	/* Setup the I2C interface. */
	GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1, (GPIO_PIN4 | GPIO_PIN5));

	USCI_B_I2C_disable(USCI_B0_BASE);

	//Initialize Master
	USCI_B_I2C_masterInit(
			USCI_B0_BASE,
			USCI_B_I2C_CLOCKSOURCE_SMCLK,
			UCS_getSMCLK(UCS_BASE),
			USCI_B_I2C_SET_DATA_RATE_400KBPS);

	//Specify slave address
	USCI_B_I2C_setSlaveAddress(USCI_B0_BASE, address);

	//Set in transmit mode
	USCI_B_I2C_setMode(USCI_B0_BASE, USCI_B_I2C_TRANSMIT_MODE);

	//Enable I2C Module to start operations
	USCI_B_I2C_enable(USCI_B0_BASE);

	//Enable transmit Interrupt
	USCI_B_I2C_clearInterruptFlag(USCI_B0_BASE, USCI_B_I2C_TRANSMIT_INTERRUPT);
	USCI_B_I2C_enableInterrupt(USCI_B0_BASE, USCI_B_I2C_TRANSMIT_INTERRUPT);
}

/* ioexp_setIO()
 *
 * Set the input/output mode of the various bits on the IO expander.
 *
 * Arguments:
 * address:	The address of the I/O Expander chip:
 * 		-ADDRRESS_H
 * 		-ADDRRESS_L
 * iomap:	The bits that describe the I/O status of each bit on the I/O expander. 1 = input, 0 = output.
 *
 * Returns:
 * NONE
 *
 */
void ioexp_setIO(uint8_t address, uint16_t iomap)
{
	uint8_t data[3];

	/* Wait for the bus to be available. */
	while ((USCI_B_I2C_isBusBusy(USCI_B0_BASE)) == USCI_B_I2C_BUS_BUSY)
	{
	}

	//Specify slave address
	USCI_B_I2C_setSlaveAddress(USCI_B0_BASE, address);

	/* Transmit the address command and iomap. */
	data[0] = 0x06;		//command byte holds the address of the port I/O control.
	data[1] = iomap & 0xFF;
	data[2] = (iomap >> 8) & 0xFF;

	_start_I2C_tx(data, sizeof(data));
}

/* ioexp_setOutput()
 *
 * Set the output bits of the I/O Expander. Only bits that are enabled to output
 * will actually change. Use ioexp_setIO to configure input/output bits.
 *
 * Arguments:
 * address:	The address of the I/O Expander chip:
 * 		-ADDRRESS_H
 * 		-ADDRRESS_L
 * output: 	The bits to set the I/O Expander output port too.
 *
 * Returns:
 * NONE
 *
 */
void ioexp_setOutput(uint8_t address, uint16_t output)
{
	uint8_t data[3];

	/* Wait for the bus to be available. */
	while ((USCI_B_I2C_isBusBusy(USCI_B0_BASE)) == USCI_B_I2C_BUS_BUSY)
	{
	}

	//Specify slave address
	USCI_B_I2C_setSlaveAddress(USCI_B0_BASE, address);

	/* Transmit the address command and iomap. */
	data[0] = 0x02;
	data[1] = output & 0xFF;
	data[2] = (output >> 8) & 0xFF;

	_start_I2C_tx(data, sizeof(data));
}

#pragma vector = USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void)
{
	uint8_t word = 0;

	switch (__even_in_range(UCB0IV, 12))
	{
		case USCI_I2C_UCTXIFG: //Vector 12: Transmit buffer empty - TXIF

			/* Try and read data from the buffer, if data is available, write it to the I2C bus, otherwise end the transmission. */
			if (TRANSMISSION_COMPLETE())
			{
				USCI_B_I2C_masterMultiByteSendStop(USCI_B0_BASE);
				USCI_B_I2C_clearInterruptFlag(USCI_B0_BASE, USCI_B_I2C_TRANSMIT_INTERRUPT); //Clear master interrupt status
			}
			else
			{
				/* Continue the transmission. */
				USCI_B_I2C_masterMultiByteSendNext(USCI_B0_BASE, exp_buf[buf_index++]);
			}

			break;

		default:
			break;
	}
}
