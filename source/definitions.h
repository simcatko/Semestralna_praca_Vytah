/**
 * @file    definitions.h
 * @brief   Obsahuje definicie a typy
 */
#include <stdint.h>

#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

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

#define DELAY 30000;

#define LED1_ADDRESS 0x10
#define LED2_ADDRESS 0x11
#define LED3_ADDRESS 0x12
#define LED4_ADDRESS 0x13
#define LED5_ADDRESS 0x14
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

#define START_BYTE 0xA0
#define ACK_BYTE 0xA1
#define MY_ADDRESS 0x00

#define ENGINE_ADDRESS 0xf1
#define ENGINE_UP 70
#define ENGINE_DOWN -70
#define ENGINE_STOP 0

#define DISPLAY_ADDRESS 0x30

#define ELEVATOR_ADDRESS 0xf0
#define TERMINAL_ADDRESS 0xd0
#define LED_IN_ELEVATOR_P_ADDRESS 0x20
#define LED_IN_ELEVATOR_1_ADDRESS 0x21
#define LED_IN_ELEVATOR_2_ADDRESS 0x22
#define LED_IN_ELEVATOR_3_ADDRESS 0x23
#define LED_IN_ELEVATOR_4_ADDRESS 0x24

#define FLOOR_P_SWITCH 0xe0
#define FLOOR_1_SWITCH 0xe1
#define FLOOR_2_SWITCH 0xe2
#define FLOOR_3_SWITCH 0xe3
#define FLOOR_4_SWITCH 0xe4

#define LOCK_THE_DOOR 0x01
#define UNLOCK_THE_DOOR 0x00

/*!
 * Tento ciselnik eprezentuje stav vytahu
 */
typedef enum {
	ARRIWING, /*!< Vytah prichadza na cielove podlazie */
	WAITING,  /*!< Vytah stoji na poschodi a caka */
	STOPPING, /*!< Vytah zastavuje na poschodi */
	LOCKING,  /*!< Vytah zatvara dvere */
	UNLOCKING, /*!< Vytah otvara dvere */
	MOVING	   /*!< Vytah cestuje na podlazie*/
} ElevatorDelayState;
/*!
 * Tento ciselnik reprezentuje smer vytahu
 */
typedef enum {
	UP,
	DOWN,
	NONE
} ElevatorDirection;
/*!
 * Tento ciselnik reprezentuje to ci sa ma navstivit poschodie
 */
typedef enum {
	DESTINATION_VISIT,
	DESTINATION_IGNORE
} DestinationState;

/*!
 * Tento ciselnik reprezentuje stav citania spravy
 */
typedef enum {
	WAITING_START_BYTE, /*!< Caka na pociatocny byte*/
	WAITING_RECEIVER_ADDRESS, /*!< Caka na adresu prijimatela */
	WAITING_SENDER_ADDRESS, /*!< Caka na adresu odosielatela */
	WAITING_DATA_LENGHT, /*!< Caka na dlzku dat */
	WAITING_DATA, /*!< Caka na data*/
	WAITING_CRC /*!< Caka kym mu posle CRC kod */
}MessageReadingState;

/*!
 * Tato struktura reprezentuje aktualny stav vytahu
 */
typedef struct elevator_state {
	uint8_t position; /*!< Poschodie na ktorom sa vytah nachadza*/
	uint8_t destination; /*!< Poschodie kam vytah smeruje*/
	DestinationState destinations[5]; /*!< Poschodia ktore ma vytah navstivit*/
	uint32_t counter; /*!< Pocitadlo na delay*/
	ElevatorDirection direction; /*!< Smer vytahu*/
	ElevatorDelayState state; /*!< Uklada stav vytahu*/
} ElevatorState;

/*!
 * Tato struktura reprezentuje detaili poschodia
 */
typedef struct floor {
	uint8_t number; /*!< Cislo poschodia*/
	uint8_t door_led_address; /*!< Adresa ledky pri dverach*/
	uint8_t door_button_address; /*!< Adresa tlacidla na poschodi*/
	uint8_t elevator_led_address; /*!< Adresa ledky vo vytahu*/
	uint8_t elevator_button_addrees; /*!< Adresa tlacidla vo vytahu*/
	uint8_t switch_address; /*!< Adresa prepinaca na poschodi*/
} Floor;

/*!
 * Tato struktura prezentuje aktualny stav displeya, ktory zobrazuje poschodie a smer
 */
typedef struct display {
	uint8_t floor;
	ElevatorDirection direction;
} Display;


extern Floor floors[5];
#endif
