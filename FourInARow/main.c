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


void reset_button()
{
	// TODO

	exit(-1); // terminate program??


}

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
			doors_close();
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

	if (winner == 1)
	{
		lcd_clear();
		lcd_write_string("You won!");
		lcd_set_backlight(0, 0, 128);
	}
	else if (winner == 2)
	{
		lcd_clear();
		lcd_write_string("You lost!");
		lcd_set_backlight(0, 0, 128);
	}

	return winner;
}

void play_game()
{
	// TODO:remove
	srand(time(NULL));

	game_state_initialize();
	int moveNum;
	while (1)
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
			lcd_write_string("Error!");
			lcd_set_backlight(128, 0, 0);
			return;
		}

		if (check_and_report_win()) return;
		random_opponent_play();
		if (check_and_report_win()) return;
	}
}

int main(void)
{
	// Initialize the hardware
	iolib_init();
	sensors_initialize();
	lcd_initialize();

	//pthread_t server_t;
	//int err;
	//int game_end;

	//err = pthread_create(&server_t, NULL, server_thread, NULL);

	play_game();

	//while(1) {

		// let say L player start first
/*
		// open door
		rotate_servo(OPEN_DOOR, SERVO_DOOR);

		// sensor get ready, obtain the column number
		int column = activate_sensor();

		// update checker_array
		update_array(column);
		check_win(); // if a player win, exit the game??.


		//receive input from R player, see begin_request_handler
		//checker release push
		rotate_servo(RELEASE_CHECKER, SERVO_PUSH);
		//sensor get ready
		column = activate_sensor();

		//door open appropriately
		update_array(column);
		open_door_remote(column);
		check_win();
*/
	//}


	return 0;
}
