#ifndef FASTMULT_H
#define FASTMULT_H

#include "stdint.h"

uint16_t fastmult(uint8_t a, uint8_t b) __preserves_regs(d);
uint16_t fastmult_IbyU(int8_t a, uint8_t b) __preserves_regs(d);

#endif
