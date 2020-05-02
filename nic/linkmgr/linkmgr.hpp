// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

//------------------------------------------------------------------------------
// APIs to use linkmgr as lib
//------------------------------------------------------------------------------

#ifndef __LINKMGR_HPP__
#define __LINKMGR_HPP__

#include "nic/sdk/linkmgr/linkmgr.hpp"
#include "nic/include/base.hpp"

using sdk::linkmgr::port_args_t;

namespace linkmgr {

using sdk::types::xcvr_state_t;
using sdk::types::xcvr_pid_t;

typedef void (*port_get_cb_t)(port_args_t *args,
                              void *ctxt,
                              hal_ret_t hal_ret_status);

hal_ret_t linkmgr_init(sdk::linkmgr::linkmgr_cfg_t *sdk_cfg);

// SVC CRUD APIs
hal_ret_t port_create(port_args_t *port_args,
                      hal_handle_t *hal_handle);
hal_ret_t port_update(port_args_t *port_args);
hal_ret_t port_delete(port_args_t *port_args);
hal_ret_t port_get(port_args_t *port_args);
hal_ret_t port_get_all(port_get_cb_t port_get_cb, void *ctxt);
hal_ret_t port_disable(uint32_t port_num = 0);
void port_event_notify(port_event_info_t *port_event_info);
void xcvr_event_notify(xcvr_event_info_t *xcvr_event_info);
hal_ret_t port_metrics_update(void);
hal_ret_t mac_stats_update(void);

}    // namespace linkmgr

#endif    // __LINKMGR_HPP__
