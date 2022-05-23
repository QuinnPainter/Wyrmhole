SECTION "hUGEWrapperCode", ROM0

_hUGE_init::
    ld h, d
    ld l, e
    jp hUGE_init

