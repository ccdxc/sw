// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __SDK_LINKMGR_HPP__
#define __SDK_LINKMGR_HPP__

#include "sdk/base.hpp"
#include "sdk/types.hpp"

namespace sdk {
namespace linkmgr {

typedef struct linkmgr_cfg_s {
    platform_type_t platform_type;
    bool            hw_mock;
} __PACK__ linkmgr_cfg_t;
extern linkmgr_cfg_t g_linkmgr_cfg;

typedef struct port_args_s {
    void                  *port_p;        // SDK returned port context
    port_type_t           port_type;      // port type
    port_speed_t          port_speed;     // port speed
    port_admin_state_t    admin_state;    // admin state of the port
    port_oper_status_t    oper_status;    // oper status of the port
    uint32_t              mac_id;         // mac id associated with the port
    uint32_t              mac_ch;         // mac channel associated with port
    uint32_t              num_lanes;      // number of lanes for the port
} __PACK__ port_args_t;

sdk_ret_t linkmgr_init(linkmgr_cfg_t *cfg);
void *port_create(port_args_t *port_args);
sdk_ret_t port_update(void *port, port_args_t *port_args);
sdk_ret_t port_delete(void *port);
sdk_ret_t port_get(void *port, port_args_t *port_args);

void
linkmgr_event_wait (void);

static inline void
port_args_init (port_args_t *args)
{
    args->port_p = NULL;
}

}    // namespace linkmgr
}    // namespace sdk

#endif    // __SDK_LINKMGR_HPP__
