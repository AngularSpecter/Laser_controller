/*
 * lcd.h
 *
 *  Created on: May 22, 2013
 *      Author: Collin
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "inc/hw_memmap.h"

#ifndef LCD_H_
#define LCD_H_

#define CURSOR_LINE1 0
#define CURSOR_LINE2 40
#define CURSOR_LINE3 20
#define CURSOR_LINE4 84

void lcd_init();
void lcd_putc(char data);
void lcd_puts(char* str, uint16_t len);
void lcd_setCursor(uint8_t location);
void lcd_clearDisplay();


#endif /* LCD_H_ */
