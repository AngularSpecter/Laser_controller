/*
 * uart.h
 *
 *  Created on: May 21, 2013
 *      Author: Collin
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "inc/hw_memmap.h"

#ifndef UART_H_
#define UART_H_

#define BAUD_RATE 9600

void uart_init();
uint8_t uart_puts(char* str, uint8_t len);
uint8_t uart_gets(char* str, uint8_t max_len);
uint8_t uart_peeks(char* str, uint8_t max_len);

#endif /* UART_H_ */
