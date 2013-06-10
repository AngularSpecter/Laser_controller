/*
 * signalmux.c
 *
 *  Created on: May 16, 2013
 *      Author: Collin
 */

#include "inc/hw_memmap.h"
#include "timer_d.h"
#include "wdt_a.h"
#include "gpio.h"
#include "ucs.h"
#include "pmm.h"
#include "pmap.h"

#include "signalmux.h"
#include "hardware.h"

/* signalmux_init()
 *
 * Setup the signal multiplexing switches. Configures the proper GPIO for later use when selecting a signal routing.
 *
 * Arguments:
 * NONE
 *
 * Returns:
 * NONE
 *
 */
void signalmux_init()
{
	/* Setup GPIO's */
	GPIO_setAsOutputPin(GPIO_PORT_P2, LASER_SOURCE_A_PIN); //Laser source A
	GPIO_setAsOutputPin(GPIO_PORT_P2, LASER_SOURCE_B_PIN); //Laser source B
	GPIO_setAsOutputPin(GPIO_PORT_P2, TRIGGER_OUTPUT_SOURCE_A_PIN); //Delayed Trigger source A
	GPIO_setAsOutputPin(GPIO_PORT_P2, TRIGGER_OUTPUT_SOURCE_B_PIN); //Delayed Trigger source B
}

/* signalmux_route()
 *
 * Configure a signal multiplexer to route a signal onto a certain signal chain.
 *
 * Arguments:
 * mux:		The switch to configure.
 * route:	The route to select on that switch. One of the following:
 * 			SIGNAL_OFF
 *	 	 	SIGNAL_LASER_TRIGGER_EXP_PORT
 *	 	 	SIGNAL_LASER_TRIGGER_MCU_OUTPUT
 *	 	 	SIGNAL_LASER_TRIGGER_EXTERNAL
 *	 	 	SIGNAL_DELAYED_TRIGGER_EXP_PORT
 *	 	 	SIGNAL_DELAYED_TRIGGER_MCU_OUTPUT
 *
 * Returns:
 * NONE
 *
 */

void signalmux_route(MULTIPLEXER_SOURCE mux, uint16_t route)
{
	uint16_t hi = 0;
	uint16_t lo = 0;

	/* Based on the supplied multiplexer, populate the hi/lo outputs. */
	switch (mux)
	{
		case MUX_EXTERNAL_TRIGGER_SOURCE:
			hi = (route & 0x01) ? (hi | TRIGGER_OUTPUT_SOURCE_A_PIN) : hi;//if the low-bit of the route input is high, set the A multiplexer pin.
			hi = (route & 0x02) ? (hi | TRIGGER_OUTPUT_SOURCE_B_PIN) : hi;//if the 2nd bit of the route input is high, set the B multiplexer pin.

			lo = (~hi & (TRIGGER_OUTPUT_SOURCE_A_PIN & TRIGGER_OUTPUT_SOURCE_B_PIN));	//set the pins that are not high, low.
			break;

		case MUX_LASER_TRIGGER_SOURCE:
			hi = (route & 0x01) ? (hi | LASER_SOURCE_A_PIN) : hi;//if the low-bit of the route input is high, set the A multiplexer pin.
			hi = (route & 0x02) ? (hi | LASER_SOURCE_B_PIN) : hi;//if the 2nd bit of the route input is high, set the B multiplexer pin.

			lo = (~hi & (LASER_SOURCE_A_PIN & LASER_SOURCE_B_PIN));	//set the pins that are not high, low.
			break;
	}

	/* Set the output pins. */
	GPIO_setOutputHighOnPin(GPIO_PORT_P2, hi);
	GPIO_setOutputLowOnPin(GPIO_PORT_P2, lo);

}
