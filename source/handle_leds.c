#include "handle_leds.h"
#include "definitions.h"
#include "komunikacia.h"

void handle_leds(DestinationState leds_state[5], ElevatorState *elevator_state) {
	for (int li = 0; li < 5; li++) {
		if (leds_state[li] != elevator_state->destinations[li]) {
			leds_state[li] = elevator_state->destinations[li];
			Floor *floor = &floors[li];

			switch (leds_state[li]) {
			case DESTINATION_IGNORE:
				set_led(LED_OFF, floor->door_led_address);
				set_led(LED_OFF, floor->elevator_led_address);
				break;
			case DESTINATION_VISIT:
				set_led(LED_ON, floor->door_led_address);
				set_led(LED_ON, floor->elevator_led_address);
				break;
			}
		}
	}
}
