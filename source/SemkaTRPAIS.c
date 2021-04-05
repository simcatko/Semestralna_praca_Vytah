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
#define LED_ON 0x01
#define LED_OFF 0x00

#define BUTTON_P 0xc0
#define BUTTON_1 0xc1
#define BUTTON_2 0xc2
#define BUTTON_3 0xc3
#define BUTTON_4 0xc4

#define DESTINATION_FLOOR_NONE 10
#define DESTINATION_FLOOR_P 0
#define DESTINATION_FLOOR_1 1
#define DESTINATION_FLOOR_2 2
#define DESTINATION_FLOOR_3 3
#define DESTINATION_FLOOR_4 4

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

char znak;

struct engine_movement {
	uint8_t start_byte;
	uint8_t receiver_address;
	uint8_t sender_adress;
	uint8_t data_size;
	int32_t movement;
	uint8_t crc;
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

void delay(int Milisekundy) {
	for (int i = 0; i <= Milisekundy * 1000; i++) {
		__asm("nop");
	}
}

//void set_led(uint8_t led_on_off){
//	uint8_t vypocetcrc[] = { LED1_ADDRESS, MY_ADDRESS, led_on_off };
//	uint8_t vysledok = gencrc(vypocetcrc, sizeof(vypocetcrc));
//	uint8_t led_packet_header[] = { START_BYTE, LED1_ADDRESS, MY_ADDRESS, 1, led_on_off, vysledok };
//	LPSCI_WriteBlocking(UART0, led_packet_header, sizeof(led_packet_header));
//}
void set_led(uint8_t led_on_off, uint8_t leds) {
	uint8_t vypocetcrc[] = { leds, MY_ADDRESS, led_on_off };
	uint8_t vysledok = gencrc(vypocetcrc, sizeof(vypocetcrc));
	uint8_t led_packet_header[] = { START_BYTE, leds, MY_ADDRESS, 1, led_on_off,
			vysledok };
	LPSCI_WriteBlocking(UART0, led_packet_header, sizeof(led_packet_header));
}

void engine(int32_t speed) {
	uint8_t *speed8 = &speed;
	uint8_t vypocetcrc[] = { ENGINE_ADDRESS, MY_ADDRESS, 0x02, speed8[0],
			speed8[1], speed8[2], speed8[3] };
	uint8_t vysledokcrc = gencrc(vypocetcrc, sizeof(vypocetcrc));
	uint8_t packet[] = { START_BYTE, ENGINE_ADDRESS, MY_ADDRESS, 0x05, 0x02,
			speed8[0], speed8[1], speed8[2], speed8[3], vysledokcrc };
//	vyskladana sprava pre vytah
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
	memcpy(&vypocetcrc[2], (void *)message, length);
	uint8_t vysledokcrc = gencrc(vypocetcrc, sizeof(vypocetcrc));
	uint8_t packet[length + 5];
	packet[0] = START_BYTE;
	packet[1] = TERMINAL_ADDRESS;
	packet[2] = MY_ADDRESS;
	packet[3] = length;
	memcpy(&packet[4], (void *)message, length);
	packet[length + 4] = vysledokcrc;
	LPSCI_WriteBlocking(UART0, packet, sizeof(packet));
}

void UART0_IRQHandler(void) {

	if (kLPSCI_RxDataRegFullFlag) {
		znak = LPSCI_ReadByte(UART0);
		switch (stav_citania) {
		case 0:
			if (znak == 0xA0 || znak == 0xA1) {
				stav_citania = 1;
				message[MESSAGE_TYPE] = znak;
			} else
				process_message = 3;
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

int main(void) {
//uint8_t vypocetcrc[] = {ENGINE_ADDRESS, MY_ADDRESS, }
//	uint8_t crc =  gencrc();
	/* Init board hardware. */
	BOARD_InitBootPins();
	BOARD_InitBootClocks();
	BOARD_InitBootPeripherals();
//#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
	/* Init FSL debug console. */
//    BOARD_InitDebugConsole();
//#endif
	lpsci_config_t config;
	BOARD_BootClockRUN();
	CLOCK_SetLpsci0Clock(0x1U);
	LPSCI_GetDefaultConfig(&config);
	config.baudRate_Bps = BOARD_DEBUG_UART_BAUDRATE;
	config.enableTx = true;
	config.enableRx = true;

//    PRINTF("Hello World\n");
	LPSCI_Init(UART0, &config, CLOCK_GetFreq(kCLOCK_CoreSysClk));
	LPSCI_EnableInterrupts(UART0, kLPSCI_RxDataRegFullInterruptEnable);
	EnableIRQ(UART0_IRQn);
	/* Force the counter to be placed into memory. */

	/* Enter an infinite loop, just incrementing a counter. */
	printf("nieco\n");
	uint8_t led1 = 0, led2 = 0, led3 = 0, led4 = 0, led5 = 0;
	engine(ENGINE_DOWN);

	uint8_t direction = ENGINE_DOWN;
	uint8_t destination = DESTINATION_FLOOR_P;
	uint8_t position = DESTINATION_FLOOR_P;
	while (1) {
		if (process_message == 1) {
			process_message = 0;
			ack(message[MESSAGE_SENDER_ADDRESS]);
			switch (message[MESSAGE_SENDER_ADDRESS]) {
			case BUTTON_P:
//				led1 = !led1;
//				set_led(led1, LED1_ADDRESS);
//				char msg[] = "Test\n";
//				terminal(msg, sizeof(msg));
//				destination = DESTINATION_FLOOR_P;
//				engine(ENGINE_DOWN);
				destination = DESTINATION_FLOOR_P;
				switch(position) {
				case DESTINATION_FLOOR_P: engine(ENGINE_STOP); break;
				case DESTINATION_FLOOR_1: engine(ENGINE_DOWN); break;
				case DESTINATION_FLOOR_2: engine(ENGINE_DOWN); break;
				case DESTINATION_FLOOR_3: engine(ENGINE_DOWN); break;
				case DESTINATION_FLOOR_4: engine(ENGINE_DOWN); break;
				}
				break;
			case BUTTON_1:
//				led2 = !led2;
//				set_led(led2, LED2_ADDRESS);
//				destination = DESTINATION_FLOOR_1;
				destination = DESTINATION_FLOOR_1;
				switch(position) {
				case DESTINATION_FLOOR_P: engine(ENGINE_UP); break;
				case DESTINATION_FLOOR_1: engine(ENGINE_STOP); break;
				case DESTINATION_FLOOR_2: engine(ENGINE_DOWN); break;
				case DESTINATION_FLOOR_3: engine(ENGINE_DOWN); break;
				case DESTINATION_FLOOR_4: engine(ENGINE_DOWN); break;
				}
				break;
			case BUTTON_2:
//				led3 = !led3;
//				set_led(led3, LED3_ADDRESS);
//				door(led3);
//				destination = DESTINATION_FLOOR_2;
				destination = DESTINATION_FLOOR_2;
				switch(position) {
				case DESTINATION_FLOOR_P: engine(ENGINE_UP); break;
				case DESTINATION_FLOOR_1: engine(ENGINE_UP); break;
				case DESTINATION_FLOOR_2: engine(ENGINE_STOP); break;
				case DESTINATION_FLOOR_3: engine(ENGINE_DOWN); break;
				case DESTINATION_FLOOR_4: engine(ENGINE_DOWN); break;
				}
				break;
			case BUTTON_3:
//				led4 = !led4;
//				set_led(led4, LED4_ADDRESS);
//				engine(ENGINE_DOWN);
				destination = DESTINATION_FLOOR_3;
				switch(position) {
				case DESTINATION_FLOOR_P: engine(ENGINE_UP); break;
				case DESTINATION_FLOOR_1: engine(ENGINE_UP); break;
				case DESTINATION_FLOOR_2: engine(ENGINE_UP); break;
				case DESTINATION_FLOOR_3: engine(ENGINE_STOP); break;
				case DESTINATION_FLOOR_4: engine(ENGINE_DOWN); break;
				}
				break;
			case BUTTON_4:
//				led5 = !led5;
//				set_led(led5, LED5_ADDRESS);
//				destination = DESTINATION_FLOOR_4;
//				engine(ENGINE_DOWN);
				destination = DESTINATION_FLOOR_3;
				switch(position) {
				case DESTINATION_FLOOR_P: engine(ENGINE_UP); break;
				case DESTINATION_FLOOR_1: engine(ENGINE_UP); break;
				case DESTINATION_FLOOR_2: engine(ENGINE_UP); break;
				case DESTINATION_FLOOR_3: engine(ENGINE_UP); break;
				case DESTINATION_FLOOR_4: engine(ENGINE_STOP); break;
				}
				break;
				break;

			case HORNY_NARAZNIK:
				engine(ENGINE_DOWN);
				break;

			case FLOOR_P_SWITCH:
				position = DESTINATION_FLOOR_P;
				switch(destination) {
				case DESTINATION_FLOOR_P: engine(ENGINE_STOP); break;
				case DESTINATION_FLOOR_1: engine(ENGINE_UP); break;
				case DESTINATION_FLOOR_2: engine(ENGINE_UP); break;
				case DESTINATION_FLOOR_3: engine(ENGINE_UP); break;
				case DESTINATION_FLOOR_4: engine(ENGINE_UP); break;
				}
				break;
			case FLOOR_1_SWITCH:
				position = DESTINATION_FLOOR_1;
				switch(destination) {
				case DESTINATION_FLOOR_P: engine(ENGINE_DOWN); break;
				case DESTINATION_FLOOR_1: engine(ENGINE_STOP); break;
				case DESTINATION_FLOOR_2: engine(ENGINE_UP); break;
				case DESTINATION_FLOOR_3: engine(ENGINE_UP); break;
				case DESTINATION_FLOOR_4: engine(ENGINE_UP); break;
				}
				break;
			case FLOOR_2_SWITCH:
				position = DESTINATION_FLOOR_2;
				switch(destination) {
				case DESTINATION_FLOOR_P: engine(ENGINE_DOWN); break;
				case DESTINATION_FLOOR_1: engine(ENGINE_DOWN); break;
				case DESTINATION_FLOOR_2: engine(ENGINE_STOP); break;
				case DESTINATION_FLOOR_3: engine(ENGINE_UP); break;
				case DESTINATION_FLOOR_4: engine(ENGINE_UP); break;
				}
				break;
			case FLOOR_3_SWITCH:
				position = DESTINATION_FLOOR_3;
				switch(destination) {
				case DESTINATION_FLOOR_P: engine(ENGINE_DOWN); break;
				case DESTINATION_FLOOR_1: engine(ENGINE_DOWN); break;
				case DESTINATION_FLOOR_2: engine(ENGINE_DOWN); break;
				case DESTINATION_FLOOR_3: engine(ENGINE_STOP); break;
				case DESTINATION_FLOOR_4: engine(ENGINE_UP); break;
				}
				break;

			case FLOOR_4_SWITCH:
				position = DESTINATION_FLOOR_4;
				switch(destination) {
				case DESTINATION_FLOOR_P: engine(ENGINE_DOWN); break;
				case DESTINATION_FLOOR_1: engine(ENGINE_DOWN); break;
				case DESTINATION_FLOOR_2: engine(ENGINE_DOWN); break;
				case DESTINATION_FLOOR_3: engine(ENGINE_DOWN); break;
				case DESTINATION_FLOOR_4: engine(ENGINE_STOP); break;
				}
				break;

			case MY_ADDRESS:
				if (message[MESSAGE_TYPE] == ACK_BYTE) {

				}
				break;

			case EMERGENCE_BREAK:
				if (data[0] == DEACTIVATE_BREAK) {
				} else {
					engine(ENGINE_STOP);
				}
			default:
				break;
			}

		}

		if (process_message == 2) {
			process_message = 0;
//			switch (message[MESSAGE_RECEIVER_ADDRESS]) {
//			case ENGINE_ADDRESS:
//				if (engine_direction == ENGINE_DOWN) {
//					engine_direction = ENGINE_UP;
//					engine(ENGINE_UP);
//				} else if (engine_direction == ENGINE_UP){
//					engine_direction = ENGINE_DOWN;
//					engine(ENGINE_DOWN);
//				} else {
//					engine_direction = ENGINE_DOWN;
//				}
//				break;
//			default:
//				break;
//			}

		}
//		printf("current state %d\n", stav_citania);
//    	engine(ENGINE_UP);
//		delay(2000);
//		set_led(LED_ON, LED1_ADDRESS);
//		set_led(LOCK_THE_DOOR, DOOR_ADDRESS);
//
//		set_led(LED_ON, LED2_ADDRESS);
//		set_led(LED_ON, LED3_ADDRESS);
//		set_led(LED_ON, LED4_ADDRESS);
//
//		set_led(LED_ON, LED5_ADDRESS);
//
//		engine(ENGINE_DOWN);
//		delay(2000);
//		set_led(LED_OFF, LED1_ADDRESS);
//		set_led(UNLOCK_THE_DOOR, DOOR_ADDRESS);
//		set_led(LED_OFF, LED2_ADDRESS);
//		set_led(LED_OFF, LED3_ADDRESS);
//		set_led(LED_OFF, LED4_ADDRESS);
//		set_led(LED_OFF, LED5_ADDRESS);
	}
	return 0;
}
