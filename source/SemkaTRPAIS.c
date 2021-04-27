/**
 * @file    Vytah_semestralna_praca.cpp
 * @brief   Application entry point.
 */

#include <definitions.h>
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
#include "handle_display.h"
#include "handle_door.h"
#include "handle_engine.h"
#include "handle_leds.h"
#include "handle_state.h"


MessageReadingState stav_citania = WAITING_START_BYTE;
uint8_t message[262];
uint8_t zostavajucedata;
uint8_t data[256];
uint8_t data_index = 0;
uint8_t process_message = 0;

/*!
 * Funkcia slúži na spracovavanie sprav vytahu
 * Spracováva správu po bajtoch
 * Globálne si drží stav spracovavania v globalnej premennej stav_citania
 * Ak sa podari nacitat celu spravu, nastavi proces_message na 1
 */
void handle_message_byte(uint8_t byte){
	switch (stav_citania) {
		case WAITING_START_BYTE:
			if (byte == START_BYTE || byte == ACK_BYTE) {
				stav_citania = WAITING_RECEIVER_ADDRESS;
				message[MESSAGE_TYPE] = byte;
			}
			break;
		case WAITING_RECEIVER_ADDRESS:
			message[MESSAGE_RECEIVER_ADDRESS] = byte;
			stav_citania = WAITING_SENDER_ADDRESS;
			break;
		case WAITING_SENDER_ADDRESS:
			message[MESSAGE_SENDER_ADDRESS] = byte;
			stav_citania = WAITING_DATA_LENGHT;
			break;
		case WAITING_DATA_LENGHT:
			message[MESSAGE_VELKOST_DAT] = byte;
			if (byte > 0) {
				zostavajucedata = byte;
				stav_citania = WAITING_DATA;
				data_index = 0;
			} else {
				stav_citania = WAITING_CRC;
			}
			break;
		case WAITING_DATA:
			if (data_index < message[MESSAGE_VELKOST_DAT]) {
				data[data_index++] = byte;
			} else {
				stav_citania = WAITING_CRC;
			}
			break;
		case WAITING_CRC:
			stav_citania = WAITING_START_BYTE;
			process_message = 1;
			break;
		}
}

/*!
 * Funkcia slúži na prijímanie správ od výťahu,
 * je volaná vždy keď prídu údaje z výťahu.
 * Správa je spracovaná po bajtoch.
 */
void UART0_IRQHandler(void) {
	uint8_t byte;
	if (kLPSCI_RxDataRegFullFlag) {
		byte = LPSCI_ReadByte(UART0);
	handle_message_byte(byte);
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
	/*!
	 * Hlavny cyklus dookola opakuje 3 kroky:
	 * 1. Spracuje spravy od vytahu
	 * 2. Upravi svoj lokalny stav
	 * 3. Posle spravy vytahu
	 */
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
