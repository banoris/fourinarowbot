/*
 * MotorControl.h
 *
 *  Created on: Dec 1, 2014
 *      Author: mcooley
 */

#ifndef MOTORCONTROL_H_
#define MOTORCONTROL_H_

// Pins for servos:
// Door open/close: EHRPWM0A (P9.22)
// Chip dropper (not implemeted): EHRPWM0B (P9.21)

/**
 * Open the doors at the top of the robot.
 */
void doors_open();

/**
 * Close the doors at the top of the robot.
 */
void doors_close();

/**
 * Retract the chip dropper.
 */
void dropper_retract();

/**
 * Move the chip dropper forward.
 */
void dropper_forward();

#endif /* MOTORCONTROL_H_ */
