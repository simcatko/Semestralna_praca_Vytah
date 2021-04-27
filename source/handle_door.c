#include "handle_door.h"
#include "definitions.h"
#include "komunikacia.h"

void handle_door(ElevatorState *elevator_state, int *doors_locked) {
	if (elevator_state->state == LOCKING && !*doors_locked) {
		*doors_locked = 1;
		set_led(LOCK_THE_DOOR, ELEVATOR_ADDRESS);
	} else if (elevator_state->state == UNLOCKING && *doors_locked) {
		*doors_locked = 0;
		set_led(UNLOCK_THE_DOOR, ELEVATOR_ADDRESS);
	}
}
