//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __FTL_STRUCTS_APOLLO_HPP__
#define __FTL_STRUCTS_APOLLO_HPP__

#include "ftl_utils.hpp"

typedef struct __attribute__((__packed__)) ftl_entry_s {
    // data after key
    uint32_t __pad_to_512b : 4;
    uint32_t more_hints: 21;
    uint32_t more_hashes : 1;
    uint32_t hint5 : 21;
    uint32_t hash5 : 8;
    uint32_t hint4 : 21;
    uint32_t hash4 : 8;
    uint32_t hint3 : 21;
    uint32_t hash3 : 8;
    uint32_t hint2 : 21;
    uint32_t hash2 : 8;
    uint32_t hint1 : 21;
    uint32_t hash1_sbit7_ebit7 : 1;
    /* FieldType = K */
    uint32_t ktype : 4; /* phvbit[320], Flit[0], FlitOffset[320] */
    /* FieldType = K */
    uint32_t sport : 16; /* phvbit[1904], Flit[3], FlitOffset[368] */
    /* FieldType = K */
    uint32_t dport : 16; /* phvbit[1888], Flit[3], FlitOffset[352] */
    /* FieldType = K */
    uint8_t src[16]; /* phvbit[1744], Flit[3], FlitOffset[208] */
    /* FieldType = K */
    uint32_t local_vnic_tag : 16; /* phvbit[328], Flit[0], FlitOffset[328] */
    /* FieldType = K */
    uint8_t dst[16]; /* phvbit[192], Flit[0], FlitOffset[192] */
    /* FieldType = K */
    uint32_t proto : 8; /* phvbit[184], Flit[0], FlitOffset[184] */
    // data before key
    uint32_t hash1_sbit0_ebit6 : 7;
    uint32_t flow_role : 1;
    uint32_t flow_index : 23;
    uint32_t entry_valid : 1;
} ftl_entry_t;

#define FTLENTRY_STR(_e, _b, _l) \
{ \
    FTL_SNPRINTF((_b), (_l), "more_hints:%d more_hashes:%d hint5:%d hash5:%#x "\
         "hint4:%d hash4:%#x hint3:%d hash3:%#x hint2:%d hash2:%#x "\
         "hint1:%d hash1:%#x ktype:%d sport:%d dport:%d "\
         "src:%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x "\
         "dst:%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x "\
         "local_vnic_tag:%d proto:%d flow_role:%d flow_index:%d entry_valid:%d",\
         (_e)->more_hints, (_e)->more_hashes, (_e)->hint5, (_e)->hash5, \
         (_e)->hint4, (_e)->hash4, (_e)->hint3, (_e)->hash3, (_e)->hint2, (_e)->hash2, \
         (_e)->hint1, (((_e)->hash1_sbit7_ebit7 << 7) | (_e)->hash1_sbit0_ebit6), \
         (_e)->ktype, (_e)->sport, (_e)->dport, (_e)->src[0], (_e)->src[1], \
         (_e)->src[2], (_e)->src[3], (_e)->src[4], (_e)->src[5], (_e)->src[6], \
         (_e)->src[7], (_e)->src[8], (_e)->src[9], (_e)->src[10], (_e)->src[11], \
         (_e)->src[12], (_e)->src[13], (_e)->src[14], (_e)->src[15], \
         (_e)->dst[0], (_e)->dst[1], \
         (_e)->dst[2], (_e)->dst[3], (_e)->dst[4], (_e)->dst[5], (_e)->dst[6], \
         (_e)->dst[7], (_e)->dst[8], (_e)->dst[9], (_e)->dst[10], (_e)->dst[11], \
         (_e)->dst[12], (_e)->dst[13], (_e)->dst[14], (_e)->dst[15], \
         (_e)->local_vnic_tag, (_e)->proto, (_e)->flow_role, (_e)->flow_index, \
         (_e)->entry_valid); \
}

