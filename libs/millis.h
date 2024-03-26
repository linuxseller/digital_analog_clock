#include <avr/interrupt.h>
#define MICROSECONDS_PER_TIMER0_OVERFLOW ((64 * 256)/(F_CPU/1000000))
#define MILLIS_INC (MICROSECONDS_PER_TIMER0_OVERFLOW / 1000)
// the fractional number of milliseconds per timer0 overflow. we shift right
// by three to fit these numbers into a byte. (for the clock speeds we care
// about - 8 and 16 MHz - this doesn't lose precision.)
#define FRACT_INC 3//128//((MICROSECONDS_PER_TIMER0_OVERFLOW % 1000) >> 3)
#define FRACT_MAX (1000 >> 3)
volatile unsigned long timer0_millis = 0;
volatile unsigned long timer0_fract = 0;
ISR(TIMER0_COMPA_vect){
    timer0_millis++;
}
// When TCNT0 == OCR0A, e comparator signals a match. A match will set the Output Compare Flag (OCF0A or OCF0B) at the next timer clock cycle.
// The output compare flag is automatically cleared when the interrupt is executed
void init_millis(){
  unsigned long ctc_match_overflow;
  ctc_match_overflow = (F_CPU / 1000)/64; //when timer1 is this value, 1ms has passed
  TCCR0A = (1 << WGM01);
  TCCR0B = (1 << CS01) | (1 << CS00); // selecting clock source
  OCR0A = ctc_match_overflow;
  // Enable the compare match interrupt
  TIMSK0 |= (1 << OCIE0A);
  sei();
}
unsigned long millis()
{
	unsigned long m;
	uint8_t oldSREG = SREG;

	// disable interrupts while we read timer0_millis or we might get an
	// inconsistent value (e.g. in the middle of a write to timer0_millis)
	cli();
	m = timer0_millis;
	SREG = oldSREG;

	return m;
}
/* unsigned long millis(){ */
/*   unsigned long millis_return; */
/*   uint8_t oldSREG = SREG; */
/*   cli(); */
/*   millis_return = timer0_millis; */
/*   SREG = oldSREG; */
/*   sei(); */
/*   return millis_return; */
/* } */
