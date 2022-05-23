; http://www.devrs.com/gb/asmcode.php#asmmath
;************************************************************
;* Fast 8x8 unsigned multiply using Log/Antilog tables v1.0 *
;************************************************************
;*
;* By Jeff Frohwein
;*
;*  Not extremely accurate but extremely fast.
;*
;*  The following code takes advantage of the fact that
;* if you take the log of two numbers, add the results,
;* and then take the antilog of the total then you have
;* done the equivalent of multiplying the two numbers:
;*
;*  x^a * x^b = x^(a+b)
;*
;*  a * b = x^(logx(a) + logx(b))
;*
; Modified by Quinn for modern RGBDS and GBSDK.
; Also added fastmult_IbyU for signed by unsigned multiplication.

SECTION "Fastmult", ROM0
; BC = A * E
_fastmult::
    ld l, e
    ld h, HIGH(STARTOF("Log Table"))
    ld e, [hl]          ; e = 32 * log_2(e)

    ld l, a
    ld a, [hl]          ; a = 32 * log_2(a)

    add a, e
    ld l, a
    ld a, 0
    adc a, 0
    ld h, a             ; hl = d + a

    add hl, hl
    set 2, h             ; hl = hl + $0400

    ld c, [hl]
    inc hl
    ld b, [hl]          ; bc = 2^([hl]/32)
    ret

_fastmult_IbyU::
    bit 7, a
    jr z, _fastmult ; if A isn't negative, we can treat it as unsigned
    cpl
    inc a ; convert to positive
    call _fastmult
    ld a, c ; convert output back to negative
    cpl
    add 1
    ld c, a
    ld a, b
    cpl
    adc 0
    ld b, a
    ret

SECTION "Log Table", ROM0, ALIGN[8]

; 32*Log_2(x) Table
;
;   FOR A=0 TO 255
;     C=4:B=2
;     FOR Z=1 TO 10
;       IF (2^C) > A THEN C=C-B ELSE C=C+B
;       B=B/2
;     NEXT Z
;     PRINT INT(C*32);",";
;   NEXT A

db 0 , 0 , 32 , 50 , 64 , 74 , 82 , 89 , 96 , 101 , 106 , 110 , 114 , 118 , 121
db 125 , 128 , 130 , 133 , 135 , 138 , 140 , 142 , 144 , 146 , 148 , 150 , 152
db 153 , 155 , 157 , 158 , 160 , 161 , 162 , 164 , 165 , 166 , 167 , 169 , 170
db 171 , 172 , 173 , 174 , 175 , 176 , 177 , 178 , 179 , 180 , 181 , 182 , 183
db 184 , 185 , 185 , 186 , 187 , 188 , 189 , 189 , 190 , 191 , 192 , 192 , 193
db 194 , 194 , 195 , 196 , 196 , 197 , 198 , 198 , 199 , 199 , 200 , 201 , 201
db 202 , 202 , 203 , 204 , 204 , 205 , 205 , 206 , 206 , 207 , 207 , 208 , 208
db 209 , 209 , 210 , 210 , 211 , 211 , 212 , 212 , 213 , 213 , 213 , 214 , 214
db 215 , 215 , 216 , 216 , 217 , 217 , 217 , 218 , 218 , 219 , 219 , 219 , 220
db 220 , 221 , 221 , 221 , 222 , 222 , 222 , 223 , 223 , 224 , 224 , 224 , 225
db 225 , 225 , 226 , 226 , 226 , 227 , 227 , 227 , 228 , 228 , 228 , 229 , 229
db 229 , 230 , 230 , 230 , 231 , 231 , 231 , 231 , 232 , 232 , 232 , 233 , 233
db 233 , 234 , 234 , 234 , 234 , 235 , 235 , 235 , 236 , 236 , 236 , 236 , 237
db 237 , 237 , 237 , 238 , 238 , 238 , 238 , 239 , 239 , 239 , 239 , 240 , 240
db 240 , 241 , 241 , 241 , 241 , 241 , 242 , 242 , 242 , 242 , 243 , 243 , 243
db 243 , 244 , 244 , 244 , 244 , 245 , 245 , 245 , 245 , 245 , 246 , 246 , 246
db 246 , 247 , 247 , 247 , 247 , 247 , 248 , 248 , 248 , 248 , 249 , 249 , 249
db 249 , 249 , 250 , 250 , 250 , 250 , 250 , 251 , 251 , 251 , 251 , 251 , 252
db 252 , 252 , 252 , 252 , 253 , 253 , 253 , 253 , 253 , 253 , 254 , 254 , 254
db 254 , 254 , 255 , 255 , 255 , 255 , 255

SECTION "Antilog Table", ROM0[$400]

; AntiLog 2^(x/32) Table
;
;   FOR A=0 to 510
;   PRINT INT(2^(A/32)+.5);",";
;   NEXT A

