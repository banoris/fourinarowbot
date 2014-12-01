/*
 * Sensors.h
 *
 *  Created on: Dec 1, 2014
 *      Author: mcooley
 */

#ifndef SENSORS_H_
#define SENSORS_H_

// Sensors are numbered from left to right
#define SENS_1_PIN  9,23
#define SENS_2_PIN  9,27
#define SENS_3_PIN  8,7
#define SENS_4_PIN  8,9
#define SENS_5_PIN  8,11
#define SENS_6_PIN  8,15
#define SENS_7_PIN  8,17

/**
 * Prepare peripherals to communicate with the chip presence sensors.
 */
void sensors_initialize();

/**
 * Returns the number of the column over which a chip is positioned. Column 1
 * is the leftmost column and column 7 is the rightmost column. Returns 0 if no
 * chip is detected. Returns a negative value if an error condition is detected
 * (such as if a chip is sensed in more than one column).
 */
int sense_chip_position();


#endif /* SENSORS_H_ */
