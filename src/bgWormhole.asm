INCLUDE "sdk/hardware.inc"

DEF TILES_PER_LINE EQU 10 ; number of map tiles copied per Hblank

SECTION "BG Worhmole Vars", WRAM0
wAnimFrameCtr: DS 1

SECTION "BG Wormhole Code", ROM0

DEF WORMHOLE_ANIM_SPEED EQU 15 ; number of frames between animation updates

_initWormhole::
    ld a, 1
    ld [wAnimFrameCtr], a
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
    ret

_cpyWormhole::
    di
    ld hl, $9800
    ld de, _bgBuffer
    ld b, %11
    ld c, (20 * 18) / TILES_PER_LINE
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
    ret

; Copies a block of data with max size 256
; Input - HL = Destination address
; Input - DE = Start address
; Input - C = Data length
; Sets	- C to 0
; Sets	- A H L D E to garbage
memcpyFast::
	ld a, [de] ; c2 b1
	ld [hli], a ; c2 b1
	inc de ; c2 b1
	dec c ; c1 b1
	jr nz, memcpyFast ; c3/2 b2
	ret ; total = 10 cycles per byte
