#include "text.h"
#include "score.h"

bcd16 score = 0;

void addScore(bcd16 val) {
    addBCD16(&score, val);
    drawBCD16((uint8_t*)0x9C00, score);
}
