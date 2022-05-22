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

// Offsets that when applied to a sprite position they change 0,0 to the middle of the screen.
#define MIDSCREEN_X_OFS ((160 / 2) + OAM_X_OFS)
#define MIDSCREEN_Y_OFS ((144 / 2) + OAM_Y_OFS)

uint8_t playerAngle = 0;
uint8_t playerDist = 0; // Distance from center of circle.

void main()
{
    lcd_off(); // Disable screen so we can copy to VRAM freely

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
    rLCDC = LCDC_ON | LCDC_OBJON | LCDC_BGON | LCDC_WIN9C00 | LCDC_WINON;
    rWY = 0xFF; // hide window
    rWX = 7; // far left

    // Setup the VBLANK interrupt.
    rIF = 0;
    rIE = IE_VBLANK;
    ENABLE_INTERRUPTS();

    // Init joypad state
    joypad_state = 0;

    while(1)
    {
        joypad_update();
        if (joypad_state & PAD_RIGHT)
        {
            playerAngle++;
        }
        else if (joypad_state & PAD_LEFT)
        {
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

ISR_VBLANK()
{
    oam_dma_copy();
    updateFXEngine();
}
