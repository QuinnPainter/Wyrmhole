#include "text.h"
#include "gamemanager.h"

bcd16 score = 0;
bcd16 highScore = 0;
uint16_t gameTime = 0; // Number of frames elapsed since the start of a "round". Used for difficulty scaling.

struct difficulty curDifficulty;

uint8_t curStage = 0;
struct difficulty difficultyTable[] = {
    { .minTimeBetweenSpawns = 10,
    .spawnTimeVariance = 0x0000,
    .spiralEnemyChance = 0x0000, },
    { .minTimeBetweenSpawns = 100,
    .spawnTimeVariance = 0x0000,
    .spiralEnemyChance = 0x7FFF, }
};
uint16_t difficultyStageLengths[] = {
    0x00FF,
    0xFFFF, // last stage should always have FFFF
};

void addScore(bcd16 val) {
    addBCD16(&score, val);
    if (score > highScore) {
        highScore = score;
    }
    drawBCD16((uint8_t*)0x9C00, score);
}

void initProgression() {
    curDifficulty = difficultyTable[0];
}

void updateProgression() {
    // cap instead of overflowing, so game stays at hardest difficulty
    if (gameTime != 0xFFFF) {
        gameTime++;
        if (gameTime > difficultyStageLengths[curStage]) {
            curStage++;
            curDifficulty = difficultyTable[curStage];
        }
    }
}
