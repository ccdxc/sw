// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __SDK_LINKMGR_INTERNAL_HPP__
#define __SDK_LINKMGR_INTERNAL_HPP__

#include <atomic>
#include "sdk/thread.hpp"
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

}    // namespace linkmgr
}    // namespace sdk

#endif    // __SDK_LINKMGR_INTERNAL_HPP__
