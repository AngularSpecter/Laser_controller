/*
 * laser.c
 *
 *  Created on: May 20, 2013
 *      Author: Collin
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "inc/hw_memmap.h"

#include "adc10_a.h"
#include "ref.h"
#include "gpio.h"

#include "laser.h"

/* laser_init()
 *
 * Setup the Laser I/O Subsystem hardware.
 *
 * Arguments:
 * NONE
 *
 * Returns:
 * NONE
 *
 */
void laser_init()
{

	//Initialize the ADC10_A Module
	/*
	 * Base Address for the ADC10_A Module
	 * Use internal ADC10_A bit as sample/hold signal to start conversion
	 * USE MODOSC 5MHZ Digital Oscillator as clock source
	 * Use default clock divider of 1
	 */
	ADC10_A_init(
			ADC10_A_BASE,
			ADC10_A_SAMPLEHOLDSOURCE_SC,
			ADC10_A_CLOCKSOURCE_ADC10OSC,
			ADC10_A_CLOCKDIVIDER_1);

	ADC10_A_enable(ADC10_A_BASE);

	/*
	 * Base Address for the ADC10_A Module
	 * Sample/hold for 16 clock cycles
	 * Do not enable Multiple Sampling
	 */
	ADC10_A_setupSamplingTimer(
			ADC10_A_BASE,
			ADC10_A_CYCLEHOLD_16_CYCLES,
			ADC10_A_MULTIPLESAMPLESDISABLE);

	//Configure internal reference
	//If ref generator busy, WAIT
	while (REF_BUSY == REF_isRefGenBusy(REF_BASE))
	{
	}

	//Select internal reference = 2.5V
	REF_setReferenceVoltage(REF_BASE, REF_VREF2_5V);

	//Internal Reference ON
	REF_enableReferenceVoltage(REF_BASE);

	/* Setup I/O's */
	GPIO_setAsInputPin(GPIO_PORT_P2, GPIO_PIN3);
}

/* laser_getValue()
 *
 * Get an input parameter from the laser module.
 *
 * Arguments:
 * valueType:	The value to retrieve from the laser module. One of the following parameters are valid:
 * 	-LaserTemperature: Retrieve the current temperature of the laser diode.
 * 	-LaserCurrent: Retrieve the actively flowing current in the laser diode.
 *
 * Returns:
 * The current value of the requested parameter.
 *
 */
uint16_t laser_getValue(LaserInputParameter valueType)
{
	int raw = 0;

	switch (valueType)
	{
		case LaserTemperature:
			/* Configure the ADC buffer to the proper channel, using the internal 2.5V reference. */
			ADC10_A_memoryConfigure(
					ADC10_A_BASE,
					LASER_TEMP_ANALOG_INPUT,
					ADC10_A_VREFPOS_INT,
					ADC10_A_VREFNEG_AVSS);

			/* Start sampling & wait for the conversion to complete. */
			ADC10_A_startConversion(ADC10_A_BASE, ADC10_A_SINGLECHANNEL);
			while (ADC10_A_isBusy(ADC10_A_BASE) == ADC10_A_BUSY)
			{
			}

			raw = ADC10_A_getResults(ADC10_A_BASE);
			return (uint16_t)raw;

		case LaserCurrent:
			/* Configure the ADC buffer to the proper channel, using the internal 2.5V reference. */
			ADC10_A_memoryConfigure(
					ADC10_A_BASE,
					LASER_CURRENT_ANALOG_INPUT,
					ADC10_A_VREFPOS_INT,
					ADC10_A_VREFNEG_AVSS);

			/* Start sampling & wait for the conversion to complete. */
			ADC10_A_startConversion(ADC10_A_BASE, ADC10_A_SINGLECHANNEL);
			while (ADC10_A_isBusy(ADC10_A_BASE) == ADC10_A_BUSY)
			{
			}

			raw = ADC10_A_getResults(ADC10_A_BASE);
			return (uint16_t)raw;

		case LaserInterlock:
			return (uint16_t)GPIO_getInputPinValue(GPIO_PORT_P2, GPIO_PIN3);

		case InterlockOverride:
			return (uint16_t) (P2DIR & GPIO_PIN3) > 0;  // P2DIR & GPIO_PIN3 == 1 = output = override on

		default:
			return 0;
	}
}


/* void laser_setOverride
 *
 * Turns the interlock monitoring pin on the mcu into a GPIO output
 * to override the hardware interlock.  Pulling the interlock pin
 * low disables the laser while holding it high enables it.
 *
 * This action is independent of laser triggering, which can be used
 * as well to disable the laser.
 */

void laser_setOverride(uint8_t state)
{
	/* if state == enable and currently disabled
	 * switch pin to output and configure to match
	 * old state of interlock                       */
    if ( state & ~laser_getValue(InterlockOverride) )
    {
    	char old_val = laser_getValue(LaserInterlock);
    	P2DIR |= GPIO_PIN3;

    	/*Set pin value to match current setting*/
    	if (old_val)
    	{
    		P2DIR |= GPIO_PIN3;
    	}
    	else
    	{
    	   P2DIR &= ~GPIO_PIN3;
    	}

    }
    /* if state = disable and currently enabled
     * reset pin to input state                        */
    else if (~state & laser_getValue(InterlockOverride) )
    {
    	P2DIR &= ~GPIO_PIN3;
    }
}

/* void laser_setInterlock
 *
 * Function to change the value of the interlock if
 * InterlockOverride is enabled.
 */
void laser_setInterlock(uint8_t state)
{
	/*only set the value if in override mode*/
	if (laser_getValue(InterlockOverride) )
	{
		if (state)
		{
			P2OUT |= GPIO_PIN3;
		}
		else
		{
		    P2OUT &= ~GPIO_PIN3;
		}
	}


}

uint16_t ticks2temp(uint16_t ticks, Units outputUnits)
{
	switch (outputUnits)
	{
	/* mV */
	case mV:
		return (uint16_t)((uint32_t)ticks * 244 / 100);

	/*tenths of a Degree Celsius*/
	case Celsius:
	    return (uint16_t)( (240000 - (uint32_t)ticks*244 ) / 35 / 10);

	/*tenths of a Degree Fahrenheit */
	case Fahrenheit:
		return (uint16_t)( (240000 - (uint32_t)ticks*244 ) / 35 / 10 * 18 + 320);

	default:
		return (uint16_t)ticks;
	}
}

uint16_t ticks2current(uint16_t ticks, Units outputUnits)
{
	switch (outputUnits)
	{
	/* mV */
	case mV:
		return (uint16_t)((uint32_t)ticks * 244 / 100);

	default:
		return (uint16_t)ticks;
	}
}
