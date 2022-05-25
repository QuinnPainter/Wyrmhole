#include "text.h"
#include "score.h"

bcd16 score = 0;
bcd16 highScore = 0;
uint16_t gameTime = 0; // Number of frames elapsed since the start of a "round". Used for difficulty scaling.

void addScore(bcd16 val) {
    addBCD16(&score, val);
    if (score > highScore) {
        highScore = score;
    }
    drawBCD16((uint8_t*)0x9C00, score);
}
