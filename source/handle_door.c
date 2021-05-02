/*!
 * \file handle_door.c
 * \brief Subor obsahuje funkciu na pracu s dverami vytahu
 */

#include <comunication.h>
#include "handle_door.h"
#include "definitions.h"


/*!
 * Funkcia porovnáva stav vytahu a stav dveri.
 * Posiela spravy vytahu aby boli tieto stavy rovnake.
 * Ak je stav vytahu LOCKING a dvere su otvorene pošle spravu aby sa zavreli,
 * ak je stav vytahu UNLOCKING a dvere su zatvorene posle spravu aby sa otvorili
 */
void handle_door(ElevatorState *elevator_state, int *doors_locked) {
	if (elevator_state->state == LOCKING && !*doors_locked) {
		*doors_locked = 1;
		set_led(LOCK_THE_DOOR, ELEVATOR_ADDRESS);
	} else if (elevator_state->state == UNLOCKING && *doors_locked) {
		*doors_locked = 0;
		set_led(UNLOCK_THE_DOOR, ELEVATOR_ADDRESS);
	}
}
