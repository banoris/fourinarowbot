/*
 * GameState.h
 *
 *  Created on: Dec 1, 2014
 *      Author: mcooley
 */

#ifndef GAMESTATE_H_
#define GAMESTATE_H_

// Array of checker positions
// Stack of moves

struct game_state
{
	int board[6][7];
	int moveNumber;
	int activePlayer;
};

/**
 * Resets the game state for a new game.
 */
void game_state_initialize();

/**
 * Return a copy of the current state of the game.
 */
struct game_state get_current_game_state();

/// Reasons a move might not be recorded.
#define ERR_BAD_SEQUENCE -1 // Played against a stale board
#define ERR_NOT_YOUR_TURN -2 // Not your turn
#define ERR_COLUMN_FULL -3 // Column is full
#define ERR_INVALID_PLAYER -4 // Player does not exist
#define ERR_INVALID_COLUMN -5 // Column does not exist

/**
 * Record that the given player placed a checker in the given column. The
 * move will not be recorded unless the moveNumber is equal to the current
 * move number. This mechanism ensures remote players are not submitting
 * moves against an stale state of the board. If the move is successfully
 * recorded, a number greater than or equal to 0 will be returned. Otherwise,
 * an error code less than 0 will be returned; these codes may correspond to
 * one of the ERR_* codes defined above.
 *
 * Columns are indexed left-to-right, where the leftmost column is 1.
 */
int record_move(int player, int column, int moveNumber);

/**
 * Check whether the game has been won. If so, the id of the winning player
 * will be returned. Otherwise, 0 will be returned to indicate no one has won.
 */
int game_won();

#endif /* GAMESTATE_H_ */
