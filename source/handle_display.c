/*!
 * \file handle_display.c
 * \brief Subor obsahuje funkcie na pracu s displayom
 */
#include <comunication.h>
#include "handle_display.h"
#include "definitions.h"

/*!
 * Funkcia porovnáva stav vytahu a stav displaya
 * posiela spravy vytahu aby boli tieto stavy rovnake
 */
void handle_display(ElevatorState *elevator_state, Display *display_state) {
	if (elevator_state->direction != display_state->direction
			|| elevator_state->position != display_state->floor) {
		display_state->direction = elevator_state->direction;
		display_state->floor = elevator_state->position;

		char display_message;

		if (display_state->floor == FLOOR_P) {
			display_message = 'P';
		} else {
			display_message = display_state->floor + '0';
		}

		switch (display_state->direction) {
		case UP:
			display(DISPLAY_DIRECTION_UP, display_message);
			break;
		case DOWN:
			display(DISPLAY_DIRECTION_DOWN, display_message);
			break;
		case NONE:
			display(DISPLAY_DIRECTION_NONE, display_message);
			break;
		}
	}
}
