//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __FTLV4_STRUCTS_APOLLO_HPP__
#define __FTLV4_STRUCTS_APOLLO_HPP__

#include "ftlv4_utils.hpp"

typedef struct __attribute__((__packed__)) ftlv4_entry_s {
    // data after key
    uint32_t __pad_to_512b : 15;
    uint32_t more_hints: 22;
    uint32_t more_hashes : 1;
    uint32_t hint2 : 22;
    uint32_t hash2 : 9;
    uint32_t hint1 : 22;
    uint32_t hash1 : 9;
    uint32_t flow_role : 1;
    uint32_t nexthop_group_index_sbit7_ebit9 : 3 ;

    uint32_t sport : 16;
    uint32_t dport : 16;
    uint32_t dst;
    uint32_t src;
    uint32_t local_vnic_tag : 16;
    uint32_t proto : 8;

    // data before key
    uint32_t nexthop_group_index_sbit0_ebit6 : 7;
    uint32_t session_index : 24;
    uint32_t entry_valid : 1;
} ftlv4_entry_t;

#define FTLV4_SET_NHGROUP_INDEX(_entry, _index) \
{\
    (_entry)->nexthop_group_index_sbit0_ebit6 = _index & 0x7f; \
    (_entry)->nexthop_group_index_sbit7_ebit9 = (_index >> 7) & 0x7; \
}

#define FTLV4_ENTRY_STR(_e, _b, _l) \
{ \
    FTLV4_SNPRINTF((_b), (_l), "more_hints:%d more_hashes:%d "\
         "hint2:%d hash2:%#x hint1:%d hash1:%#x sport:%d dport:%d "\
         "src:%08x dst:%08x local_vnic_tag:%d proto:%d "\
         "flow_role:%d nexthop_group_index:%d "\
         "session_index:%d entry_valid:%d",\
         (_e)->more_hints, (_e)->more_hashes, \
         (_e)->hint2, (_e)->hash2, \
         (_e)->hint1, (_e)->hash1, \
         (_e)->sport, (_e)->dport, (_e)->src, (_e)->dst, \
         (_e)->local_vnic_tag, (_e)->proto, (_e)->flow_role, \
         ((_e)->nexthop_group_index_sbit0_ebit6 | ((_e)->nexthop_group_index_sbit7_ebit9 << 7)), \
         (_e)->session_index, (_e)->entry_valid); \
}

# define FTLV4_ENTRY_CLEAR_HINTS(_e) \
{\
    (_e)->__pad_to_512b = 0; \
    (_e)->more_hints = 0; \
    (_e)->more_hashes = 0; \
    (_e)->hash2 = 0; \
    (_e)->hint2 = 0; \
    (_e)->hint1 = 0; \
    (_e)->hash1= 0; \
}

#define FTLV4_ENTRY_CLEAR_KEY(_e) \
{ \
    (_e)->sport = 0; \
    (_e)->dport = 0; \
    (_e)->src = 0;\
    (_e)->dst = 0;\
    (_e)->proto = 0;\
    (_e)->local_vnic_tag = 0;\
    (_e)->entry_valid = 0; \
}

#define FTLV4_ENTRY_CLEAR_DATA(_e) \
{\
    (_e)->flow_role = 0;\
    (_e)->session_index = 0;\
    (_e)->nexthop_group_index_sbit0_ebit6 = 0;\
    (_e)->nexthop_group_index_sbit7_ebit9 = 0;\
}

#define FTLV4_ENTRY_CLEAR_KEY_DATA(_e) \
{\
    FTLV4_ENTRY_CLEAR_KEY(_e);\
    FTLV4_ENTRY_CLEAR_DATA(_e);\
}

#define FTLV4_ENTRY_CLEAR(_e) \
{\
    FTLV4_ENTRY_CLEAR_KEY(_e);\
    FTLV4_ENTRY_CLEAR_DATA(_e);\
    FTLV4_ENTRY_CLEAR_HINTS(_e);\
}

