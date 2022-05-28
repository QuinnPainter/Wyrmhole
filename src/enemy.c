#include "sdk/hardware.h"
#include "sdk/oam.h"
#include "fastmult.h"
#include "angles.h"
#include "enemy.h"
#include "collision.h"
#include "bullet.h"
#include "gamemanager.h"
#include "random.h"

#define NUM_ENEMIES 8
#define ENEMY_START_OAM_INDEX 4
#define ENEMY_TILEINDEX 0x30
#define ENEMY_DEATH_TILEINDEX 0x6C

#define EB_STRAIGHTSPEED 0x00A0 // Basic enemy moving toward the edge
#define EB_TARGETDIST 160
#define EB_DRIFTSPEED 0x0030
#define EB_DRIFTTIME 250 // number of frames it spends drifting before going away

#define ES_STRAIGHTSPEED 0x0050
#define ES_SIDESPEED 0x00F0
#define ES_DRIFTSPEED 0x0060

#define OFFSCREENSPEED 0x0100 // speed of enemy moving offscreen
#define DEATHTIME 0x1B // number of frames spent in death animation

enum EnemyTypes {
    ETYPE_INACTIVE = 0,
    ETYPE_OBSTACLE,
    ETYPE_BASIC,
    ETYPE_SPIRAL,
};

struct Enemy {
    uint8_t type;
    uint8_t aistate;
    uint8_t timer;
    uint16_t angle;
    uint16_t distance; // 8.8 fixed point
};

struct Enemy enemyArray[NUM_ENEMIES];
uint16_t spawnTimer;

void initEnemies() {
    spawnTimer = 60;
    for (uint8_t i = 0; i < NUM_ENEMIES; i++) {
        enemyArray[i].type = ETYPE_INACTIVE;
        enemyArray[i].aistate = 0;
        shadow_oam[ENEMY_START_OAM_INDEX + (i * 2)].y = 0;
        shadow_oam[ENEMY_START_OAM_INDEX + (i * 2)].attr = 0;
        shadow_oam[ENEMY_START_OAM_INDEX + (i * 2) + 1].y = 0;
        shadow_oam[ENEMY_START_OAM_INDEX + (i * 2) + 1].attr = 0;
    }
}

