/*!
 * \file handle_comunication.c
 * \brief Subor obsahuje funkcie na posielanie sprav vytahu
 */
#include <comunication.h>


/*!
 * Vypocet kontrolneho suctu prevzate https://www.devcoons.com/crc8/
 */
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

/*!
 * Sluzi na naformatovanie a odoslanie spravy pre led (pouzivame aj na posielanie sprav dveram)
 */
void set_led(uint8_t led_on_off, uint8_t leds) {
	uint8_t vypocetcrc[] = { leds, MY_ADDRESS, led_on_off };
	uint8_t vysledok = gencrc(vypocetcrc, sizeof(vypocetcrc));
	uint8_t led_packet_header[] = { START_BYTE, leds, MY_ADDRESS, 1, led_on_off,
			vysledok };
	LPSCI_WriteBlocking(UART0, led_packet_header, sizeof(led_packet_header));
}
/*!
 * Sluzi na naformatovanie a odoslanie spravy motoru
 */
void engine(int32_t speed) {
	uint8_t *speed8 = (uint8_t*)&speed;
	uint8_t vypocetcrc[] = { ENGINE_ADDRESS, MY_ADDRESS, 0x02, speed8[0],
			speed8[1], speed8[2], speed8[3] };
	uint8_t vysledokcrc = gencrc(vypocetcrc, sizeof(vypocetcrc));
	uint8_t packet[] = { START_BYTE, ENGINE_ADDRESS, MY_ADDRESS, 0x05, 0x02,
			speed8[0], speed8[1], speed8[2], speed8[3], vysledokcrc };
	LPSCI_WriteBlocking(UART0, packet, sizeof(packet));
}

/*!
 * Sluzi na poslanie potvrdenia o prijati spravy
 */
void ack(uint8_t Adresa_prijimatela) {
	uint8_t vypocetcrc[] = { Adresa_prijimatela, MY_ADDRESS };
	uint8_t vysledokcrc = gencrc(vypocetcrc, sizeof(vypocetcrc));
	uint8_t packet[] = { ACK_BYTE, Adresa_prijimatela, MY_ADDRESS, 0x00,
			vysledokcrc };
	LPSCI_WriteBlocking(UART0, packet, sizeof(packet));
}

/*!
 * Sluzi na naformatovanie a odoslanie spravy pre display
 */
void display(uint8_t direction, uint8_t floor) {
	uint8_t vypocetcrc[] = { DISPLAY_ADDRESS, MY_ADDRESS, direction, floor };
	uint8_t vysledokcrc = gencrc(vypocetcrc, sizeof(vypocetcrc));
	uint8_t packet[] = { START_BYTE, DISPLAY_ADDRESS, MY_ADDRESS, 0x02, direction, floor, vysledokcrc };
	LPSCI_WriteBlocking(UART0, packet, sizeof(packet));
}

/*!
 * Sluzi na naformatovanie a odoslanie spravy pre terminal
 */
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
