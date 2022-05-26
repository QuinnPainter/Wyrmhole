#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include "helpers.h"
#include "bcd.h"

extern bcd16 score;
extern bcd16 highScore;
extern uint16_t gameTime;

extern uint16_t minTimeBetweenSpawns;
extern uint16_t spawnTimeVariance;
extern uint16_t spiralEnemyChance;

void addScore(bcd16 value);
void updateProgression();

#endif
