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
/* TODO: insert other definitions and declarations here. */
#define LED1_ADDRESS 0x10
#define LED2_ADDRESS 0x11
#define LED3_ADDRESS 0x12
#define LED4_ADDRESS 0x13
#define LED5_ADDRESS 0x14

#define LED_IN_ELEVATOR_P_ADDRESS 0x20
#define LED_IN_ELEVATOR_1_ADDRESS 0x21
#define LED_IN_ELEVATOR_2_ADDRESS 0x22
#define LED_IN_ELEVATOR_3_ADDRESS 0x23
#define LED_IN_ELEVATOR_4_ADDRESS 0x24


#define LED_ON 0x01
#define LED_OFF 0x00

#define BUTTON_P 0xc0
#define BUTTON_1 0xc1
#define BUTTON_2 0xc2
#define BUTTON_3 0xc3
#define BUTTON_4 0xc4

#define BUTTON_IN_ELEVATOR_P 0xb0
#define BUTTON_IN_ELEVATOR_1 0xb1
#define BUTTON_IN_ELEVATOR_2 0xb2
#define BUTTON_IN_ELEVATOR_3 0xb3
#define BUTTON_IN_ELEVATOR_4 0xb4

#define FLOOR_NONE 10
#define FLOOR_P 0
#define FLOOR_1 1
#define FLOOR_2 2
#define FLOOR_3 3
#define FLOOR_4 4

#define HORNY_NARAZNIK 0xe5
#define DOLNY_NARAZNIK 0xdf

#define START_BYTE 0xA0
#define ACK_BYTE 0xA1
#define MY_ADDRESS 0x00

#define LOCK_THE_DOOR 0x01
#define UNLOCK_THE_DOOR 0x00
#define ELEVATOR_ADDRESS 0xf0
#define TERMINAL_ADDRESS 0xd0

#define EMERGENCE_BREAK 0xf
#define DEACTIVATE_BREAK 0x00
#define ACTIVATE_BREAK 0x01

#define ENGINE_ADDRESS 0xf1
#define ENGINE_UP 70
#define ENGINE_DOWN -70
#define ENGINE_STOP 0

#define MESSAGE_TYPE 0
#define MESSAGE_RECEIVER_ADDRESS 1
#define MESSAGE_SENDER_ADDRESS 2
#define MESSAGE_VELKOST_DAT 3

#define FLOOR_P_SWITCH 0xe0
#define FLOOR_1_SWITCH 0xe1
#define FLOOR_2_SWITCH 0xe2
#define FLOOR_3_SWITCH 0xe3
#define FLOOR_4_SWITCH 0xe4

volatile uint8_t Sprava[10], Index = 0;
volatile uint8_t Sprava_Complete = 0;
uint8_t stav_citania = 0;
uint8_t message[262];
uint8_t zostavajucedata;
uint8_t data[256];
uint8_t data_index = 0;
uint8_t process_message = 0;

typedef enum {
	ARRIWING,
	WAITING,
	STOPPING,
	LOCKING,
	UNLOCKING,
	MOVING
} ElevatorDelayState;

typedef enum {
	DESTINATION_VISIT,
	DESTINATION_IGNORE
} DestinationState;

typedef struct elevator_state {
	uint8_t position;
	uint8_t destination;
	DestinationState destinations[5];
	uint32_t counter;
	ElevatorDelayState state;
} ElevatorState;

typedef struct floor {
	uint8_t number;
	uint8_t door_led_address;
	uint8_t door_button_address;
	uint8_t elevator_led_address;
	uint8_t elevator_button_addrees;
	uint8_t switch_address;
} Floor;

Floor floors[] = {
	{
		.number = 0,
		.door_led_address = LED1_ADDRESS,
		.door_button_address = BUTTON_P,
		.elevator_led_address = LED_IN_ELEVATOR_P_ADDRESS,
		.elevator_button_addrees = BUTTON_IN_ELEVATOR_P,
		.switch_address = FLOOR_P_SWITCH
	},
	{
		.number = 1,
		.door_led_address = LED2_ADDRESS,
		.door_button_address = BUTTON_1,
		.elevator_led_address = LED_IN_ELEVATOR_1_ADDRESS,
		.elevator_button_addrees = BUTTON_IN_ELEVATOR_1,
		.switch_address = FLOOR_1_SWITCH
	},
	{
		.number = 2,
		.door_led_address = LED3_ADDRESS,
		.door_button_address = BUTTON_2,
		.elevator_led_address = LED_IN_ELEVATOR_2_ADDRESS,
		.elevator_button_addrees = BUTTON_IN_ELEVATOR_2,
		.switch_address = FLOOR_2_SWITCH
	},
	{
		.number = 3,
		.door_led_address = LED4_ADDRESS,
		.door_button_address = BUTTON_3,
		.elevator_led_address = LED_IN_ELEVATOR_3_ADDRESS,
		.elevator_button_addrees = BUTTON_IN_ELEVATOR_3,
		.switch_address = FLOOR_3_SWITCH
	},
	{
		.number = 4,
		.door_led_address = LED5_ADDRESS,
		.door_button_address = BUTTON_4,
		.elevator_led_address = LED_IN_ELEVATOR_4_ADDRESS,
		.elevator_button_addrees = BUTTON_IN_ELEVATOR_4,
		.switch_address = FLOOR_4_SWITCH
	}
};

