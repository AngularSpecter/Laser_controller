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
#include "delay.h"
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
 * parameter:	The parameter value to the command (if a write operation)
 *
 * Returns:
 * The output of the command to return to the user.
 *
 */
uint32_t reserved_handler(char* parameter)
{
	return NAK; //invalid command
}

/* laserCurrent_handler()
 *
 * Returns the laser diode current as reported by the laser interface.
 *
 * Arguments:
 * parameter:	The parameter value to the command (if a write operation)
 *
 * Returns:
 * The output of the command to return to the user.
 *
 */
uint32_t laserCurrent_handler(char* parameter)
{
	uint32_t param = 0;

	/* This is a read-only command. */
	if (strlen(parameter) != 0)
	{
		return NAK;
	}

	param = laser_getValue(LaserCurrent);
	sprintf(parameter, "%d", param); //return the current or newly set trigger delay.
	return 0;
}

/* laserTemp_handler()
 *
 * Returns the laser diode temperature as reported by the laser interface.
 *
 * Arguments:
 * parameter:	The parameter value to the command (if a write operation)
 *
 * Returns:
 * The output of the command to return to the user.
 *
 */
uint32_t laserTemp_handler(char* parameter)
{
	uint32_t param = 0;

	/* This is a read-only command. */
	if (strlen(parameter) != 0)
	{
		return NAK;
	}

	param = laser_getValue(LaserTemperature);
	sprintf(parameter, "%d", param); //return the current or newly set trigger delay.
	return 0;
}

/* laserInterlock_handler()
 *
 * Returns the laser interlock status as measured from the external input.
 *
 * Arguments:
 * parameter:	The parameter value to the command (if a write operation)
 *
 * Returns:
 * The output of the command to return to the user.
 *
 */
uint32_t laserInterlock_handler(char* parameter)
{
	uint32_t param = 0;

	/* This is a read-only command. */
	if (strlen(parameter) != 0)
	{
		return NAK;
	}

	param = laser_getValue(LaserInterlock);
	sprintf(parameter, "%d", param); //return the current or newly set trigger delay.
	return 0;
}

/* laserTriggerDelay_handler()
 *
 * Sets and returns the laser trigger delay.
 *
 * Arguments:
 * parameter:	The parameter value to the command (if a write operation)
 *
 * Returns:
 * The output of the command to return to the user.
 *
 */
uint32_t laserTriggerDelay_handler(char* parameter)
{
	uint32_t param = 0;

	/* Set the new laser trigger delay. */
	if (parameter != NULL)
	{
		param = strtol(parameter, NULL, 10); //convert the parameter to the numeric value.
		delay_setDelay(param);
	}

	param = delay_getDelay();
	sprintf(parameter, "%d", param); //return the current or newly set trigger delay.
	return 0;
}

/* laserTriggerWidth_handler()
 *
 * Sets and returns the laser trigger output pulse width.
 *
 * Arguments:
 * parameter:	The parameter value to the command (if a write operation)
 *
 * Returns:
 * The output of the command to return to the user.
 *
 */
uint32_t laserTriggerWidth_handler(char* parameter)
{
	uint32_t param = 0;

	/* Set the new laser trigger output pulse width. */
	if (parameter != NULL)
	{
		param = strtol(parameter, NULL, 10); //convert the parameter to the numeric value.
		delay_setPulseWidth(param);
	}

	param = delay_getPulseWidth();
	sprintf(parameter, "%d", param); //return the current or newly set trigger output width.
	return 0;
}

uint32_t laserTriggerEnable_handler(char* parameter)
{
	uint32_t param = 0;

	/* Enable or disable the delay trigger. */
	if (parameter != NULL)
	{
		param = strtol(parameter, NULL, 10); //convert the parameter to the numeric value.
		delay_setEnable(param);
	}

	param = delay_isEnabled();
	sprintf(parameter, "%d", param);
	return 0;
}

