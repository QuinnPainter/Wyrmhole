#ifndef COLLISION_H
#define COLLISION_H

#include "stdint.h"

struct collisionArrayEntry {
    uint8_t objType;
    uint8_t yTop;
    uint8_t yBottom;
    uint8_t xLeft;
    uint8_t xRight;
    uint8_t info;
};

#define COLLISION_INDEX_PLAYER 0
#define COLLISION_INDEX_ENEMIES 1 // 8 enemy slots
#define COLLISION_INDEX_BULLETS 9 // 5 bullet slots

enum CollisionObjTypes {
    OBJTYPE_DISABLED = 0,
    OBJTYPE_PLAYER,
    OBJTYPE_ENEMY,
    OBJTYPE_PLAYERBULLET,
};

extern struct collisionArrayEntry collisionArray[];

void initCollision() __preserves_regs(b, c, d);
uint8_t objCollisionCheck(uint8_t objIndex, uint8_t objType);

#endif
