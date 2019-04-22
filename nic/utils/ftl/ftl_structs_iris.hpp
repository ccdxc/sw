//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __FTL_STRUCTS_IRIS_HPP__
#define __FTL_STRUCTS_IRIS_HPP__

typedef struct __attribute__((__packed__)) ftl_entry_s {
    // data after key
    uint32_t __pad_to_512b : 5;
    uint32_t more_hints: 18;
    uint32_t more_hashs : 1;
    uint32_t hint5 : 18;
    uint32_t hash5 : 11;
    uint32_t hint4 : 18;
    uint32_t hash4 : 11;
    uint32_t hint3 : 18;
    uint32_t hash3 : 11;
    uint32_t hint2 : 18;
    uint32_t hash2 : 11;
    uint32_t hint1_sbit4_ebit17 : 14;

     /* FieldType = K */
    uint32_t dir : 1; /* phvbit[245], Flit[0], FlitOffset[245] */
    /* FieldType = K */
    uint32_t inst : 1; /* phvbit[244], Flit[0], FlitOffset[244] */
    /* FieldType = K */
    uint32_t type : 4; /* phvbit[240], Flit[0], FlitOffset[240] */
    /* FieldType = K */
    uint32_t vrf : 16; /* phvbit[2720], Flit[5], FlitOffset[160] */
    /* FieldType = K */
    uint32_t proto : 8; /* phvbit[2712], Flit[5], FlitOffset[152] */
    /* FieldType = K */
    uint32_t sport : 16; /* phvbit[2624], Flit[5], FlitOffset[64] */
    /* FieldType = K */
    uint32_t dport : 16; /* phvbit[2608], Flit[5], FlitOffset[48] */
    /* FieldType = K */
    uint8_t src[16]; /* phvbit[2304], Flit[4], FlitOffset[256] */
    /* FieldType = K */
    uint8_t dst[16]; /* phvbit[2176], Flit[4], FlitOffset[128] */

     // data before key
    uint32_t hint1_sbit0_ebit3 : 4;
    uint32_t hash1 : 11;
    uint32_t flow_index : 20;
    uint32_t export_en : 4;
    uint32_t entry_valid : 1;
} ftl_entry_t;

#define __SET_HINT_HASH_1(_e, _h, _s) {\
    (_e)->hint1_sbit4_ebit17 = ((_h) >> 4);\
    (_e)->hint1_sbit0_ebit3 = ((_h) & 0xF);\
    (_e)->hash1 = (_s);\
}
#define __SET_HINT_HASH_2(_e, _h, _s) \
    { (_e)->hint2 = (_h); (_e)->hash2 = (_s); }
#define __SET_HINT_HASH_3(_e, _h, _s) \
    { (_e)->hint3 = (_h); (_e)->hash2 = (_s); }
#define __SET_HINT_HASH_4(_e, _h, _s) \
    { (_e)->hint4 = (_h); (_e)->hash2 = (_s); }
#define __SET_HINT_HASH_5(_e, _h, _s) \
    { (_e)->hint5 = (_h); (_e)->hash2 = (_s); }
#define __SET_HINT_HASH_MORE(_e, _h, _s) \
    { (_e)->more_hints = (_h); (_e)->more_hashs = (_s); }
#define FTL_ENTRY_SET_HINT_HASH(_i, _e, _h, _s) \
{\
    assert(_i);\
    switch (_i) { \
    case 1: __SET_HINT_HASH_1(_e, _h, _s); break; \
    case 2: __SET_HINT_HASH_2(_e, _h, _s); break; \
    case 3: __SET_HINT_HASH_3(_e, _h, _s); break; \
    case 4: __SET_HINT_HASH_4(_e, _h, _s); break; \
    case 5: __SET_HINT_HASH_5(_e, _h, _s); break; \
    default: __SET_HINT_HASH_MORE(_e, _h, 1); break; \
    } \
}

#define __GET_HINT_HASH_1(_e, _h, _s) {\
    (_h) = ((_e)->hint1_sbit4_ebit17 << 4) | (_e)->hint1_sbit0_ebit3;\
    (_s) = (_e)->hash1;\
}
#define __GET_HINT_HASH_2(_e, _h, _s) \
    { (_h) = (_e)->hint2; (_s) = (_e)->hash2; }
#define __GET_HINT_HASH_3(_e, _h, _s) \
    { (_h) = (_e)->hint3; (_s) = (_e)->hash2; }
#define __GET_HINT_HASH_4(_e, _h, _s) \
    { (_h) = (_e)->hint4; (_s) = (_e)->hash2; }
#define __GET_HINT_HASH_5(_e, _h, _s) \
    { (_h) = (_e)->hint5; (_s) = (_e)->hash2; }
#define __GET_HINT_HASH_MORE(_e, _h, _s) \
    { (_h) = (_e)->more_hints; (_s) = (_e)->more_hashs; }
#define FTL_ENTRY_GET_HINT_HASH(_i, _e, _h, _s) \
{\
    assert(_i);\
    switch (_i) { \
    case 1: __GET_HINT_HASH_1(_e, _h, _s); break; \
    case 2: __GET_HINT_HASH_2(_e, _h, _s); break; \
    case 3: __GET_HINT_HASH_3(_e, _h, _s); break; \
    case 4: __GET_HINT_HASH_4(_e, _h, _s); break; \
    case 5: __GET_HINT_HASH_5(_e, _h, _s); break; \
    default: __GET_HINT_HASH_MORE(_e, _h, _s); break; \
    } \
}

#endif // __FTL_STRUCTS_IRIS_HPP__
