#include "sdk/joypad.h"
#include "sdk/oam.h"
#include "sdk/hardware.h"
#include "fastmult.h"
#include "angles.h"
#include "player.h"

uint8_t playerAngle = 0;
uint8_t playerDist = 0; // Distance from center of circle.

const uint8_t tileTable[] = {
    18, 19, 16, 17, // bottom right quadrant
    14, 15, 12, 13,
    10, 11, 8, 9,
    6, 7, 4, 5,
    0, 1, 2, 3, // bottom left
    4, 5, 6, 7,
    8, 9, 10, 11,
    12, 13, 14, 15,
    17, 16, 19, 18, // top left
    13, 12, 15, 14,
    9, 8, 11, 10,
    5, 4, 7, 6,
    3, 2, 1, 0, // top right
    7, 6, 5, 4,
    11, 10, 9, 8,
    15, 14, 13, 12
};

const uint8_t attrTable[] = {
    ATTR_XFLIP, 0, ATTR_YFLIP, ATTR_XFLIP | ATTR_YFLIP
};

void initPlayer() {
    // Setup player sprite
    shadow_oam[0].y = 0;
    shadow_oam[0].x = 0;
    shadow_oam[0].tile = 1;
    shadow_oam[0].attr = 0x00;
}

void updatePlayer() {
    if (joypad_state & PAD_RIGHT) {
        playerAngle++;
    }
    else if (joypad_state & PAD_LEFT) {
        playerAngle--;
    }

    if ((joypad_state & PAD_UP) && playerDist < 255) { playerDist++; }
    else if ((joypad_state & PAD_DOWN) && playerDist > 2) { playerDist--; }

    //-8 to compensate for fact sprite is 16x16, so -8 to base coordinates around the middle
    uint8_t baseX = (fastmult_IbyU(CosTable[playerAngle], playerDist) >> 8) + MIDSCREEN_X_OFS - 8;
    uint8_t baseY = (fastmult_IbyU(SinTable[playerAngle], playerDist) >> 8) + MIDSCREEN_Y_OFS - 8;
    shadow_oam[0].y = baseY;
    shadow_oam[0].x = baseX;
    shadow_oam[1].y = baseY + 8;
    shadow_oam[1].x = baseX;
    shadow_oam[2].y = baseY;
    shadow_oam[2].x = baseX + 8;
    shadow_oam[3].y = baseY + 8;
    shadow_oam[3].x = baseX + 8;

    // + 0x08 to offset by a half-step, so the top, bottom, left, right states are "flat"
    uint8_t rotState = ((playerAngle + 0x08) & 0xF0) >> 4;
    
    shadow_oam[0].attr = attrTable[rotState >> 2];
    shadow_oam[1].attr = attrTable[rotState >> 2];
    shadow_oam[2].attr = attrTable[rotState >> 2];
    shadow_oam[3].attr = attrTable[rotState >> 2];

    uint8_t tileTableBase = rotState * 4;
    shadow_oam[0].tile = tileTable[tileTableBase];
    shadow_oam[1].tile = tileTable[tileTableBase + 1];
    shadow_oam[2].tile = tileTable[tileTableBase + 2];
    shadow_oam[3].tile = tileTable[tileTableBase + 3];
}

