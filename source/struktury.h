#include <stdint.h>

#ifndef STRUKTURY_H_
#define STRUKTURY_H_

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
typedef enum {
	ARRIWING,
	WAITING,
	STOPPING,
	LOCKING,
	UNLOCKING,
	MOVING
} ElevatorDelayState;

typedef enum {
	UP,
	DOWN,
	NONE
} ElevatorDirection;

typedef enum {
	DESTINATION_VISIT,
	DESTINATION_IGNORE
} DestinationState;

typedef struct elevator_state {
	uint8_t position;
	uint8_t destination;
	DestinationState destinations[5];
	uint32_t counter;
	ElevatorDirection direction;
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

typedef struct display {
	uint8_t floor;
	ElevatorDirection direction;
} Display;


extern Floor floors[5];
#endif
