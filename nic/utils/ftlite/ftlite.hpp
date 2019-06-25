//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __FTLITE_HPP__
#define __FTLITE_HPP__

#include <stdint.h>

#include "include/sdk/base.hpp"
#include "ftlite_ipv4_structs.hpp"
#include "ftlite_ipv6_structs.hpp"

namespace ftlite {

using namespace ftlite::internal;

struct __attribute__((__packed__)) common_meta_t {
    uint8_t action;
    uint16_t len;
    uint16_t sindex;
};

struct meta_t {
    union __attribute__((__packed__)) {
        struct {
            uint32_t msb : 9;
            uint32_t index : 23;
        };
        uint32_t value;
    } hash;
    uint32_t ipv6 : 1;
    uint32_t ptype : 1; // Parent Table Type
    uint32_t pslot : 3; // Parent Hint Slot
    uint32_t level : 3;
    uint32_t pindex; // Parent Index
    uint32_t lindex; // Leaf Index
public:
    void tostr(char *buff, uint32_t size) {
        FTLITE_SNPRINTF(buff, size, "hash:%#08x (msb = %#03x, index = %d) "
                        "ipv6:%d ptype:%d pslot:%d level:%d "
                        "pindex:%d lindex:%d",
                        hash.value, hash.msb, hash.index,
                        ipv6, ptype, pslot, level, pindex, lindex);
    }
};

struct __attribute__((__packed__)) v4info_t {
    ipv4_entry_t pentry;
    uint8_t pad0[32];
    ipv4_entry_t lentry;
    uint8_t pad1[32];
};

struct __attribute__((__packed__)) v6info_t {
    ipv6_entry_t pentry;
    ipv6_entry_t lentry;
};

union __attribute__((__packed__)) info_t {
    v4info_t v4;
    v6info_t v6;
};
  
struct __attribute__((__packed__)) session_meta_t {
    uint8_t fields[64];
};

struct __attribute__((__packed__)) insert_params_t {
    common_meta_t cmeta;
    meta_t imeta;
    meta_t rmeta;
    info_t iflow;
    info_t rflow;
    session_meta_t session;
};

struct init_params_t {
    uint32_t v4tid;
    uint32_t v6tid;
};

sdk_ret_t init(init_params_t *params);
sdk_ret_t insert(insert_params_t *params);

} // namespace ftlite

#endif // __FTLITE_HPP__
