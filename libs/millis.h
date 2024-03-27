#include <avr/interrupt.h>
volatile unsigned long timer0_millis = 0;
volatile unsigned long timer0_fract = 0;
ISR(TIMER0_COMPA_vect){
    timer0_millis++;
}
// When TCNT0 == OCR0A, e comparator signals a match. A match will set the Output Compare Flag (OCF0A or OCF0B) at the next timer clock cycle.
// The output compare flag is automatically cleared when the interrupt is executed
void init_millis()
{
    unsigned long ctc_match_overflow;
    ctc_match_overflow = (F_CPU / 1000)/64; //when TC0 is this value, 1ms has passed
    TCCR0A = (1 << WGM01);
    TCCR0B = (1 << CS01) | (1 << CS00); // set prescaler value (64 here)
    // OCR0A = ctc_match_overflow;
    *(uint8_t*)0x44 = ctc_match_overflow;
    TIMSK0 |= (1 << OCIE0A);
    sei();
}
unsigned long millis()
{
	unsigned long m;
	uint8_t oldSREG = SREG;
	cli();
	m = timer0_millis;
	SREG = oldSREG;

	return m;
}
