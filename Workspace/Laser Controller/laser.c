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

		default:
			return 0;
	}
}

