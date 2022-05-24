; Each collision array entry:
; Byte 1 = Object Type
; Byte 2 = Y Position (of top)
; Byte 3 = Y Position (of bottom)
; Byte 4 = X Position (of left)
; Byte 5 = X Position (of right)
; Byte 6 = Object Info (CANNOT BE $FF)
;   Usually used to store the index number of an object.
SECTION "ObjCollisionArray", WRAM0, ALIGN[8]
_collisionArray::
ObjCollisionArray:
    DS 6 * 11 ; 6 bytes * 11 collision objects.
ObjCollisionArrayEnd:

SECTION "CollisionHRAM", HRAM
Scratchpad: DS 2

SECTION "CollisionCode", ROM0

; Initialises collision
; Sets - A E H L to garbage
_initCollision::
    ld hl, ObjCollisionArray
    ld e, ObjCollisionArrayEnd - ObjCollisionArray
    xor a
:   ld [hli], a
	dec e
	jr nz, :-
    ret

; Input - A = Index of object to check collision for
; Input - E = Object type to collide with (all objects that don't have this type are ignored)
; Sets - A = Collided object info if collided, $FF if not
; Sets - B C D E H L to garbage
_objCollisionCheck::
    ld l, a
    rlca
    rlca
    add l
    add l ; Multiply by 6 to get array address from index
    ld l, a

    ld h, HIGH(ObjCollisionArray)
    ld l, a ; HL = address of base object
    ld b, a ; save location of base object for later

    ld c, e ; C = Object type to check
    inc l ; skip past this object type
    ld a, [hli]
    ld d, a ; D = Top Y
    ld a, [hli]
    ld e, a ; E = Bottom Y
    ld a, [hli]
    ldh [Scratchpad], a ; Scratchpad 0 = Left X
    ld a, [hli]
    ldh [Scratchpad + 1], a ; Scratchpad 1 = Right X

    ld l, LOW(ObjCollisionArray) ; reset to the start of the array
.checkColLoop:
    ld a, b          ; \
    cp l             ; |  Check if the current object is the same as the base object
    jr z, .noCol5Inc ; /
    ld a, [hli]       ; \
    cp c              ; |  Check if the objects has the right type
    jr nz, .noCol4Inc ; /
    ld a, [hli]
    cp e ; C unset if this object bottom Y <= other object top Y (no collision)
    jr nc, .noCol3Inc
    ld a, [hli]
    dec a ; decrement to make consistent with other side
    cp d ; C set if other object bottom Y < this object top Y (no collision)
    jr c, .noCol2Inc
    ldh a, [Scratchpad + 1]
    dec a ; decrement to make consistent with other side
    cp [hl] ; C set if this object right X < other object left X (no collision)
    inc l
    jr c, .noCol1Inc
    ldh a, [Scratchpad]
    cp [hl] ; C unset if other object right X <= this object left X (no collision)
    jr nc, .noCol1Inc
    ; all conditions passed - we have a collision
    inc l
    ld a, [hl] ; save object info in A
    ret ; A != $FF - there was a collision
.noCol5Inc:
    inc l
.noCol4Inc:
    inc l
.noCol3Inc:
    inc l
.noCol2Inc:
    inc l
.noCol1Inc:
    inc l
    inc l ; skip movement info byte
    ld a, LOW(ObjCollisionArrayEnd) ; \
    cp l                            ; | Check if we got to the end of the array
    jr nz, .checkColLoop            ; /
    ld a, $FF ; A = FF - no collision
    ret
