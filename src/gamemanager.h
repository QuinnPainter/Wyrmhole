#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include "helpers.h"
#include "bcd.h"

extern bcd16 score;
extern bcd16 highScore;
extern uint16_t gameTime;

struct difficulty {
    uint16_t minTimeBetweenSpawns;
    uint16_t spawnTimeVariance; // needs to be a bitmask. add this to minTime to get the maxTime
    uint16_t spiralEnemyChance; // if random less than this, that enemy will be spawned
    // otherwise basic "go straight" enemy will be spawned
    uint16_t formation2Chance;
    uint16_t formation3Chance;
    uint16_t formation4Chance;
    uint16_t formation5Chance;
};

extern struct difficulty curDifficulty;

void addScore(bcd16 value);
void initProgression();
void updateProgression();

#endif
