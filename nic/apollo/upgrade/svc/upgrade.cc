//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/globals.hpp"
#include "nic/sdk/include/sdk/platform.hpp"
#include "nic/sdk/lib/ipc/ipc.hpp"
#include "nic/sdk/upgrade/core/logger.hpp"
#include "nic/apollo/upgrade/svc/upgrade.hpp"

// callbacks from upgrade processing thread
static void
upg_sync_response_hdlr (sdk::ipc::ipc_msg_ptr msg, const void *ret)
{
    UPG_TRACE_INFO("Received response msg from upgrademgr, status %u",
                   *(upg_status_t *)msg->data());
    *(upg_status_t *)ret = *(upg_status_t *)msg->data();
}

static inline UpgradeStatus
proto_status (upg_status_t status)
{
    switch (status) {
    case UPG_STATUS_OK: return UpgradeStatus::UPGRADE_STATUS_OK;
    case UPG_STATUS_FAIL: return UpgradeStatus::UPGRADE_STATUS_FAIL;
    case UPG_STATUS_CRITICAL: return UpgradeStatus::UPGRADE_STATUS_FAIL;
    default: return UpgradeStatus::UPGRADE_STATUS_FAIL;
    }
}

Status
UpgSvcImpl::UpgRequest(ServerContext *context,
                       const pds::UpgradeRequest *proto_req,
                       pds::UpgradeResponse *proto_rsp) {
    upg_ev_req_msg_t msg;
    upg_status_t status;

    UPG_TRACE_INFO("Received new upgrade request");
    if (!proto_req) {
        UPG_TRACE_ERR("Invalid request");
        proto_rsp->set_status(UpgradeStatus::UPGRADE_STATUS_INVALID_ARG);
        return Status::CANCELLED;
    }
    auto request = proto_req->request();
    // get request type
    if (request.requesttype() == pds::UpgradeRequestType::UPGRADE_REQUEST_START) {
        msg.id = UPG_REQ_MSG_ID_START;
    } else {
        UPG_TRACE_ERR("Invalid request type");
        goto err_exit;
    }
    // get upgrade mode
    if (request.mode() == pds::UpgradeMode::UPGRADE_MODE_GRACEFUL) {
        msg.upg_mode = sdk::platform::upg_mode_t::UPGRADE_MODE_GRACEFUL;
    } else if (request.mode() == pds::UpgradeMode::UPGRADE_MODE_HITLESS) {
        msg.upg_mode = sdk::platform::upg_mode_t::UPGRADE_MODE_HITLESS;
    } else {
        UPG_TRACE_ERR("Invalid upgrade mode");
        goto err_exit;
    }
    msg.fw_pkgname = request.packagename();
    if (msg.fw_pkgname.empty()) {
        UPG_TRACE_ERR("Invalid upgrade package name");
        goto err_exit;
    }
    sdk::ipc::request(SDK_IPC_ID_UPGMGR, UPG_REQ_MSG_ID_START, &msg,
                      sizeof(msg), upg_sync_response_hdlr, &status);
    proto_rsp->set_status(proto_status(status));
    return Status::OK;
err_exit:
    proto_rsp->set_status(UpgradeStatus::UPGRADE_STATUS_INVALID_ARG);
    return Status::CANCELLED;
}
