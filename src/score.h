#ifndef SCORE_H
#define SCORE_H

#include "helpers.h"
#include "bcd.h"

extern bcd16 score;
extern bcd16 highScore;
extern uint16_t gameTime;

void addScore(bcd16 value);

#endif
