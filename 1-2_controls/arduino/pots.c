// Read in potentiometer value

#include <avr/io.h>
#include <util/delay.h>
#include "pinDefines.h"
#include "USART.h"

static inline void initFreeRunningADC(void) {
  ADMUX |= (1 << REFS0);   /* reference voltage on AVCC */
  ADMUX |= (1 << ADLAR);   /* left-right adjust, give 8-bit value */

  ADCSRA |= (1 << ADEN);   /* enable ADC */
  ADCSRA |= (1 << ADPS2);  /* ADC clock prescaler /16 */
  ADCSRA |= (1 << ADATE);  /* auto-trigger enable */
  ADCSRA |= (1 << ADSC);   /* start conversion */
}

int main(void) {
  initUSART();
  initFreeRunningADC();

  while(1) {
    transmitByte(ADCH);
    _delay_ms(5);
  }
  return 0;
}
