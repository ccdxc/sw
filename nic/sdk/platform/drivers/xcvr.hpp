// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __XCVR_HPP__
#define __XCVR_HPP__

#include <cstring>
#include "include/sdk/types.hpp"
#include "nic/sdk/linkmgr/linkmgr.hpp"

namespace sdk {
namespace platform {

#define XCVR_SPROM_CACHE_SIZE   512
#define XCVR_MAX_PORTS          2
#define XCVR_SPROM_READ_MAX     10
#define XCVR_SPROM_READ_SIZE    128

#define AN_USER_CAP_10GBKR     0x4
#define AN_USER_CAP_40GBKR4    0x8
#define AN_USER_CAP_40GBCR4    0x10
#define AN_USER_CAP_100GBKP4   0x40
#define AN_USER_CAP_100GBKR4   0x80
#define AN_USER_CAP_100GBCR4   0x100
#define AN_USER_CAP_25GBKRCR_S 0x200
#define AN_USER_CAP_25GBKRCR   0x400

#define AN_FEC_REQ_25GB_RSFEC 0x2
#define AN_FEC_REQ_25GB_FCFEC 0x4

using sdk::linkmgr::xcvr_event_notify_t;
using sdk::linkmgr::port_args_t;
using sdk::types::xcvr_type_t;
using sdk::types::xcvr_pid_t;
using sdk::types::cable_type_t;
using sdk::types::xcvr_state_t;

extern bool xcvr_valid_enable;

typedef struct xcvr_s {
    xcvr_type_t    type;         // QSFP28, QSFP, SFP
    cable_type_t   cable_type;   // CU, Fiber
    xcvr_state_t   state;
    xcvr_pid_t     pid;
    uint8_t        sprom_read_count;
    bool           debounce;
    port_an_args_t port_an_args;
    port_speed_t   cable_speed;     // speed of inserted xcvr
    uint8_t        cache[XCVR_SPROM_CACHE_SIZE];
} __PACK__ xcvr_t;

extern xcvr_t g_xcvr[XCVR_MAX_PORTS];

inline void
xcvr_reset (int port) {
    memset(&g_xcvr[port], 0, sizeof(xcvr_t));
}

inline xcvr_state_t
xcvr_state (int port) {
    return g_xcvr[port].state;
}

inline void
xcvr_set_state (int port, xcvr_state_t state) {
    g_xcvr[port].state = state;
}

inline uint8_t *
xcvr_cache (int port) {
    return g_xcvr[port].cache;
}

inline void
xcvr_set_cache (int port, uint8_t *data, int len) {
    memcpy(g_xcvr[port].cache, data, len);
}

inline void
xcvr_set_pid (int port, xcvr_pid_t pid) {
    g_xcvr[port].pid = pid;
}

inline xcvr_pid_t
xcvr_pid (int port) {
    return g_xcvr[port].pid;
}

inline xcvr_type_t
xcvr_type (int port) {
    return g_xcvr[port].type;
}

inline bool
xcvr_get_debounce (int port) {
    return g_xcvr[port].debounce;
}

inline void
xcvr_set_debounce (int port) {
    g_xcvr[port].debounce = true;
}

inline void
xcvr_reset_debounce (int port) {
    g_xcvr[port].debounce = false;
}

inline void
set_cable_type (int port, cable_type_t cable_type)
{
    g_xcvr[port].cable_type = cable_type;
}

inline cable_type_t
cable_type (int port)
{
    return g_xcvr[port].cable_type;
}

inline port_an_args_t*
xcvr_get_an_args (int port)
{
    return &g_xcvr[port].port_an_args;
}

inline void
xcvr_set_an_args (int port, uint32_t user_cap,
                  bool fec_ability, uint32_t fec_request)
{
    port_an_args_t *port_an_args = xcvr_get_an_args(port);
    port_an_args->user_cap    = user_cap;
    port_an_args->fec_ability = fec_ability;
    port_an_args->fec_request = fec_request;
}

inline void
xcvr_set_type (int port, xcvr_type_t type) {
    g_xcvr[port].type = type;
}

inline bool
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
bool xcvr_valid(int port);
sdk_ret_t xcvr_get(int port, port_args_t *port_arg);
sdk_ret_t xcvr_enable(int port, bool enable, uint8_t mask);

static inline bool
xcvr_valid_check_enabled (void)
{
    return xcvr_valid_enable;
}

static inline void
xcvr_set_valid_check (bool enable)
{
    xcvr_valid_enable = enable;
}

inline port_speed_t
cable_speed (int port)
{
    return g_xcvr[port].cable_speed;
}

inline void
xcvr_set_cable_speed (int port, port_speed_t speed)
{
    g_xcvr[port].cable_speed = speed;
}

} // namespace platform
} // namespace sdk
#endif
