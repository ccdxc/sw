//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __NICMGR_NCSI_HPP__
#define __NICMGR_NCSI_HPP__

#include "nic/sdk/lib/ipc/ipc.hpp"
#include "nic/utils/eventmgr/eventmgr.hpp"
#include "nic/hal/core/event_ipc.hpp"
#include "nic/hal/plugins/cfg/ncsi/ncsi.hpp"
#include "nic/hal/plugins/cfg/ncsi/ncsi_ipc.hpp"

namespace nicmgr {

using hal::ncsi_ipc_msg_t;

void ncsi_ipc_handler_cb(sdk::ipc::ipc_msg_ptr msg, const void *ctxt);
void nicmgr_ncsi_ipc_init(void);

sdk_ret_t ncsi_ipc_vlan_filter(ncsi_ipc_msg_t *msg);
sdk_ret_t ncsi_ipc_mac_filter(ncsi_ipc_msg_t *msg);
sdk_ret_t ncsi_ipc_bcast_filter(ncsi_ipc_msg_t *msg);
sdk_ret_t ncsi_ipc_mcast_filter(ncsi_ipc_msg_t *msg);
sdk_ret_t ncsi_ipc_vlan_mode(ncsi_ipc_msg_t *msg);
sdk_ret_t ncsi_ipc_channel(ncsi_ipc_msg_t *msg);

} // namespace nicmgr
#endif    // __NICMGR_NCSI_HPP__
