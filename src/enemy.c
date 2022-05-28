#include "sdk/hardware.h"
#include "sdk/oam.h"
#include "fastmult.h"
#include "angles.h"
#include "enemy.h"
#include "collision.h"
#include "bullet.h"
#include "gamemanager.h"
#include "random.h"
#include "cbtfx.h"
#include "sfx/SFX_enemy_death.h"
#include "sfx/SFX_enemy_death_special.h"

#define NUM_ENEMIES 8
#define ENEMY_START_OAM_INDEX 4
#define ENEMY_TILEINDEX 0x30
#define ENEMY_DEATH_TILEINDEX 0x6C

#define EB_STRAIGHTSPEED 0x00A0 // Basic enemy moving toward the edge
#define EB_TARGETDIST 160
#define EB_DRIFTSPEED 0x0030
#define EB_DRIFTTIME 600 // number of frames it spends drifting before going away

#define ES_STRAIGHTSPEED 0x0050
#define ES_SIDESPEED 0x00F0
#define ES_DRIFTSPEED 0x0060

#define ESH_STRAIGHTSPEED 0x0090
#define ESH_TARGETDIST 100
#define ESH_SHOOTSPEED 100
#define ESH_DRIFTSPEED 0x0040

#define ESP_STRAIGHTSPEED 0x0080
#define ESP_SIDESPEED 0x0210
#define ESP_TARGETDIST 90
#define ESP_STOPTIME 20
#define ESP_SPINTIME 300
#define ESP_OFFSCREENSPEED 0x0300


#define OFFSCREENSPEED 0x0100 // speed of enemy moving offscreen
#define DEATHTIME 0x001B // number of frames spent in death animation

#define SPAWN_FORMATION(numEnemies) {\
            uint16_t eTypeRand = genRandom();\
            uint8_t chosenType = ETYPE_BASIC;\
            if (eTypeRand < curDifficulty.obstacleEnemyChance) {\
                chosenType = ETYPE_OBSTACLE;\
            } else if (eTypeRand < curDifficulty.spiralEnemyChance) {\
                chosenType = ETYPE_SPIRAL;\
            } else if (eTypeRand < curDifficulty.shooterEnemyChance) {\
                chosenType = ETYPE_SHOOTER;\
            }\
            uint16_t formSpawnRand = genRandom();\
            uint8_t whichFormation = formSpawnRand & 0x3;\
            uint8_t startAngle = formSpawnRand >> 8;\
            for (uint8_t i = 0; i < numEnemies; i++) {\
                spawnEnemy(chosenType, startAngle + formation ## numEnemies ## Array[(whichFormation * numEnemies) + i], (uint8_t)formSpawnRand & 0x80);\
            }\
        }

const uint8_t formation2Array[] = {
    0, 128, // straight across (180 deg)
    0, 128, // straight across (180 deg)
    0, 64, // L shape (90 deg)
    0, 32 // 45 deg
};
const uint8_t formation3Array[] = {
    0, 85, 170, // spread out over circle
    0, 85, 170, // spread out over circle
    0, 64, 128, // T shape
    0, 32, 64 // spread over quadrant
};
const uint8_t formation4Array[] = {
    0, 64, 128, 192, // + shape
    0, 64, 128, 192, // + shape
    0, 42, 85, 128, // spread over half
    0, 21, 42, 64, // spread over quadrant
};
const uint8_t formation5Array[] = {
    0, 51, 102, 153, 204, // spread over circle
    0, 51, 102, 153, 204, // spread over circle
    0, 32, 64, 96, 128, // spread over half
    0, 16, 32, 48, 64, // spread over quadrant
};

const uint16_t enemyScores[] = {
    0x0005, // Basic
    0x0010, // Spiral
    0x0015, // Shooter
    0x0100, // Special
};

enum EnemyTypes {
    ETYPE_INACTIVE = 0,
    ETYPE_OBSTACLE,
    ETYPE_BASIC,
    ETYPE_SPIRAL,
    ETYPE_SHOOTER,
    ETYPE_SPECIAL
};

struct Enemy {
    uint8_t type;
    uint8_t aistate;
    uint16_t timer;
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

