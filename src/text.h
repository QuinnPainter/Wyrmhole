#ifndef TEXT_H
#define TEXT_H

#include <stdint.h>
#include "bcd.h"

// Copy a string into locked VRAM
void copyStringVRAM(const uint8_t* src, uint8_t* dst);

// Draws a BCD number
void drawBCD16(uint8_t* dst, bcd16 input);

extern const uint8_t PausedString[];
extern const uint8_t GameOverString[];
extern const uint8_t ScoreString[];
extern const uint8_t HighScoreString[];
extern const uint8_t RetryString[];
extern const uint8_t Retry2String[];

#endif//TEXT_H