uint8_t gencrc(uint8_t *data, size_t len) {
	uint8_t crc = 0;
	for (uint8_t i = 0; i < len; ++i) {
		uint8_t inbyte = data[i];
		for (uint8_t j = 0; j < 8; ++j) {
			uint8_t mix = (crc ^ inbyte) & 0x01;
			crc >>= 1;
			if (mix)
				crc ^= 0x8C;
			inbyte >>= 1;
		}
	}
	return crc;
}

void set_led(uint8_t led_on_off, uint8_t leds) {
	uint8_t vypocetcrc[] = { leds, MY_ADDRESS, led_on_off };
	uint8_t vysledok = gencrc(vypocetcrc, sizeof(vypocetcrc));
	uint8_t led_packet_header[] = { START_BYTE, leds, MY_ADDRESS, 1, led_on_off,
			vysledok };
	LPSCI_WriteBlocking(UART0, led_packet_header, sizeof(led_packet_header));
}

void engine(int32_t speed) {
	uint8_t *speed8 = (uint8_t*)&speed;
	uint8_t vypocetcrc[] = { ENGINE_ADDRESS, MY_ADDRESS, 0x02, speed8[0],
			speed8[1], speed8[2], speed8[3] };
	uint8_t vysledokcrc = gencrc(vypocetcrc, sizeof(vypocetcrc));
	uint8_t packet[] = { START_BYTE, ENGINE_ADDRESS, MY_ADDRESS, 0x05, 0x02,
			speed8[0], speed8[1], speed8[2], speed8[3], vysledokcrc };
	LPSCI_WriteBlocking(UART0, packet, sizeof(packet));
}

void engine_speed() {
	uint8_t vypocetcrc[] = { ENGINE_ADDRESS, MY_ADDRESS, 0x04 };
	uint8_t vysledokcrc = gencrc(vypocetcrc, sizeof(vypocetcrc));
	uint8_t packet[] = { START_BYTE, ENGINE_ADDRESS, MY_ADDRESS, 0x01, 0x03,
			vysledokcrc };
	LPSCI_WriteBlocking(UART0, packet, sizeof(packet));
}
void ack(uint8_t Adresa_prijimatela) {
	uint8_t vypocetcrc[] = { Adresa_prijimatela, MY_ADDRESS };
	uint8_t vysledokcrc = gencrc(vypocetcrc, sizeof(vypocetcrc));
	uint8_t packet[] = { ACK_BYTE, Adresa_prijimatela, MY_ADDRESS, 0x00,
			vysledokcrc };
	LPSCI_WriteBlocking(UART0, packet, sizeof(packet));
}

void door(uint8_t unlock_open_door) {
	uint8_t vypocetcrc[] = { ELEVATOR_ADDRESS, MY_ADDRESS, unlock_open_door };
	uint8_t vysledokcrc = gencrc(vypocetcrc, sizeof(vypocetcrc));
	uint8_t packet[] = { START_BYTE, ELEVATOR_ADDRESS, MY_ADDRESS, 0x01,
			unlock_open_door, vysledokcrc };
	LPSCI_WriteBlocking(UART0, packet, sizeof(packet));
}

void terminal(char *message, size_t length) {
	uint8_t vypocetcrc[length + 2];
	vypocetcrc[0] = TERMINAL_ADDRESS;
	vypocetcrc[1] = MY_ADDRESS;
	memcpy(&vypocetcrc[2], (void*) message, length);
	uint8_t vysledokcrc = gencrc(vypocetcrc, sizeof(vypocetcrc));
	uint8_t packet[length + 5];
	packet[0] = START_BYTE;
	packet[1] = TERMINAL_ADDRESS;
	packet[2] = MY_ADDRESS;
	packet[3] = length;
	memcpy(&packet[4], (void*) message, length);
	packet[length + 4] = vysledokcrc;
	LPSCI_WriteBlocking(UART0, packet, sizeof(packet));
}

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

