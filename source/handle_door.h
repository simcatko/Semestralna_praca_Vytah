/**
 * @file    handle_door.h
 * @brief   Kniznica
 */

#include <stdint.h>
#include <fsl_lpsci.h>
#include <definitions.h>

#ifndef HANDLE_DOOR_H_
#define HANDLE_DOOR_H_

void handle_door(ElevatorState *elevator_state, int *doors_locked);


#endif
