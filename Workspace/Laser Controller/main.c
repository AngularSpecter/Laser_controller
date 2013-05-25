/*
 * main.c
 *
 *  Created on: May 14, 2013
 *      Author: Collin
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "inc/hw_memmap.h"
#include "timer_d.h"
#include "wdt_a.h"
#include "gpio.h"
#include "ucs.h"
#include "pmm.h"
#include "pmap.h"

#include "delay.h"
#include "signalmux.h"
#include "hardware.h"
#include "buffer.h"
#include "tca6416A.h"
#include "laser.h"
#include "laser_uart.h"
#include "lcd.h"

void main()
{
	volatile uint16_t i = 0;
	char test[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz123456789";
	//char test2[10];
	// Stop watchdog timer
	WDT_A_hold(WDT_A_BASE);

	/* Setup the clock subsystem. */
	UCS_setExternalClockSource(UCS_BASE, XTAL_FREQUENCY, 0);

	GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_PJ, GPIO_PIN4); //setup the crystal output pin
	GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_PJ, GPIO_PIN5);

	UCS_HFXT1Start(UCS_BASE, UCS_XT1_DRIVE0);								//start the oscillator.

	UCS_clockSignalInit(UCS_BASE, UCS_MCLK, UCS_XT1CLK_SELECT, UCS_CLOCK_DIVIDER_1);//setup the master clock
	UCS_clockSignalInit(UCS_BASE, UCS_ACLK, UCS_XT1CLK_SELECT, UCS_CLOCK_DIVIDER_1);//setup the alternate clock
	UCS_clockSignalInit(UCS_BASE, UCS_SMCLK, UCS_XT1CLK_SELECT, UCS_CLOCK_DIVIDER_1);//setup the alternate clock

	__enable_interrupt();

	/* Setup signal routing. */
	signalmux_init();
	signalmux_route(MUX_LASER_TRIGGER_SOURCE, SIGNAL_OFF);
	signalmux_route(MUX_DELAYED_TRIGGER_SOURCE, SIGNAL_OFF);

	/* Setup the delay subsystem. */
	delay_init();

	/* Setup the I/O expander. */
	lcd_init();

	/* Setup the laser subsystem. */
	laser_init();

	/* Setup the UART. */
	uart_init();
	//i = laser_getValue(LaserTemperature);
	//i = laser_getValue(LaserCurrent);


	while (1)
	{
		i ^= 0xFFFF;
	}
}

