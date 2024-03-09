all: build

build:
	avr-gcc -g -Os -mmcu=atmega328p libs/light_ws2812.c -c -o libs/light_ws2812.o -DF_CPU=16000000
	avr-gcc -g -Wall -Wextra -Os -mmcu=atmega328p libs/light_ws2812.o main.c -o a.elf -DF_CPU=16000000
	avr-size --format=avr --mcu=atmega328p a.elf

flash: build
	avrdude -D -c /etc/avrdude.conf -p atmega328p -c stk500v1 -P  /dev/ttyUSB0 -b 57600 -U flash:w:a.elf:e
