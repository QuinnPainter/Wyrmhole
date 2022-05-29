#include "text.h"
#include "gamemanager.h"

bcd16 score;
bcd16 highScore = 0;
uint16_t gameTime; // Number of frames elapsed since the start of a "round". Used for difficulty scaling.
uint8_t curStage;

struct difficulty curDifficulty;

struct difficulty difficultyTable[] = {
    { .minTimeBetweenSpawns = 100,
    .spawnTimeVariance = 0x001F,
    .obstacleEnemyChance = 0x3FFF,
    .spiralEnemyChance = 0x7FFF,
    .shooterEnemyChance = 0x0000,
    .specialEnemyChance = 0x0000,
    .formation2Chance = 0x3FFF,
    .formation3Chance = 0x7FFF,
    .formation4Chance = 0x0000,
    .formation5Chance = 0x0000, },
    { .minTimeBetweenSpawns = 100,
    .spawnTimeVariance = 0x001F,
    .obstacleEnemyChance = 0x3FFF,
    .spiralEnemyChance = 0x7FFF,
    .shooterEnemyChance = 0xBE00,
    .specialEnemyChance = 0xBFFF,
    .formation2Chance = 0x3FFF,
    .formation3Chance = 0x7E00,
    .formation4Chance = 0x7FFF,
    .formation5Chance = 0x0000, },
    { .minTimeBetweenSpawns = 10, // short explosion of enemies
    .spawnTimeVariance = 0x0003,
    .obstacleEnemyChance = 0x3FFF,
    .spiralEnemyChance = 0x7FFF,
    .shooterEnemyChance = 0xBE00,
    .specialEnemyChance = 0xBFFF,
    .formation2Chance = 0x3FFF,
    .formation3Chance = 0x7E00,
    .formation4Chance = 0x7FFF,
    .formation5Chance = 0x0000, },
    { .minTimeBetweenSpawns = 10, // special reward
    .spawnTimeVariance = 0x0003,
    .obstacleEnemyChance = 0x0000,
    .spiralEnemyChance = 0x0000,
    .shooterEnemyChance = 0x0000,
    .specialEnemyChance = 0x7FFF,
    .formation2Chance = 0x3FFF,
    .formation3Chance = 0x5E00,
    .formation4Chance = 0x5FFF,
    .formation5Chance = 0x0000, },
    { .minTimeBetweenSpawns = 70,
    .spawnTimeVariance = 0x001F,
    .obstacleEnemyChance = 0x1FFF,
    .spiralEnemyChance = 0x3FFF,
    .shooterEnemyChance = 0x7FFF,
    .specialEnemyChance = 0xBFFF,
    .formation2Chance = 0x1FFF,
    .formation3Chance = 0x3FFF,
    .formation4Chance = 0x7E00,
    .formation5Chance = 0x7FFF, },
    { .minTimeBetweenSpawns = 50,
    .spawnTimeVariance = 0x001F,
    .obstacleEnemyChance = 0x1FFF,
    .spiralEnemyChance = 0x3FFF,
    .shooterEnemyChance = 0x7FFF,
    .specialEnemyChance = 0xBFFF,
    .formation2Chance = 0x1FFF,
    .formation3Chance = 0x2FFF,
    .formation4Chance = 0x4FFF,
    .formation5Chance = 0x7FFF, },
    { .minTimeBetweenSpawns = 30,
    .spawnTimeVariance = 0x001F,
    .obstacleEnemyChance = 0x1FFF,
    .spiralEnemyChance = 0x3FFF,
    .shooterEnemyChance = 0x7FFF,
    .specialEnemyChance = 0xBFFF,
    .formation2Chance = 0x1FFF,
    .formation3Chance = 0x2FFF,
    .formation4Chance = 0x4FFF,
    .formation5Chance = 0x7FFF, },
};
uint16_t difficultyStageLengths[] = {
    0x0400,
    0x0800,
    0x09FF,
    0x0AFF,
    0x0FFF,
    0x1FFF,
    0xFFFF, // last stage should always be FFFF
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
