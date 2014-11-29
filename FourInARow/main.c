/*
 * main.c
 *
 *  Created on: Nov 24, 2014
 *
 */


#include <stdio.h>
#include <pthread.h>
#include "BBB_hd44780.h"
#include "BBBio_lib/BBBiolib.h"
#include "mongoose.h"


/* Servo values for TG9 servos: */
/* Servo 0 degree angle pulse high time in msec */
#define SRV_0    0.45
/* Servo 180 degree angle pulse high time in msec */
#define SRV_180  2.45

/* Pulse repetition frequency in Hz */
#define FRQ 50.0f
/* Pulse period in msec */
#define PER (1.0E3/FRQ)

/*TODO Find the appropriate angle */
#define RELEASE_CHECKER 23
#define OPEN_DOOR 23


static const char *html_form =
  "<html><body>POST example."
  "<form method=\"POST\" action=\"/handle_post_request\">"
  "Input 1: <input type=\"text\" name=\"input_1\" /> <br/>"
  "<input type=\"submit\" />"
  "</form></body></html>";



/*
	@param i - angle of the servo motor
	@param motor - door servo or checker pusher servo
	//TODO - find the angle for opening and closing the door
*/
void rotate_servo(const int i, char motor)
{
	float SM_1_duty; /* Servomotor 1 , connect to ePWM0A */
	/* Calculate duty cycle */
	/* Note: the 100-X duty cyle is to account for the level shifter that inverts */
	SM_1_duty = 100.0
			- ((SRV_0 / PER) + (i / 180.0) * ((SRV_180 - SRV_0) / PER)) * 100.0;
	printf("Angle : %d , duty : %f\n", i, SM_1_duty);
	BBBIO_PWMSS_Setting(BBBIO_PWMSS0, FRQ, SM_1_duty, SM_1_duty); /* Set up PWM */
	BBBIO_ehrPWM_Enable(BBBIO_PWMSS0); /* Enable PWM, generate waveform */
	sleep(1); /* Allow time for servo to settle and for humans to see something. */
	BBBIO_ehrPWM_Disable(BBBIO_PWMSS0); /* Disable PWM, stop generating waveform */
}


void open_door(int sensor_column)
{
	int stop = 0;

	// keep reading sensor until we get the appropriate column
	while(stop != 1) {

		if (activate_sensor() == sensor_column) {
				rotate_servo(OPEN_DOOR);
				stop = 1;
		}

	}

}

/*
 * Activate sensors and poll
 * @ return - the position of the sensor e.g. column 3
*/

int activate_sensor()
{
	int sensor_column;
	// poll sensor


	return sensor_column; // return which sensor is blocked
}


void reset_button()
{
	// map button to pin

	exit(-1); // terminate program??


}


static int begin_request_handler(struct mg_connection *conn) {
  const struct mg_request_info *ri = mg_get_request_info(conn);
  char post_data[1024], input[sizeof(post_data)];
  int post_data_len;

  if (!strcmp(ri->uri, "/handle_post_request")) {
    // User has submitted a form, show submitted data and a variable value
    post_data_len = mg_read(conn, post_data, sizeof(post_data));

    // Parse form data
    mg_get_var(post_data, post_data_len, "input_1", input, sizeof(input));

    // check user input i.e. the column number
    switch (input) {
		case '1': // column 1

			break;
		case '2':

			break;

		case '3':

			break;

		case '4':

			break;

		case '5':

			break;

		case '6':

			break;

		case '7':

			break;

		default:
			break;
	}


  } else {
    // Show HTML form.
    mg_printf(conn, "HTTP/1.0 200 OK\r\n"
              "Content-Length: %d\r\n"
              "Content-Type: text/html\r\n\r\n%s",
              (int) strlen(html_form), html_form);
  }
  return 1;  // Mark request as processed
}


static void *server_thread(void arg)
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


int main(void)
{
	pthread_t server_t;
	int err;


	err = pthread_create(&server_t, NULL, server_thread, NULL);

	// wait for remote player --> semaphore??

	// let say L player start first

	// sensor get ready, obtain the column number

	// R player turn --> door open
	/* receive input from R player -->
	 * checker release push
	 * sensor get ready
	 * door open appropriately
	*/

	/* L player turn
	 * Door open
	 * Sensor get ready, obtain column number
	*/

	// play until game end --> human player ends it??


	return 0;
}
