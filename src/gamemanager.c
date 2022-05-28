#include "text.h"
#include "gamemanager.h"

bcd16 score;
bcd16 highScore = 0;
uint16_t gameTime; // Number of frames elapsed since the start of a "round". Used for difficulty scaling.
uint8_t curStage;

struct difficulty curDifficulty;

struct difficulty difficultyTable[] = {
    { .minTimeBetweenSpawns = 10,
    .spawnTimeVariance = 0x0000,
    .spiralEnemyChance = 0x0000,
    .formation2Chance = 0x0000,
    .formation3Chance = 0x0000,
    .formation4Chance = 0x0000,
    .formation5Chance = 0xFFFF, },
    { .minTimeBetweenSpawns = 100,
    .spawnTimeVariance = 0x0000,
    .spiralEnemyChance = 0x7FFF,
    .formation2Chance = 0x0000,
    .formation3Chance = 0x0000,
    .formation4Chance = 0x0000,
    .formation5Chance = 0x0000, },
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
    score = 0;
    curStage = 0;
    curDifficulty = difficultyTable[0];
    gameTime = 0;
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
