/*
 * LCD.c
 *
 * Driver for character LCDs based on the HD44780 controller.
 *
 *  Created on: Nov 17, 2014
 *      Author: mcooley
 */

#include "LCD.h"
#include "BBBio_lib/BBBiolib.h"
#include <time.h>

// Character mode and command mode
#define LCD_MODE_CHARACTER 0
#define LCD_MODE_COMMAND   1

// Commands
#define LCD_CLEARDISPLAY   0x01
#define LCD_RETURNHOME     0x02
#define LCD_ENTRYMODESET   0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT    0x10
#define LCD_FUNCTIONSET    0x20
#define LCD_SETCGRAMADDR   0x40
#define LCD_SETDDRAMADDR   0x80

// Entry flags
#define LCD_ENTRYRIGHT          0x00
#define LCD_ENTRYLEFT           0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// Control flags
#define LCD_DISPLAYON  0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON   0x02
#define LCD_CURSOROFF  0x00
#define LCD_BLINKON    0x01
#define LCD_BLINKOFF   0x00

// Move flags
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE  0x00
#define LCD_MOVERIGHT   0x04
#define LCD_MOVELEFT    0x00

// Function set flags
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE    0x08
#define LCD_1LINE    0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS  0x00

// Backlight PWM
#define LCD_BACKLIGHT_PWM_HZ 100.0f

/**
 * Send a pulse on the E(nable) line so the LCD latches whatever value is
 * currently on the data lines.
 */
void lcd_pulse()
{
	struct timespec pulse_length;
	pulse_length.tv_sec = 0;
	pulse_length.tv_nsec = 1000; // 1 us. Spec: > 450ns

	pin_high(LCD_E_PIN);
	clock_nanosleep(CLOCK_MONOTONIC, 0, &pulse_length, NULL);
	pin_low(LCD_E_PIN);
	clock_nanosleep(CLOCK_MONOTONIC, 0, &pulse_length, NULL);
	pin_high(LCD_E_PIN);
	clock_nanosleep(CLOCK_MONOTONIC, 0, &pulse_length, NULL);
}

/**
 * Write the given byte to the LCD. Mode can be either LCD_MODE_COMMAND for
 * writing to the instruction register or LCD_MODE_CHARACTER for writing to
 * the data register.
 */
void lcd_write(uint8_t command, int mode)
{
	struct timespec command_execution_wait;
	command_execution_wait.tv_sec = 0;
	command_execution_wait.tv_nsec = 50000; // 50 us. Spec: most commands have an execution time of 37 us

	// Go into command or character mode
	if (mode == LCD_MODE_COMMAND)
	{
		pin_high(LCD_RS_PIN); // Command register
	}
	else if (mode == LCD_MODE_CHARACTER)
	{
		pin_low(LCD_RS_PIN); // Data register
	}

	// Send the upper 4 bits
	(((command >> 4) & 1) > 0) ? pin_low(LCD_DB4_PIN) : pin_high(LCD_DB4_PIN);
	(((command >> 5) & 1) > 0) ? pin_low(LCD_DB5_PIN) : pin_high(LCD_DB5_PIN);
	(((command >> 6) & 1) > 0) ? pin_low(LCD_DB6_PIN) : pin_high(LCD_DB6_PIN);
	(((command >> 7) & 1) > 0) ? pin_low(LCD_DB7_PIN) : pin_high(LCD_DB7_PIN);
	lcd_pulse();
	clock_nanosleep(CLOCK_MONOTONIC, 0, &command_execution_wait, NULL);

	// Send the lower 4 bits
	(((command >> 0) & 1) > 0) ? pin_low(LCD_DB4_PIN) : pin_high(LCD_DB4_PIN);
	(((command >> 1) & 1) > 0) ? pin_low(LCD_DB5_PIN) : pin_high(LCD_DB5_PIN);
	(((command >> 2) & 1) > 0) ? pin_low(LCD_DB6_PIN) : pin_high(LCD_DB6_PIN);
	(((command >> 3) & 1) > 0) ? pin_low(LCD_DB7_PIN) : pin_high(LCD_DB7_PIN);
	lcd_pulse();
	clock_nanosleep(CLOCK_MONOTONIC, 0, &command_execution_wait, NULL);
}

