// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// common definitions for core upgrade data structure for external uses
///
//----------------------------------------------------------------------------

#ifndef __PDS_UPG_CORE_IPC_HPP__
#define __PDS_UPG_CORE_IPC_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/ipc/ipc.hpp"
#include "nic/apollo/upgrade/core/logger.hpp"
#include "nic/apollo/upgrade/include/upgrade.hpp"

namespace upg {
namespace ipc {

typedef enum ipc_svc_dom_id_s {
    IPC_SVC_DOM_ID_NONE = 0, ///< invalid
    IPC_SVC_DOM_ID_A    = 1, ///< first and default domain
    IPC_SVC_DOM_ID_B    = 2, ///< second domain
} ipc_svc_dom_id_t;

/// ipc callback which expects service name and id filled by the endpoint
typedef void (*upg_async_response_cb_t)(sdk::ipc::ipc_msg_ptr msg,
                                        const void *cookie, const void *ctxt);
/// broadcast to a domain
void broadcast(ipc_svc_dom_id_t dom_id, upg_stage_t stage, void *cookie);
/// unicast request to a service in a domain
void request(ipc_svc_dom_id_t dom_id, upg_stage_t stage,
             uint32_t svc_id, void *cookie);
/// initialize the ipc service
void init(upg_async_response_cb_t rsp_cb);

}   // namespace ipc
}   // namespace upg

#endif    //__PDS_UPG_CORE_NOTIFY_HPP___
