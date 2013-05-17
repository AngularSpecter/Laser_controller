/*
 * hardware.h
 *
 *	Define the hardware setup for the device and the
 *	associated functions to configure the processor.
 *
 *  Created on: May 12, 2013
 *      Author: Collin
 */

#include "inc/hw_memmap.h"
#include "timer_d.h"
#include "wdt_a.h"
#include "gpio.h"
#include "ucs.h"
#include "pmm.h"
#include "pmap.h"
#include "usci_uart.h"

#ifndef HARDWARE_H_
#define HARDWARE_H_

#define XTAL_FREQUENCY	16000000

#define SDA_PIN		GPIO_PIN4
#define SCL_PIN		GPIO_PIN5

#define TX1_PIN		GPIO_PIN1
#define RX1_PIN		GPIO_PIN2

#define LASER_SOURCE_A_PIN				GPIO_PIN7
#define LASER_SOURCE_B_PIN 				GPIO_PIN6
#define DELAYED_TRIGGER_SOURCE_A_PIN 	GPIO_PIN5
#define DELAYED_TRIGGER_SOURCE_B_PIN 	GPIO_PIN4

#define UART1_BAUD_RATE	9600

void hardware_setup();

#endif /* HARDWARE_H_ */
