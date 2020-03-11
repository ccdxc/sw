// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// common definitions for core upgrade data structure for external uses
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/ipc/ipc.hpp"
#include "lib/operd/operd.hpp"
#include "lib/operd/logger.hpp"
#include "lib/operd/decoder.h"
#include "nic/apollo/upgrade/core/logger.hpp"
#include "nic/apollo/upgrade/include/upgrade.hpp"
#include "nic/apollo/upgrade/core/ipc/notify_endpoint.hpp"

namespace upg {

sdk::operd::logger_ptr g_upg_log =
    sdk::operd::logger::create(UPGRADE_LOG_NAME);
void
send_notification (upg_event_msg_t& msg) {
    UPG_TRACE_INFO("Sending notification ..\n");
    UPG_TRACE_DEBUG("event stageid %s, thread %s, thread_id %u\n",
           upg_stage2str(msg.stage), msg.rsp_thread_name, msg.rsp_thread_id);

    sdk::ipc::request(msg.rsp_thread_id, PDS_IPC_MSG_ID_UPGRADE,
                      &msg, sizeof(msg), NULL);
}

void
broadcast_notification (upg_event_msg_t& msg) {
    UPG_TRACE_INFO("Broadcasting notification ..\n");
    UPG_TRACE_DEBUG("event stageid %s, thread %s, thread_id %u\n",
           upg_stage2str(msg.stage), msg.rsp_thread_name, msg.rsp_thread_id);

    sdk::ipc::broadcast(PDS_IPC_MSG_ID_UPGRADE, &msg, sizeof(msg));
}

}
