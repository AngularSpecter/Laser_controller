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
#include "ucs.h"

#include "tca6416A.h"
#include "lcd.h"
#include "hardware.h"

#define RW_BIT		10
#define ENABLE_BIT 	9
#define RS_BIT		8

#define INDEX_TO_BIT(x)		(1 << x)
#define ENABLE				INDEX_TO_BIT(ENABLE_BIT)
#define RW					INDEX_TO_BIT(RW_BIT)
#define RS					INDEX_TO_BIT(RS_BIT)

#define LCD_IO(x)			ioexp_setOutput(ADDRESS_H, x)

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
		__delay_cycles(XTAL_FREQUENCY / 1000);
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

	_lcd_delay(2);//satisfy setup & hold time

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
	uint16_t port = 0x0000;

	/* Write the command. */
	port = (data & 0xFF) | RS;
	LCD_IO(port);

	/* Set ENABLE high. */
	port = port | ENABLE ;
	LCD_IO(port);

	//_lcd_delay(2);//satisfy setup & hold time

	/* Set ENABLE low to latch the command. */
	port = port & ~ENABLE;
	LCD_IO(port);
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
	LCD_IO(0x00);					//initialize output all low
	ioexp_setIO(ADDRESS_H, 0x00);	//set all the pins to output.


	/* Wait for the LCD to initialize. */
	_lcd_delay(20);

	_lcd_command(0x38);	//Function set, 8-bit interface, 2 line
	_lcd_delay(2);

	_lcd_command(0x06);	//Entry Mode set, automatic increments, no shift.
	_lcd_delay(2);

	_lcd_command(0x0E);	//Display On
	_lcd_delay(2);

	_lcd_command(0x01); //reset display
	_lcd_delay(2);
}

/* lcd_putc()
 *
 * Print a character to the LCD.
 *
 * '\r' will reset the cursor to the home position
 *
 * Arguments:
 * data: The character to print the the LCD.
 *
 * Returns:
 * NONE
 *
 */
void lcd_putc(char data)
{
	/* Catch any special characters and adjust the LCD appropriately.
	 * Otherwise, just write to the LCD.
	 */
	switch(data)
	{
		/* Reset the display. */
		case '\r':
			_lcd_command(0x02);
			break;

		default:
			_lcd_character(data);
			break;
	}
}

/* lcd_puts()
 *
 * Print a string of characters to the LCD.
 *
 * Arguments:
 * str:	The string to print to the LCD.
 * len:	The length of the string to print to the LCD.
 *
 * Returns:
 * NONE
 *
 */
void lcd_puts(char* str, uint16_t len)
{
	uint8_t k = 0;

	for(k = 0;k < len;k++)
	{
		lcd_putc(str[k]);
	}
}

/* lcd_setCursor()
 *
 * Set the cursor on the LCD to a new location.
 *
 * Arguments:
 * location:	The new location to set the cursor too.
 *
 * Returns:
 * NONE
 *
 */
void lcd_setCursor(uint8_t location)
{
	_lcd_command((0x80 | (location & 0x7F)));//issue set DRAM address command with the requested location.
}


/* lcd_clearDisplay()
 *
 * Clear all of the characters on the screen.
 *
 * Arguments:
 * NONE
 *
 * Returns:
 * NONE
 *
 */
void lcd_clearDisplay()
{
	_lcd_command(0x01);
}
