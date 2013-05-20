/*
 * delay.h
 *
 * Provides functionality for creating, configuring and managing
 * a configurable, precision signal delay chain.
 *
 * The delay chain is driven by TimerD and two of the compare/capture registers.
 *
 *  Created on: May 14, 2013
 *      Author: Collin
 */

#ifndef DELAY_H_
#define DELAY_H_


void delay_init();
void delay_setDelay(uint16_t new_delay);
void delay_setPulseWidth(uint16_t new_width);

#endif /* DELAY_H_ */
