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

/// Servo values for TG9 servos
// 0 degree angle pulse high time in msec
#define SRV_0    0.45
// 180 degree angle pulse high time in msec
#define SRV_180  2.45
// Pulse repetition frequency in Hz
#define FRQ 50.0f
// Pulse period in msec
#define PER (1.0E3/FRQ)

/**
 * Convert an angle (from 0 to 180 degrees) to a duty cycle (from 0 to 100 percent).
 * This math is from BBBIOlib sample code.
 */
float angle_to_duty_cycle(int angle)
{
	return 100.0 - ((SRV_0 / PER) + (angle / 180.0) * ((SRV_180 - SRV_0) / PER)) * 100.0;
}

void doors_open()
{
	float duty = angle_to_duty_cycle(DOORS_OPEN_ANGLE + DOOR_OVERROTATION);
    BBBIO_PWMSS_Setting(BBBIO_PWMSS0, FRQ, duty, 0);
	BBBIO_ehrPWM_Enable(BBBIO_PWMSS0);

	struct timespec open_time;
	open_time.tv_sec = 0;
	open_time.tv_nsec = DOOR_OPEN_TIME_NS;
	clock_nanosleep(CLOCK_MONOTONIC, 0, &open_time, NULL);

	BBBIO_ehrPWM_Disable(BBBIO_PWMSS0);
	duty = angle_to_duty_cycle(DOORS_OPEN_ANGLE);
    BBBIO_PWMSS_Setting(BBBIO_PWMSS0, FRQ, duty, 0);
	BBBIO_ehrPWM_Enable(BBBIO_PWMSS0);

	struct timespec rest_time;
	rest_time.tv_sec = 0;
	rest_time.tv_nsec = DOOR_REST_TIME_NS;
	clock_nanosleep(CLOCK_MONOTONIC, 0, &rest_time, NULL);

	BBBIO_ehrPWM_Disable(BBBIO_PWMSS0);
}

void doors_close()
{
	float duty = angle_to_duty_cycle(DOORS_CLOSED_ANGLE - DOOR_OVERROTATION);
    BBBIO_PWMSS_Setting(BBBIO_PWMSS0, FRQ, duty, 0);
	BBBIO_ehrPWM_Enable(BBBIO_PWMSS0);

	struct timespec close_time;
	close_time.tv_sec = 0;
	close_time.tv_nsec = DOOR_CLOSE_TIME_NS;
	clock_nanosleep(CLOCK_MONOTONIC, 0, &close_time, NULL);

	BBBIO_ehrPWM_Disable(BBBIO_PWMSS0);
	duty = angle_to_duty_cycle(DOORS_CLOSED_ANGLE);
    BBBIO_PWMSS_Setting(BBBIO_PWMSS0, FRQ, duty, 0);
	BBBIO_ehrPWM_Enable(BBBIO_PWMSS0);

	struct timespec rest_time;
	rest_time.tv_sec = 0;
	rest_time.tv_nsec = DOOR_REST_TIME_NS;
	clock_nanosleep(CLOCK_MONOTONIC, 0, &rest_time, NULL);

	BBBIO_ehrPWM_Disable(BBBIO_PWMSS0);
}
