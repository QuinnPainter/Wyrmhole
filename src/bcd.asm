SECTION "BCD Functions", ROM0

; Input - DE = Address of value to add to
; Input - BC = 4 digit BCD amount of money to add
; Sets - A H L to garbage
_addBCD16::
    ; Lower 2 digits
    ld a, [de]
    add c
    daa
    ld [de], a
    inc de
    ; Upper 2 digits
    ld a, [de]
    adc b
    daa
    ld [de], a
    ret nc
    ; Number has overflowed past 9999, so just cap it at 9999
    ld a, $99
    ld [de], a
    dec de
    ld [de], a
    ret
