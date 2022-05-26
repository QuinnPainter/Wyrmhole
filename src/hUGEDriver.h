#ifndef HUGEDRIVER_H
#define HUGEDRIVER_H

#include "stdint.h"

enum hUGE_channel_t {HT_CH1 = 0, HT_CH2, HT_CH3, HT_CH4};
enum hUGE_mute_t    {HT_CH_PLAY = 0, HT_CH_MUTE};

void hUGE_init(uint8_t* songPtr) __sdcccall(0);
void hUGE_dosound();
void hUGE_mute_channel(enum hUGE_channel_t ch, enum hUGE_mute_t mute) __sdcccall(0);

extern const uint8_t MUSIC_INGAME[];

#endif
