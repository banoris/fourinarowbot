/*
 * MotorControl.c
 *
 *  Created on: Dec 1, 2014
 *      Author: mcooley
 */

#include "MotorControl.h"
#include "BBBio_lib/BBBiolib.h"
#include <time.h>

/// Angles for doors
#define DOORS_OPEN_ANGLE 135
#define DOORS_CLOSED_ANGLE 48
#define DROPPER_RETRACTED_ANGLE 135
#define DROPPER_FORWARD_ANGLE 65

// We over-rotate the servo by a few degrees and then immediately pull back
// to the desired rest position. This helps ensure that the mechanism moves
// all the way and that the servo doesn't have to run its motor
// continuously while in the rest position.
#define DOOR_OVERROTATION 4

// No position feedback is available, so we have to rely on timing to guess
// whether the doors have opened or not. Timings here were determined
// experimentally.
#define DOOR_OPEN_TIME_NS 400000000 // 400ms
#define DOOR_CLOSE_TIME_NS 400000000 // 400ms
#define DOOR_REST_TIME_NS 100000000 // 100ms
#define DROPPER_MOVE_TIME_NS 600000000 // 600ms

/// Servo values for TG9 servos
// 0 degree angle pulse high time in msec
#define SRV_0    0.45
// 180 degree angle pulse high time in msec
#define SRV_180  2.45
// Pulse repetition frequency in Hz
#define FRQ 50.0f
// Pulse period in msec
#define PER (1.0E3/FRQ)

int doors_opened = -1;

//TODO: mutex to keep the doors from rotating at the same time
int current_door_angle = DOORS_OPEN_ANGLE;
int current_dropper_angle = DROPPER_RETRACTED_ANGLE;

/**
 * Convert an angle (from 0 to 180 degrees) to a duty cycle (from 0 to 100 percent).
 * This math is from BBBIOlib sample code.
 */
float angle_to_duty_cycle(int angle)
{
	return 100.0 - ((SRV_0 / PER) + (angle / 180.0) * ((SRV_180 - SRV_0) / PER)) * 100.0;
}

void start_pwm()
{
	float door = angle_to_duty_cycle(current_door_angle);
	float dropper = angle_to_duty_cycle(current_dropper_angle);
    BBBIO_PWMSS_Setting(BBBIO_PWMSS0, FRQ, door, dropper);
	BBBIO_ehrPWM_Enable(BBBIO_PWMSS0);
}

void doors_open()
{
	float duty;
	int overrotating;
	if (doors_opened != 1)
	{
		current_door_angle = DOORS_OPEN_ANGLE + DOOR_OVERROTATION;
		overrotating = 1;
	}
	else
	{
		current_door_angle = DOORS_OPEN_ANGLE;
		overrotating = 0;
	}

	start_pwm();

	struct timespec open_time;
	open_time.tv_sec = 0;
	open_time.tv_nsec = DOOR_OPEN_TIME_NS;
	clock_nanosleep(CLOCK_MONOTONIC, 0, &open_time, NULL);

	BBBIO_ehrPWM_Disable(BBBIO_PWMSS0);

	if (overrotating)
	{
		current_door_angle = DOORS_OPEN_ANGLE;
		start_pwm();

		struct timespec rest_time;
		rest_time.tv_sec = 0;
		rest_time.tv_nsec = DOOR_REST_TIME_NS;
		clock_nanosleep(CLOCK_MONOTONIC, 0, &rest_time, NULL);

		BBBIO_ehrPWM_Disable(BBBIO_PWMSS0);
	}

	doors_opened = 1;
}

void doors_close()
{
	float duty;
	int overrotating;
	if (doors_opened != 0)
	{
		current_door_angle = DOORS_CLOSED_ANGLE - DOOR_OVERROTATION;
		overrotating = 1;
	}
	else
	{
		current_door_angle = DOORS_CLOSED_ANGLE;
		overrotating = 0;
	}

    start_pwm();

	struct timespec close_time;
	close_time.tv_sec = 0;
	close_time.tv_nsec = DOOR_CLOSE_TIME_NS;
	clock_nanosleep(CLOCK_MONOTONIC, 0, &close_time, NULL);

	BBBIO_ehrPWM_Disable(BBBIO_PWMSS0);

	if (overrotating)
	{
		current_door_angle = DOORS_CLOSED_ANGLE;
		start_pwm();

		struct timespec rest_time;
		rest_time.tv_sec = 0;
		rest_time.tv_nsec = DOOR_REST_TIME_NS;
		clock_nanosleep(CLOCK_MONOTONIC, 0, &rest_time, NULL);

		BBBIO_ehrPWM_Disable(BBBIO_PWMSS0);
	}

	doors_opened = 0;
}

void dropper_retract()
{
	current_dropper_angle = DROPPER_RETRACTED_ANGLE;
	start_pwm();

	struct timespec move_time;
	move_time.tv_sec = 0;
	move_time.tv_nsec = DROPPER_MOVE_TIME_NS;
	clock_nanosleep(CLOCK_MONOTONIC, 0, &move_time, NULL);

	BBBIO_ehrPWM_Disable(BBBIO_PWMSS0);
}

void dropper_forward()
{
	current_dropper_angle = DROPPER_FORWARD_ANGLE;
	start_pwm();

	struct timespec move_time;
	move_time.tv_sec = 0;
	move_time.tv_nsec = DROPPER_MOVE_TIME_NS;
	clock_nanosleep(CLOCK_MONOTONIC, 0, &move_time, NULL);

	BBBIO_ehrPWM_Disable(BBBIO_PWMSS0);
}