# define FTL_ENTRY_CLEAR_HINTS(_e) \
{\
    (_e)->__pad_to_512b = 0; \
    (_e)->more_hints = 0; \
    (_e)->more_hashes = 0; \
    (_e)->hash5 = 0; \
    (_e)->hint5 = 0; \
    (_e)->hash4 = 0; \
    (_e)->hint4 = 0; \
    (_e)->hash3 = 0; \
    (_e)->hint3 = 0; \
    (_e)->hash2 = 0; \
    (_e)->hint2 = 0; \
    (_e)->hint1 = 0; \
    (_e)->hash1_sbit7_ebit7 = 0; \
    (_e)->hash1_sbit0_ebit6 = 0; \
}

#define FTL_ENTRY_CLEAR_KEY(_e) \
{ \
    (_e)->ktype = 0; \
    (_e)->sport = 0; \
    (_e)->dport = 0; \
    (_e)->ktype = 0; \
    (_e)->ktype = 0; \
    memset((_e)->src, 0, 16);\
    memset((_e)->dst, 0, 16);\
    (_e)->proto = 0;\
    (_e)->local_vnic_tag = 0;\
    (_e)->entry_valid = 0; \
}

#define FTL_ENTRY_CLEAR_DATA(_e) \
{\
    (_e)->flow_role = 0;\
    (_e)->flow_index = 0;\
}

#define FTL_ENTRY_CLEAR_KEY_DATA(_e) \
{\
    FTL_ENTRY_CLEAR_KEY(_e);\
    FTL_ENTRY_CLEAR_DATA(_e);\
}

#define FTL_ENTRY_CLEAR(_e) \
{\
    FTL_ENTRY_CLEAR_KEY(_e);\
    FTL_ENTRY_CLEAR_DATA(_e);\
    FTL_ENTRY_CLEAR_HINTS(_e);\
}

#define FTL_ENTRY_COPY_KEY(_d, _s) \
{ \
    (_d)->ktype = (_s)->ktype; \
    (_d)->sport = (_s)->sport; \
    (_d)->dport = (_s)->dport; \
    memcpy((_d)->src, (_s)->src, 16);\
    memcpy((_d)->dst, (_s)->dst, 16);\
    (_d)->proto = (_s)->proto;\
    (_d)->local_vnic_tag = (_s)->local_vnic_tag;\
}

#define FTL_ENTRY_COPY_DATA(_d, _s) \
{\
    (_d)->flow_role = (_s)->flow_role;\
    (_d)->flow_index = (_s)->flow_index;\
}

#define FTL_ENTRY_COPY_KEY_DATA(_d, _s) \
        FTL_ENTRY_COPY_KEY(_d, _s);\
        FTL_ENTRY_COPY_DATA(_d, _s);\

#define FTLENTRY_HASH_KEY_BUILD(_hkey, _entry) \
{ \
    FTL_ENTRY_COPY_KEY(_hkey, _entry);\
    FTL_ENTRY_CLEAR_DATA(_hkey);\
    FTL_ENTRY_CLEAR_HINTS(_hkey);\
}

// TODO
static inline bool
ftl_entry_key_compare(ftl_entry_t *src, ftl_entry_t *dst) {
    if (src->ktype != dst->ktype) return false;
    if (src->sport != dst->sport) return false;
    if (src->dport != dst->dport) return false;
    if (src->proto != dst->proto) return false;
    if (src->local_vnic_tag != dst->local_vnic_tag) return false;
    if (memcmp(src->src, dst->src, 16)) return false;
    if (memcmp(src->dst, dst->dst, 16)) return false;
    return true;
}

#define __SET_HINT_HASH_1(_e, _h, _s) {\
    (_e)->hint1 = _h;\
    (_e)->hash1_sbit7_ebit7 = ((_s) >> 7);\
    (_e)->hash1_sbit0_ebit6 = ((_s) & 0x7F);\
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
    { (_e)->more_hints = (_h); (_e)->more_hashes = (_s); }
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
    (_h) = (_e)->hint1;\
    (_s) = ((_e)->hash1_sbit7_ebit7 << 7) |\
           ((_e)->hash1_sbit0_ebit6);\
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
    { (_h) = (_e)->more_hints; (_s) = (_e)->more_hashes; }
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

#endif // __FTL_STRUCTS_APOLLO_HPP__
