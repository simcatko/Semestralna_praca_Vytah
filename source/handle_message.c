#include "handle_message.h"

void Handle_switch(ElevatorState *elevator_state, Floor *floor) {
	elevator_state->position = floor->number;
}

void Handle_button(ElevatorState *elevator_state, Floor *floor) {
	elevator_state->destinations[floor->number] = DESTINATION_VISIT;
}
// spracovava spravu co je prijata
void handle_message(uint8_t message[262], ElevatorState *elevator_state) {
	switch (message[MESSAGE_SENDER_ADDRESS]) {
	case BUTTON_P:
		Handle_button(elevator_state, &floors[0]);
		break;
	case BUTTON_1:
		Handle_button(elevator_state, &floors[1]);
		break;
	case BUTTON_2:
		Handle_button(elevator_state, &floors[2]);
		break;
	case BUTTON_3:
		Handle_button(elevator_state, &floors[3]);
		break;
	case BUTTON_4:
		Handle_button(elevator_state, &floors[4]);
		break;
	case BUTTON_IN_ELEVATOR_P:
		Handle_button(elevator_state, &floors[0]);
		break;
	case BUTTON_IN_ELEVATOR_1:
		Handle_button(elevator_state, &floors[1]);
		break;
	case BUTTON_IN_ELEVATOR_2:
		Handle_button(elevator_state, &floors[2]);
		break;
	case BUTTON_IN_ELEVATOR_3:
		Handle_button(elevator_state, &floors[3]);
		break;
	case BUTTON_IN_ELEVATOR_4:
		Handle_button(elevator_state, &floors[4]);
		break;
	case FLOOR_P_SWITCH:
		Handle_switch(elevator_state, &floors[0]);
		break;
	case FLOOR_1_SWITCH:
		Handle_switch(elevator_state, &floors[1]);
		break;
	case FLOOR_2_SWITCH:
		Handle_switch(elevator_state, &floors[2]);
		break;
	case FLOOR_3_SWITCH:
		Handle_switch(elevator_state, &floors[3]);
		break;
	case FLOOR_4_SWITCH:
		Handle_switch(elevator_state, &floors[4]);
		break;
	default:
		break;
	}
}

