/*
 * delay.c
 *
 * Implements the delay line.
 *
 *
 *  Created on: May 14, 2013
 *      Author: Collin
 */

#include <stdio.h>
#include <stdint.h>

#include "timer_d.h"
#include "gpio.h"

#include "inc/hw_memmap.h"

#include "delay.h"

#define MAX_WIDTH			5000
#define MAX_DELAY_PERIOD	(0xFFFF - MAX_WIDTH)

#define DEFAULT_WIDTH		500
#define DEFAULT_DELAY		10000

static uint16_t next_delay = DEFAULT_DELAY;
static uint16_t delay = DEFAULT_DELAY;

static uint16_t next_width = DEFAULT_WIDTH;
static uint16_t width = DEFAULT_WIDTH;

typedef enum
{
	delay_idle = 0,			//waiting for an incoming edge
	delay_triggered		//The pulse has been triggered, waiting for the delay to complete.

} DELAY_STATE;

static DELAY_STATE state = delay_idle;

/* delay_init()
 *
 * Setup the TimerD hardware for use as a delay chain.
 *
 * Arguments:
 * NONE
 *
 * Returns:
 * NONE
 *
 */
void delay_init()
{
	/* Setup the state machine. */
	state = delay_idle;

	next_delay = DEFAULT_DELAY;
	delay = DEFAULT_DELAY;

	next_width = DEFAULT_WIDTH;
	width = DEFAULT_WIDTH;

	/*** Configure TimerD0 for capture mode on signals incoming to TTL_IN. ***/

	/* Setup the timer component. */
	GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1, GPIO_PIN7);
	TIMER_D_configureContinuousMode(
			TIMER_D0_BASE,
			TIMER_D_CLOCKSOURCE_SMCLK,
			TIMER_D_CLOCKSOURCE_DIVIDER_1,
			TIMER_D_CLOCKINGMODE_EXTERNAL_CLOCK,
			TIMER_D_TDIE_INTERRUPT_DISABLE,
			TIMER_D_DO_CLEAR);
	TIMER_D_startCounter(TIMER_D0_BASE, TIMER_D_CONTINUOUS_MODE);

	TIMER_D_clearCaptureCompareInterruptFlag(TIMER_D0_BASE, TIMER_D_CAPTURECOMPARE_REGISTER_1);
	TIMER_D_initCapture(
			TIMER_D0_BASE,
			TIMER_D_CAPTURECOMPARE_REGISTER_1,
			TIMER_D_CAPTUREMODE_RISING_EDGE,
			TIMER_D_CAPTURE_INPUTSELECT_CCIxA,
			TIMER_D_CAPTURE_SYNCHRONOUS,
			TIMER_D_CAPTURECOMPARE_INTERRUPT_ENABLE,
			TIMER_D_OUTPUTMODE_OUTBITVALUE,
			TIMER_D_SINGLE_CAPTURE_MODE);

	/* Setup the I/O forthe TTL OUT Signal. */
	GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P3, GPIO_PIN1); //setup MCU_TTL1 as a compare output.

}

/* delay_setDelay()
 *
 * Change the delay of the delay chain.
 *
 * Arguments:
 * new_delay:		The new delay of the delay chain.
 *
 * Returns:
 * NONE
 *
 */
void delay_setDelay(uint16_t new_delay)
{
	if (new_delay > MAX_DELAY_PERIOD)
	{
		next_delay = new_delay;	//queue up the next delay value.
	}
	else
	{
		next_delay = MAX_DELAY_PERIOD;
	}
}

/* delay_getDelay()
 *
 * Get the delay set-point for the delay chain.
 *
 * Arguments:
 * NONE
 *
 * Returns:
 * The delay set-point.
 *
 */
uint16_t delay_getDelay()
{
	//return the next delay, since all following pulses will use it and not the current delay.
	return next_delay;
}

/* delay_setPulseWidth()
 *
 * Change the width of the output pulse.
 *
 * Arguments:
 * new_width:		The new width of the output pulse.
 *
 * Returns:
 * NONE
 *
 */
void delay_setPulseWidth(uint16_t new_width)
{
	if (new_width > MAX_DELAY_PERIOD)
	{
		next_width = new_width;	//queue up the next delay value.
	}
	else
	{
		next_width = MAX_WIDTH;
	}
}

/* delay_getPulseWidth()
 *
 * Get the width set-point of the output pulse.
 *
 * Arguments:
 * NONE
 *
 * Returns:
 * The current pulse-width set-point.
 *
 */
uint16_t delay_getPulseWidth()
{
	//return the next width, since all following pulses will use it and not the current width.
	return next_width;
}

#pragma vector = TIMER1_D1_VECTOR
__interrupt void TIMER1_D1_ISR(void)
{
	if (TD1IV & TD1IV_TDIFG)
	{
		/* The end of the delay period has been reached, stop the timer. */
		if (state == delay_triggered)
		{
			TIMER_D_stop(TIMER_D1_BASE);
			state = delay_idle;	//Pulse output is complete, return to idle.
		}
	}
}

// TD0_D1 Interrupt vector
#pragma vector = TIMER0_D1_VECTOR
__interrupt void TIMER0_D1_ISR(void)
{
	if (TD0IV & TD0IV_TDCCR1)
	{
		switch (state)
		{
			/* The delay subsystem hasn't been triggered yet, and the rising edge has just been detected. */
			case delay_idle:

				/* Update the delay and width that may have changed since the last trigger. */
				delay = next_delay;
				width = next_width;

				/* Setup the output timer to create a PWM of a fixed width, delayed by the requested amount. */
				TIMER_D_configureUpMode(
						TIMER_D1_BASE,
						TIMER_D_CLOCKSOURCE_ACLK,
						TIMER_D_CLOCKSOURCE_DIVIDER_1,
						TIMER_D_CLOCKINGMODE_EXTERNAL_CLOCK,
						(delay + width),
						TIMER_D_TDIE_INTERRUPT_ENABLE,
						TIMER_D_CCIE_CCR0_INTERRUPT_DISABLE,
						TIMER_D_DO_CLEAR);

				TIMER_D_initCompare(
						TIMER_D1_BASE,
						TIMER_D_CAPTURECOMPARE_REGISTER_2,
						TIMER_D_CAPTURECOMPARE_INTERRUPT_DISABLE,
						TIMER_D_OUTPUTMODE_SET_RESET,
						delay);

				state = delay_triggered; //The delay has now been triggered, move to the triggered state.

				TIMER_D_startCounter(TIMER_D1_BASE, TIMER_D_UP_MODE); //start the delay period.
				break;

			default:
				break;
		}
	}
}
