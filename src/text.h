#ifndef TEXT_H
#define TEXT_H

#include <stdint.h>
#include "bcd.h"

// Copy a string into locked VRAM
void copyStringVRAM(const uint8_t* src, uint8_t* dst);

// Draws a BCD number
void drawBCD16(uint8_t* dst, bcd16 input);

extern const uint8_t PlayString[];

#endif//TEXT_H