void Handle_switch(ElevatorState *elevator_state, Floor *floor) {
	elevator_state->position = floor->number;
}

void Handle_button(ElevatorState *elevator_state, Floor *floor) {
	set_led(LED_ON, floor->door_led_address);
	set_led(LED_ON, floor->elevator_led_address);
	elevator_state->destinations[floor->number] = DESTINATION_VISIT;
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

	ElevatorState elevator_state = {
		.destination = FLOOR_P,
		.destinations = {
			DESTINATION_VISIT,
			DESTINATION_IGNORE,
			DESTINATION_IGNORE,
			DESTINATION_IGNORE,
			DESTINATION_IGNORE
		},
		.position = FLOOR_3,
		.counter = 0,
		.state = MOVING
	};

	while (1) {
		if (process_message == 1) {
			process_message = 0;
			ack(message[MESSAGE_SENDER_ADDRESS]);
			switch (message[MESSAGE_SENDER_ADDRESS]) {
			case BUTTON_P: Handle_button(&elevator_state, &floors[0]); break;
			case BUTTON_1: Handle_button(&elevator_state, &floors[1]); break;
			case BUTTON_2: Handle_button(&elevator_state, &floors[2]); break;
			case BUTTON_3: Handle_button(&elevator_state, &floors[3]); break;
			case BUTTON_4: Handle_button(&elevator_state, &floors[4]); break;
			case BUTTON_IN_ELEVATOR_P: Handle_button(&elevator_state, &floors[0]); break;
			case BUTTON_IN_ELEVATOR_1: Handle_button(&elevator_state, &floors[1]); break;
			case BUTTON_IN_ELEVATOR_2: Handle_button(&elevator_state, &floors[2]); break;
			case BUTTON_IN_ELEVATOR_3: Handle_button(&elevator_state, &floors[3]); break;
			case BUTTON_IN_ELEVATOR_4: Handle_button(&elevator_state, &floors[4]); break;
			case FLOOR_P_SWITCH: Handle_switch(&elevator_state, &floors[0]); break;
			case FLOOR_1_SWITCH: Handle_switch(&elevator_state, &floors[1]); break;
			case FLOOR_2_SWITCH: Handle_switch(&elevator_state, &floors[2]); break;
			case FLOOR_3_SWITCH: Handle_switch(&elevator_state, &floors[3]); break;
			case FLOOR_4_SWITCH: Handle_switch(&elevator_state, &floors[4]); break;
			default: break;
			}
		}

		switch(elevator_state.state) {
		case MOVING:
			if (elevator_state.position == elevator_state.destination) {
				elevator_state.state = ARRIWING;
				elevator_state.counter = 300000;
			}
			break;
		case ARRIWING:
			if (elevator_state.counter > 0) elevator_state.counter--;
			else {
				Floor *floor = &floors[elevator_state.destination];

				set_led(LED_OFF, floor->door_led_address);
				set_led(LED_OFF, floor->elevator_led_address);

				engine(ENGINE_STOP);
				elevator_state.state = STOPPING;
				elevator_state.counter = 1000000;
			}
			break;
		case STOPPING:
			if (elevator_state.counter > 0) elevator_state.counter--;
			else {
				set_led(UNLOCK_THE_DOOR, ELEVATOR_ADDRESS);
				elevator_state.state = UNLOCKING;
				elevator_state.counter = 1000000;
			}
			break;
		case UNLOCKING:
			if (elevator_state.counter > 0) elevator_state.counter--;
			else {
				elevator_state.state = WAITING;
				elevator_state.counter = 1000000;
			}
			break;
		case WAITING:
			if (elevator_state.counter > 0) elevator_state.counter--;
			else {
				for (int i = 0; i < 5; i++) {
					if (elevator_state.destinations[i] == DESTINATION_VISIT) {
						elevator_state.destination = i;
						set_led(LOCK_THE_DOOR, ELEVATOR_ADDRESS);

						elevator_state.state = LOCKING;
						elevator_state.counter = 1000000;
						elevator_state.destinations[i] = DESTINATION_IGNORE;
						break;
					}
				}
			}
			break;
		case LOCKING:
			if (elevator_state.counter > 0) elevator_state.counter--;
			else {
				if (elevator_state.destination < elevator_state.position) {
					engine(ENGINE_DOWN);
				} else {
					engine(ENGINE_UP);
				}
				elevator_state.state = MOVING;
			}
			break;
		default: break;
		}
	}

	return 0;
}
