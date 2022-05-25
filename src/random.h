#ifndef RANDOM_H
#define RANDOM_H

#include "stdint.h"

extern uint16_t randState;

uint16_t genRandom() __preserves_regs(d, e);

#endif
