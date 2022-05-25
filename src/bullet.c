#include "sdk/hardware.h"
#include "sdk/oam.h"
#include "fastmult.h"
#include "angles.h"
#include "bullet.h"
#include "collision.h"

#define NUM_BULLETS 5
#define BULLET_START_OAM_INDEX (40 - 5)
#define BULLET_TILEINDEX 0xA0

struct Bullet {
    bool active;
    uint8_t angle;
    uint16_t distance; // 8.8 fixed point
    uint16_t speed;
};

struct Bullet bulletArray[NUM_BULLETS];

void initBullets() {
    for (uint8_t i = 0; i < NUM_BULLETS; i++) {
        bulletArray[i].active = false;
        shadow_oam[BULLET_START_OAM_INDEX + i].y = 0;
        shadow_oam[BULLET_START_OAM_INDEX + i].attr = 0;
    }
}

void updateBullets() {
    for (uint8_t i = 0; i < NUM_BULLETS; i++) {
        if (bulletArray[i].active == false) { continue; }

        bulletArray[i].distance += bulletArray[i].speed;
        uint8_t distance = bulletArray[i].distance >> 8;
        if (distance < 25 || distance > 253) { deleteBullet(i); continue; }

        if (distance < 40)
            { shadow_oam[BULLET_START_OAM_INDEX + i].tile = BULLET_TILEINDEX + 4; }
        else if (distance < 90)
            { shadow_oam[BULLET_START_OAM_INDEX + i].tile = BULLET_TILEINDEX + 2; }
        else
            { shadow_oam[BULLET_START_OAM_INDEX + i].tile = BULLET_TILEINDEX + 0; }

        uint8_t angle = bulletArray[i].angle;
        //-4 to compensate for fact sprite is 8x8, so -4 to base coordinates around the middle
        uint8_t baseX = (fastmult_IbyU(CosTable[angle], distance) >> 8) + MIDSCREEN_X_OFS - 4;
        uint8_t baseY = (fastmult_IbyU(SinTable[angle], distance) >> 8) + MIDSCREEN_Y_OFS - 4;
        shadow_oam[BULLET_START_OAM_INDEX + i].y = baseY;
        shadow_oam[BULLET_START_OAM_INDEX + i].x = baseX;

        collisionArray[COLLISION_INDEX_BULLETS + i].objType = OBJTYPE_PLAYERBULLET;
        collisionArray[COLLISION_INDEX_BULLETS + i].yTop = baseY - 1;
        collisionArray[COLLISION_INDEX_BULLETS + i].yBottom = baseY + 1;
        collisionArray[COLLISION_INDEX_BULLETS + i].xLeft = baseX - 1;
        collisionArray[COLLISION_INDEX_BULLETS + i].xRight = baseX + 1;
        collisionArray[COLLISION_INDEX_BULLETS + i].info = i;
    }
}

void fireBullet(uint8_t angle, uint8_t distance, uint16_t speed) {
    for (uint8_t i = 0; i < NUM_BULLETS; i++) {
        if (bulletArray[i].active == false) {
            bulletArray[i].active = true;
            bulletArray[i].angle = angle;
            bulletArray[i].distance = (uint16_t)distance << 8;
            bulletArray[i].speed = speed;
            return;
        }
    }
    // we have no available bullets - so don't fire anything
    // todo - put player bullets into different pool
}

void deleteBullet(uint8_t i) {
    bulletArray[i].active = false;
    shadow_oam[BULLET_START_OAM_INDEX + i].y = 0;
    collisionArray[COLLISION_INDEX_BULLETS + i].objType = OBJTYPE_DISABLED;
}
