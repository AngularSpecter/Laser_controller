/*
 * tick.h
 *
 *	Provides functions for timing long, asynchronous periods.
 *
 *  Created on: May 27, 2013
 *      Author: Collin
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "inc/hw_memmap.h"
#include "timer_a.h"

#ifndef TICK_H_
#define TICK_H_

typedef uint64_t tick_t;

#define MILLISECOND 	1
#define SECOND 			(1000 * MILLISECOND)

#define tick_executeAfter(timer, time, action) if((tick_getTime() - timer) > time) { action; timer = tick_getTime(); }

void tick_init();
tick_t tick_getTime();

#endif /* TICK_H_ */
