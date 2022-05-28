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
#include "cbtfx.h"
#include "enemy.h"
#include "collision.h"
#include "text.h"
#include "gamemanager.h"
#include "random.h"
#include "main.h"

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
#define WINDOW_Y_PRESSSTART (144 - (8 * 3))
#define PRESS_START_FLASH_SPEED 40
#define PRESS_START_FAST_FLASH_SPEED 10
#define PRESS_START_FLASH_TIME 100 // number of frames spent flashing before moving to the game

bool playMusic = false;
uint8_t pressStartFlashState = true;
uint8_t pressStartFlashCtr = PRESS_START_FLASH_SPEED;
uint8_t menuStartPressed = false;
uint8_t menuStateCtr;

void startGame() {
    initCollision();
    initBullets();
    initPlayer();
    updatePlayer(); // put player sprite in the right spot
    initEnemies();
    initProgression();
    HALT(); // wait for vblank / sprite DMA to happen, so there's no "glitch" when we enable sprites
    // because the sprite changes from the init functions won't be in obj ram yet until vblank
    rLCDC = rLCDC | LCDC_OBJON; // enable sprites
    rWY = WINDOW_X_SCORE;
    rWX = WINDOW_Y_SCORE;
    copyStringVRAM(PausedString, (uint8_t*)0x9C04);
    addScore(0); // draw score
}

void main() {
    lcd_off(); // Disable screen so we can copy to VRAM freely

    gb_decompress(playerTiles, (uint8_t*)0x8000);
    gb_decompress(enemyTiles, (uint8_t*)0x8300);
    gb_decompress(bulletTiles, (uint8_t*)0x8700);
    gb_decompress(wormholeTiles, (uint8_t*)0x9000);
    gb_decompress(fontTiles, (uint8_t*)0x9100);
    gb_decompress(titleTiles, (uint8_t*)0x8800);
    gb_decompress(titleTiles2, (uint8_t*)0x9370);
    gb_decompress(titleTiles3, (uint8_t*)0x8F10);

    drawTilemap((uint8_t*)0x99C0, titleTilemap, titleTilemap_end, 0x80);
    drawTilemap((uint8_t*)0x9AE0, titleTilemap2, titleTilemap2_end, 0x37);
    drawTilemap((uint8_t*)0x9C00, titleTilemap3, titleTilemap3_end, 0xF1);

    joypad_state = joypad_pressed = 0;

    initWormhole();

    rBGP = 0b11100100;
    rOBP0 = 0b11100100;
    rOBP1 = 0b11100100;

    // Setup the OAM for sprite drawing
    oam_init();

    // Init sound registers
    rAUDENA = 0xFF; // Turn on sound controller
    rAUDTERM = 0xFF; // Enable all channels
    rAUDVOL = 0x77; // Set master volume to max
    hUGE_init(MUSIC_INGAME);

    // Make sure sprites and the background are drawn (also turns the screen on)
    // Also sets up the window for the in game menus
    rLCDC = LCDC_ON | LCDC_OBJON | LCDC_BGON | LCDC_WIN9C00 | LCDC_WINON | LCDC_BG8800 | LCDC_OBJ16;
    rSCX = 0;
    rSCY = 8 * 14;
    rWX = 7 + 4;
    rWY = WINDOW_Y_PRESSSTART;

    // Setup the VBLANK interrupt.
    rIF = 0;
    rIE = IE_VBLANK;
    ENABLE_INTERRUPTS();

    while (1) { // Initial splash screen loop
        pressStartFlashCtr--;
        if (pressStartFlashCtr == 0) {
            pressStartFlashState = !pressStartFlashState;
            pressStartFlashCtr = menuStartPressed ? PRESS_START_FAST_FLASH_SPEED : PRESS_START_FLASH_SPEED;
        }
        rWY = pressStartFlashState ? WINDOW_Y_PRESSSTART : 200;
        if (menuStartPressed) {
            menuStateCtr--;
            if (menuStateCtr == 0) { break; }
        } else {
            joypad_update();
            if (joypad_pressed & PAD_START) {
                menuStateCtr = PRESS_START_FLASH_TIME;
                pressStartFlashState = true;
                pressStartFlashCtr = PRESS_START_FAST_FLASH_SPEED;
                menuStartPressed = true;
                randState = ((uint16_t)rDIV << 8) | rDIV; // not the best seeding, but whatever
            }
        }
        HALT();
    }

    playMusic = true;
    startGame();

    while(1) {
        updateWormholeAnim();
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
        updateBullets();
        updateEnemies();
        updateProgression();
        updatePlayer();
        HALT();
    }
}

ISR_VBLANK() {
    oam_dma_copy();
    cpyWormhole();
    if (playMusic) { hUGE_dosound(); }
    CBTFX_update();
}
