#include "light_ws2812.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

void inline ws2812_sendarray(uint8_t *data,uint16_t datlen)
{
  uint8_t curbyte,ctr,masklo;
  uint8_t sreg_prev;
  uint8_t *port = (uint8_t*) _SFR_MEM_ADDR(PORTB);

  masklo  = ~(1<<PB5)&PORTB;
  uint8_t maskhi = (1<<PB5);
  maskhi |= PORTB;

  sreg_prev=SREG;
  cli();

  while (datlen--) {
    curbyte=*data++;

    asm volatile(
    "       ldi   %0,8     \n\t"// load 8 in register[curbyte]
    "loop%=:               \n\t"
    "       st    X,%3     \n\t"    //  '1' [02] '0' [02] - re
    "       brid .+0       \n\t"
    "       sbrs  %1,7     \n\t" //skip if bit in register is set    //  '1' [04] '0' [03]
    "       st    X,%4     \n\t"    //  '1' [--] '0' [05] - fe-low
    "       lsl   %1       \n\t"    //  '1' [05] '0' [06]
    "       brid .+0       \n\t"
    "       brid .+0       \n\t"
    "       brid .+0       \n\t"
    "       brcc skipone%= \n\t"    //  '1' [+1] '0' [+2] -
    "       st   X,%4      \n\t"    //  '1' [+3] '0' [--] - fe-high
    "skipone%=:                "     //  '1' [+3] '0' [+2] -
    "       brid .+0       \n\t"
    "       dec   %0       \n\t"    //  '1' [+4] '0' [+3]
    "       brne  loop%=   \n\t"    //  '1' [+5] '0' [+4]
    :	"=&d" (ctr)
    :	"r" (curbyte), "x" (port), "r" (maskhi), "r" (masklo)
    );
  }
  SREG=sreg_prev;
}
