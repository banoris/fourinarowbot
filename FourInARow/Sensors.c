/*
 * Sensors.c
 *
 *  Created on: Dec 1, 2014
 *      Author: mcooley
 */

#include "Sensors.h"
#include "BBBio_lib/BBBiolib.h"

void sensors_initialize()
{
	// Enable GPIO devices
	BBBIO_sys_Enable_GPIO(0);
	BBBIO_sys_Enable_GPIO(1);
	BBBIO_sys_Enable_GPIO(2);
	BBBIO_sys_Enable_GPIO(3);

	// Set sensor pins to inputs
	iolib_setdir(SENS_1_PIN, BBBIO_DIR_IN);
	iolib_setdir(SENS_2_PIN, BBBIO_DIR_IN);
	iolib_setdir(SENS_3_PIN, BBBIO_DIR_IN);
	iolib_setdir(SENS_4_PIN, BBBIO_DIR_IN);
	iolib_setdir(SENS_5_PIN, BBBIO_DIR_IN);
	iolib_setdir(SENS_6_PIN, BBBIO_DIR_IN);
	iolib_setdir(SENS_7_PIN, BBBIO_DIR_IN);
}

int sense_chip_position()
{
	int states[7];

	states[0] = is_low(SENS_1_PIN);
	states[1] = is_low(SENS_2_PIN);
	states[2] = is_low(SENS_3_PIN);
	states[3] = is_low(SENS_4_PIN);
	states[4] = is_low(SENS_5_PIN);
	states[5] = is_low(SENS_6_PIN);
	states[6] = is_low(SENS_7_PIN);

	int chipColumn = 0;
	int i;
	for (i = 0; i < 7; ++i)
	{
		if (states[i] == 1)
		{
			if (chipColumn == 0)
			{
				chipColumn = i + 1;
			}
			else
			{
				// Chip detected in more than one column
				chipColumn = -1;
			}
		}

	}

	return chipColumn;
}
