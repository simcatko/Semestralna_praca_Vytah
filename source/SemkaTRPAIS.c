/**
 * @file    Vytah_semestralna_praca.cpp
 * @brief   Application entry point.
 */

#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MKL25Z4.h"
#include "fsl_debug_console.h"
#include <fsl_lpsci.h>
#include <stdbool.h>
#include "komunikacia.h"
#include "handle_message.h"
#include "struktury.h"

/* TODO: insert other definitions and declarations here. */

#define LED_ON 0x01
#define LED_OFF 0x00

#define FLOOR_NONE 10
#define FLOOR_P 0
#define FLOOR_1 1
#define FLOOR_2 2
#define FLOOR_3 3
#define FLOOR_4 4

#define HORNY_NARAZNIK 0xe5
#define DOLNY_NARAZNIK 0xdf

#define LOCK_THE_DOOR 0x01
#define UNLOCK_THE_DOOR 0x00

#define EMERGENCE_BREAK 0xf
#define DEACTIVATE_BREAK 0x00
#define ACTIVATE_BREAK 0x01

#define MESSAGE_TYPE 0
#define MESSAGE_RECEIVER_ADDRESS 1

#define MESSAGE_VELKOST_DAT 3

#define DISPLAY_DIRECTION_UP 0x01
#define DISPLAY_DIRECTION_DOWN 0x02
#define DISPLAY_DIRECTION_NONE 0x03

#define DELAY 50000;

volatile uint8_t Sprava[10], Index = 0;
volatile uint8_t Sprava_Complete = 0;
uint8_t stav_citania = 0;
uint8_t message[262];
uint8_t zostavajucedata;
uint8_t data[256];
uint8_t data_index = 0;
uint8_t process_message = 0;

void UART0_IRQHandler(void) {
	uint8_t znak;
	if (kLPSCI_RxDataRegFullFlag) {
		znak = LPSCI_ReadByte(UART0);
		switch (stav_citania) {
		case 0:
			if (znak == 0xA0 || znak == 0xA1) {
				stav_citania = 1;
				message[MESSAGE_TYPE] = znak;
			}
			break;
		case 1:
			message[MESSAGE_RECEIVER_ADDRESS] = znak;
			stav_citania = 2;
			break;
		case 2:
			message[MESSAGE_SENDER_ADDRESS] = znak;
			stav_citania = 3;
			break;
		case 3:
			message[MESSAGE_VELKOST_DAT] = znak;
			if (znak > 0) {
				zostavajucedata = znak;
				stav_citania = 4;
				data_index = 0;
			} else {
				stav_citania = 5;
			}
			break;
		case 4:
			if (data_index < message[MESSAGE_VELKOST_DAT]) {
				data[data_index++] = znak;
			} else {
				stav_citania = 5;
			}
			break;
		case 5:
			stav_citania = 0;
			process_message = 1;
			break;
		}
	}
}

// rie3i zmeny stavu vytahu
void handle_state(ElevatorState *elevator_state) {
	switch (elevator_state->state) {
	case MOVING:
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
			for (int i = 0; i < 5; i++) {
				if (elevator_state->destinations[i] == DESTINATION_VISIT) {
					elevator_state->destination = i;
					elevator_state->state = LOCKING;
					elevator_state->counter = DELAY
					;
					break;
				}
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

void handle_door(ElevatorState *elevator_state, int *doors_locked) {
	if (elevator_state->state == LOCKING && !*doors_locked) {
		*doors_locked = 1;
		set_led(LOCK_THE_DOOR, ELEVATOR_ADDRESS);
	} else if (elevator_state->state == UNLOCKING && *doors_locked) {
		*doors_locked = 0;
		set_led(UNLOCK_THE_DOOR, ELEVATOR_ADDRESS);
	}
}
int main(void) {
	BOARD_InitBootPins();
	BOARD_InitBootClocks();
	BOARD_InitBootPeripherals();

	lpsci_config_t config;
	BOARD_BootClockRUN();
	CLOCK_SetLpsci0Clock(0x1U);
	LPSCI_GetDefaultConfig(&config);
	config.baudRate_Bps = BOARD_DEBUG_UART_BAUDRATE;
	config.enableTx = true;
	config.enableRx = true;

	LPSCI_Init(UART0, &config, CLOCK_GetFreq(kCLOCK_CoreSysClk));
	LPSCI_EnableInterrupts(UART0, kLPSCI_RxDataRegFullInterruptEnable);
	EnableIRQ(UART0_IRQn);

	engine(ENGINE_DOWN);

	ElevatorState elevator_state = { .destination = FLOOR_P, .destinations = {
			DESTINATION_VISIT, DESTINATION_IGNORE, DESTINATION_IGNORE,
			DESTINATION_IGNORE, DESTINATION_IGNORE }, .position = FLOOR_3,
			.counter = 0, .direction = DOWN, .state = MOVING };

	Display display_state = { .floor = FLOOR_P, .direction = DOWN };

	ElevatorDirection engine_direction = NONE;

	DestinationState leds_state[5] = { DESTINATION_VISIT, DESTINATION_IGNORE,
			DESTINATION_IGNORE, DESTINATION_IGNORE, DESTINATION_IGNORE };

	int doors_locked = 0;

	while (1) {
		if (process_message == 1) {
			process_message = 0;
			ack(message[MESSAGE_SENDER_ADDRESS]);
			handle_message(message, &elevator_state); // spracovanie spravy
		}
		handle_state(&elevator_state);

		handle_display(&elevator_state, &display_state);

		handle_engine(&engine_direction, &elevator_state);

		handle_leds(leds_state, &elevator_state);

		handle_door(&elevator_state, &doors_locked);
	}

	return 0;
}
