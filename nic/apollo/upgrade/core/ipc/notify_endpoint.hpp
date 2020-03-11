// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// common definitions for core upgrade data structure for external uses
///
//----------------------------------------------------------------------------

#ifndef __PDS_UPG_CORE_NOTIFY_HPP__
#define __PDS_UPG_CORE_NOTIFY_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/ipc/ipc.hpp"
#include "nic/apollo/upgrade/core/logger.hpp"
#include "nic/apollo/upgrade/include/upgrade.hpp"

namespace upg {
void send_notification(upg_event_msg_t& msg);
void broadcast_notification(upg_event_msg_t& msg);
}
#endif    //__PDS_UPG_CORE_NOTIFY_HPP___
