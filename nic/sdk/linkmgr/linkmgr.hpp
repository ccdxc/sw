// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __SDK_LINKMGR_HPP__
#define __SDK_LINKMGR_HPP__

#include "include/sdk/base.hpp"
#include "include/sdk/types.hpp"
#include "include/sdk/catalog.hpp"

namespace sdk {
namespace linkmgr {

typedef void (*port_event_notify_t)(uint32_t port_num,
                                    port_event_t port_event);

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
    bool                process_mode;
} __PACK__ linkmgr_cfg_t;
extern linkmgr_cfg_t g_linkmgr_cfg;

typedef struct port_args_s {
    void                  *port_p;                    // SDK returned port context
    uint32_t              port_num;                   // uplink port number
    port_type_t           port_type;                  // port type
    port_speed_t          port_speed;                 // port speed
    port_admin_state_t    admin_state;                // admin state of the port
    port_oper_status_t    oper_status;                // oper status of the port
    port_fec_type_t       fec_type;                   // FEC type
    bool                  auto_neg_enable;            // Enable AutoNeg
    bool                  mac_stats_reset;            // mac stats reset
    uint32_t              mac_id;                     // mac id associated with the port
    uint32_t              mac_ch;                     // mac channel associated with port
    uint32_t              num_lanes;                  // number of lanes for the port
    uint32_t              debounce_time;              // Debounce time in ms
    uint32_t              mtu;                        // mtu
    uint64_t              *stats_data;                // MAC stats info
    uint32_t              sbus_addr[MAX_PORT_LANES];  // set the sbus addr for each lane
} __PACK__ port_args_t;

sdk_ret_t linkmgr_init  (linkmgr_cfg_t *cfg);
void      *port_create  (port_args_t *port_args);
sdk_ret_t port_update   (void *port, port_args_t *port_args);
sdk_ret_t port_delete   (void *port);
sdk_ret_t port_get      (void *port, port_args_t *port_args);
void linkmgr_start (void);
void linkmgr_set_link_poll_enable (bool enable);

static inline void
port_args_init (port_args_t *args)
{
    args->port_p = NULL;
}

}    // namespace linkmgr
}    // namespace sdk

#endif    // __SDK_LINKMGR_HPP__
