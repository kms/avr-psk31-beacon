/*
 * Karl-Martin Skontorp LA9PMA <kms@skontorp.net>
 * $Id: fRound.h 93 2006-05-22 09:30:07Z kms $
 */

#ifndef _FROUND_H
#define _FROUND_H 1

#include <stdint.h>


typedef struct {
    uint16_t pattern:10;
    uint8_t length:4;
} varicode;

extern varicode psk31[];

#endif
