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

#define DELAYED_TRIGGER		1
#define GENERATED_TRIGGER	2

void delay_init(uint8_t mode);
uint16_t delay_getMode();

void delay_setDelay(uint16_t new_delay);
uint16_t delay_getDelay();

void delay_setPulseWidth(uint16_t new_width);
uint16_t delay_getPulseWidth();

void delay_setEnable(uint8_t enable);
uint16_t delay_isEnabled();

void delay_setPeriod(uint16_t new_period);
uint16_t delay_getPeriod();



#endif /* DELAY_H_ */
