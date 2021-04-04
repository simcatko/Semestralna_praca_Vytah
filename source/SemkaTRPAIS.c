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
#define START_BYTE 0xA0
#define ACK_BYTE 0xA1
#define MY_ADDRESS 0x00
#define LOCK_THE_DOOR 0x01
#define UNLOCK_THE_DOOR 0x00
#define DOOR_ADDRESS 0xf0
#define ENGINE_ADDRESS 0xf1
#define ENGINE_UP 70
#define ENGINE_DOWN -70
#define MESSAGE_RECEIVER_ADDRESS 0
#define MESSAGE_SENDER_ADDRESS 1
#define MESSAGE_VELKOST_DAT 2


volatile uint8_t Sprava[10], Index = 0;
volatile uint8_t Sprava_Complete = 0;
uint8_t stav_citania = 0;
uint8_t message[262];
uint8_t zostavajucedata;
uint8_t data[256];
uint8_t data_index =0;
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
void ack(uint8_t Adresa_prijimatela) {
	uint8_t vypocetcrc[] = { Adresa_prijimatela, MY_ADDRESS };
	uint8_t vysledokcrc = gencrc(vypocetcrc, sizeof(vypocetcrc));
	uint8_t packet[] = { ACK_BYTE, Adresa_prijimatela, MY_ADDRESS, 0x00,vysledokcrc };
	LPSCI_WriteBlocking(UART0, packet, sizeof(packet));
}

void UART0_IRQHandler(void) {

	if (kLPSCI_RxDataRegFullFlag) {
		znak = LPSCI_ReadByte(UART0);
		switch (stav_citania) {
		case 0:
			if (znak == 0xA0) stav_citania = 1;
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
				data[data_index++]=znak;
			} else {
				stav_citania = 5;
			}
			break;
		case 5:
			stav_citania = 0;
			process_message = 1;
//			spracuj_spravu();
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
	while (1) {
		if (process_message) {
			ack(message[MESSAGE_SENDER_ADDRESS]);
			printf("nacitana sprava %x prijimatel %x odosielatel %x data length\n", message[MESSAGE_RECEIVER_ADDRESS],message[MESSAGE_SENDER_ADDRESS], message[MESSAGE_VELKOST_DAT]);
			process_message = 0;
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
