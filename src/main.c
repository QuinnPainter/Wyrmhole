#include "sdk/interrupt.h"
#include "sdk/oam.h"
#include "sdk/video.h"
#include "sdk/system.h"
#include "sdk/joypad.h"
#include "sdk/hardware.h"
#include "helpers.h"
#include "angles.h"
#include "fastmult.h"
#include "bgWormhole.h"
#include "gbdecompress.h"
#include "gameassets.h"
#include "player.h"
#include "bullet.h"
#include "hUGEDriver.h"
#include "enemy.h"
#include "collision.h"
#include "text.h"
#include "gamemanager.h"
#include "random.h"

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
#define C 12
uint8_t bgBuffer[] = {
    C, C, B, A, A, 9, 9, 8, 8, 8, 8, 8, 8, 9, 9, A, A, B, C, C,
    C, B, A, A, 9, 8, 8, 8, 7, 7, 7, 7, 8, 8, 8, 9, A, A, B, C,
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
    C, B, A, A, 9, 8, 8, 8, 7, 7, 7, 7, 8, 8, 8, 9, A, A, B, C,
    C, C, B, A, A, 9, 9, 8, 8, 8, 8, 8, 8, 9, 9, A, A, B, C, C
};
#undef A
#undef B
#undef C

#define WINDOW_X_SCORE (160 - 24)
#define WINDOW_Y_SCORE (144 - 8)
#define WINDOW_X_PAUSED (160 - (24 + (8 * 7)))

void main() {
    lcd_off(); // Disable screen so we can copy to VRAM freely

    gb_decompress(playerTiles, (uint8_t*)0x8000);
    gb_decompress(enemyTiles, (uint8_t*)0x8300);
    gb_decompress(bulletTiles, (uint8_t*)0x8A00);
    gb_decompress(wormholeTiles, (uint8_t*)0x9000);
    gb_decompress(fontTiles, (uint8_t*)0x9200);

    // todo - seeding
    randState = 0x1337;

    initWormhole();
    initCollision();

    rBGP = 0b11100100;
    rOBP0 = 0b11100100;
    rOBP1 = 0b11100100;

    // Setup the OAM for sprite drawing
    oam_init();

    initBullets();
    initPlayer();
    initEnemies();
    initProgression();

    copyStringVRAM(PausedString, (uint8_t*)0x9C05);
    addScore(0); // draw score

    //hUGE_init(MUSIC_INGAME);

    // Make sure sprites and the background are drawn (also turns the screen on)
    // Also sets up the window for the in game menus
    rLCDC = LCDC_ON | LCDC_OBJON | LCDC_BGON | LCDC_WIN9C00 | LCDC_WINON | LCDC_BG8800 | LCDC_OBJ16;
    rWY = WINDOW_X_SCORE;
    rWX = WINDOW_Y_SCORE;
    rSCX = 0;
    rSCY = 8 * 14;

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
        if (joypad_pressed & PAD_START) {
            rWX = WINDOW_X_PAUSED;
            while (1) {
                HALT();
                joypad_update();
                if (joypad_pressed & PAD_START) { break; }
            }
            rWX = WINDOW_X_SCORE;
        }
        updatePlayer();
        updateBullets();
        updateEnemies();
        updateProgression();
        HALT();
    }
}

ISR_VBLANK() {
    oam_dma_copy();
    //hUGE_dosound();
}
