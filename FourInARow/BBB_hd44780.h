/*
 * BBB_hd44780.h
 *
 *  Created on: Nov 17, 2014
 *      Author: mcooley
 */

#ifndef BBB_HD44780_H_
#define BBB_HD44780_H_

// Pins for LCD
// Note: assumes inverting level shifters are used
#define LCD_RS_PIN  8,8
#define LCD_E_PIN   8,10
#define LCD_DB4_PIN 8,18
#define LCD_DB5_PIN 8,16
#define LCD_DB6_PIN 8,14
#define LCD_DB7_PIN 8,12

void initialize_lcd();
void write_string(char string[]);
void clear_lcd();

#endif /* BBB_HD44780_H_ */
