/*
 * BBB_hd44780.h
 *
 *  Created on: Nov 17, 2014
 *      Author: mcooley
 */

#ifndef BBB_HD44780_H_
#define BBB_HD44780_H_

#include <stdint.h>

// Pins for LCD
// Note: assumes inverting level shifters are used
#define LCD_RS_PIN  8,8
#define LCD_E_PIN   8,10
#define LCD_DB4_PIN 8,18
#define LCD_DB5_PIN 8,16
#define LCD_DB6_PIN 8,14
#define LCD_DB7_PIN 8,12

// Pins for backlight:
// Red:   EHRPWM1A (9,14)
// Green: EHRPWM1B (9,16)
// Blue:  EHRPWM2B (8,13)

/**
 * Prepare peripherals to communicate with the LCD and clear the screen.
 */
void lcd_initialize();

/**
 * Write a string to the LCD. The newline character can be used to print
 * multi-line messages. The user must ensure that the string fits within
 * the LCD boundary (likely 16x2 characters).
 */
void lcd_write_string(char string[]);

/**
 * Clears the LCD screen.
 */
void lcd_clear();

/**
 * Sets the color of the LCD backlight. Red, green, and blue intensities range
 * from 0 to 255.
 */
void lcd_set_backlight(uint8_t red, uint8_t green, uint8_t blue);

#endif /* BBB_HD44780_H_ */
