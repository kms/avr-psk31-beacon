/* avr-psk31-beacon
 * PSK31 Beacon generator with AF output.
 *
 * (c) Karl-Martin Skontorp <kms@skontorp.net> ~ http://22pf.org/ ~ LA9PMA
 * Licensed under the GNU GPL 2.0 or later.
 */

#define F_CPU 18432E3
#define BB_F 732
#define SIN_LUT_SIZE 128
//#define SYM_PERIOD ((0.032 * SIN_LUT_SIZE) / (1.0 / BB_F))
#define SYM_PERIOD 2304

#include <math.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "varicode.h"
#include "fifo.h"

uint8_t txChar(void);


void prepareSinLUT(void);

int8_t sinLUT[SIN_LUT_SIZE];

ISR(SIG_OUTPUT_COMPARE0A) {
    static uint8_t shift;
    static uint8_t k;
    static uint8_t i;
    static uint16_t p;

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

    uint16_t x;

    x = sinLUT[i++ % SIN_LUT_SIZE];

    if (shift) {
        x *= sinLUT[(p / 36)];
    } else {
        x *= 0x7f;
    }

    x = x >> 7;
    x += 128;

    OCR0A = 255 - x;
}

uint8_t txChar() {
    static char txString[] = 
        "\t"
        "\t\t...  LA9PMA/B LA9PMA/B  ...\r\n"
        "\t\t... Experimental Beacon ...\r\n"
        "\t\tLA9PMA/B < JO59fg70 Tonsberg >\r\n"
        "\t\tLA9PMA/B < 5W in dipole >\r\n"
        "\t\tLA9PMA/B < 28.321.732MHz >\r\n"
        "\t\tLA9PMA/B < Rpts to kms@skontorp.net >\r\n"
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
        if (*++txStringPos == '\0') {
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

    for (l = 0; l < SIN_LUT_SIZE; l++) {
        double d;

        d = sin((((((double) l) / SIN_LUT_SIZE)) - 0.5) * M_PI * 2.0);
        d *= 127.0;
        sinLUT[l] = d;
    }
}

int main(void) {
    wdt_reset();
    wdt_disable();

    PORTB = 0xff;
    PORTC = 0xff;
    PORTD = 0xff;

    prepareSinLUT();

    TCCR0A = _BV(COM0A1) | _BV(COM0A0) | _BV(WGM01) | _BV(WGM00);
    TCCR0B = _BV(CS00);
    TIMSK0 = _BV(OCIE0A);

    DDRD = _BV(DDD6);
    DDRD |= _BV(DDD0);

    //UCSR0B = _BV(TXEN0);
    //UCSR0B |= _BV(UDRIE0);
    //UBRR0L = 9;

    set_sleep_mode(SLEEP_MODE_IDLE);

    sei();

    for (;;) {
        sleep_mode();
    }
}
