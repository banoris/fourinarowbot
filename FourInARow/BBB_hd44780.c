/*
 * BBB_hd44780.c
 *
 *  Created on: Nov 17, 2014
 *      Author: mcooley
 */

#include "BBB_hd44780.h"
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

void pulse_lcd()
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

void write_lcd(char command, int mode)
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
	pulse_lcd();
	clock_nanosleep(CLOCK_MONOTONIC, 0, &command_execution_wait, NULL);

	// Send the lower 4 bits
	(((command >> 0) & 1) > 0) ? pin_low(LCD_DB4_PIN) : pin_high(LCD_DB4_PIN);
	(((command >> 1) & 1) > 0) ? pin_low(LCD_DB5_PIN) : pin_high(LCD_DB5_PIN);
	(((command >> 2) & 1) > 0) ? pin_low(LCD_DB6_PIN) : pin_high(LCD_DB6_PIN);
	(((command >> 3) & 1) > 0) ? pin_low(LCD_DB7_PIN) : pin_high(LCD_DB7_PIN);
	pulse_lcd();
	clock_nanosleep(CLOCK_MONOTONIC, 0, &command_execution_wait, NULL);
}

void initialize_lcd()
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

	// Prepare for 4-bit operation
	write_lcd(0x33, LCD_MODE_COMMAND);
	write_lcd(0x32, LCD_MODE_COMMAND);

    // Initialize registers
	write_lcd(LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF, LCD_MODE_COMMAND);
	write_lcd(LCD_FUNCTIONSET | LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS, LCD_MODE_COMMAND);
	write_lcd(LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT, LCD_MODE_COMMAND);

	clear_lcd();
}

void clear_lcd()
{
	struct timespec command_execution_wait;
	command_execution_wait.tv_sec = 0;
	command_execution_wait.tv_nsec = 3000000; // 3ms. This command takes a long time.

	write_lcd(LCD_CLEARDISPLAY, LCD_MODE_COMMAND);
	clock_nanosleep(CLOCK_MONOTONIC, 0, &command_execution_wait, NULL);
}

void move_cursor(unsigned int column, unsigned int row)
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
	write_lcd(LCD_SETDDRAMADDR | (column + offset), LCD_MODE_COMMAND);
}

void write_string(char string[])
{
    int index = 0;
    int currentRow = 0;
    char current;
    while ((current = string[index]) != '\0')
    {
    	if (current == '\n')
    	{
    		++currentRow;
    		move_cursor(0, currentRow);
    	}
    	else
    	{
    		write_lcd(current, LCD_MODE_CHARACTER);
    	}
    	++index;
    }
}
