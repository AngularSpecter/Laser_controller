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

#include "application.h"
#include "delay.h"
#include "signalmux.h"
#include "hardware.h"
#include "buffer.h"
#include "tca6416A.h"
#include "tick.h"
#include "laser.h"
#include "laser_uart.h"
#include "lcd.h"

/* _update_lcd()
 *
 * Updates the LCD with front panel UI info.
 *
 * Arguments:
 * NONE
 *
 * Returns:
 * NONE
 *
 */
void _update_lcd()
{
	char str[32];
	uint16_t len = 0;

	/* Initialize. */
	memset(str, 0, sizeof(str));

	/* Interleave all of the string concatenations, etc... with LCD writes to maximize hardware usage. */
	lcd_setCursor(CURSOR_LINE1);
	len = sprintf(str, "Laser Temp: %d    ", laser_getValue(LaserTemperature));
	lcd_puts(str, len);

	lcd_setCursor(CURSOR_LINE2);
	len = sprintf(str, "Laser Current: %d    ", laser_getValue(LaserCurrent));
	lcd_puts(str, len);

	lcd_setCursor(CURSOR_LINE3);
	len = sprintf(str, "Delay=%d us", delay_getDelay());
	lcd_puts(str, len);

	lcd_setCursor(CURSOR_LINE4);
	len = sprintf(str, "Duration=%d us", delay_getPulseWidth());
	lcd_puts(str, len);
}

void main()
{
	volatile uint16_t i = 0;
	tick_t lcd_timer = 0;
//	char cmd[] = ":triggerPeriod=200;";

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

	/* Setup the time keeping subsystem. */
	tick_init();

	/*Setup up the clock source mux subsystem*/
	signalmux_init();

	/* Setup the delay subsystem. */
	delay_init(GENERATED_TRIGGER);

	/* Setup the I/O expander. */
	lcd_init();

	/* Setup the laser subsystem. */
	laser_init();

	/* Setup the UART. */
	uart_init();

	/* DEBUG. */
	//_uart_debugRX(cmd, sizeof(cmd));		//set up a debug command.

	while (1)
	{
		/* Process any incoming commands. */
		application_processCmdTasks();

		i = TIMER_A_getCounterValue(TIMER_A0_BASE);
		tick_executeAfter(lcd_timer, 500 * MILLISECOND, _update_lcd());
	}
}

