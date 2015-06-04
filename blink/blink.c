#include <avr/io.h>
#include <avr/delay.h>

#define LED PD6
#define output_low(port,pin) (port &= ~(1<<pin))
#define output_high(port,pin) (port |= (1<<pin))
#define set_output(portdir,pin) (portdir &= ~(1<<pin))
#define set_input(portdir,pin) (portdir |= (1<<pin))

int main() {
  set_output(DDRD, LED);
  while (1) {
    output_high (PORTD, LED);
    _delay_ms (200);
    output_low (PORTD, LED);
    _delay_ms (200);
  }
}
