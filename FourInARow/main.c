/*
 * main.c
 *
 *  Created on: Nov 24, 2014
 *
 */

#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <netdb.h>
#include "GameState.h"
#include "LCD.h"
#include "MotorControl.h"
#include "Sensors.h"
#include "NewGameButton.h"
#include "BBBio_lib/BBBiolib.h"
#include "mongoose.h"

// Temporary; remove when random opponent play is removed
#include <time.h>
#include <stdlib.h>

// Used for game reset logic
int game_finished = 0;
pthread_mutex_t game_finished_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t game_finished_condition  = PTHREAD_COND_INITIALIZER;

// Used to signal that the remote opponent has requested a play
int remote_column = 0;
pthread_mutex_t remote_column_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t remote_column_condition  = PTHREAD_COND_INITIALIZER;

const char* game_state_json = "{\"activePlayer\":%d, \"moveNumber\":%d, \"board\":[%s]}";

void send_board_state(struct mg_connection *conn)
{
	struct game_state state = get_current_game_state();
	char board_state[6 * 7 * 2];
	int i = 0;
	int r, c;
	for (r = 0; r < 6; ++r)
	{
		for (c = 0; c < 7; ++c)
		{
			board_state[i] = state.board[r][c] + '0';
			if (r == 5 && c == 6)
			{
				board_state[i + 1] = '\0';
			}
			else
			{
				board_state[i + 1] = ',';
			}
			i += 2;
		}
	}

	char response[256];
	int response_length = snprintf(response, sizeof(response), game_state_json,
			state.activePlayer, state.moveNumber, board_state);
	mg_printf(conn,
			"HTTP/1.1 200 OK\r\n"
			"Content-Length: %d\r\n"
			"Content-Type: application/json\r\n\r\n"
			"%s",
			response_length, response);
}

int begin_request_handler(struct mg_connection *conn)
{
	const struct mg_request_info* ri = mg_get_request_info(conn);

	if (!strcmp(ri->uri, "/game_state"))
	{
		if (!strcmp(ri->request_method, "POST"))
		{
			char post_data[1024], move_number_str[8], column_str[8];
			int post_data_length = mg_read(conn, post_data, sizeof(post_data));
			mg_get_var(post_data, post_data_length, "moveNumber", move_number_str, sizeof(move_number_str));
			mg_get_var(post_data, post_data_length, "column", column_str, sizeof(column_str));

			int move_number = atoi(move_number_str);
			int column = atoi(column_str);

			int result = record_move(2, column, move_number);
			//TODO: Send back the error

			if (result > 0)
			{
				// Continue the game thread to place the chip
				pthread_mutex_lock(&remote_column_mutex);
				remote_column = column;
				pthread_cond_signal(&remote_column_condition);
				pthread_mutex_unlock(&remote_column_mutex);
			}
		}

		send_board_state(conn);
		return 1; // Don't process any further
	}

	return 0; // Let mongoose serve static files
}

int detect_human_play()
{
	doors_open();
	int placing = 0;
	while (1)
	{
		int col = sense_chip_position();
		if (col != 0)
		{
			placing = col;
		}
		else if (col == 0 && placing != 0)
		{
			// A chip has been placed
			return placing;
		}

		// Sleep for 10ms
		struct timespec sleep_time;
		sleep_time.tv_sec = 0;
		sleep_time.tv_nsec = 10000000;
		clock_nanosleep(CLOCK_MONOTONIC, 0, &sleep_time, NULL);
	}

	return -1; // Should never get here
}

