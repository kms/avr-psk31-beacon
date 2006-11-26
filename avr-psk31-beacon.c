/*
 * Karl-Martin Skontorp LA9PMA <kms@skontorp.net>
 * $Id: fRound.h 93 2006-05-22 09:30:07Z kms $
 */

#define F_CPU 12E6
#define BB_F 732
#define SIN_LUT_SIZE 32
//#define SYM_PERIOD ((0.032 * SIN_LUT_SIZE) / (1.0 / BB_F))
#define SYM_PERIOD 1500

#include <math.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "varicode.h"

volatile uint8_t i;
volatile uint16_t p;

uint8_t txChar(void);

int8_t sinLUT[SIN_LUT_SIZE][16];

ISR(SIG_OUTPUT_COMPARE0A) {
    static uint8_t shift;

    p++;

    if (p == SYM_PERIOD / 2) {
	if (txChar() == 0) {
	    shift = 1;
	} else {
	    shift = 0;
	}

	if (PORTD & _BV(PIND0)) {
	    PORTD &= ~(_BV(PIND0));
	} else {
	    PORTD |= _BV(PIND0);
	}
    } else if (p == SYM_PERIOD) {
	p = 0;
	if (shift) {
	    i += SIN_LUT_SIZE / 2;
	}
    }

    int16_t x;

    if (shift) {
	if (p < SYM_PERIOD / 2) {
	    x = sinLUT[i++ % SIN_LUT_SIZE][12 - (p / 128)];
	} else {
	    x = sinLUT[i++ % SIN_LUT_SIZE][p / 128];
	}
    } else {
	x = sinLUT[i++ % SIN_LUT_SIZE][0];
    }

    x += 128;
    OCR0A = (uint8_t) x;
}

uint8_t txChar() {
    static char txString[] = 
	"\t\t\t"
	"\t\t... LA9PMA/B LA9PMA/B ...\r\n"
	"\t\tLA9PMA/B < JO59fe Tonsberg >\r\n"
	//	"\t\tLA9PMA/B < 5W in dipole >\r\n"
	//	"\t\tLA9PMA/B < 28.321.732MHz >\r\n"
	"\t\tLA9PMA/B < Op: kms@skontorp.net  +47 952 47 971 >\r\n"
	"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";
    static char *txStringPos = txString;
    static int8_t txBitPos = 0;
    uint8_t result;
    uint8_t patternLength;

    patternLength = psk31[(uint8_t) *txStringPos].length;

    if (txBitPos == patternLength || (txBitPos == patternLength + 1)) {
	txBitPos++;
	return 0;
    } else if (txBitPos == patternLength + 2) {
	txBitPos = 0;
	if (*txStringPos++ == '\0') {
	    txStringPos = txString;
	}
	patternLength = psk31[(uint8_t) *txStringPos].length;
    }

    if (psk31[(uint8_t) *txStringPos].pattern & (1 << (patternLength - 1 - txBitPos))) {
	result = 1;
    } else {
	result = 0;
    }

    txBitPos++;

    return result;
}

void prepareSinLUT() {
    uint8_t l;
    uint8_t m;

    for (l = 0; l < SIN_LUT_SIZE; l++) {
	double d;

	for (m = 0; m < 16; m++) {
	    d = sin((((((double) l) / SIN_LUT_SIZE)) - 0.5) * M_PI * 2.0) * cos(((double) m)/24.0 * M_PI);
	    d *= 127.0;
	    sinLUT[l][m] = d;
	}
    }
}

int main(void) {
    wdt_reset();
    wdt_disable();

    prepareSinLUT();

    TCCR0A = _BV(COM0A1) | _BV(COM0A0) | _BV(WGM01) | _BV(WGM00);
    TCCR0B = _BV(CS00);
    TIMSK0 = _BV(OCIE0A);

    DDRD = _BV(PIND6);
    DDRD |= _BV(PIND0);

    set_sleep_mode(SLEEP_MODE_IDLE);

    sei();

    for (;;) {
	sleep_mode();
    }
}
