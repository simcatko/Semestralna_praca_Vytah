#include "handle_engine.h"
#include "definitions.h"
#include "komunikacia.h"

void handle_engine(ElevatorDirection *engine_direction, ElevatorState *elevator_state) {
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
