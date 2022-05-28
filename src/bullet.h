#ifndef BULLET_H
#define BULLET_H

#include "helpers.h"

enum bulletType {
    B_INACTIVE = 0,
    B_PLAYER,
    B_ENEMY
};

void initBullets();
void updateBullets();
void fireBullet(uint8_t type, uint8_t angle, uint8_t distance, uint16_t speed);
void deleteBullet(uint8_t i);

#endif