void drop_checker(int targetColumn)
{
	doors_close();

	int last_pos = -1;
	int drop_next_sample = 0;
	while (1)
	{
		int position = sense_chip_position();

		if (position == targetColumn || drop_next_sample)
		{
			doors_open();

			// Wait for the chip to clear
			while (sense_chip_position() == targetColumn)
			{
				// Sleep for 10ms
				struct timespec sleep_time;
				sleep_time.tv_sec = 0;
				sleep_time.tv_nsec = 10000000;
				clock_nanosleep(CLOCK_MONOTONIC, 0, &sleep_time, NULL);
			}
			doors_close();
			return;
		}

		// Falling edge passing previous column should trigger drop next time
		if (position == 0 && targetColumn > 1 && last_pos == targetColumn - 1)
		{
			drop_next_sample = 1;
		}

		// Sleep for 30ms
		struct timespec sleep_time;
		sleep_time.tv_sec = 0;
		sleep_time.tv_nsec = 30000000;
		clock_nanosleep(CLOCK_MONOTONIC, 0, &sleep_time, NULL);

		last_pos = position;
	}
}

void remote_opponent_play()
{
	doors_close();

	lcd_clear();
	lcd_write_string("Opponent's turn\nwaiting...");
	lcd_set_backlight(128, 0, 0);

	int column;

	pthread_mutex_lock(&remote_column_mutex);
    while (!remote_column)
    {
    	pthread_cond_wait(&remote_column_condition, &remote_column_mutex);
    }
    column = remote_column;
    remote_column = 0;
    pthread_mutex_unlock(&remote_column_mutex);

	char msg[] = "Opponent playing\nat column x";
	msg[27] = (char)column + '0';
	lcd_clear();
	lcd_write_string(msg);

	drop_checker(column);
}

void random_opponent_play()
{
	doors_close();

	int moveNum = get_current_game_state().moveNumber;

	int column, result;

	do
	{
		column = (rand() % 7) + 1;
		result = record_move(2, column, moveNum);
	}
	while (result == ERR_COLUMN_FULL);

	if (result < 0)
	{
		lcd_write_string("Error!");
		return;
	}

	char msg[] = "Opponent playing\nat column x";
	msg[27] = (char)column + '0';
	lcd_clear();
	lcd_write_string(msg);
	lcd_set_backlight(128, 0, 0);

	drop_checker(column);
}

int check_and_report_win()
{
	int winner = game_won();

	if (winner != 0)
	{
		lcd_clear();
		lcd_set_backlight(0, 0, 128);

		if (winner == 1)
		{
			lcd_write_string("You won!");
		}
		else if (winner == 2)
		{
			lcd_write_string("You lost!");
		}

		// Wait for 5s so the player can see the message, then end
		struct timespec sleep_time;
		sleep_time.tv_sec = 5;
		sleep_time.tv_nsec = 0;
		clock_nanosleep(CLOCK_MONOTONIC, 0, &sleep_time, NULL);
	}

	return winner;
}

void print_welcome()
{
	lcd_clear();
	lcd_set_backlight(0, 0, 128);
	lcd_write_string("Let's play!\nNew game started");

	// Sleep for 2s
	struct timespec message_time;
	message_time.tv_sec = 2;
	message_time.tv_nsec = 0;
	clock_nanosleep(CLOCK_MONOTONIC, 0, &message_time, NULL);

	// Print IP address
	struct ifaddrs* if_head;
	struct ifaddrs* if_cur;
	char host[NI_MAXHOST];
	if (getifaddrs(&if_head) != -1)
	{
	    for (if_cur = if_head; if_cur != NULL; if_cur = if_cur->ifa_next)
	    {
	        if (if_cur->ifa_addr != NULL)
	        {
		        if (getnameinfo(if_cur->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST) == 0)
		        {
		        	// Ignore localhost
			        if(if_cur->ifa_addr->sa_family == AF_INET && strcmp(if_cur->ifa_name, "lo"))
			        {
			        	char message[40];
			        	snprintf(message, sizeof(message), "Let's play!\n%s", host);
			        	lcd_clear();
			        	lcd_write_string(message);

			        	// Sleep for 3s
			        	struct timespec message_time;
			        	message_time.tv_sec = 3;
			        	message_time.tv_nsec = 0;
			        	clock_nanosleep(CLOCK_MONOTONIC, 0, &message_time, NULL);
			        }
		        }
	        }
	    }
	}
}

