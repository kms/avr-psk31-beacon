/* avr-psk31-beacon
 * PSK31 Beacon generator with AF output.
 *
 * (c) Karl-Martin Skontorp <kms@skontorp.net> ~ http://22pf.org/ ~ LA9PMA
 * Licensed under the GNU GPL 2.0 or later.
 */

#ifndef _VARICODE_H
#define _VARICODE_H 1

#include <stdint.h>

typedef struct {
    uint16_t pattern:10;
    uint8_t length:4;
} varicode;

extern varicode psk31[];

#endif
