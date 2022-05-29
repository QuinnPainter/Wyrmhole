#include "sdk/joypad.h"
#include "sdk/oam.h"
#include "sdk/hardware.h"
#include "sdk/interrupt.h"
#include "sdk/video.h"
#include "fastmult.h"
#include "angles.h"
#include "bullet.h"
#include "player.h"
#include "collision.h"
#include "text.h"
#include "gamemanager.h"
#include "main.h"
#include "cbtfx.h"
#include "sfx/SFX_player_death.h"
#include "sfx/SFX_player_transport.h"

enum PlayerStates {
    STATE_ALIVE = 0,
    STATE_TELEPORTING1,
    STATE_TELEPORTING2,
    STATE_DYING,
};

struct ExplodeSprite {
    uint16_t y;
    uint16_t x;
};

#define PLAYER_SPEED 0x0200
#define TELEPORT_SPEED 0x1C
#define DEATH_DELAY 100 // Delay after dying before the game over screen shows, in frames.
#define NORMAL_DISTANCE 160 // Distance that the player usually sits at.
#define TP_TILEINDEX 0x14 // start tile index of teleport tiles
#define EXPLODE_TILEINDEX 0x28
#define START_EXPLODESPEED 0x0200
#define MIN_EXPLODESPEED 0x00A0
#define EXPLODESPEED_DECREASE 0x000A

uint16_t playerAngle; // 8.8 fixed
uint8_t playerDist; // Distance from center of circle.
uint8_t playerState;
uint8_t stateTimer;
uint16_t explodeSpeed;
struct ExplodeSprite exSprites[4];

const uint8_t tileTable[] = {
    18, 16, // bottom right quadrant
    14, 12,
    10, 8,
    6, 4,
    0, 2, // bottom left
    4, 6,
    8, 10,
    12, 14,
    16, 18, // top left
    12, 14,
    8, 10,
    4, 6,
    2, 0, // top right
    6, 4,
    10, 8,
    14, 12
};

const uint8_t attrTable[] = {
    ATTR_XFLIP, 0, ATTR_YFLIP, ATTR_XFLIP | ATTR_YFLIP
};

void initPlayer() {
    playerAngle = 0x03F00; // start at the bottom
    playerDist = NORMAL_DISTANCE;
    playerState = STATE_ALIVE;
    shadow_oam[2].y = 0;
    shadow_oam[3].y = 0; // hide extra explosion sprites
}

