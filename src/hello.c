#include <stdio.h>

#if __GNU__ && __AVR__

#ifndef F_CPU
#define F_CPU 16000000
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>

/*
 * This demonstrate how to use the avr_mcu_section.h file
 * The macro adds a section to the ELF file with useful
 * information for the simulator
 */
#include "avr_mcu_section.h"
AVR_MCU(F_CPU, "atmega128rfa1");

static int uart_putchar(char c, FILE *stream) {
	if (c == '\n')
		uart_putchar('\r', stream);
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;
	return 0;
}

static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL,
                                         _FDEV_SETUP_WRITE);
#endif


int main () {
#if __GNU__ && __AVR__
    sei();
	stdout = &mystdout;
#endif

	printf("Hello, world!\n");
    fflush(stdout);

#if __GNU__ && __AVR__
	// this quits the simulator, since interupts are off
	// this is a "feature" that allows running tests cases and exit
    cli();
	sleep_cpu();
#endif
}
