/*
 * GameState.c
 *
 *  Created on: Dec 1, 2014
 *      Author: mcooley
 */

#include "GameState.h"
#include <pthread.h>

// The canonical state of the game. This can only be updated in record_move()
// and in game_state_initialize(), which synchronize things and enforce the
// rules of the game.
struct game_state master_state;

pthread_mutex_t master_state_mutex = PTHREAD_MUTEX_INITIALIZER;

void game_state_initialize()
{
	pthread_mutex_lock(&master_state_mutex);

	// Empty the board
	int row, column;
	for (row = 0; row < 6; ++row)
	{
		for (column = 0; column < 7; ++column)
		{
			master_state.board[row][column] = 0;
		}
	}

	// Player 1 is likely to go first, although this is not enforced on the
	// first turn.
	master_state.activePlayer = 1;
	master_state.moveNumber = 0;

	pthread_mutex_unlock(&master_state_mutex);
}

struct game_state get_current_game_state()
{
	pthread_mutex_lock(&master_state_mutex);
	struct game_state ret = master_state;
	pthread_mutex_unlock(&master_state_mutex);
	return ret;
}

int record_move(int player, int column, int moveNumber)
{
	// If the arguments are invalid, bail out.
	if (player != 1 && player != 2) return ERR_INVALID_PLAYER;
	if (column < 1 || column > 7) return ERR_INVALID_COLUMN;

	// Now we need to check things against the master state, so lock it
	pthread_mutex_lock(&master_state_mutex);
	int returnValue;

	// If the move number isn't right, return an error.
	if (moveNumber != master_state.moveNumber)
	{
		returnValue = ERR_BAD_SEQUENCE;
		goto CLEANUP;
	}

	// At the beginning of the game, either player may play. Otherwise, the
	// player can only play if it's their turn.
	if (moveNumber != 0 && player != master_state.activePlayer)
	{
		returnValue = ERR_NOT_YOUR_TURN;
		goto CLEANUP;
	}

	// Attempt to place the checker in the given column.
	int row;
	for (row = 5; row >= 0; --row)
	{
		if (master_state.board[row][column - 1] == 0)
		{
			master_state.board[row][column - 1] = player;
			break;
		}
	}

	// If we couldn't place the checker, return an error.
	if (row < 0)
	{
		returnValue = ERR_COLUMN_FULL;
		goto CLEANUP;
	}

	// Set up for the next turn.
	++(master_state.moveNumber);
	returnValue = master_state.moveNumber;
	if (player == 1)
	{
		master_state.activePlayer = 2;
	}
	else if (player == 2)
	{
		master_state.activePlayer = 1;
	}

	CLEANUP:
	pthread_mutex_unlock(&master_state_mutex);
	return returnValue;
}

// Should only be called from game_won. Assumes the master state is locked.
int check_win_above(int row, int column)
{
	if (row < 3) return 0; // Off the board
	int r;
	for (r = row - 1; r > row - 4; --r)
	{
		if (master_state.board[r][column] != master_state.board[row][column]) return 0;
	}
	return master_state.board[row][column]; // Winner!
}

// Should only be called from game_won. Assumes the master state is locked.
int check_win_right(int row, int column)
{
	if (column > 3) return 0; // Off the board
	int c;
	for (c = column + 1; c < column + 4; ++c)
	{
		if (master_state.board[row][c] != master_state.board[row][column]) return 0;
	}
	return master_state.board[row][column]; // Winner!
}

// Should only be called from game_won. Assumes the master state is locked.
int check_win_diagonal_up_and_right(int row, int column)
{
	if (row < 3 || column > 3) return 0; // Off the board
	int r, c;
	for (r = row - 1, c = column + 1; r > row - 4; --r, ++c)
	{
		if (master_state.board[r][c] != master_state.board[row][column]) return 0;
	}
	return master_state.board[row][column]; // Winner!
}

// Should only be called from game_won. Assumes the master state is locked.
int check_win_diagonal_up_and_left(int row, int column)
{
	if (row < 3 || column < 3) return 0; // Off the board
	int r, c;
	for (r = row - 1, c = column - 1; r > row - 4; --r, --c)
	{
		if (master_state.board[r][c] != master_state.board[row][column]) return 0;
	}
	return master_state.board[row][column]; // Winner!
}

int game_won()
{
	int row, column, winner;

	pthread_mutex_lock(&master_state_mutex);

	for (column = 0; column <= 6; ++column)
	{
		for (row = 5; row >= 0; --row)
		{
			if (master_state.board[row][column] != 0)
			{
				// There is a checker at (row, column). Check if it's the first
				// checker in a winning sequence

				winner = check_win_above(row, column);
				if (winner != 0) goto CLEANUP;

				winner = check_win_right(row, column);
				if (winner != 0) goto CLEANUP;

				winner = check_win_diagonal_up_and_right(row, column);
				if (winner != 0) goto CLEANUP;

				winner = check_win_diagonal_up_and_left(row, column);
				if (winner != 0) goto CLEANUP;
			}
			else
			{
				// No checker here, so we don't need to search any rows above
				// this one in this column
				break;
			}
		}
	}

	CLEANUP:
	pthread_mutex_unlock(&master_state_mutex);
	return winner;
}
