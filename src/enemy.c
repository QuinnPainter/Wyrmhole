#include "sdk/hardware.h"
#include "sdk/oam.h"
#include "fastmult.h"
#include "angles.h"
#include "enemy.h"

#define NUM_ENEMIES 5
#define ENEMY_START_OAM_INDEX 2
#define ENEMY_TILEINDEX 0x30

struct Enemy {
    bool active;
    uint8_t angle;
    uint16_t distance; // 8.8 fixed point
    uint16_t speed;
};

struct Enemy enemyArray[NUM_ENEMIES];

void initEnemies() {
    for (uint8_t i = 0; i < NUM_ENEMIES; i++) {
        enemyArray[i].active = false;
        shadow_oam[ENEMY_START_OAM_INDEX + (i * 2)].y = 0;
        shadow_oam[ENEMY_START_OAM_INDEX + (i * 2)].attr = 0;
        shadow_oam[ENEMY_START_OAM_INDEX + (i * 2) + 1].y = 0;
        shadow_oam[ENEMY_START_OAM_INDEX + (i * 2) + 1].attr = 0;
    }
    spawnEnemy();
}

void updateEnemies() {
    for (uint8_t i = 0; i < NUM_ENEMIES; i++) {
        if (enemyArray[i].active == false) { continue; }

        enemyArray[i].distance += enemyArray[i].speed;
        uint8_t distance = enemyArray[i].distance >> 8;

        uint8_t tileOffset = 8;
        if (distance < 40)
            { tileOffset = 0; }
        else if (distance < 90)
            { tileOffset = 4; }

        uint8_t oamIndex1 = ENEMY_START_OAM_INDEX + (i * 2);
        shadow_oam[oamIndex1].tile = ENEMY_TILEINDEX + tileOffset;
        shadow_oam[oamIndex1 + 1].tile = ENEMY_TILEINDEX + tileOffset + 2;

        uint8_t angle = enemyArray[i].angle;
        //-8 to compensate for fact sprite is 16x16, so -8 to base coordinates around the middle
        uint8_t baseX = (fastmult_IbyU(CosTable[angle], distance) >> 8) + MIDSCREEN_X_OFS - 8;
        uint8_t baseY = (fastmult_IbyU(SinTable[angle], distance) >> 8) + MIDSCREEN_Y_OFS - 8;
        shadow_oam[oamIndex1].y = baseY;
        shadow_oam[oamIndex1].x = baseX;
        shadow_oam[oamIndex1 + 1].y = baseY;
        shadow_oam[oamIndex1 + 1].x = baseX + 8;
    }
}

void spawnEnemy() {
    for (uint8_t i = 0; i < NUM_ENEMIES; i++) {
        if (enemyArray[i].active == false) {
            enemyArray[i].active = true;
            enemyArray[i].angle = 0;
            enemyArray[i].distance = 0;
            enemyArray[i].speed = 0x0100;
            return;
        }
    } 
}

void deleteEnemy(uint8_t i) {
    enemyArray[i].active = false;
    shadow_oam[ENEMY_START_OAM_INDEX + (i * 2)].y = 0;
    shadow_oam[ENEMY_START_OAM_INDEX + (i * 2) + 1].y = 0;
}
