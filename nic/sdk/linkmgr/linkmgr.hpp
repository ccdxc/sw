// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __SDK_LINKMGR_HPP__
#define __SDK_LINKMGR_HPP__

#include "include/sdk/base.hpp"
#include "include/sdk/types.hpp"
#include "lib/catalog/catalog.hpp"
#include "platform/utils/mpartition.hpp"
#include "include/sdk/timestamp.hpp"
#include "lib/shmmgr/shmmgr.hpp"
#include "lib/utils/in_mem_fsm_logger.hpp"

namespace sdk {
namespace linkmgr {

using sdk::types::xcvr_state_t;
using sdk::types::xcvr_pid_t;
using sdk::utils::in_mem_fsm_logger;

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
    port_admin_state_t  admin_state; // default port admin state
    mpartition          *mempartition; // memory partition context from HAL
    sdk::lib::shmmgr    *shmmgr; // shared memory manager instance from HAL
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
    port_fec_type_t       fec_type;                   // oper FEC type
    port_fec_type_t       user_fec_type;              // configured FEC type
    port_fec_type_t       derived_fec_type;           // fec type based on AN or user config
    bool                  toggle_fec_mode;            // toggle FEC modes during fixed speed bringup
    port_pause_type_t     pause;                      // mac pause
    bool                  tx_pause_enable;            // mac tx pause enable
    bool                  rx_pause_enable;            // mac rx pause enable
    cable_type_t          cable_type;                 // CU/Fiber cable type
    bool                  auto_neg_cfg;               // user AutoNeg config
    bool                  auto_neg_enable;            // Enable AutoNeg
    bool                  toggle_neg_mode;            // toggle between auto_neg and force modes
    bool                  mac_stats_reset;            // mac stats reset
    uint32_t              mac_id;                     // mac id associated with the port
    uint32_t              mac_ch;                     // mac channel associated with port
    uint32_t              num_lanes_cfg;              // user configured number of lanes for the port
    uint32_t              num_lanes;                  // number of lanes for the port
    uint32_t              debounce_time;              // Debounce time in ms
    uint32_t              mtu;                        // mtu
    uint64_t              *stats_data;                // MAC stats info
    xcvr_event_info_t     xcvr_event_info;            // transceiver info
    port_an_args_t        *port_an_args;              // an cfg based on xcvr
    port_link_sm_t        link_sm;                    // internal port state machine
    port_loopback_mode_t  loopback_mode;              // port loopback mode - MAC/PHY
    uint32_t              num_link_down;              // number of link down events
    char                  last_down_timestamp[TIME_STR_SIZE]; // time at which link last went down
    timespec_t            bringup_duration;           // time taken for last link bringup
    uint32_t              breakout_modes;             // supported breakout modes
    uint8_t               mac_addr[6];                // MAC addr of port
    uint32_t              sbus_addr[MAX_PORT_LANES];  // set the sbus addr for each lane
    in_mem_fsm_logger     *sm_logger;                 // state machine transition Logger
} __PACK__ port_args_t;

sdk_ret_t linkmgr_init(linkmgr_cfg_t *cfg);
void *port_create(port_args_t *port_args);
sdk_ret_t port_update(void *port_p, port_args_t *port_args);
sdk_ret_t port_delete(void *port_p);
sdk_ret_t port_get(void *port_p, port_args_t *port_args);
void linkmgr_set_link_poll_enable(bool enable);
sdk_ret_t port_args_set_by_xcvr_state(port_args_t *port_args);
sdk_ret_t port_update_xcvr_event(
            void *port_p, xcvr_event_info_t *xcvr_event_info);
void port_set_leds(uint32_t port_num, port_event_t event);
sdk_ret_t start_aacs_server(int port);
void stop_aacs_server(void);
port_admin_state_t port_default_admin_state(void);
void linkmgr_threads_stop(void);
void linkmgr_threads_wait(void);
void port_store_user_config(port_args_t *port_args);
// TODO : below better to use linkmgr events
sdk_ret_t port_shutdown (void *port_p);
sdk_ret_t port_pb_shutdown (void *port_p);

// \@brief     get the stats base address
// \@param[in] ifindex ifindex of the port
// \@return    on success, returns stats base address for port,
//             else INVALID_MEM_ADDRESS
mem_addr_t port_stats_addr(uint32_t ifindex);

static inline void
port_args_init (port_args_t *args)
{
    memset(args, 0, sizeof(port_args_t));
}

}    // namespace linkmgr
}    // namespace sdk

using sdk::linkmgr::linkmgr_cfg_t;
using sdk::linkmgr::port_args_t;

#endif    // __SDK_LINKMGR_HPP__
