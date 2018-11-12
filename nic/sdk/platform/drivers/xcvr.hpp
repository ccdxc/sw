// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __XCVR_HPP__
#define __XCVR_HPP__

#include <cstring>
#include "include/sdk/types.hpp"
#include "include/sdk/linkmgr.hpp"

#define XCVR_SPROM_CACHE_SIZE   512
#define XCVR_MAX_PORTS          2
#define XCVR_SPROM_READ_MAX     10

namespace sdk {
namespace platform {

using sdk::linkmgr::xcvr_event_notify_t;
using sdk::types::xcvr_type_t;
using sdk::types::xcvr_cable_type_t;
using sdk::types::xcvr_state_t;

typedef struct xcvr_s {
    xcvr_type_t             type;
    xcvr_cable_type_t       cable_type;
    xcvr_state_t            state;
    uint8_t                 sprom_read_count;
    uint8_t                 cache[XCVR_SPROM_CACHE_SIZE];
} __PACK__ xcvr_t;

extern xcvr_t g_xcvr[XCVR_MAX_PORTS];

void
xcvr_reset (int port) {
    memset(&g_xcvr[port], 0, sizeof(xcvr_t));
}

xcvr_state_t
xcvr_state (int port) {
    return g_xcvr[port].state;
}

void
xcvr_set_state (int port, xcvr_state_t state) {
    g_xcvr[port].state = state;
}

uint8_t *
xcvr_cache (int port) {
    return g_xcvr[port].cache;
}

void
xcvr_set_cache (int port, uint8_t *data, int len) {
    memcpy(g_xcvr[port].cache, data, len);
}

xcvr_type_t
xcvr_type (int port) {
    return g_xcvr[port].type;
}

void
xcvr_set_type (int port, xcvr_type_t type) {
    g_xcvr[port].type = type;
}

bool
xcvr_sprom_read_count_inc (int port) {
    g_xcvr[port].sprom_read_count ++;
    if (g_xcvr[port].sprom_read_count == XCVR_SPROM_READ_MAX) {
        g_xcvr[port].sprom_read_count = 0;
        return false;
    }
    return true;
}

sdk_ret_t xcvr_poll_init(void);
void xcvr_poll_timer(void);
void xcvr_init(xcvr_event_notify_t xcvr_notify_cb);

} // namespace platform
} // namespace sdk
#endif
