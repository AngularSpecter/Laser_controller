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

	/*Only allow a value to be written if in override mode*/
	if (strlen(parameter) != 0)
	{
		if ( laser_getValue(InterlockOverride) )
		{
			param = strtol(parameter, NULL, 10); //convert the parameter to the numeric value.
			laser_setInterlock(param);
		}
		else
		{
			return NAK;
		}
	}

	param = laser_getValue(LaserInterlock);
	sprintf(parameter, "%d", param); //return the current or newly set trigger delay.
	return 0;
}

/* interlockOverride_handler()
 *
 * Returns the laser interlock override status.  Allows the MCU to override the external
 * hardware interlock
 *
 * Arguments:
 * parameter:	The parameter value to the command (if a write operation)
 *
 * Returns:
 * The output of the command to return to the user.
 *
 */
uint32_t interlockOverride_handler(char* parameter)
{
	uint32_t param = 0;

	if (strlen(parameter) != 0)
	{
		param = strtol(parameter, NULL, 10); //convert the parameter to the numeric value.
		laser_setOverride(param);
	}

	param = laser_getValue(InterlockOverride);
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
	if (strlen(parameter) != 0)
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
	if (strlen(parameter) != 0)
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
	if (strlen(parameter) != 0)
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

	if (strlen(parameter) != 0)
	{
		param = strtol(parameter, NULL, 10); //convert the parameter to the numeric value.

		/* Reconfigure the delay trigger based on the parameter. */
		switch (param)
		{
			    /* Delay line disconnected */
		    case EMISSION_OFF:
		    	delay_init(EMISSION_OFF);
		    	break;

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
	if (strlen(parameter) != 0)
	{
		param = strtol(parameter, NULL, 10); //convert the parameter to the numeric value.
		delay_setPeriod(param);
	}

	param = delay_getPeriod();
	sprintf(parameter, "%d", param); //return the current or newly set trigger output width.
	return 0;
}

/* application_processCommand()
 *
 * Calls the appropriate command processing function from the available handlers.
 * Responds with the appropriate response to the command.
 *
 * Arguments:
 * cmdPtr:	A pointer to the command string.
 * valuePtr: A string that holds the value of the command, if one exists.
 *
 * Returns:
 * NONE
 *
 */
void application_processCommand(char* cmdPtr, char* valuePtr)
{
	uint8_t cmd_len = 0;
	uint8_t k = 0;
	uint32_t output = 0;
	char cmd_buffer[BUFFER_LEN];

	cmdHandlerFunc handler = &reserved_handler;

	memset(cmd_buffer, 0, sizeof(cmd_buffer));

	output = NAK; //default error
	for (k = 0; k < COMMAND_COUNT;k++)
	{
		if(strncmp(cmdPtr, cmd_str_table[k], strlen(cmd_str_table[k])) == 0)
		{
			/* A valid command has been found; call the handler. */
			handler = command_jmp_table[k];
			output = handler(valuePtr);
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
		if (output == NO_OUTPUT)
		{
			cmd_len = sprintf(cmd_buffer, ":%s;", cmd_str_table[k]);
		}
		else
		{
			cmd_len = sprintf(cmd_buffer, ":%s=%s;", cmd_str_table[k], valuePtr);
		}
		uart_puts(cmd_buffer, cmd_len);
	}

}

/* Use a pattern recognition state machine to process input commands. */
typedef enum
{
	IDLE = 0, /* Waiting for a command to begin. */
	RX_CMD, /* Receiving a command name. */
	RX_VALUE, /* Receiving a command value. */
	ERROR /* Something bad happened. */

} CommandSequenceState;

/* application_processCmdTasks()
 *
 * Processes input from the UART, tokenizes the input into commands and calls the appropriate command
 * handlers.
 *
 * Arguments:
 * NONE
 *
 * Returns:
 * NONE
 *
 */
void application_processCmdTasks()
{
	static CommandSequenceState sequenceState = IDLE;
	static char cmdBuffer[BUFFER_LEN + 1];
	static char *cmdPtr = cmdBuffer;
	static char valueBuffer[BUFFER_LEN + 1];
	static char *valuePtr = valueBuffer;
	char holdingBuffer[BUFFER_LEN];
	uint8_t i = 0;
	uint8_t dataLen = 0;

	memset(holdingBuffer, 0, BUFFER_LEN);

	/* Read all of the available data. */
	dataLen = uart_gets(holdingBuffer, BUFFER_LEN);

	for (i = 0; i < dataLen; i++)
	{
		/* Process data based on state. */
		switch (sequenceState)
		{
			case IDLE:
				/* If in the idle state, search for the start of command delimiter. */
				if (holdingBuffer[i] == CMD_START_DELIMITER)
				{
					/* A command has been detected, initialize the RX mechanisms. */
					memset(cmdBuffer, 0, sizeof(cmdBuffer));
					memset(valueBuffer, 0, sizeof(valueBuffer));
					cmdPtr = cmdBuffer;
					valuePtr = valueBuffer;

					sequenceState = RX_CMD; //move to the next state
				}
				break;

			case RX_CMD:
				if (holdingBuffer[i] == '=')
				{
					/* The command portion is complete, move on to receive the command value. */
					sequenceState = RX_VALUE;
					break;
				}

				if (holdingBuffer[i] == CMD_START_DELIMITER)
				{
					/* This command has finished, process it. */
					application_processCommand(cmdBuffer, valueBuffer);

					/* A new command has been detected, initialize the RX mechanisms. */
					memset(cmdBuffer, 0, sizeof(cmdBuffer));
					memset(valueBuffer, 0, sizeof(valueBuffer));
					cmdPtr = cmdBuffer;
					valuePtr = valueBuffer;

					sequenceState = RX_CMD; //move to the correct state
					break;
				}

				if (holdingBuffer[i] == CMD_END_DELIMITER)
				{
					/* The command has finished, process it. */
					application_processCommand(cmdBuffer, valueBuffer);
					sequenceState = IDLE;
					break;
				}

				/* Record the command. */
				if ((uint8_t) (cmdPtr - cmdBuffer) < BUFFER_LEN)
				{
					*cmdPtr = holdingBuffer[i];
					cmdPtr++;
				}
				else
				{
					sequenceState = ERROR; //to long, NAK the command.
				}

				break;

			case RX_VALUE:
				if (holdingBuffer[i] == CMD_END_DELIMITER)
				{
					/* The command has finished, process it. */
					application_processCommand(cmdBuffer, valueBuffer);
					sequenceState = IDLE;
					break;
				}

				if (holdingBuffer[i] == CMD_START_DELIMITER)
				{
					/* This command has finished, process it. */
					application_processCommand(cmdBuffer, valueBuffer);

					/* A new command has been detected, initialize the RX mechanisms. */
					memset(cmdBuffer, 0, sizeof(cmdBuffer));
					memset(valueBuffer, 0, sizeof(valueBuffer));
					cmdPtr = cmdBuffer;
					valuePtr = valueBuffer;

					sequenceState = RX_CMD; //move to the correct state
					break;
				}

				/* Record the value. */
				if ((uint8_t) (valuePtr - valueBuffer) < BUFFER_LEN)
				{
					*valuePtr = holdingBuffer[i];
					valuePtr++;
				}
				else
				{
					sequenceState = ERROR; //to long, NAK the command.
				}

				break;

			case ERROR:
				/* Reply with a NAK to indicate failure. */
				uart_puts(":NAK;", 5);
				break;

			default:

				break;
		}

	}
}

