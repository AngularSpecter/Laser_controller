/*
 * application.h
 *
 *	Contains declarations for the main functions of the Laser controller.
 *	-RS-232 Automation protocol declarations
 *
 *  Created on: May 25, 2013
 *      Author: Collin
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifndef APPLICATION_H_
#define APPLICATION_H_

#define CMD_DATAMASK 	0x0000FFFF
#define NAK 			0x80000000//commands only use the bottom 16 bits of the return value for actual data, use the upper bits to return success/failure.
#define ACK				0x00000000
#define NO_PARAMETER 	0x40000000

#define CMD_START_DELIMITER	':'
#define CMD_END_DELIMITER	';'
#define CMD_PARAM_DELIMITER '='

/* Define the commands and the command jump table with X-Macros. */
typedef uint32_t (*cmdHandlerFunc)(uint32_t parameter);

/* Define all of the commands in this table. The following X-Macros will create all of the required structures.
 *
 * To add a new command:
 *
 * 1) Add an entry to the following table using the following syntax:
 * TABLE_ENTRY(<Command Name>, <Command string>) \
 *
 * 2) Implement a command handler function with the following signature:
 * 	uint32_t <Command Name>_handler(uint32_t parameter)
 *
 * */

#define COMMAND_TABLE(TABLE_ENTRY) 				\
		TABLE_ENTRY(laserCurrent, "current") 	\
		TABLE_ENTRY(laserTemp, "temp") 			\
		TABLE_ENTRY(laserInterlock, "interlock")\
		TABLE_ENTRY(laserTriggerDelay, "delay")	\
		TABLE_ENTRY(reserved, NULL)

/* Define the number of commands. */
#define CMD_COUNT_ENTRY(x, y) 	uint8_t x;
typedef struct
{
	COMMAND_TABLE(CMD_COUNT_ENTRY)
}cmdSizeStruct;

#define COMMAND_COUNT	sizeof(cmdSizeStruct)


/* Define the command handler prototypes. */
#define CMD_HANDLER_PROTO(x, y)	uint32_t x##_handler(uint32_t parameter);
COMMAND_TABLE(CMD_HANDLER_PROTO)

#ifdef APPLICATION

/* Define the command string table (to compare input commands to). */
#define CMD_STR_ENTRY(x, y)		y,
char* cmd_str_table[] = { COMMAND_TABLE(CMD_STR_ENTRY) };

/* Define the command handler jump table. */
#define CMD_JMP_ENTRY(x, y) &x##_handler,
cmdHandlerFunc command_jmp_table[] = { COMMAND_TABLE(CMD_JMP_ENTRY) };

#endif

void application_processCommandsTask();
#endif /* APPLICATION_H_ */
