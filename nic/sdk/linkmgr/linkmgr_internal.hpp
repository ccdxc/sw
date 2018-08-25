// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __SDK_LINKMGR_INTERNAL_HPP__
#define __SDK_LINKMGR_INTERNAL_HPP__

#include <atomic>
#include "include/sdk/thread.hpp"
#include "port.hpp"

namespace sdk {
namespace linkmgr {

sdk_ret_t
linkmgr_notify (uint8_t operation, linkmgr_entry_data_t *data);

sdk_ret_t
port_link_poll_timer_add(port *port);

sdk_ret_t
port_link_poll_timer_delete(port *port);

bool
is_linkmgr_ctrl_thread();

sdk::lib::thread *current_thread (void);

uint32_t glbl_mode_mgmt (mac_mode_t mac_mode);
uint32_t ch_mode_mgmt (mac_mode_t mac_mode, uint32_t ch);
uint32_t glbl_mode (mac_mode_t mac_mode);
uint32_t ch_mode (mac_mode_t mac_mode, uint32_t ch);

serdes_info_t*
serdes_info_get(uint32_t sbus_addr,
                uint32_t port_speed,
                uint32_t cable_type);

}    // namespace linkmgr
}    // namespace sdk

#endif    // __SDK_LINKMGR_INTERNAL_HPP__
