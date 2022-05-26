INCLUDE "sdk/hardware.inc"

DEF TILES_PER_LINE EQU 10 ; number of map tiles copied per Hblank
DEF CHUNKS_PER_FRAME EQU 4 ; how many TILES_PER_LINE sized chunks to copy per frame
DEF CHUNKS_ON_SCREEN EQU (((20 * 18) / TILES_PER_LINE) / CHUNKS_PER_FRAME) ; this must be less than ANIM_SPEED
DEF DRAW_POSITION_1 EQU ($9800 + (14 * 32)) ; screen address of the top-left corner of wormhole (14 lines down)
DEF DRAW_POSITION_2 EQU ($9C00 + (14 * 32))
DEF WORMHOLE_ANIM_SPEED EQU 15 ; number of frames between animation updates

SECTION "BG Worhmole Vars", WRAM0
wAnimFrameCtr: DS 1
wCurBufAddr: DS 2
wCurDrawAddr: DS 2
wCpyCtr: DS 1
wWhichMap: DS 1

SECTION "BG Wormhole Code", ROM0

_initWormhole::
    ld a, 1
    ld [wAnimFrameCtr], a
    xor a
    ld [wCpyCtr], a
    ld [wWhichMap], a
    ret

_updateWormholeAnim::
    ld hl, wAnimFrameCtr
    dec [hl]
    ret nz
    ld a, WORMHOLE_ANIM_SPEED
    ld [hl], a

    ld hl, _bgBuffer
    ld b, $F
REPT (20 * 18)
    ld a, [hl]
    dec a
    and b
    ld [hli], a
ENDR

    ld hl, wCurBufAddr
    ld a, LOW(_bgBuffer) ; set wCurBufAddr
    ld [hli], a
    ld a, HIGH(_bgBuffer)
    ld [hli], a
    ld a, [wWhichMap]
    xor $FF ; not cpl, we need z flag
    ld [wWhichMap], a
    jr z, .map1
    ld a, LOW(DRAW_POSITION_2) ; set wCurDrawAddr
    ld [hli], a
    ld a, HIGH(DRAW_POSITION_2)
    ld [hli], a
    jr .mapDone
.map1:
    ld a, LOW(DRAW_POSITION_1) ; set wCurDrawAddr
    ld [hli], a
    ld a, HIGH(DRAW_POSITION_1)
    ld [hli], a
.mapDone:
    ld a, CHUNKS_ON_SCREEN
    ld [hli], a ; set wCpyCtr
    ret

_cpyWormhole::
    ld a, [wCpyCtr]
    and a
    ret z ; return if done copying
    ld hl, wCurBufAddr
    ld a, [hli]
    ld e, a
    ld a, [hli]
    ld d, a ; DE = wCurBufAddr
    ld a, [hli]
    ld h, [hl]
    ld l, a ; HL = wCurDrawAddr
    ld b, %11
    ld c, CHUNKS_PER_FRAME
    di
    ; Wait until we're not in HBlank
:   ldh a, [rSTAT]
    and b
    jr z, :-
.cpyLineLp:
    ; Wait for HBlank
:   ldh a, [rSTAT]
    and b
    jr nz, :-
REPT TILES_PER_LINE
    ld a, [de] ;c2 b1
    ld [hli], a ; c2 b1
    inc de ; c2 b1
ENDR
    bit 0, c
    jr z, :+
    ld a, 12 ; go to next line
    add l
    ld l, a
    ld a, 0
    adc h
    ld h, a
:   dec c
    jr nz, .cpyLineLp
    ei
    ld a, l ; writeback addresses
    ld [wCurDrawAddr], a
    ld a, h
    ld [wCurDrawAddr + 1], a
    ld a, e
    ld [wCurBufAddr], a
    ld a, d
    ld [wCurBufAddr + 1], a

    ld hl, wCpyCtr ; check if at end of buffer
    dec [hl]
    ret nz
    ldh a, [rLCDC] ; done copying, swap buffers
    xor LCDCF_BG9C00
    ldh [rLCDC], a
    ret