void updatePlayer() {
    if (playerState < STATE_DYING) { // Alive, tp1 or tp2
        if (playerState == STATE_ALIVE) {
            if (joypad_state & PAD_RIGHT) {
                playerAngle += PLAYER_SPEED;
            } else if (joypad_state & PAD_LEFT) {
                playerAngle -= PLAYER_SPEED;
            }

            if (joypad_pressed & PAD_A) {
                fireBullet(B_PLAYER, playerAngle >> 8, playerDist, -0x0300);
            } else if (joypad_pressed & PAD_B) {
                playerState = STATE_TELEPORTING1; // initiate teleport
                // set tiles
                // + 0x08 to offset by a half-step, so the top, bottom, left, right states are "flat"
                uint8_t rotState = (((playerAngle >> 8) + 0x08) & 0xF0) >> 4;
                uint8_t attrVal = attrTable[rotState >> 2];
                shadow_oam[0].attr = attrVal;
                shadow_oam[1].attr = attrVal;
                uint8_t tileTableBase = rotState << 1;
                shadow_oam[0].tile = TP_TILEINDEX + tileTable[tileTableBase];
                shadow_oam[1].tile = TP_TILEINDEX + tileTable[tileTableBase + 1];
                // disable collision
                collisionArray[COLLISION_INDEX_PLAYER].objType = OBJTYPE_DISABLED;
                CBTFX_PLAY_SFX_player_transport;
            }
        } else if (playerState == STATE_TELEPORTING1) {
            uint8_t oldDist = playerDist;
            playerDist -= TELEPORT_SPEED;
            if (playerDist > oldDist) { // check for underflow
                playerDist = 0;
                playerAngle += (128 << 8); // add 180 degrees
                playerState = STATE_TELEPORTING2;
            }
        } else if (playerState == STATE_TELEPORTING2) {
            playerDist += TELEPORT_SPEED;
            if (playerDist > NORMAL_DISTANCE) {
                playerDist = NORMAL_DISTANCE;
                playerState = STATE_ALIVE;
            }
        }

        //-8 to compensate for fact sprite is 16x16, so -8 to base coordinates around the middle
        uint8_t baseX = (fastmult_IbyU(CosTable[playerAngle >> 8], playerDist) >> 8) + MIDSCREEN_X_OFS - 8;
        uint8_t baseY = (fastmult_IbyU(SinTable[playerAngle >> 8], playerDist) >> 8) + MIDSCREEN_Y_OFS - 8;
        shadow_oam[0].y = baseY;
        shadow_oam[0].x = baseX;
        shadow_oam[1].y = baseY;
        shadow_oam[1].x = baseX + 8;

        if (playerState == STATE_ALIVE) {
            // + 0x08 to offset by a half-step, so the top, bottom, left, right states are "flat"
            uint8_t rotState = (((playerAngle >> 8) + 0x08) & 0xF0) >> 4;

            uint8_t attrVal = attrTable[rotState >> 2];
            shadow_oam[0].attr = attrVal;
            shadow_oam[1].attr = attrVal;

            uint8_t tileTableBase = rotState << 1;
            shadow_oam[0].tile = tileTable[tileTableBase];
            shadow_oam[1].tile = tileTable[tileTableBase + 1];

            collisionArray[COLLISION_INDEX_PLAYER].objType = OBJTYPE_PLAYER;
            collisionArray[COLLISION_INDEX_PLAYER].yTop = baseY - 4;
            collisionArray[COLLISION_INDEX_PLAYER].yBottom = baseY + 4;
            collisionArray[COLLISION_INDEX_PLAYER].xLeft = baseX - 4;
            collisionArray[COLLISION_INDEX_PLAYER].xRight = baseX + 4;
            collisionArray[COLLISION_INDEX_PLAYER].info = 0;

            uint8_t colData = objCollisionCheck(COLLISION_INDEX_PLAYER, OBJTYPE_ENEMY);
            if (colData != 0xFF) {
                stateTimer = DEATH_DELAY;
                playerState = STATE_DYING;
                for (uint8_t i = 0; i < 4; i++) {
                    shadow_oam[i].y = baseY + 4;
                    shadow_oam[i].x = baseX + 4;
                    shadow_oam[i].tile = EXPLODE_TILEINDEX;
                    shadow_oam[i].attr = 0;
                    exSprites[i].y = (uint16_t)(baseY + 4) << 8;
                    exSprites[i].x = (uint16_t)(baseX + 4) << 8;
                }
                explodeSpeed = START_EXPLODESPEED;
                CBTFX_PLAY_SFX_player_death;
            }
        }
    } else if (playerState == STATE_DYING) {
        if (explodeSpeed > MIN_EXPLODESPEED) {
            explodeSpeed -= EXPLODESPEED_DECREASE;
        }
        for (uint8_t i = 0; i < 4; i++) {
            if (i == 1 || i == 3) {
                exSprites[i].y -= explodeSpeed;
            } else {
                exSprites[i].y += explodeSpeed;
            }
            if (i == 2 || i == 3) {
                exSprites[i].x -= explodeSpeed;
            } else {
                exSprites[i].x += explodeSpeed;
            }
            shadow_oam[i].y = exSprites[i].y >> 8;
            shadow_oam[i].x = exSprites[i].x >> 8;
        }
        stateTimer--;
        if (stateTimer == 0) {
            rWX = 7;
            rWY = 144 - (8 * 9);
            rLCDC = rLCDC & ~LCDC_OBJON; // disable sprites
            uint16_t vAddr = 0x9C00;
            vram_memset(0x9C00, 0, 32 * 9); // clear window area
            copyStringVRAM(GameOverString, (uint8_t*)0x9C20);
            copyStringVRAM(ScoreString, (uint8_t*)0x9C61);
            drawBCD16((uint8_t*)0x9C6E, score);
            copyStringVRAM(HighScoreString, (uint8_t*)0x9C81);
            drawBCD16((uint8_t*)0x9C8E, highScore);
            copyStringVRAM(RetryString, (uint8_t*)0x9CC0);
            copyStringVRAM(Retry2String, (uint8_t*)0x9CE0);
            while (1) {
                joypad_update();
                if (joypad_pressed & PAD_START) {
                    startGame();
                    // sdcc seems bugged and generates only one "inc sp" then "ret"
                    // if I just put a normal "return;" statement here.
                    // this leaves the stack in the wrong place and crashes the game.
                    // this bodge seems to fix it. why is sdcc so garbage???
                    __asm__("inc sp");
                    __asm__("inc sp");
                    return;
                }
                HALT();
            }
        }
    } 
}