uint32_t lasertriggerMode_handler(char* parameter)
{
	/* Setup trigger signal chain. */
	uint32_t param = 0;

	if (parameter != NULL)
	{
		param = strtol(parameter, NULL, 10); //convert the parameter to the numeric value.

		/* Reconfigure the delay trigger based on the parameter. */
		switch (param)
		{
			/* External Delay. */
			case DELAYED_TRIGGER:
				delay_init(DELAYED_TRIGGER);
				break;

				/* Internally Generated. */
			case GENERATED_TRIGGER:
				delay_init(GENERATED_TRIGGER);
				break;

			default:
				return NAK; //invalid parameter
		}
	}

	param = delay_getMode();
	sprintf(parameter, "%d", param);
	return 0;
}

uint32_t lasertriggerPeriod_handler(char* parameter)
{
	uint32_t param = 0;

	/* Set the new laser trigger output pulse width. */
	if (parameter != NULL)
	{
		param = strtol(parameter, NULL, 10); //convert the parameter to the numeric value.
		delay_setPeriod(param);
	}

	param = delay_getPeriod();
	sprintf(parameter, "%d", param); //return the current or newly set trigger output width.
	return 0;
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
	char param_buffer[BUFFER_LEN];
	uint8_t rx_len = 0;
	uint8_t cmd_len = 0;
	char* cmd_start = NULL;
	char* cmd_end = NULL;
	char* param_start = NULL;

	uint32_t output = 0;

	cmdHandlerFunc handler = &reserved_handler;

	/* Initialize the buffer. */
	memset(cmd_buffer, 0, sizeof(cmd_buffer));
	memset(param_buffer, 0, sizeof(param_buffer));

	/* Peek at the received characters in the buffer to see if a whole command has been received. */
	rx_len = uart_peeks(cmd_buffer, BUFFER_LEN);

	/* Search for a valid command block in the peeked buffer. */
	cmd_start = strchr(cmd_buffer, CMD_START_DELIMITER);
	if (cmd_start == NULL)
	{
		return; //if NULL is returned, the start delimiter was not found, therefore no command has been received.
	}
	cmd_start++; //skip the CMD_START_DELIMITER.

	cmd_end = strchr(cmd_buffer, CMD_END_DELIMITER);
	if (cmd_end == NULL)
	{
		return; //A command has been started, but has not been received completely yet.
	}
	cmd_end++; //include the CMD_END_DELIMITER

	/* A command has been received, remove it from the input stream (but no extra characters). */
	cmd_len = (cmd_end - cmd_buffer);
	rx_len = uart_gets(cmd_buffer, cmd_len); //retrieve only the characters that are part of the command.
	if (rx_len != cmd_len)
	{
		return; //something bad happened, ignore this command.
	}

	/* Process the command. */
	param_start = strchr(cmd_buffer, CMD_PARAM_DELIMITER); //get the start of the parameter portion of the
	if (param_start != NULL)
	{
		strncpy(param_buffer, param_start + 1, (cmd_end - param_start));
	}

	/* Search for the command, and call the appropriate handler function. */
	output = NAK; //default error
	for (k = 0; k < COMMAND_COUNT;k++)
	{
		if(strncmp(cmd_start, cmd_str_table[k], strlen(cmd_str_table[k])) == 0)
		{
			/* A valid command has been found; call the handler. */
			handler = command_jmp_table[k];
			output = handler(param_buffer);
			break;
		}
	}

	if (output & NAK)
	{
		/* Reply with a NAK to indicate failure. */
		uart_puts(":NAK;", 5);
	}
	else
	{
		/* Reply to the command. */
		cmd_len = 0;
		if (output == NO_OUTPUT)
		{
			cmd_len = sprintf(cmd_buffer, ":%s;", cmd_str_table[k]);
		}
		else
		{
			cmd_len = sprintf(cmd_buffer, ":%s=%s;", cmd_str_table[k], param_buffer);
		}
		uart_puts(cmd_buffer, cmd_len);
	}

}