void play_game()
{
	game_state_initialize();

	print_welcome();

	struct timespec message_time;
	message_time.tv_sec = 5;
	message_time.tv_nsec = 0;

	int moveNum = -1;
	while (moveNum <= LAST_MOVE)
	{
		moveNum = get_current_game_state().moveNumber;
		lcd_clear();
		lcd_write_string("Your turn!\nPlace a checker.");
		lcd_set_backlight(128, 128, 0);

		int column = detect_human_play();

		int result = record_move(1, column, moveNum);

		if (result < 0)
		{
			lcd_clear();
			lcd_write_string("Invalid move.\nStopping game.");
			lcd_set_backlight(128, 0, 0);

			// Wait for 5s so the player can see the message, then end
			clock_nanosleep(CLOCK_MONOTONIC, 0, &message_time, NULL);
			return;
		}

		if (check_and_report_win()) return;
		remote_opponent_play();
		if (check_and_report_win()) return;
	}

	lcd_clear();
	lcd_write_string("It's a draw!");
	lcd_set_backlight(0, 0, 128);

	// Wait for 5s so the player can see the message, then end
	clock_nanosleep(CLOCK_MONOTONIC, 0, &message_time, NULL);

	return;
}

void* game_thread()
{
	play_game();

	pthread_mutex_lock(&game_finished_mutex);
	game_finished = 1;
    pthread_cond_signal(&game_finished_condition);
    pthread_mutex_unlock(&game_finished_mutex);

    return 0;
}

void* game_reset_thread()
{
	while (1)
	{
		if (new_game_button_pressed())
		{
			pthread_mutex_lock(&game_finished_mutex);
			game_finished = 1;
		    pthread_cond_signal(&game_finished_condition);
		    pthread_mutex_unlock(&game_finished_mutex);
		}

		// Sleep for 100ms
		struct timespec sleep_time;
		sleep_time.tv_sec = 0;
		sleep_time.tv_nsec = 100000000;
		clock_nanosleep(CLOCK_MONOTONIC, 0, &sleep_time, NULL);
	}

	return 0; // Should never get here
}

int main(void)
{
	// Initialize the hardware
	iolib_init();
	sensors_initialize();
	lcd_initialize();
	new_game_button_initialize();

	// TODO:remove
	srand(time(NULL));

	int err;
	pthread_t watchdog_pthread;
	pthread_t game_pthread;

	// Launch a thread to monitor the new game button
	err = pthread_create(&watchdog_pthread, NULL, game_reset_thread, NULL);
	if (err)
	{
		lcd_clear();
		lcd_write_string("Internal error\ncouldn't start");
		lcd_set_backlight(128, 0, 0);
		return 0;
	}

	// Launch mongoose web server
	const char *options[] = {
			"listening_ports", "80",
			"document_root", "/root/html",
			NULL
		};
	struct mg_callbacks callbacks = {0};
	callbacks.begin_request = begin_request_handler; // on HTTP request
	mg_start(&callbacks, NULL, options);

	while (1)
	{
		// Start a new game
		err = pthread_create(&game_pthread, NULL, game_thread, NULL);
		if (err)
		{
			lcd_clear();
			lcd_write_string("Internal error\ncouldn't start");
			lcd_set_backlight(128, 0, 0);
			return 0;
		}

		// Game restarts after one has finished or the new game button
		// has been pressed.
		pthread_mutex_lock( &game_finished_mutex );
	    while (!game_finished)
	    {
	    	pthread_cond_wait( &game_finished_condition, &game_finished_mutex );
	    }
	    game_finished = 0;
	    pthread_mutex_unlock( &game_finished_mutex );

	    // If necessary, cancel the current game
	    pthread_cancel(game_pthread);
	    pthread_join(game_pthread, NULL);

	    // We restart the web server for every game. This is a hack
	    // to workaround the situation where the FourInARow service starts up
	    // before all the network interfaces are up and have IP addresses. It
	    // seems that mongoose does not gracefully start accepting connections
	    // on the interfaces it's listening on once they do come up all the
	    // way. This gives the user a workaround for now: they can press the
	    // new game button to restart the server and put things in a working
	    // state.
	   // mg_stop(server_context);
	}

	return 0;
}
