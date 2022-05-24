#ifndef BCD_H
#define BCD_H

#include "stdint.h"

// obviously functions the same as a uint16, just signals that it should be treated as BCD
typedef uint16_t bcd16;

// Adds a value to a BCD16 variable in memory
void addBCD16(bcd16* val1, bcd16 val2);

#endif//BCD_H
