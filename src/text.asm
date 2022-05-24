include "sdk/hardware.inc"

NEWCHARMAP MainCharmap
CHARMAP "0", $20
CHARMAP "1", "0" + 1
CHARMAP "2", "1" + 1
CHARMAP "3", "2" + 1
CHARMAP "4", "3" + 1
CHARMAP "5", "4" + 1
CHARMAP "6", "5" + 1
CHARMAP "7", "6" + 1
CHARMAP "8", "7" + 1
CHARMAP "9", "8" + 1
CHARMAP "A", "9" + 1
CHARMAP "B", "A" + 1
CHARMAP "C", "B" + 1
CHARMAP "D", "C" + 1
CHARMAP "E", "D" + 1
CHARMAP "F", "E" + 1
CHARMAP "G", "F" + 1
CHARMAP "H", "G" + 1
CHARMAP "I", "H" + 1
CHARMAP "J", "I" + 1
CHARMAP "K", "J" + 1
CHARMAP "L", "K" + 1
CHARMAP "M", "L" + 1
CHARMAP "N", "M" + 1
CHARMAP "O", "N" + 1
CHARMAP "P", "O" + 1
CHARMAP "Q", "P" + 1
CHARMAP "R", "Q" + 1
CHARMAP "S", "R" + 1
CHARMAP "T", "S" + 1
CHARMAP "U", "T" + 1
CHARMAP "V", "U" + 1
CHARMAP "W", "V" + 1
CHARMAP "X", "W" + 1
CHARMAP "Y", "X" + 1
CHARMAP "Z", "Y" + 1
CHARMAP "!", "Z" + 1
CHARMAP ".", "!" + 1
CHARMAP " ", $10 ; Empty space

SECTION "TextBufferHRAM", HRAM
Scratchpad: DS 4

SECTION "CopyStringVRAM", ROM0
_copyStringVRAM:: ; src is DE, dst is BC
    ld h, d
    ld l, e
.lp:
    ld a, [hli]
    or a
    ret z
    ld d, a
:   ldh a, [rSTAT]
    and a, STATF_BUSY
    jr nz, :-
    ld a, d
    ld [bc], a
    inc bc
    jr .lp

SECTION "DrawBCD", ROM0
_drawBCD16:: ; dst address in DE, num to draw in BC 
    ld hl, Scratchpad

    ld a, b
    and $F0
    swap a
    add "0"
    ld [hli], a

    ld a, b
    and $0F
    add "0"
    ld [hli], a

    ld a, c
    and $F0
    swap a
    add "0"
    ld [hli], a

    ld a, c
    and $0F
    add "0"
    ld [hli], a

    ld h, d
    ld l, e
    ld de, Scratchpad
    ld c, 4
    jr LCDMemcpyFast

; Copy data into VRAM, while making sure VRAM is accesible.
; Input - HL = Destination address
; Input - DE = Source address
; Input - C  = Length
; Sets  - C to 0
; Sets  - HL DE = HL DE + C
LCDMemcpyFast:
    ldh a, [rSTAT]          ; \
    and STATF_BUSY          ; | Wait for VRAM to be ready
    jr nz, LCDMemcpyFast    ; /
    ld a, [de]
    ld [hli], a
    inc de
    dec c
    jr nz, LCDMemcpyFast
    ret


SECTION "PausedString", ROM0
_PausedString:: DB "PAUSED", 0

SECTION "GameOverString", ROM0
_GameOverString:: DB "      GAME OVER", 0

SECTION "ScoreString", ROM0
_ScoreString:: DB "SCORE", 0

SECTION "HighScoreString", ROM0
_HighScoreString:: DB "BEST", 0

SECTION "RetryString", ROM0
_RetryString:: DB  "  PRESS START", 0
_Retry2String:: DB "          TO RETRY", 0
