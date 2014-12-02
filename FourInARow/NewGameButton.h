/*
 * NewGameButton.h
 *
 *  Created on: Dec 2, 2014
 *      Author: mcooley
 */

#ifndef NEWGAMEBUTTON_H_
#define NEWGAMEBUTTON_H_

#define NEW_GAME_PIN 9,12

/**
 * Prepare hardware for reading the state of the new game button.
 */
void new_game_button_initialize();

/**
 * Check if the new game button is pressed. May include a short delay for
 * debouncing.
 */
int new_game_button_pressed();

#endif /* NEWGAMEBUTTON_H_ */
