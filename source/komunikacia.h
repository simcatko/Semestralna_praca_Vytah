#include <stdint.h>
#include <fsl_lpsci.h>



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

uint8_t gencrc(uint8_t *data, size_t len);
void set_led(uint8_t led_on_off, uint8_t leds);
void engine(int32_t speed);
void engine_speed();
void ack(uint8_t Adresa_prijimatela);
void door(uint8_t unlock_open_door);
void display(uint8_t direction, uint8_t floor);
void terminal(char *message, size_t length);
