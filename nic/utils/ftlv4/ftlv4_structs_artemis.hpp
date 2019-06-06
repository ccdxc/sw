//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __FTLV4_STRUCTS_APOLLO_HPP__
#define __FTLV4_STRUCTS_APOLLO_HPP__

#include "ftlv4_utils.hpp"

typedef struct __attribute__((__packed__)) ftlv4_entry_s {
    // data after key
    uint32_t __pad_to_512b : 25;
    uint32_t more_hints: 22;
    uint32_t more_hashes : 1;
    uint32_t hint2 : 22;
    uint32_t hash2 : 9;
    uint32_t hint1 : 22;
    uint32_t hash1 : 9;
    uint32_t flow_role : 1;
    uint32_t entry_valid : 1;

    // key
    uint32_t sport : 16;
    uint32_t dport : 16;
    uint32_t vpc_id : 8;
    uint32_t src;
    uint32_t dst;
    uint32_t proto : 8;

    uint32_t session_index : 24;
    uint32_t epoch : 8;
} ftlv4_entry_t;

#define FTLV4_ENTRY_STR(_e, _b, _l) \
{ \
    FTLV4_SNPRINTF((_b), (_l), "more_hints:%d more_hashes:%d "\
         "hint2:%d hash2:%#x hint1:%d hash1:%#x sport:%d dport:%d "\
         "src:%08x dst:%08x vpc_id:%d proto:%d "\
         "flow_role:%d epoch:%d session_index:%d entry_valid:%d",\
         (_e)->more_hints, (_e)->more_hashes, \
         (_e)->hint2, (_e)->hash2, \
         (_e)->hint1, (_e)->hash1, \
         (_e)->sport, (_e)->dport, (_e)->src, (_e)->dst, \
         (_e)->vpc_id, (_e)->proto, (_e)->flow_role, \
         (_e)->epoch, (_e)->session_index, (_e)->entry_valid); \
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
    (_e)->vpc_id = 0;\
    (_e)->entry_valid = 0; \
}

#define FTLV4_ENTRY_CLEAR_DATA(_e) \
{\
    (_e)->flow_role = 0;\
    (_e)->session_index = 0;\
    (_e)->epoch = 0;\
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
    (_d)->vpc_id = (_s)->vpc_id;\
}

#define FTLV4_ENTRY_COPY_DATA(_d, _s) \
{\
    (_d)->flow_role = (_s)->flow_role;\
    (_d)->session_index = (_s)->session_index;\
    (_d)->epoch = (_s)->epoch;\
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
    if (src->vpc_id != dst->vpc_id) return false;
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
