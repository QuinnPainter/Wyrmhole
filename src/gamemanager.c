#include "text.h"
#include "gamemanager.h"

bcd16 score = 0;
bcd16 highScore = 0;
uint16_t gameTime = 0; // Number of frames elapsed since the start of a "round". Used for difficulty scaling.

uint16_t minTimeBetweenSpawns = 10;
uint16_t spawnTimeVariance = 0x0000; // needs to be a bitmask. add this to minTime to get the maxTime
uint16_t spiralEnemyChance = 0x7FFF; // if random less than this, that enemy will be spawned
// otherwise basic "go straight" enemy will be spawned

void addScore(bcd16 val) {
    addBCD16(&score, val);
    if (score > highScore) {
        highScore = score;
    }
    drawBCD16((uint8_t*)0x9C00, score);
}

void updateProgression() {
    // cap instead of overflowing, so game stays at hardest difficulty
    if (gameTime != 0xFFFF) {
        gameTime++;
    }
}
