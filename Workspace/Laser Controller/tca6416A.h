/*
 * tca6416A.h
 *
 *	Provides a driver interface for the TCA6416A I2C I/O expander.
 *
 *  Created on: May 19, 2013
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

#include "usci_b_i2c.h"
#include "ucs.h"

#include "hardware.h"

#ifndef TCA6416A_H_
#define TCA6416A_H_

/* Define the possible addresses of the module. */
#define ADDRRESS_L 	0x20	//ADDR pin is low
#define ADDRRESS_H 	0x21	//ADDR pin is high

#define IOEXP_BIT0	0x0001
#define IOEXP_BIT1	0x0002
#define IOEXP_BIT2	0x0004
#define IOEXP_BIT3	0x0008
#define IOEXP_BIT4	0x0010
#define IOEXP_BIT5 	0x0020
#define IOEXP_BIT6	0x0040
#define IOEXP_BIT7	0x0080

#define IOEXP_BIT8	0x0100
#define IOEXP_BIT9	0x0200
#define IOEXP_BIT10	0x0400
#define IOEXP_BIT11	0x0800
#define IOEXP_BIT12	0x1000
#define IOEXP_BIT13	0x2000
#define IOEXP_BIT14	0x4000
#define IOEXP_BIT15	0x8000


void ioexp_init(uint8_t address);
void ioexp_setIO(uint8_t address, uint16_t iomap);



#endif /* TCA6416A_H_ */
