SECTION "AnglesTable", ROM0
_SinTable::
; angles are measured 0 - 65536.0 in rgbasm
DEF MULTIPLIER EQU 15000
DEF NUM_DIRECTIONS EQU 256
ANGLE = 0.0
REPT NUM_DIRECTIONS
VAL = MUL(SIN(ANGLE), MULTIPLIER) ; 16.16 fixed point number
    ;db (VAL >> 16) & $FF ; save integer part
    db (VAL >> 8) & $FF ; save fractional part
ANGLE = ANGLE + ((65536 / NUM_DIRECTIONS) << 16) ; left shift to convert to fixed point
ENDR

_CosTable::
ANGLE = 0.0
REPT NUM_DIRECTIONS
VAL = MUL(COS(ANGLE), MULTIPLIER) ; 16.16 fixed point number
    ;db (VAL >> 16) & $FF ; save integer part
    db (VAL >> 8) & $FF ; save fractional part
ANGLE = ANGLE + ((65536 / NUM_DIRECTIONS) << 16) ; left shift to convert to fixed point
ENDR
