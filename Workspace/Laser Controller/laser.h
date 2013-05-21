/*
 * laser.h
 *
 *	Encapsulate all of the laser functionality into a single file.
 *
 *  Created on: May 20, 2013
 *      Author: Collin
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "inc/hw_memmap.h"


#ifndef LASER_H_
#define LASER_H_

#define LASER_TEMP_ANALOG_INPUT 	ADC10_A_INPUT_A0
#define LASER_CURRENT_ANALOG_INPUT 	ADC10_A_INPUT_A3

typedef enum
{
	LaserTemperature = 0,
	LaserCurrent,
	LaserInterlock
}LaserInputParameter;

#endif /* LASER_H_ */
