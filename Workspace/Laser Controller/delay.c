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
#include "signalmux.h"

#define MAX_WIDTH			5000
#define MAX_DELAY_PERIOD	(0xFFFF - MAX_WIDTH)
#define MAX_PERIOD			(0xFFFF - MAX_WIDTH)

#define DEFAULT_WIDTH		500
#define DEFAULT_DELAY		10000
#define DEFAULT_PERIOD		10000

static uint16_t next_delay = DEFAULT_DELAY;
static uint16_t delay = DEFAULT_DELAY;

static uint16_t next_width = DEFAULT_WIDTH;
static uint16_t width = DEFAULT_WIDTH;

static uint16_t period = DEFAULT_PERIOD;

typedef enum
{
	delay_idle = 0,			//waiting for an incoming edge
	delay_triggered,		//The pulse has been triggered, waiting for the delay to complete.
	delay_disabled,			//trigger output is disabled.
	delay_generator			//The trigger signals

} DELAY_STATE;

static DELAY_STATE state = delay_idle;

/* _delay_setupGeneratedTrigger()
 *
 * Configure the TimerD module to create a generated trigger.
 * Uses the <period> and <width> variables as PWM parameters.
 *
 * Arguments:
 * NONE
 *
 * Returns:
 * NONE
 *
 */
void _delay_setupGeneratedTrigger()
{
	/* Setup the output timer to create a PWM of a fixed width, delayed by the requested amount. */
	TIMER_D_configureUpMode(
			TIMER_D1_BASE,
			TIMER_D_CLOCKSOURCE_ACLK,
			TIMER_D_CLOCKSOURCE_DIVIDER_1,
			TIMER_D_CLOCKINGMODE_EXTERNAL_CLOCK,
			period,
			TIMER_D_TDIE_INTERRUPT_DISABLE,
			TIMER_D_CCIE_CCR0_INTERRUPT_DISABLE,
			TIMER_D_DO_CLEAR);

	TIMER_D_initCompare(
			TIMER_D1_BASE,
			TIMER_D_CAPTURECOMPARE_REGISTER_2,
			TIMER_D_CAPTURECOMPARE_INTERRUPT_DISABLE,
			TIMER_D_OUTPUTMODE_RESET_SET,
			width);

	/* Setup the I/O for the TTL OUT Signal. */
	GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P3, GPIO_PIN1); //setup MCU_TTL1 as a compare output.

	TIMER_D_startCounter(TIMER_D1_BASE, TIMER_D_UP_MODE); //start the delay period.
}

/* delay_init()
 *
 * Setup the TimerD hardware for use as a delay chain.
 *
 * Arguments:
 * mode:	The mode to configure the delay system:
 * 			DELAYED_TRIGGER
 * 			GENERATED_TRIGGER
 *
 * Returns:
 * NONE
 *
 */
void delay_init(uint8_t mode)
{
	/* Stop all of the timers, in case they were started by a previous call. */
	TIMER_D_stop(TIMER_D0_BASE);
	TIMER_D_stop(TIMER_D1_BASE);

	if (mode == DELAYED_TRIGGER)
	{
		/* Setup signal routing. */
		signalmux_init();
		signalmux_route(MUX_LASER_TRIGGER_SOURCE, SIGNAL_LASER_TRIGGER_EXTERNAL);
		signalmux_route(MUX_DELAYED_TRIGGER_SOURCE, SIGNAL_DELAYED_TRIGGER_MCU_OUTPUT);

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

		/* Setup the I/O for the TTL OUT Signal. */
		GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P3, GPIO_PIN1); //setup MCU_TTL1 as a compare output.
	}
	else if (mode == GENERATED_TRIGGER)
	{
		state = delay_generator;

		period = DEFAULT_PERIOD;
		_delay_setupGeneratedTrigger();
	}
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
	if (new_delay < MAX_DELAY_PERIOD)
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
	if (new_width < MAX_WIDTH)
	{
		next_width = new_width;	//queue up the next delay value.
	}
	else
	{
		next_width = MAX_WIDTH;
	}

	if (state == delay_generator)
	{
		width = next_width;	//this won't be assigned in the interrupt, so assign it here.

		/* Re-configure the output to use the new period. */
		_delay_setupGeneratedTrigger();
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

/* delay_setPeriod()
 *
 * Set the new period of the of the generated trigger.
 *
 * Arguments:
 * new_period:	The new period to use in the generated trigger.
 *
 * Returns:
 * NONE
 *
 */
void delay_setPeriod(uint16_t new_period)
{
	if (new_period < MAX_PERIOD)
	{
		period = new_period;	//queue up the next delay value.
	}
	else
	{
		period = MAX_PERIOD;
	}

	/* Re-configure the output to use the new period. */
	_delay_setupGeneratedTrigger();
}

/* delay_getPeriod()
 *
 * Get the period of the generated trigger.
 *
 * Arguments:
 * NONE
 *
 * Returns:
 * The current output period.
 *
 */
uint16_t delay_getPeriod()
{
	return period;
}

/* delay_setEnable()
 *
 * Enable or disable the delay trigger.
 *
 * Arguments:
 * enable:	A non-zero value will enable the trigger delay. 0 will disable the subsystem.
 *
 * Returns:
 * NONE
 *
 */
void delay_setEnable(uint8_t enable)
{
	if (enable)
	{
		state = delay_idle;
	}
	else
	{
		state = delay_disabled;

		/* Stop all of the timers, in case they were started by a previous call. */
		TIMER_D_stop(TIMER_D0_BASE);
		TIMER_D_stop(TIMER_D1_BASE);
	}
}

/* delay_isEnabled()
 *
 * Get the status of the trigger delay.
 *
 * Arguments:
 * NONE
 *
 * Returns:
 * 1, if the delay system is active, 0 otherwise.
 *
 */
uint16_t delay_isEnabled()
{
	return (state != delay_disabled);
}

/* delay_getMode()
 *
 * Get the mode that the delay system is configured for.
 *
 * Arguments:
 * NONE
 *
 * Returns:
 * The mode that the system was configured with in delay_init():
 * DELAYED_TRIGGER
 * GENERATED_TRIGGER
 *
 */
uint16_t delay_getMode()
{

	/* Check the state of the delay system, it can be used to derive the mode
	 * the module is configured for.
	 */
	if (state == delay_generator)
	{
		return GENERATED_TRIGGER;
	}

	return DELAYED_TRIGGER;

}

#pragma vector = TIMER1_D1_VECTOR
__interrupt void TIMER1_D1_ISR(void)
{
	if (TD1IV & TD1IV_TDIFG)
	{
		TIMER_D_stop(TIMER_D1_BASE);
		/* The end of the delay period has been reached, stop the timer. */
		if (state == delay_triggered)
		{
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
