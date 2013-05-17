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

static uint32_t delay = 0;

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
	/*** Configure TimerD0 for dual capture mode on signals incoming to TTL_IN. ***/

	/* Setup the timer component. */
	TIMER_D_configureContinuousMode(
			TIMER_D0_BASE,
			TIMER_D_CLOCKSOURCE_SMCLK,
			TIMER_D_CLOCKSOURCE_DIVIDER_1,
			TIMER_D_CLOCKINGMODE_EXTERNAL_CLOCK,
			TIMER_D_TDIE_INTERRUPT_DISABLE,
			TIMER_D_DO_CLEAR); //configure continuous count up mode for the timer.

	//TIMER_D_startCounter(TIMER_D0_BASE, TIMER_D_CONTINUOUS_MODE); //start the timer.

	TIMER_D_clearCaptureCompareInterruptFlag(TIMER_D0_BASE, TIMER_D_CAPTURECOMPARE_REGISTER_2);	//clear the interrupt before it is enabled.

	/* Setup the capture mode for the TTL IN Signal. */
	GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P3, GPIO_PIN1);//setup MCU_TTL1 as a compare output.

	TIMER_D_initCapture(
			TIMER_D0_BASE,
			TIMER_D_CAPTURECOMPARE_REGISTER_2,
			TIMER_D_CAPTUREMODE_RISING_AND_FALLING_EDGE,
			TIMER_D_CAPTURE_INPUTSELECT_CCIxA,
			TIMER_D_CAPTURE_SYNCHRONOUS,
			TIMER_D_CAPTURECOMPARE_INTERRUPT_ENABLE,
			TIMER_D_OUTPUTMODE_OUTBITVALUE,
			TIMER_D_DUAL_CAPTURE_MODE);

	/* Setup the compare mode for the TTL OUT Signal. */
	TIMER_D_initCompare(
			TIMER_D1_BASE,
			TIMER_D_CAPTURECOMPARE_REGISTER_2,
			TIMER_D_CAPTURECOMPARE_INTERRUPT_DISABLE,
			TIMER_D_OUTPUTMODE_SET_RESET,
			50000);

	/*** Configure TimerD1 for output compare mode to generate a delayed copy of TTL_IN ***/
	TIMER_D_configureContinuousMode(
			TIMER_D1_BASE,
			TIMER_D_CLOCKSOURCE_ACLK,
			TIMER_D_CLOCKSOURCE_DIVIDER_1,
			TIMER_D_CLOCKINGMODE_EXTERNAL_CLOCK,
			TIMER_D_TDIE_INTERRUPT_DISABLE,
			TIMER_D_DO_CLEAR);

	TIMER_D_setCompareValue(TIMER_D1_BASE, TIMER_D_CAPTURECOMPARE_REGISTER_2, 25000);

	TIMER_D_startCounter(TIMER_D1_BASE, TIMER_D_CONTINUOUS_MODE);	//start the timer.

}

// TD0_D1 Interrupt vector
#pragma vector = TIMER0_D1_VECTOR
__interrupt void TIMER0_D1_ISR(void)
{
	switch (__even_in_range(TD0IV, 0x1E))
	{
		// Vector  0:  No interrupt
		case 0x0:
			break;
			// Vector  2:  TDCCR1 CCIFG
		case 0x2:
			break;
			// Vector  4:  TDCCR2 CCIFG
		case 0x4:

			TIMER_D_getCaptureCompareCount(TIMER_D0_BASE, TIMER_D_CAPTURECOMPARE_REGISTER_1);

			TIMER_D_getCaptureCompareLatchCount(TIMER_D0_BASE, TIMER_D_CAPTURECOMPARE_REGISTER_1);
			break;

			// Vector  6:  TDCCR3 CCIFG
		case 0x6:
			break;
			// Vector  8:  TDCCR4 CCIFG
		case 0x8:
			break;
			// Vector 10:  TDCCR5 CCIFG
		case 0xA:
			break;
			// Vector 12:  TDCCR5 CCIFG
		case 0xC:
			break;
			// Vector 14:  -
		case 0xE:
			break;
			// Vector 16:  TDIFG
		case 0x10:
			break;
			// Vector 18:  TDHINT TDHFLIFG
		case 0x12:
			break;
			// Vector 20:  TDHINT TDHFHIFG
		case 0x14:
			break;
			// Vector 22:  TDHINT TDHLKIFG
		case 0x16:
			break;
			// Vector 24:  TDHINT TDHUNLKIFG
		case 0x18:
			break;
			// Vector 26:  -
		case 0x1A:
			break;
			// Vector 28:  -
		case 0x1C:
			break;
			// Vector 28:  -
		case 0x1E:
			break;
		default:
			break;
	}
}
