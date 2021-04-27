#include "handle_state.h"
#include "definitions.h"


void handle_state_moving(ElevatorState *elevator_state){
	if (elevator_state->position == elevator_state->destination) {
				elevator_state->state = ARRIWING;
				elevator_state->counter = DELAY
				;
			} else {
				if (elevator_state->direction == UP) {
					for (int i = elevator_state->position + 1;
							i < elevator_state->destination; i++) {
						if (elevator_state->destinations[i] == DESTINATION_VISIT) {
							elevator_state->destination = i;
						}
					}
				} else if (elevator_state->direction == DOWN) {
					for (int i = elevator_state->position -1;
							i > elevator_state->destination; i--) {
						if (elevator_state->destinations[i] == DESTINATION_VISIT) {
							elevator_state->destination = i;
						}
					}
				}
			}
}
/*!
 * Stavova masina
 * Funkcia, ktora upravuje lokalny stav vytahu.
 */
void handle_state(ElevatorState *elevator_state) {
	switch (elevator_state->state) {
	case MOVING:
		handle_state_moving(elevator_state);
		break;
	case ARRIWING:
		if (elevator_state->counter > 0)
			elevator_state->counter--;
		else {
			elevator_state->destinations[elevator_state->destination] =
					DESTINATION_IGNORE;
			elevator_state->state = STOPPING;
			elevator_state->direction = NONE;
			elevator_state->counter = DELAY
			;
		}
		break;
	case STOPPING:
		if (elevator_state->counter > 0)
			elevator_state->counter--;
		else {
			elevator_state->state = UNLOCKING;
			elevator_state->counter = DELAY
			;
		}
		break;
	case UNLOCKING:
		if (elevator_state->counter > 0)
			elevator_state->counter--;
		else {
			elevator_state->state = WAITING;
			elevator_state->counter = 12 * DELAY
			;
		}
		break;
	case WAITING:
		if (elevator_state->counter > 0)
			elevator_state->counter--;
		else {
			int destination = -1;
			int distance = 6;

			for (int i = 0; i < 5; i++) {
				if (elevator_state->destinations[i] == DESTINATION_VISIT) {
					int new_distance = abs(elevator_state->position - i);
					if (new_distance < distance) {
						distance = new_distance;
						destination = i;
					}
				}
			}

			if (destination > -1) {
				elevator_state->destination = destination;
				elevator_state->state = LOCKING;
				elevator_state->counter = DELAY;
			}
		}
		break;
	case LOCKING:
		if (elevator_state->counter > 0)
			elevator_state->counter--;
		else {
			if (elevator_state->destination < elevator_state->position) {
				elevator_state->direction = DOWN;
			} else {
				elevator_state->direction = UP;
			}
			elevator_state->state = MOVING;
		}
		break;
	default:
		break;
	}
}
