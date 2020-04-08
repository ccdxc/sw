//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines disruptive upgrade implementation for nicmgr
///
//----------------------------------------------------------------------------

#ifndef __NICMGR_LIB_UPGRADE_HPP__
#define __NICMGR_LIB_UPGRADE_HPP__

namespace nicmgr {
namespace upg {

typedef int (*upg_device_reset_status_cb_t)(void);
typedef void (*upg_event_response_cb_t)(sdk_ret_t status, void *cookie);

sdk_ret_t upg_compat_check_handler(void);
sdk_ret_t upg_link_down_handler(void *cookie);
sdk_ret_t upg_failed_handler(void *cookie);
sdk_ret_t upg_host_down_handler(void *cookie);
void upg_ev_init(upg_device_reset_status_cb_t reset_status_cb,
                 upg_event_response_cb_t event_response_cb);

}   // namespace upg
}   // namespace nicmgr

#endif   // __NICMGR_LIB_UPGRADE_HPP__
