#include <stdint.h>
#include <fsl_lpsci.h>
#include <definitions.h>


uint8_t gencrc(uint8_t *data, size_t len);
void set_led(uint8_t led_on_off, uint8_t leds);
void engine(int32_t speed);
void engine_speed();
void ack(uint8_t Adresa_prijimatela);
void door(uint8_t unlock_open_door);
void display(uint8_t direction, uint8_t floor);
void terminal(char *message, size_t length);