/**
 * Move the cursor to the specified column and row. Column and row indexes
 * start at 0. The user must ensure that the column and row are valid for
 * the LCD that is attached. In any case, no more than 4 rows are supported.
 */
void lcd_move_cursor(unsigned int column, unsigned int row)
{
	unsigned int offset = 0x00;
	switch (row)
	{
	case 0:
		offset = 0x00; break;
	case 1:
		offset = 0x40; break;
	case 2:
		offset = 0x14; break;
	case 3:
		offset = 0x54; break;
	}
	lcd_write(LCD_SETDDRAMADDR | (column + offset), LCD_MODE_COMMAND);
}

void lcd_initialize()
{
	// Enable GPIO devices
	BBBIO_sys_Enable_GPIO(0);
	BBBIO_sys_Enable_GPIO(1);
	BBBIO_sys_Enable_GPIO(2);

    // Set up GPIO ports
	iolib_setdir(LCD_RS_PIN, BBBIO_DIR_OUT);
	iolib_setdir(LCD_E_PIN, BBBIO_DIR_OUT);
	iolib_setdir(LCD_DB4_PIN, BBBIO_DIR_OUT);
	iolib_setdir(LCD_DB5_PIN, BBBIO_DIR_OUT);
	iolib_setdir(LCD_DB6_PIN, BBBIO_DIR_OUT);
	iolib_setdir(LCD_DB7_PIN, BBBIO_DIR_OUT);

	// Turn off backlight
	lcd_set_backlight(0, 0, 0);

	// Prepare for 4-bit operation
	lcd_write(0x33, LCD_MODE_COMMAND);
	lcd_write(0x32, LCD_MODE_COMMAND);

    // Initialize registers
	lcd_write(LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF, LCD_MODE_COMMAND);
	lcd_write(LCD_FUNCTIONSET | LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS, LCD_MODE_COMMAND);
	lcd_write(LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT, LCD_MODE_COMMAND);

	lcd_clear();
}

void lcd_clear()
{
	struct timespec command_execution_wait;
	command_execution_wait.tv_sec = 0;
	command_execution_wait.tv_nsec = 3000000; // 3ms. This command takes a long time.

	lcd_write(LCD_CLEARDISPLAY, LCD_MODE_COMMAND);
	clock_nanosleep(CLOCK_MONOTONIC, 0, &command_execution_wait, NULL);
}

void lcd_write_string(char string[])
{
    int index = 0;
    int currentRow = 0;
    char current;
    while ((current = string[index]) != '\0')
    {
    	if (current == '\n')
    	{
    		++currentRow;
    		lcd_move_cursor(0, currentRow);
    	}
    	else
    	{
    		lcd_write(current, LCD_MODE_CHARACTER);
    	}
    	++index;
    }
}

void lcd_set_backlight(uint8_t red, uint8_t green, uint8_t blue)
{
	float redDuty = 100.0 * red / 255.0;
	float greenDuty = 100.0 * green / 255.0;
	float blueDuty = 100.0 * blue / 255.0;

	if (redDuty < 0.01 && greenDuty < 0.01)
	{
		BBBIO_ehrPWM_Disable(BBBIO_PWMSS1);
	}
	else
	{
		// The PWM misbehaves when the duty cycle is exactly 0.
		if (redDuty < 0.01) redDuty = 0.01;
		if (greenDuty < 0.01) greenDuty = 0.01;

		BBBIO_PWMSS_Setting(BBBIO_PWMSS1, LCD_BACKLIGHT_PWM_HZ, redDuty, greenDuty);
		BBBIO_ehrPWM_Enable(BBBIO_PWMSS1);
	}
	if (blueDuty < 0.01)
	{
		BBBIO_ehrPWM_Disable(BBBIO_PWMSS2);
	}
	else
	{
		BBBIO_PWMSS_Setting(BBBIO_PWMSS2, LCD_BACKLIGHT_PWM_HZ, 0.0, blueDuty);
		BBBIO_ehrPWM_Enable(BBBIO_PWMSS2);
	}
}
