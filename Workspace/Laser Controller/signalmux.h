/*
 * signalmux.h
 *
 *  Created on: May 16, 2013
 *      Author: Collin
 */

#include <stdint.h>

#include "inc/hw_memmap.h"
#include "gpio.h"


#ifndef SIGNALMUX_H_
#define SIGNALMUX_H_

typedef enum
{
	MUX_LASER_TRIGGER_SOURCE = 0,
	MUX_EXTERNAL_TRIGGER_SOURCE
}MULTIPLEXER_SOURCE;

#define SIGNAL_OFF 				0

/* MUX_LASER_TRIGGER_SOURCE Signal Chains */
#define SIGNAL_LASER_OFF			    0
#define SIGNAL_LASER_TRIGGER_EXP_PORT	1
#define SIGNAL_LASER_TRIGGER_MCU_OUTPUT	2
#define SIGNAL_LASER_TRIGGER_EXTERNAL	3

/* MUX_DELAYED_TRIGGER_SOURCE Signal Chains */
#define SIGNAL_EXTERNAL_TRIGGER_OFF			0
#define SIGNAL_EXTERNAL_TRIGGER_EXP_PORT	1
#define SIGNAL_EXTERNAL_TRIGGER_MCU_AUX_OUTPUT	2
#define SIGNAL_EXTERNAL_TRIGGER_MCU_OUTPUT	3

void signalmux_init();

void signalmux_route(MULTIPLEXER_SOURCE mux, uint16_t signal);

#endif /* SIGNALMUX_H_ */