dw 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 2
dw 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2
dw 2 , 2 , 2 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 4 , 4
dw 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 5 , 6
dw 6 , 6 , 6 , 6 , 6 , 6 , 6 , 7 , 7 , 7 , 7 , 7 , 7 , 7 , 8 , 8 , 8 , 8 , 8 , 9
dw 9 , 9 , 9 , 9 , 10 , 10 , 10 , 10 , 10 , 11 , 11 , 11 , 11 , 12 , 12 , 12 , 12
dw 13 , 13 , 13 , 13 , 14 , 14 , 14 , 15 , 15 , 15 , 16 , 16 , 16 , 17 , 17 , 17
dw 18 , 18 , 19 , 19 , 19 , 20 , 20 , 21 , 21 , 22 , 22 , 23 , 23 , 24 , 24 , 25
dw 25 , 26 , 26 , 27 , 27 , 28 , 29 , 29 , 30 , 31 , 31 , 32 , 33 , 33 , 34 , 35
dw 36 , 36 , 37 , 38 , 39 , 40 , 41 , 41 , 42 , 43 , 44 , 45 , 46 , 47 , 48 , 49
dw 50 , 52 , 53 , 54 , 55 , 56 , 57 , 59 , 60 , 61 , 63 , 64 , 65 , 67 , 68 , 70
dw 71 , 73 , 74 , 76 , 78 , 79 , 81 , 83 , 85 , 87 , 89 , 91 , 92 , 95 , 97 , 99
dw 101 , 103 , 105 , 108 , 110 , 112 , 115 , 117 , 120 , 123 , 125 , 128 , 131
dw 134 , 137 , 140 , 143 , 146 , 149 , 152 , 156 , 159 , 162 , 166 , 170 , 173
dw 177 , 181 , 185 , 189 , 193 , 197 , 202 , 206 , 211 , 215 , 220 , 225 , 230
dw 235 , 240 , 245 , 251 , 256 , 262 , 267 , 273 , 279 , 285 , 292 , 298 , 304
dw 311 , 318 , 325 , 332 , 339 , 347 , 354 , 362 , 370 , 378 , 386 , 395 , 403
dw 412 , 421 , 431 , 440 , 450 , 459 , 470 , 480 , 490 , 501 , 512 , 523 , 535
dw 546 , 558 , 571 , 583 , 596 , 609 , 622 , 636 , 650 , 664 , 679 , 693 , 709
dw 724 , 740 , 756 , 773 , 790 , 807 , 825 , 843 , 861 , 880 , 899 , 919 , 939
dw 960 , 981 , 1002 , 1024 , 1046 , 1069 , 1093 , 1117 , 1141 , 1166 , 1192
dw 1218 , 1244 , 1272 , 1300 , 1328 , 1357 , 1387 , 1417 , 1448 , 1480 , 1512
dw 1545 , 1579 , 1614 , 1649 , 1685 , 1722 , 1760 , 1798 , 1838 , 1878 , 1919
dw 1961 , 2004 , 2048 , 2093 , 2139 , 2186 , 2233 , 2282 , 2332 , 2383 , 2435
dw 2489 , 2543 , 2599 , 2656 , 2714 , 2774 , 2834 , 2896 , 2960 , 3025 , 3091
dw 3158 , 3228 , 3298 , 3371 , 3444 , 3520 , 3597 , 3676 , 3756 , 3838 , 3922
dw 4008 , 4096 , 4186 , 4277 , 4371 , 4467 , 4565 , 4664 , 4767 , 4871 , 4978
dw 5087 , 5198 , 5312 , 5428 , 5547 , 5668 , 5793 , 5919 , 6049 , 6182 , 6317
dw 6455 , 6597 , 6741 , 6889 , 7039 , 7194 , 7351 , 7512 , 7677 , 7845 , 8016
dw 8192 , 8371 , 8555 , 8742 , 8933 , 9129 , 9329 , 9533 , 9742 , 9955 , 10173
dw 10396 , 10624 , 10856 , 11094 , 11337 , 11585 , 11839 , 12098 , 12363 , 12634
dw 12910 , 13193 , 13482 , 13777 , 14079 , 14387 , 14702 , 15024 , 15353 , 15689
dw 16033 , 16384 , 16743 , 17109 , 17484 , 17867 , 18258 , 18658 , 19066 , 19484
dw 19911 , 20347 , 20792 , 21247 , 21713 , 22188 , 22674 , 23170 , 23678 , 24196
dw 24726 , 25268 , 25821 , 26386 , 26964 , 27554 , 28158 , 28774 , 29404 , 30048
dw 30706 , 31379 , 32066 , 32768 , 33485 , 34219 , 34968 , 35734 , 36516 , 37316
dw 38133 , 38968 , 39821 , 40693 , 41584 , 42495 , 43425 , 44376 , 45348 , 46341
dw 47356 , 48393 , 49452 , 50535 , 51642 , 52772 , 53928 , 55109 , 56316 , 57549
dw  58809 , 60097 , 61413 , 62757
