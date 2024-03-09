#include <avr/interrupt.h>
volatile unsigned long timer0_millis = 0;
ISR(TIMER0_COMPA_vect)
{
  timer0_millis++;
}
// When TCNT0 == OCR0A, e comparator signals a match. A match will set the Output Compare Flag (OCF0A or OCF0B) at the next timer clock cycle.
// The output compare flag is automatically cleared when the interrupt is executed
void init_millis(){
  unsigned long ctc_match_overflow;
  ctc_match_overflow = ((F_CPU / 1000) / 8); //when timer1 is this value, 1ms has passed
  // (Set timer to clear when matching ctc_match_overflow) | (Set clock divisor to 8)
  // WGM0[2:0] = 0x2
  TCCR0A = (1 << COM0A1) | (1 << WGM01);
  TCCR0B = (1 << CS01); // selecting clock source
  OCR0A = ctc_match_overflow;
  // Enable the compare match interrupt
  TIMSK0 |= (1 << OCIE0A);
  sei();
}

unsigned long millis(){
  unsigned long millis_return;
  uint8_t oldSREG = SREG;
  cli();
  millis_return = timer0_millis;
  SREG = oldSREG;
  return millis_return;
}
