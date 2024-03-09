#ifndef LIGHT_WS2812_H_
#define LIGHT_WS2812_H_

#include <avr/io.h>
#include <avr/interrupt.h>

struct cRGB  { uint8_t g; uint8_t r; uint8_t b; };

void ws2812_sendarray(uint8_t *array,uint16_t length);

#endif /* LIGHT_WS2812_H_ */
