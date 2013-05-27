/*
 * application.c
 *
 *  Created on: May 25, 2013
 *      Author: Collin
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "inc/hw_memmap.h"
#include "gpio.h"
#include "pmap.h"
#include "ucs.h"

#define APPLICATION 1
#include "application.h"
#include "buffer.h"
#include "hardware.h"
#include "laser.h"
#include "laser_uart.h"
#include "lcd.h"
#include "tca6416A.h"

/* reserved_handler()
 *
 * Called when an unrecognized or unsupported command is received.
 *
 * Arguments:
 * parameter:	The numeric parameter to the command.
 *
 * Returns:
 * The output of the command to return to the user.
 *
 */
uint32_t reserved_handler(uint32_t parameter)
{
	return NAK; //invalid command
}

/* laserCurrent_handler()
 *
 * Returns the laser diode current as reported by the laser interface.
 *
 * Arguments:
 * parameter:	The numeric parameter to the command.
 *
 * Returns:
 * The output of the command to return to the user.
 *
 */
uint32_t laserCurrent_handler(uint32_t parameter)
{
	/* This is a read-only command. */
	if(parameter != NO_PARAMETER)
	{
		return NAK;
	}

	return laser_getValue(LaserCurrent);
}

/* laserTemp_handler()
 *
 * Returns the laser diode temperature as reported by the laser interface.
 *
 * Arguments:
 * parameter:	The numeric parameter to the command.
 *
 * Returns:
 * The output of the command to return to the user.
 *
 */
uint32_t laserTemp_handler(uint32_t parameter)
{
	/* This is a read-only command. */
	if(parameter != NO_PARAMETER)
	{
		return NAK;
	}

	return laser_getValue(LaserTemperature);
}

/* laserInterlock_handler()
 *
 * Returns the laser interlock status as measured from the external input.
 *
 * Arguments:
 * parameter:	The numeric parameter to the command.
 *
 * Returns:
 * The output of the command to return to the user.
 *
 */
uint32_t laserInterlock_handler(uint32_t parameter)
{
	return 0;
}

/* laserTriggerDelay_handler()
 *
 * Sets and returns the laser trigger delay.
 *
 * Arguments:
 * parameter:	The numeric parameter to the command.
 *
 * Returns:
 * The output of the command to return to the user.
 *
 */
uint32_t laserTriggerDelay_handler(uint32_t parameter)
{
	/* Set the new laser trigger delay. */
	if(parameter != NO_PARAMETER)
	{
		delay_setDelay(parameter);
	}

	return delay_getDelay();//return the current or newly set trigger delay.
}

/* application_processCommandsTask()
 *
 * Processes received commands and calls the appropriate command handler.
 *
 * Arguments:
 * NONE
 *
 * Returns:
 * NONE
 *
 */
void application_processCommandsTask()
{
	uint16_t k = 0;
	char cmd_buffer[BUFFER_LEN];
	uint8_t rx_len = 0;
	uint8_t cmd_len = 0;
	char* cmd_start = NULL;
	char* cmd_end = NULL;
	char* param_start = NULL;

	uint32_t parameter = 0;
	uint32_t output = 0;

	cmdHandlerFunc handler = &reserved_handler;

	/* Initialize the buffer. */
	memset(cmd_buffer, 0, sizeof(cmd_buffer));

	/* Peek at the received characters in the buffer to see if a whole command has been received. */
	rx_len = uart_peeks(cmd_buffer, BUFFER_LEN);

	/* Search for a valid command block in the peeked buffer. */
	cmd_start = strchr(cmd_buffer, CMD_START_DELIMITER) ;
	if (cmd_start == NULL)
	{
		return; //if NULL is returned, the start delimiter was not found, therefore no command has been received.
	}
	cmd_start++;//skip the CMD_START_DELIMITER.

	cmd_end = strchr(cmd_buffer, CMD_END_DELIMITER);
	if (cmd_end == NULL)
	{
		return; //A command has been started, but has not been received completely yet.
	}
	cmd_end++;//include the CMD_END_DELIMITER

	/* A command has been received, remove it from the input stream (but no extra characters). */
	cmd_len = (cmd_end - cmd_buffer);
	rx_len = uart_gets(cmd_buffer, cmd_len); //retrieve only the characters that are part of the command.
	if (rx_len != cmd_len)
	{
		return; //something bad happened, ignore this command.
	}

	/* Process the command. */
	param_start = strchr(cmd_buffer, CMD_PARAM_DELIMITER); //get the start of the parameter portion of the
	if (param_start == NULL)
	{
		parameter = NO_PARAMETER; //no parameter was attached to the command, report it to the handler.
	}
	else
	{
		/* Process the parameter. */
		parameter = strtol((param_start + 1), NULL, 10); //convert the parameter to a number (param_start is the pointer to the delimiter, so add one)
	}

	/* Search for the command, and call the appropriate handler function. */
	for (k = 0; k < COMMAND_COUNT;k++)
	{
		if(strncmp(cmd_start, cmd_str_table[k], strlen(cmd_str_table[k])) == 0)
		{
			/* A valid command has been found; call the handler. */
			handler = command_jmp_table[k];
			output = handler(parameter);
			break;
		}
	}

	if (output == NAK)
	{
		/* Reply with a NAK to indicate failure. */
		uart_puts(":NAK;", 5);
	}
	else
	{
		/* Reply to the command. */
		cmd_len = 0;
		if (output == NO_PARAMETER)
		{
			cmd_len = sprintf(cmd_buffer, ":%s;", cmd_str_table[k]);
		}
		else
		{
			cmd_len = sprintf(cmd_buffer, ":%s=%d;", cmd_str_table[k], output);
		}
		uart_puts(cmd_buffer, cmd_len);
	}

}
