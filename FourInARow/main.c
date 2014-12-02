/*
 * main.c
 *
 *  Created on: Nov 24, 2014
 *
 */

#include <stdio.h>
#include <pthread.h>
#include "GameState.h"
#include "LCD.h"
#include "MotorControl.h"
#include "Sensors.h"
#include "NewGameButton.h"
#include "BBBio_lib/BBBiolib.h"
//#include "mongoose.h"

// Temporary; remove when random opponent play is removed
#include <time.h>
#include <stdlib.h>

static const char *html_form =
  "<html><body>POST example."
  "<form method=\"POST\" action=\"/handle_post_request\">"
  "Input 1: <input type=\"text\" name=\"input_1\" /> <br/>"
  "<input type=\"submit\" />"
  "</form></body></html>";

// Used for game reset logic
int game_finished = 0;
pthread_mutex_t game_finished_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t game_finished_condition  = PTHREAD_COND_INITIALIZER;

/*
static int begin_request_handler(struct mg_connection *conn) {
  const struct mg_request_info *ri = mg_get_request_info(conn);
  char post_data[1024], input[sizeof(post_data)];
  int post_data_len;

  if (!strcmp(ri->uri, "/handle_post_request")) {
    // User has submitted a form, show submitted data and a variable value
    post_data_len = mg_read(conn, post_data, sizeof(post_data));

    // Parse form data
    mg_get_var(post_data, post_data_len, "input_1", input, sizeof(input));

    int column = atoi(input);

    update_array(column); // update the checker array from the received input

    // check user input i.e. the column number


  } else {
    // Show HTML form.
    mg_printf(conn, "HTTP/1.0 200 OK\r\n"
              "Content-Length: %d\r\n"
              "Content-Type: text/html\r\n\r\n%s",
              (int) strlen(html_form), html_form);
  }
  return 1;  // Mark request as processed
}


static void *server_thread(void *arg)
{
	struct mg_context *ctx;
	const char *options[] = {"listening_port", "8080", NULL};

	struct mg_callbacks callbacks; // Called when mongoose has received new HTTP request
	memset(&callbacks, 0, sizeof(callbacks));
	callbacks.begin_request = begin_request_handler;

	ctx = mg_start(&callbacks, NULL, options);
	getchar(); // press enter to stop
	mg_stop(ctx);

	return NULL;
}
*/

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

	while (1)
	{
		if (sense_chip_position() == targetColumn)
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

		// Sleep for 10ms
		struct timespec sleep_time;
		sleep_time.tv_sec = 0;
		sleep_time.tv_nsec = 10000000;
		clock_nanosleep(CLOCK_MONOTONIC, 0, &sleep_time, NULL);
	}
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

void play_game()
{
	game_state_initialize();

	lcd_clear();
	lcd_write_string("New game started\nLet's play!");
	lcd_set_backlight(0, 0, 128);

	// Sleep for 5s
	struct timespec message_time;
	message_time.tv_sec = 5;
	message_time.tv_nsec = 0;
	clock_nanosleep(CLOCK_MONOTONIC, 0, &message_time, NULL);

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
		random_opponent_play();
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
	}

	return 0;
}
