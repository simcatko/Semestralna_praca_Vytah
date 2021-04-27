#include <definitions.h>
#include <stdint.h>
#include <fsl_lpsci.h>

#ifndef HANDLE_MESSAGE_H_
#define HANDLE_MESSAGE_H_

#define MESSAGE_SENDER_ADDRESS 2




void handle_message(uint8_t message[262], ElevatorState *elevator_state);

#endif
