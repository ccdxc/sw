//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include "nic/apollo/upgrade/svc/upgrade.hpp"
#include "nic/apollo/upgrade/core/logger.hpp"

Status
UpgSvcImpl::UpgradeRequest(ServerContext *context,
                           const pds::UpgRequest *proto_req,
                           pds::UpgResponse *proto_rsp) {
    g_upg_log->info("Received upgrade request");
    if (!proto_req) {
        proto_rsp->set_status(UpgStatus::UPG_STATUS_INVALID_ARG);
        return Status::CANCELLED;
    }
    auto request = proto_req->request();
    // start the upgrade sequencer, update the result
    // TODO 
    proto_rsp->set_status(UpgStatus::UPG_STATUS_OK);
    return Status::OK;
}
