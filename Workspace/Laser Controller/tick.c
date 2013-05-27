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

static tick_t tick = 0;

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
	TIMER_A_clearCaptureCompareInterruptFlag(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0);
	TIMER_A_configureUpMode(
			TIMER_A0_BASE,
			TIMER_A_CLOCKSOURCE_SMCLK,
			TIMER_A_CLOCKSOURCE_DIVIDER_1,
			16000,
			TIMER_A_TAIE_INTERRUPT_DISABLE,
			TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE,
			TIMER_A_DO_CLEAR);

	TIMER_A_enableInterrupt(TIMER_A0_BASE);

	TIMER_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);
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
	//TIMER_A_disableInterrupt(TIMER_A0_BASE);
	tickValue = tick;
	//TIMER_A_enableInterrupt(TIMER_A0_BASE);

	return tickValue;
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TA0_ISR(void)
{
	//Any access, read or write, of the TAIV register automatically resets the
	//highest "pending" interrupt flag
	switch (__even_in_range(TA0IV, 14))
	{
		case 0:
			break;                          //No interrupt
		case 2:
			break;
		case 4:
			break;                          //CCR2 not used
		case 6:
			break;                          //CCR3 not used
		case 8:
			break;                          //CCR4 not used
		case 10:
			break;                          //CCR5 not used
		case 12:
			break;                          //CCR6 not used
		case 14:
			tick++;
			break;
		default:
			break;
	}
}