#define FTLV4_ENTRY_COPY_KEY(_d, _s) \
{ \
    (_d)->sport = (_s)->sport; \
    (_d)->dport = (_s)->dport; \
    (_d)->src = (_s)->src; \
    (_d)->dst = (_s)->dst;\
    (_d)->proto = (_s)->proto;\
    (_d)->local_vnic_tag = (_s)->local_vnic_tag;\
}

#define FTLV4_ENTRY_COPY_DATA(_d, _s) \
{\
    (_d)->flow_role = (_s)->flow_role;\
    (_d)->session_index = (_s)->session_index;\
    (_d)->nexthop_group_index_sbit0_ebit6 = (_s)->nexthop_group_index_sbit0_ebit6;\
    (_d)->nexthop_group_index_sbit7_ebit9 = (_s)->nexthop_group_index_sbit7_ebit9;\
}

#define FTLV4_ENTRY_COPY_KEY_DATA(_d, _s) \
        FTLV4_ENTRY_COPY_KEY(_d, _s);\
        FTLV4_ENTRY_COPY_DATA(_d, _s);\

#define FTLV4_ENTRY_HASH_KEY_BUILD_BUILD(_hkey, _entry) \
{ \
    (_hkey)->entry_valid = 0; \
    FTLV4_ENTRY_COPY_KEY(_hkey, _entry);\
    FTLV4_ENTRY_CLEAR_DATA(_hkey);\
    FTLV4_ENTRY_CLEAR_HINTS(_hkey);\
}

// TODO
static inline bool
ftlv4_entry_key_compare(ftlv4_entry_t *src, ftlv4_entry_t *dst) {
    if (src->sport != dst->sport) return false;
    if (src->dport != dst->dport) return false;
    if (src->proto != dst->proto) return false;
    if (src->local_vnic_tag != dst->local_vnic_tag) return false;
    if (src->src != dst->src) return false;
    if (src->dst != dst->dst) return false;
    return true;
}

#define __FTLV4_SET_HINT_HASH_1(_e, _h, _s) \
    { (_e)->hint1 = _h; (_e)->hash1 = _s; }
#define __FTLV4_SET_HINT_HASH_2(_e, _h, _s) \
    { (_e)->hint2 = (_h); (_e)->hash2 = (_s); }
#define __FTLV4_SET_HINT_HASH_MORE(_e, _h, _s) \
    { (_e)->more_hints = (_h); (_e)->more_hashes = (_s); }
#define FTLV4_ENTRY_SET_HINT_HASH(_i, _e, _h, _s) \
{\
    assert(_i);\
    switch (_i) { \
    case 1: __FTLV4_SET_HINT_HASH_1(_e, _h, _s); break; \
    case 2: __FTLV4_SET_HINT_HASH_2(_e, _h, _s); break; \
    default: __FTLV4_SET_HINT_HASH_MORE(_e, _h, 1); break; \
    } \
}
#define __FTLV4_GET_HINT_HASH_1(_e, _h, _s) \
    { (_h) = (_e)->hint1; (_s) = (_e)->hash1; }
#define __FTLV4_GET_HINT_HASH_2(_e, _h, _s) \
    { (_h) = (_e)->hint2; (_s) = (_e)->hash2; }
#define __FTLV4_GET_HINT_HASH_MORE(_e, _h, _s) \
    { (_h) = (_e)->more_hints; (_s) = (_e)->more_hashes; }
#define FTLV4_ENTRY_GET_HINT_HASH(_i, _e, _h, _s) \
{\
    assert(_i);\
    switch (_i) { \
    case 1: __FTLV4_GET_HINT_HASH_1(_e, _h, _s); break; \
    case 2: __FTLV4_GET_HINT_HASH_2(_e, _h, _s); break; \
    default: __FTLV4_GET_HINT_HASH_MORE(_e, _h, _s); break; \
    } \
}

#endif // __FTLV4_STRUCTS_APOLLO_HPP__
