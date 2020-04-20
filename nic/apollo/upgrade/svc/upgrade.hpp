//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __UPGRADE_SVC_UPGRADE_HPP__
#define __UPGRADE_SVC_UPGRADE_HPP__

#include "grpc++/grpc++.h"
#include "gen/proto/upgrade.grpc.pb.h"
#include "nic/sdk/upgrade/include/upgrade.hpp"

using grpc::Status;
using grpc::ServerContext;

using pds::UpgSvc;
using pds::UpgradeRequest;
using pds::UpgradeSpec;
using pds::UpgradeResponse;
using pds::UpgradeStatus;

/// \brief upgrade event request
/// ipc from grpc main thread to request processing thread
typedef enum upg_ev_req_msg_id_s {
    UPG_REQ_MSG_ID_NONE  = 0,
    UPG_REQ_MSG_ID_START = 1,
} upg_ev_req_msg_id_t;

/// \brief upgrade event params
/// passed from grpc main thread to request processing thread
typedef struct upg_ev_req_msg_s {
    upg_ev_req_msg_id_t id;              ///< message id
    sdk::platform::upg_mode_t upg_mode;  ///< upgrade mode
    std::string fw_pkgname;              ///< firmware package name with path
    // TODO fill other details
    upg_status_t rsp_status;             ///< upgrade status
} upg_ev_req_msg_t;

class UpgSvcImpl final : public UpgSvc::Service {
public:
    Status UpgRequest(ServerContext *context, const pds::UpgradeRequest *req,
                      pds::UpgradeResponse *rsp) override;
};

#endif    // __UPGRADE_SVC_UPGRADE_HPP__
