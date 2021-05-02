/**
 * @file    definitions.c
 * @brief   Obsahuje staticke data
 */

#include "definitions.h"

/*!
 * V tejto premennej su ulozene detaili ku kazdemu poschodiu
 */
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
