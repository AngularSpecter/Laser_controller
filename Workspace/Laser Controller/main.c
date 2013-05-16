/*
 * main.c
 *
 *  Created on: May 14, 2013
 *      Author: Collin
 */


#include <stdio.h>
#include <stdint.h>

#include "inc/hw_memmap.h"
#include "timer_d.h"
#include "wdt_a.h"
#include "gpio.h"
#include "ucs.h"
#include "pmm.h"
#include "pmap.h"

#include "delay.h"

void main()
{
	  // Stop watchdog timer
	  WDT_A_hold(WDT_A_BASE);

	  delay_init();

	  while(1);
}

