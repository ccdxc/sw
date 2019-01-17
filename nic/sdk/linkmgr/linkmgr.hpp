// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __SDK_LINKMGR_HPP__
#define __SDK_LINKMGR_HPP__

#include "include/sdk/base.hpp"
#include "include/sdk/types.hpp"
#include "nic/sdk/lib/catalog/catalog.hpp"

namespace sdk {
namespace linkmgr {

using sdk::types::xcvr_state_t;
using sdk::types::xcvr_pid_t;

typedef void (*port_event_notify_t)(uint32_t port_num,
                                    port_event_t port_event,
                                    port_speed_t port_speed);

typedef void (*port_log_fn_t)(sdk_trace_level_e trace_level,
                              const char *msg);

typedef void (*xcvr_event_notify_t)(xcvr_event_info_t *xcvr_event_info);

typedef enum linkmgr_thread_id_e {
    LINKMGR_THREAD_ID_MIN         = 0,
    LINKMGR_THREAD_ID_CFG         = LINKMGR_THREAD_ID_MIN,
    LINKMGR_THREAD_ID_PERIODIC    = 1,
    LINKMGR_THREAD_ID_AACS_SERVER = 2,
    LINKMGR_THREAD_ID_CTRL        = 7,  // match with HAL_THREAD_ID_LINKMGR_CTRL
    LINKMGR_THREAD_ID_MAX         = 8,
} linkmgr_thread_id_t;

typedef struct linkmgr_cfg_s {
    platform_type_t     platform_type;
    sdk::lib::catalog   *catalog;
    void                *server_builder;
    const char          *cfg_path;
    port_event_notify_t port_event_cb;
    xcvr_event_notify_t xcvr_event_cb;
    port_log_fn_t       port_log_fn;
    bool                process_mode;
} __PACK__ linkmgr_cfg_t;
extern linkmgr_cfg_t g_linkmgr_cfg;

typedef struct port_args_s {
    void                  *port_p;                    // SDK returned port context
    uint32_t              port_num;                   // uplink port number
    port_type_t           port_type;                  // port type
    port_speed_t          port_speed;                 // port speed
    port_admin_state_t    admin_state;                // admin state of the port
    port_admin_state_t    user_admin_state;           // user configured admin state of the port
    port_oper_status_t    oper_status;                // oper status of the port
    port_fec_type_t       fec_type;                   // FEC type
    port_pause_type_t     pause;                      // mac pause
    cable_type_t          cable_type;                 // CU/Fiber cable type
    bool                  auto_neg_enable;            // Enable AutoNeg
    bool                  mac_stats_reset;            // mac stats reset
    uint32_t              mac_id;                     // mac id associated with the port
    uint32_t              mac_ch;                     // mac channel associated with port
    uint32_t              num_lanes;                  // number of lanes for the port
    uint32_t              debounce_time;              // Debounce time in ms
    uint32_t              mtu;                        // mtu
    uint64_t              *stats_data;                // MAC stats info
    int                   xcvr_port_num;              // Xcvr port num
    xcvr_state_t          xcvr_state;                 // Xcvr state
    xcvr_pid_t            xcvr_pid;                   // Xcvr pid
    port_an_args_t        *port_an_args;              // an cfg based on xcvr
    port_link_sm_t        link_sm;                    // internal port state machine
    port_loopback_mode_t  loopback_mode;              // port loopback mode - MAC/PHY
    uint32_t              num_link_down;              // number of link down events
    uint32_t              breakout_modes;             // supported breakout modes
    uint8_t               mac_addr[6];                // MAC addr of port
    uint32_t              sbus_addr[MAX_PORT_LANES];  // set the sbus addr for each lane
} __PACK__ port_args_t;

sdk_ret_t linkmgr_init(linkmgr_cfg_t *cfg);
void *port_create(port_args_t *port_args);
sdk_ret_t port_update(void *port, port_args_t *port_args);
sdk_ret_t port_delete(void *port);
sdk_ret_t port_get(void *port, port_args_t *port_args);
void linkmgr_start (void);
void linkmgr_set_link_poll_enable(bool enable);
sdk_ret_t port_args_set_by_xcvr_state(port_args_t *port_args);

static inline void
port_args_init (port_args_t *args)
{
    args->port_p = NULL;
}

}    // namespace linkmgr
}    // namespace sdk

using sdk::linkmgr::linkmgr_cfg_t;
using sdk::linkmgr::port_args_t;

#endif    // __SDK_LINKMGR_HPP__
