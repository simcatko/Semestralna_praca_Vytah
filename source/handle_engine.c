/*!
 * \file handle_engine.c
 * \brief Subor obsahuje funkciu na riadene motora vytahu
 */

#include <comunication.h>
#include "handle_engine.h"
#include "definitions.h"

/*!
 * Funkcia porovnáva stav vytahu a stav motora
 * posiela spravy vytahu aby boli tieto stavy rovnake
 */
void handle_engine(ElevatorDirection *engine_direction,
		ElevatorState *elevator_state) {
	if (*engine_direction != elevator_state->direction) {
		*engine_direction = elevator_state->direction;

		switch (*engine_direction) {
		case UP:
			engine(ENGINE_UP);
			break;
		case DOWN:
			engine(ENGINE_DOWN);
			break;
		case NONE:
			engine(ENGINE_STOP);
			break;
		}
	}
}
