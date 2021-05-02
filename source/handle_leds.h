/**
 * @file    handle_leds.h
 * @brief   Kniznica
 */

#include <stdint.h>
#include <fsl_lpsci.h>
#include <definitions.h>

#ifndef HANDLE_LEDS_H
#define HANDLE_LEDS_H

void handle_leds(DestinationState leds_state[5], ElevatorState *elevator_state);


#endif
