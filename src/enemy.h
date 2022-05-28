#ifndef ENEMY_H
#define ENEMY_H

#include "helpers.h"

void initEnemies();
void updateEnemies();
void spawnEnemy(uint8_t type, uint8_t angle);
void deleteEnemy(uint8_t i);

#endif
