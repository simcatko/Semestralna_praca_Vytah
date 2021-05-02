/*!
 * \file handle_state.c
 * \brief Subor obsahuje funkcie na pracu so stavom vytahu
 */

#include "handle_state.h"
#include "definitions.h"

/*!
 * Tato funkcia sa vola ked je vytah prave v pohybe.
 * Kontroluje ci sa uz vytah nachadza na cielovom poschodi ak ano tak zmeni stav na ARRIWING
 * V pripade ak este nieje na cielovom poschodi kontroluje ci medzi tym nebol privolany na blizsie poschodie
 * v aktualnom smere pohybu
 */
void handle_state_moving(ElevatorState *elevator_state) {
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
			for (int i = elevator_state->position - 1;
					i > elevator_state->destination; i--) {
				if (elevator_state->destinations[i] == DESTINATION_VISIT) {
					elevator_state->destination = i;
				}
			}
		}
	}
}

/*!
 * Funkcia je zavolana, ked sa vytah priblizil cielovemu poschodiu
 * Odpocitava delay aby vytah nezastal priskoro a nasledne oznaci poschodie za navstivene
 * nastavi stav na STOPPING
 */
void handle_state_arriwing(ElevatorState *elevator_state) {
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
}

/*!
 * Funkcia je volana ked vytah zastavuje najskor odpocitava delay po ktorom by vytah uz mal stat
 * nasledne prechadza do stavu UNLOCKING
 */
void handle_state_stopping(ElevatorState *elevator_state) {
	if (elevator_state->counter > 0)
		elevator_state->counter--;
	else {
		elevator_state->state = UNLOCKING;
		elevator_state->counter = DELAY
		;
	}
}

/*!
 * Funkcia je volana ked sa na vytahu otvaraju dvere
 * najskor odpocitava delay po ktorom by mali byt dvere otvorene
 * prechadza do stavu WAITING
 */
void handle_state_unlocking(ElevatorState *elevator_state) {
	if (elevator_state->counter > 0)
		elevator_state->counter--;
	else {
		elevator_state->state = WAITING;
		elevator_state->counter = 12 * DELAY
		;
	}
}

/*!
 * funkcia je volana ked vytah stoji na poschodi a ma otvorene dvere
 * najskor sa odpocitava delay aby stihli ludia vystupit nastupit
 * nasledne kontorluje ci nebol privolany na nejake poschodie
 * pripadne na viacero poschodi
 * ak bol privolany na viacero poschodi tak zvoli poschodie ktore je najblizsie
 * k jeho aktualnej pozicii
 * ak sa najde taketo poschodie prechadza do stavu LOCKING
 */
void handle_state_waiting(ElevatorState *elevator_state) {
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
			elevator_state->counter = DELAY
			;
		}
	}
}

/*!
 * Funckia je volana ked sa na vytahu zatvaraju dvere
 * najskor odpocitava cas po ktorom by mali byt dvere zatvorene
 * nasledne nastavy smer ktorym sa bude pohybovat vytah
 * prechadza do stavu MOVING
 */
void handle_state_locking(ElevatorState *elevator_state) {
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
}
/*!
 * Stavova masina
 * Funkcia, ktora upravuje lokalny stav vytahu.
 *
 * ![Stavovy diagram](Stavovydiagram.png)
 */
void handle_state(ElevatorState *elevator_state) {
	switch (elevator_state->state) {
	case MOVING:
		handle_state_moving(elevator_state);
		break;
	case ARRIWING:
		handle_state_arriwing(elevator_state);
		break;
	case STOPPING:
		handle_state_stopping(elevator_state);
		break;
	case UNLOCKING:
		handle_state_unlocking(elevator_state);
		break;
	case WAITING:
		handle_state_waiting(elevator_state);
		break;
	case LOCKING:
		handle_state_locking(elevator_state);
		break;
	default:
		break;
	}
}
