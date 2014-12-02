/*
 * NewGameButton.c
 *
 *  Created on: Dec 2, 2014
 *      Author: mcooley
 */

#include "NewGameButton.h"
#include "BBBio_lib/BBBiolib.h"

void new_game_button_initialize()
{
	iolib_setdir(NEW_GAME_PIN, BBBIO_DIR_IN);
}

int new_game_button_pressed()
{
	return is_low(NEW_GAME_PIN);
}