void updateEnemies() {
    spawnTimer--;
    if (spawnTimer == 0) {
        spawnTimer = (genRandom() & curDifficulty.spawnTimeVariance) + curDifficulty.minTimeBetweenSpawns;
        uint16_t rand2 = genRandom();
        if (rand2 < curDifficulty.spiralEnemyChance) {
            spawnEnemy(ETYPE_SPIRAL);
        } else {
            spawnEnemy(ETYPE_OBSTACLE);
        }
    }

    for (uint8_t i = 0; i < NUM_ENEMIES; i++) {
        switch (enemyArray[i].type) { // Process AI
            case ETYPE_INACTIVE: {
                if (enemyArray[i].aistate == 0) { continue; }
                enemyArray[i].timer--;
                if (enemyArray[i].timer == 0) {
                    deleteEnemy(i);
                }
                continue;
            }
            case ETYPE_OBSTACLE: { // Inanimate object that goes straight and nothing else.
                uint8_t oldDist = (uint8_t)(enemyArray[i].distance >> 8);
                // todo - this should probably be a table?
                if (enemyArray[i].distance < (20 << 8)) {
                    enemyArray[i].distance += 0x0060;
                } else if (enemyArray[i].distance < (50 << 8)) {
                    enemyArray[i].distance += 0x00B0;
                } else if (enemyArray[i].distance < (90 << 8)) {
                    enemyArray[i].distance += 0x0120;
                } else if (enemyArray[i].distance < (130 << 8)) {
                    enemyArray[i].distance += 0x0200;
                } else if (enemyArray[i].distance < (180 << 8)) {
                    enemyArray[i].distance += 0x03F0;
                } else {
                    enemyArray[i].distance += 0x0500;
                }
                if ((uint8_t)(enemyArray[i].distance >> 8) < oldDist) { // check for overflow
                    deleteEnemy(i);
                    continue;
                }
                break;
            }
            case ETYPE_BASIC: // Moves straight to the edge, sticks around there for a while, then leaves
                switch (enemyArray[i].aistate) {
                    case 0: // Moving to edge
                        enemyArray[i].distance += EB_STRAIGHTSPEED;
                        if ((uint8_t)(enemyArray[i].distance >> 8) > EB_TARGETDIST) {
                            enemyArray[i].aistate = ((uint8_t)genRandom() & 1) + 1; // set state to 1 or 2 to determine drift direction
                            enemyArray[i].timer = EB_DRIFTTIME;
                        }
                        break;
                    case 1: // Drifting counterclockwise
                        enemyArray[i].angle -= EB_DRIFTSPEED;
                        goto DONEDRIFT;
                    case 2: // Drifting clockwise
                        enemyArray[i].angle += EB_DRIFTSPEED;
DONEDRIFT:
                        enemyArray[i].timer--;
                        if (enemyArray[i].timer == 0) {
                            enemyArray[i].aistate = 3; // move to next state
                        }
                        break;
                    case 3: // moving offscreen
                        enemyArray[i].distance += OFFSCREENSPEED;
                        if ((uint8_t)(enemyArray[i].distance >> 8) < 10) { // check for overflow
                            deleteEnemy(i);
                            continue;
                        }
                        break;
                }
                break;
            case ETYPE_SPIRAL:
                switch (enemyArray[i].aistate & 0x7F) {
                    case 0: // Moving to edge
                        enemyArray[i].distance += ES_STRAIGHTSPEED;
                        if (enemyArray[i].aistate & 0x80)
                            { enemyArray[i].angle -= ES_SIDESPEED; }
                        else
                            { enemyArray[i].angle += ES_SIDESPEED; }
                        if ((uint8_t)(enemyArray[i].distance >> 8) > EB_TARGETDIST) {
                            enemyArray[i].aistate |= 1;
                            enemyArray[i].timer = EB_DRIFTTIME;
                        }
                        break;
                    case 1: // Drifting
                        if (enemyArray[i].aistate & 0x80)
                            { enemyArray[i].angle -= ES_DRIFTSPEED; }
                        else
                            { enemyArray[i].angle += ES_DRIFTSPEED; }
                        enemyArray[i].timer--;
                        if (enemyArray[i].timer == 0) {
                            enemyArray[i].aistate = 2; // move to next state
                        }
                        break;
                    case 2: // moving offscreen
                        enemyArray[i].distance += OFFSCREENSPEED;
                        if ((uint8_t)(enemyArray[i].distance >> 8) < 10) { // check for overflow
                            deleteEnemy(i);
                            continue;
                        }
                        break;
                }
                break;
        }

        uint8_t distance = enemyArray[i].distance >> 8;
        uint8_t tileOffset = 8;
        if (distance < 40)
            { tileOffset = 0; }
        else if (distance < 90)
            { tileOffset = 4; }
        tileOffset += (enemyArray[i].type - 1) * 12;

        uint8_t oamIndex1 = ENEMY_START_OAM_INDEX + (i * 2);
        shadow_oam[oamIndex1].tile = ENEMY_TILEINDEX + tileOffset;
        shadow_oam[oamIndex1 + 1].tile = ENEMY_TILEINDEX + tileOffset + 2;

        uint8_t angle = enemyArray[i].angle >> 8;
        //-8 to compensate for fact sprite is 16x16, so -8 to base coordinates around the middle
        uint8_t baseX = (fastmult_IbyU(CosTable[angle], distance) >> 8) + MIDSCREEN_X_OFS - 8;
        uint8_t baseY = (fastmult_IbyU(SinTable[angle], distance) >> 8) + MIDSCREEN_Y_OFS - 8;
        shadow_oam[oamIndex1].y = baseY;
        shadow_oam[oamIndex1].x = baseX;
        shadow_oam[oamIndex1 + 1].y = baseY;
        shadow_oam[oamIndex1 + 1].x = baseX + 8;

        collisionArray[COLLISION_INDEX_ENEMIES + i].objType = OBJTYPE_ENEMY;
        collisionArray[COLLISION_INDEX_ENEMIES + i].yTop = baseY - 7;
        collisionArray[COLLISION_INDEX_ENEMIES + i].yBottom = baseY + 7;
        collisionArray[COLLISION_INDEX_ENEMIES + i].xLeft = baseX - 7;
        collisionArray[COLLISION_INDEX_ENEMIES + i].xRight = baseX + 7;
        collisionArray[COLLISION_INDEX_ENEMIES + i].info = i;

        uint8_t colData = objCollisionCheck(COLLISION_INDEX_ENEMIES + i, OBJTYPE_PLAYERBULLET);
        if (colData != 0xFF) {
            deleteBullet(colData);
            if (enemyArray[i].type != ETYPE_OBSTACLE) {
                addScore(0x0005);
                enemyArray[i].type = ETYPE_INACTIVE;
                enemyArray[i].aistate = 1;
                enemyArray[i].timer = DEATHTIME;
                collisionArray[COLLISION_INDEX_ENEMIES + i].objType = OBJTYPE_DISABLED;
                uint8_t oamIndex1 = ENEMY_START_OAM_INDEX + (i * 2);
                shadow_oam[oamIndex1].tile = ENEMY_DEATH_TILEINDEX;
                shadow_oam[oamIndex1 + 1].tile = ENEMY_DEATH_TILEINDEX + 2;
                continue;
            }
        }
    }
}

void spawnEnemy(uint8_t type) {
    uint16_t randNum = genRandom();
    uint8_t aistate = 0;
    if (type == ETYPE_SPIRAL) { // choose random direction for spiralling enemies
        aistate |= (uint8_t)randNum & 0x80;
    }
    for (uint8_t i = 0; i < NUM_ENEMIES; i++) {
        if (enemyArray[i].type == ETYPE_INACTIVE && enemyArray[i].aistate == 0) {
            enemyArray[i].type = type;
            enemyArray[i].aistate = aistate;
            enemyArray[i].angle = randNum;
            enemyArray[i].distance = 0;
            return;
        }
    }
}

void deleteEnemy(uint8_t i) {
    enemyArray[i].type = ETYPE_INACTIVE;
    enemyArray[i].aistate = 0;
    shadow_oam[ENEMY_START_OAM_INDEX + (i * 2)].y = 0;
    shadow_oam[ENEMY_START_OAM_INDEX + (i * 2) + 1].y = 0;
    collisionArray[COLLISION_INDEX_ENEMIES + i].objType = OBJTYPE_DISABLED;
}
