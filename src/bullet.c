#include "sdk/hardware.h"
#include "sdk/oam.h"
#include "fastmult.h"
#include "angles.h"
#include "bullet.h"
#include "collision.h"
#include "cbtfx.h"
#include "sfx/SFX_player_fire.h"
#include "sfx/SFX_enemy_fire.h"

#define NUM_BULLETS 6
#define ENEMY_MAX_BULLETS 3
#define PLAYER_MAX_BULLETS 3
#define BULLET_START_OAM_INDEX (40 - 6)
#define BULLET_TILEINDEX 0x70
#define BULLET_ENEMY_TILEINDEX 0x76

struct Bullet {
    uint8_t type;
    uint8_t angle;
    uint16_t distance; // 8.8 fixed point
    uint16_t speed;
};

struct Bullet bulletArray[NUM_BULLETS];

void initBullets() {
    for (uint8_t i = 0; i < NUM_BULLETS; i++) {
        bulletArray[i].type = B_INACTIVE;
        shadow_oam[BULLET_START_OAM_INDEX + i].y = 0;
        shadow_oam[BULLET_START_OAM_INDEX + i].attr = 0;
    }
}

void updateBullets() {
    for (uint8_t i = 0; i < NUM_BULLETS; i++) {
        if (bulletArray[i].type == B_INACTIVE) { continue; }

        bulletArray[i].distance += bulletArray[i].speed;
        uint8_t distance = bulletArray[i].distance >> 8;
        if (distance < 25 || distance > 253) { deleteBullet(i); continue; }

        uint8_t tileOfs = bulletArray[i].type == B_PLAYER ? BULLET_TILEINDEX : BULLET_ENEMY_TILEINDEX;
        if (distance < 40)
            { shadow_oam[BULLET_START_OAM_INDEX + i].tile = tileOfs + 4; }
        else if (distance < 90)
            { shadow_oam[BULLET_START_OAM_INDEX + i].tile = tileOfs + 2; }
        else
            { shadow_oam[BULLET_START_OAM_INDEX + i].tile = tileOfs + 0; }

        uint8_t angle = bulletArray[i].angle;
        //-4 to compensate for fact sprite is 8x8, so -4 to base coordinates around the middle
        uint8_t baseX = (fastmult_IbyU(CosTable[angle], distance) >> 8) + MIDSCREEN_X_OFS - 4;
        uint8_t baseY = (fastmult_IbyU(SinTable[angle], distance) >> 8) + MIDSCREEN_Y_OFS - 4;
        shadow_oam[BULLET_START_OAM_INDEX + i].y = baseY;
        shadow_oam[BULLET_START_OAM_INDEX + i].x = baseX;

        collisionArray[COLLISION_INDEX_BULLETS + i].objType = bulletArray[i].type == B_PLAYER ? OBJTYPE_PLAYERBULLET : OBJTYPE_ENEMY;
        collisionArray[COLLISION_INDEX_BULLETS + i].yTop = baseY - 2;
        collisionArray[COLLISION_INDEX_BULLETS + i].yBottom = baseY + 2;
        collisionArray[COLLISION_INDEX_BULLETS + i].xLeft = baseX - 2;
        collisionArray[COLLISION_INDEX_BULLETS + i].xRight = baseX + 2;
        collisionArray[COLLISION_INDEX_BULLETS + i].info = i;
    }
}

void fireBullet(uint8_t type, uint8_t angle, uint8_t distance, uint16_t speed) {
    {
        uint8_t bCtr = 0;
        for (uint8_t i = 0; i < NUM_BULLETS; i++)
            { if (bulletArray[i].type == type) { bCtr++; } }
        if (type == B_PLAYER && bCtr >= PLAYER_MAX_BULLETS) { return; }
        if (type == B_ENEMY && bCtr >= ENEMY_MAX_BULLETS) { return; }
    }

    for (uint8_t i = 0; i < NUM_BULLETS; i++) {
        if (bulletArray[i].type == B_INACTIVE) {
            bulletArray[i].type = type;
            bulletArray[i].angle = angle;
            bulletArray[i].distance = (uint16_t)distance << 8;
            bulletArray[i].speed = speed;
            if (type == B_PLAYER) { CBTFX_PLAY_SFX_player_fire; }
            else { CBTFX_PLAY_SFX_enemy_fire; }
            return;
        }
    }
}

void deleteBullet(uint8_t i) {
    bulletArray[i].type = B_INACTIVE;
    shadow_oam[BULLET_START_OAM_INDEX + i].y = 0;
    collisionArray[COLLISION_INDEX_BULLETS + i].objType = OBJTYPE_DISABLED;
}
