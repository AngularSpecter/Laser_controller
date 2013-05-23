/*
 * lcd.c
 *
 *  Created on: May 22, 2013
 *      Author: Collin
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "inc/hw_memmap.h"
#include "gpio.h"
#include "pmap.h"

#include "tca6416A.h"

#define RW_BIT		15
#define ENABLE_BIT 	11
#define RS_BIT		10

#define INDEX_TO_BIT(x)		(1 << x)
#define ENABLE				INDEX_TO_BIT(ENABLE_BIT)
#define RW					INDEX_TO_BIT(RW_BIT)
#define RS					INDEX_TO_BIT(RS_BIT)

#define LCD_IO(x)	ioexp_setOutput(ADDRESS_H, port)

/* _lcd_delay()
 *
 * Delay a specified number of milliseconds.
 *
 * Arguments:
 * msec_delay:	The number of milliseconds to delay.
 *
 * Returns:
 * NONE
 *
 */
void _lcd_delay(uint8_t msec_delay)
{
	uint8_t k = 0;
	for(k = 0;k < msec_delay;k++)
	{
		__delay_cycles(16000);
	}
}

/* _lcd_command()
 *
 * Send an LCD command.
 *
 * Arguments:
 * cmd:	The command to send.
 *
 * Returns:
 * NONE
 *
 */
void _lcd_command(uint8_t cmd)
{
	uint16_t port = 0x0000;

	/* Write the command. */
	port = (cmd & 0xFF);
	LCD_IO(port);

	/* Set ENABLE high. */
	port = port | ENABLE ;
	LCD_IO(port);

	/* Set ENABLE low to latch the command. */
	port = port & ~ENABLE;
	LCD_IO(port);
}


/* _lcd_character()
 *
 * Send a character to the LCD.
 *
 * Arguments:
 * data: The character to write to the LCD.
 *
 * Returns:
 * NONE
 *
 */
void _lcd_character(char data)
{

}

/* lcd_init()
 *
 * Setup LCD hardware and prepare the display.
 *
 * Arguments:
 * NONE
 *
 * Returns:
 * NONE
 *
 */
void lcd_init()
{
	/* Setup the I/O expander. */
	ioexp_init(ADDRESS_H);

	/* Wait for the LCD to initialize. */
	_lcd_delay(20);

	_lcd_command(0x38);	//Function set, 8-bit interface, 2 line
	_lcd_delay(1);

	_lcd_command(0x0C);//Display On
	_lcd_delay(1);
}
