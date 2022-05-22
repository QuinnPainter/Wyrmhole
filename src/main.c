#include "sdk/interrupt.h"
#include "sdk/oam.h"
#include "sdk/video.h"
#include "sdk/system.h"
#include "sdk/joypad.h"
#include "sdk/hardware.h"
#include "fxengine.h"
#include "helpers.h"
#include "angles.h"
#include "fastmult.h"
#include "bgWormhole.h"
#include "gbdecompress.h"
#include "gameassets.h"

// Offsets that when applied to a sprite position they change 0,0 to the middle of the screen.
#define MIDSCREEN_X_OFS ((160 / 2) + OAM_X_OFS)
#define MIDSCREEN_Y_OFS ((144 / 2) + OAM_Y_OFS)

// "diamond" style
/*uint8_t bgBuffer[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 9, 8, 8, 9, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 9, 8, 7, 7, 8, 9, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 9, 8, 7, 6, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 9, 8, 7, 6, 5, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 9, 8, 7, 6, 5, 4, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0,
    0, 0, 0, 9, 8, 7, 6, 5, 4, 3, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0,
    0, 0, 9, 8, 7, 6, 5, 4, 3, 2, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0,
    0, 9, 8, 7, 6, 5, 4, 3, 2, 1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0,
    9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    0, 9, 8, 7, 6, 5, 4, 3, 2, 1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0,
    0, 0, 9, 8, 7, 6, 5, 4, 3, 2, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0,
    0, 0, 0, 9, 8, 7, 6, 5, 4, 3, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0,
    0, 0, 0, 0, 9, 8, 7, 6, 5, 4, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 9, 8, 7, 6, 5, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 9, 8, 7, 6, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 9, 8, 7, 7, 8, 9, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 9, 8, 8, 9, 0, 0, 0, 0, 0, 0, 0, 0
};*/
// "circle" style
#define A 10
#define B 11
uint8_t bgBuffer[] = {
    0, 0, B, A, A, 9, 9, 8, 8, 8, 8, 8, 8, 9, 9, A, A, B, 0, 0,
    0, B, A, A, 9, 8, 8, 8, 7, 7, 7, 7, 8, 8, 8, 9, A, A, B, 0,
    B, A, A, 9, 8, 8, 7, 7, 6, 6, 6, 6, 7, 7, 8, 8, 9, A, A, B,
    B, A, 9, 8, 7, 7, 6, 6, 5, 5, 5, 5, 6, 6, 7, 7, 8, 9, A, B,
    A, 9, 8, 8, 7, 6, 5, 5, 4, 4, 4, 4, 5, 5, 6, 7, 8, 8, 9, A,
    A, 9, 8, 7, 6, 5, 5, 4, 3, 3, 3, 3, 4, 5, 5, 6, 7, 8, 9, A,
    9, 8, 8, 7, 6, 5, 4, 3, 3, 2, 2, 3, 3, 4, 5, 6, 7, 8, 8, 9,
    9, 8, 7, 6, 5, 4, 3, 3, 2, 1, 1, 2, 3, 3, 4, 5, 6, 7, 8, 9,
    9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    9, 8, 7, 6, 5, 4, 3, 3, 2, 1, 1, 2, 3, 3, 4, 5, 6, 7, 8, 9,
    9, 8, 8, 7, 6, 5, 4, 3, 3, 2, 2, 3, 3, 4, 5, 6, 7, 8, 8, 9,
    A, 9, 8, 7, 6, 5, 5, 4, 3, 3, 3, 3, 4, 5, 5, 6, 7, 8, 9, A,
    A, 9, 8, 8, 7, 6, 5, 5, 4, 4, 4, 4, 5, 5, 6, 7, 8, 8, 9, A,
    B, A, 9, 8, 7, 7, 6, 6, 5, 5, 5, 5, 6, 6, 7, 7, 8, 9, A, B,
    B, A, A, 9, 8, 8, 7, 7, 6, 6, 6, 6, 7, 7, 8, 8, 9, A, A, B,
    0, B, A, A, 9, 8, 8, 8, 7, 7, 7, 7, 8, 8, 8, 9, A, A, B, 0,
    0, 0, B, A, A, 9, 9, 8, 8, 8, 8, 8, 8, 9, 9, A, A, B, 0, 0
};
#undef A
#undef B

uint8_t playerAngle = 0;
uint8_t playerDist = 0; // Distance from center of circle.

void main() {
    lcd_off(); // Disable screen so we can copy to VRAM freely

    gb_decompress(playerTiles, (uint8_t*)0x8000);
    gb_decompress(wormholeTiles, (uint8_t*)0x9000);

    initWormhole();

    rBGP = 0b11100100;
    rOBP0 = 0b11100100;
    rOBP1 = 0b11100100;

    // Setup the OAM for sprite drawing
    oam_init();

    // Setup player sprite
    shadow_oam[0].y = 0;
    shadow_oam[0].x = 0;
    shadow_oam[0].tile = 1;
    shadow_oam[0].attr = 0x00;

    initFXEngine();

    // Make sure sprites and the background are drawn (also turns the screen on)
    // Also sets up the window for the in game menus
    rLCDC = LCDC_ON | LCDC_OBJON | LCDC_BGON | LCDC_WIN9C00 | LCDC_WINON | LCDC_BG8800;
    rWY = 0xFF; // hide window
    rWX = 7; // far left
    rSCX = 0;
    rSCY = 0;

    // Setup the VBLANK interrupt.
    rIF = 0;
    rIE = IE_VBLANK;
    ENABLE_INTERRUPTS();

    // Init joypad state
    joypad_state = 0;

    while(1) {
        updateWormholeAnim();
        cpyWormhole();
        joypad_update();
        if (joypad_state & PAD_RIGHT) {
            playerAngle++;
        }
        else if (joypad_state & PAD_LEFT) {
            playerAngle--;
        }

        if ((joypad_state & PAD_UP) && playerDist < 255) { playerDist++; }
        else if ((joypad_state & PAD_DOWN) && playerDist > 2) { playerDist--; }
        // -4 to compensate for fact object is 8x8 and position represents the top left corner
        shadow_oam[0].x = (fastmult_IbyU(CosTable[playerAngle], playerDist) >> 8) + MIDSCREEN_X_OFS - 4;
        shadow_oam[0].y = (fastmult_IbyU(SinTable[playerAngle], playerDist) >> 8) + MIDSCREEN_Y_OFS - 4;

        HALT();
    }
}

ISR_VBLANK() {
    oam_dma_copy();
    updateFXEngine();
}