/*

CoffeeBaT-FX (CBT-FX)

Very simple sound effect """""Driver""""" by Coffee Bat

...And as the name implies it is very close to CBT to make the sfx...

Also thanks to bbbbbr for getting my code to ACTUALLY work

*/
#include "cbtfx.h"
#include "sdk/hardware.h"
#include "hUGEDriver.h"
#define MUSIC_DRIVER_CH2_ON hUGE_mute_channel(HT_CH2, 0);
#define MUSIC_DRIVER_CH2_OFF hUGE_mute_channel(HT_CH2, 1);
#define MUSIC_DRIVER_CH4_ON hUGE_mute_channel(HT_CH4, 0);
#define MUSIC_DRIVER_CH4_OFF hUGE_mute_channel(HT_CH4, 1);

const unsigned char CBTFX_HEADER[] = "CBT-FX BY COFFEEBAT 2021-2022";
const uint8_t * CBTFX_pointer;
uint8_t CBTFX_size = 0;
uint8_t CBTFX_repeater = 0;
uint8_t CBTFX_panning = 0;
uint8_t CBTFX_priority = 0;
uint8_t CBTFX_ch_used = 0;

//Restart values and point to the new sfx
void CBTFX_init(const unsigned char * SFX) {
    if (*SFX & 0x0f < CBTFX_priority) return;
    // To avoid hanging notes
    if (CBTFX_ch_used & 128) rNR21 = rNR22 = rNR23 = rNR24 = 0;
    if (CBTFX_ch_used & 32) rNR41 = rNR42 = rNR43 = rNR44 = 0;
    CBTFX_priority = *SFX & 0x0f;
    CBTFX_ch_used = *SFX++;
    CBTFX_size = *SFX++;
    CBTFX_repeater = 0;
    CBTFX_pointer = SFX;
    if (CBTFX_ch_used & 128) MUSIC_DRIVER_CH2_OFF;
    if (CBTFX_ch_used & 32) MUSIC_DRIVER_CH4_OFF;
}

void CBTFX_update(void) {
    if (CBTFX_size != 0){ // If we have an SFX to play...

    	if(CBTFX_repeater != 0){ // If we are still playing a frame
    		CBTFX_repeater--; // Remove one from the frame counter
    	}else{

        	CBTFX_repeater = *CBTFX_pointer++; // Load the frame's length

            uint8_t mask = 0; // Mask to avoid muting an unused channel
            if (CBTFX_ch_used & 128) mask |= 0x22;
            if (CBTFX_ch_used & 32) mask |= 0x88;
            rNR51 &= ~mask; // Mask out the CH2 and CH4 pan values
            rNR51 |= mask & *CBTFX_pointer++; // And write ours
            
            if (CBTFX_ch_used & 128){
                rNR21 = *CBTFX_pointer++;
                rNR22 = *CBTFX_pointer & 0xf0; // To assure the envelope is set to 0
            }
            
            if (CBTFX_ch_used & 32){
                rNR42 = *CBTFX_pointer << 4; // Volume for the noise channel is the lower 4 bits of the same byte
            }

            CBTFX_pointer++;

            // If CH2 isn't used, we omit this data
            if (CBTFX_ch_used & 128) {
                rNR23 = *CBTFX_pointer++; // The lower 8 bits of the frequency
                rNR24 = *CBTFX_pointer++; // Higher 3 bits of the frequency + making sure the length isn't used and triggering the channel
            }

            // If CH4 isn't used, we omit this data
            if (CBTFX_ch_used & 32) {
                rNR43 = *CBTFX_pointer++; // Noise freq
                rNR44 = 0x80; // Trigger the noise channel
            }

            CBTFX_size--;

            if(CBTFX_size == 0){
           		CBTFX_priority = 0;
                if (CBTFX_ch_used & 128){
                    MUSIC_DRIVER_CH2_ON;
                    rNR21 = rNR22 = rNR23 = rNR24 = 0;
                }
                if (CBTFX_ch_used & 32){
                    MUSIC_DRIVER_CH4_ON;
                    rNR41 = rNR42 = rNR43 = rNR44 = 0;
                }
            }

        }
    }
}
