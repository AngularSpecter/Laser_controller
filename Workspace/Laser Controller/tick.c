/*
 * tick.c
 *
 *  Created on: May 27, 2013
 *      Author: Collin
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "inc/hw_memmap.h"
#include "timer_a.h"

#include "tick.h"

static uint32_t tick = 0;

/* tick_init()
 *
 * Initializes the tick hardware components (timer_a)
 *
 * Arguments:
 * NONE
 *
 * Returns:
 * NONE
 *
 */
void tick_init()
{
	tick = 0;

	/* Configure the timer. */
	TIMER_A_startUpMode(
			TIMER_A0_BASE,
			TIMER_A_CLOCKSOURCE_SMCLK,
			TIMER_A_CLOCKSOURCE_DIVIDER_64,
			250,
			TIMER_A_TAIE_INTERRUPT_ENABLE,
			TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE,
			TIMER_A_DO_CLEAR);
}

/* tick_getTime()
 *
 * Get the current time in ticks.
 *
 * Arguments:
 * NONE
 *
 * Returns:
 * The current time in ticks.
 *
 */
tick_t tick_getTime()
{
	tick_t tickValue = 0;

	/* Atomically retrieve the tick counter. */
	TIMER_A_disableInterrupt(TIMER_A0_BASE);
	tickValue = tick;
	TIMER_A_enableInterrupt(TIMER_A0_BASE);

	return tickValue;
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TA0_ISR(void)
{
	tick++;
}
