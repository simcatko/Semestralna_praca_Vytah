/**
 * @file    handle_engine.h
 * @brief   Kniznica
 */

#include <stdint.h>
#include <fsl_lpsci.h>
#include <definitions.h>

#ifndef HANDLE_ENGINE_H_
#define HANDLE_ENGINE_H_

void handle_engine(ElevatorDirection *engine_direction, ElevatorState *elevator_state);

#endif