        uint16_t formationRand = genRandom();
        if (formationRand < curDifficulty.formation2Chance) {
            SPAWN_FORMATION(2)
        } else if (formationRand < curDifficulty.formation3Chance) {
            SPAWN_FORMATION(3)
        } else if (formationRand < curDifficulty.formation4Chance) {
            SPAWN_FORMATION(4)
        } else if (formationRand < curDifficulty.formation5Chance) {
            SPAWN_FORMATION(5)
        } else { // Spawn 1 enemy
            uint16_t eTypeRand = genRandom();
            uint8_t spawnDir = (eTypeRand & 1) << 7;
            if (eTypeRand < curDifficulty.obstacleEnemyChance) {
                spawnEnemy(ETYPE_OBSTACLE, genRandom(), spawnDir);
            } else if (eTypeRand < curDifficulty.spiralEnemyChance) {
                spawnEnemy(ETYPE_SPIRAL, genRandom(), spawnDir);
            } else if (eTypeRand < curDifficulty.shooterEnemyChance) {
                spawnEnemy(ETYPE_SHOOTER, genRandom(), spawnDir);
            } else if (eTypeRand < curDifficulty.specialEnemyChance) {
                spawnEnemy(ETYPE_SPECIAL, genRandom(), spawnDir);
            } else {
                spawnEnemy(ETYPE_BASIC, genRandom(), spawnDir);
            }
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
                switch (enemyArray[i].aistate & 0x7F) {
                    case 0: // Moving to edge
                        enemyArray[i].distance += EB_STRAIGHTSPEED;
                        if ((uint8_t)(enemyArray[i].distance >> 8) > EB_TARGETDIST) {
                            enemyArray[i].aistate |= 1;
                            enemyArray[i].timer = EB_DRIFTTIME;
                        }
                        break;
                    case 1: // Drifting
                        if (enemyArray[i].aistate & 0x80)
                            { enemyArray[i].angle -= EB_DRIFTSPEED; }
                        else
                            { enemyArray[i].angle += EB_DRIFTSPEED; }
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
            case ETYPE_SHOOTER:
                switch (enemyArray[i].aistate & 0x7F) {
                    case 0: // Moving to edge
                        enemyArray[i].distance += ESH_STRAIGHTSPEED;
                        if ((uint8_t)(enemyArray[i].distance >> 8) > ESH_TARGETDIST) {
                            enemyArray[i].aistate = 1;
                            enemyArray[i].timer = ESH_SHOOTSPEED;
                        }
                        break;
                    case 1: // Shooting
                        if (enemyArray[i].aistate & 0x80)
                            { enemyArray[i].angle -= ESH_DRIFTSPEED; }
                        else
                            { enemyArray[i].angle += ESH_DRIFTSPEED; }
                        enemyArray[i].timer--;
                        if (enemyArray[i].timer == 0) {
                            enemyArray[i].timer = ESH_SHOOTSPEED;
                            fireBullet(B_ENEMY, enemyArray[i].angle >> 8, enemyArray[i].distance >> 8, 0x0200);
                        }
                        break;
                }
                break;
            case ETYPE_SPECIAL:
                switch (enemyArray[i].aistate & 0x7F) {
                    case 0: // Moving to desired circle
                        enemyArray[i].distance += ESP_STRAIGHTSPEED;
                        if (enemyArray[i].aistate & 0x80)
                            { enemyArray[i].angle -= ESP_SIDESPEED; }
                        else
                            { enemyArray[i].angle += ESP_SIDESPEED; }
                        if ((uint8_t)(enemyArray[i].distance >> 8) > ESP_TARGETDIST) {
                            enemyArray[i].aistate |= 1;
                            enemyArray[i].timer = ESP_SPINTIME;
                        }
                        break;
                    case 1: // Spinning around
                        if (enemyArray[i].aistate & 0x80)
                            { enemyArray[i].angle -= ESP_SIDESPEED; }
                        else
                            { enemyArray[i].angle += ESP_SIDESPEED; }
                        enemyArray[i].timer--;
                        if (enemyArray[i].timer == 0) {
                            enemyArray[i].aistate = 2; // move to next state
                            enemyArray[i].timer = ESP_STOPTIME;
                        }
                        break;
                    case 2: // stop for short time
                        enemyArray[i].timer--;
                        if (enemyArray[i].timer == 0) {
                            enemyArray[i].aistate = 3; // move to next state
                        }
                        break;
                    case 3: // moving offscreen
                        enemyArray[i].distance += ESP_OFFSCREENSPEED;
                        if ((uint8_t)(enemyArray[i].distance >> 8) < 10) { // check for overflow
                            deleteEnemy(i);
                            continue;
                        }
                        break;
                }
        }

        uint8_t distance = enemyArray[i].distance >> 8;
        uint8_t tileOffset = 8;
        uint8_t colBoxSize = 7; // distance from center to edge
        if (distance < 40)
            { tileOffset = 0; colBoxSize = 3; }
        else if (distance < 90)
            { tileOffset = 4; colBoxSize = 5; }
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
        collisionArray[COLLISION_INDEX_ENEMIES + i].yTop = baseY - colBoxSize;
        collisionArray[COLLISION_INDEX_ENEMIES + i].yBottom = baseY + colBoxSize;
        collisionArray[COLLISION_INDEX_ENEMIES + i].xLeft = baseX - colBoxSize;
        collisionArray[COLLISION_INDEX_ENEMIES + i].xRight = baseX + colBoxSize;
        collisionArray[COLLISION_INDEX_ENEMIES + i].info = i;

        uint8_t colData = objCollisionCheck(COLLISION_INDEX_ENEMIES + i, OBJTYPE_PLAYERBULLET);
        if (colData != 0xFF) {
            deleteBullet(colData);
            if (enemyArray[i].type != ETYPE_OBSTACLE) {
                addScore(enemyScores[enemyArray[i].type - 2]);
                if (enemyArray[i].type == ETYPE_SPECIAL) { CBTFX_PLAY_SFX_enemy_death_special; }
                else { CBTFX_PLAY_SFX_enemy_death; }
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

void spawnEnemy(uint8_t type, uint8_t angle, uint8_t dir) {
    for (uint8_t i = 0; i < NUM_ENEMIES; i++) {
        if (enemyArray[i].type == ETYPE_INACTIVE && enemyArray[i].aistate == 0) {
            enemyArray[i].type = type;
            enemyArray[i].aistate = dir;
            enemyArray[i].angle = (uint16_t)angle << 8;
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
