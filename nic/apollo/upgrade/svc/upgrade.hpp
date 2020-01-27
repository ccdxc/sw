//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __UPGRADE_SVC_UPGRADE_HPP__
#define __UPGRADE_SVC_UPGRADE_HPP__

#include "grpc++/grpc++.h"
#include "gen/proto/upgrade.grpc.pb.h"

using grpc::Status;
using grpc::ServerContext;

using pds::UpgSvc;
using pds::UpgRequest;
using pds::UpgSpec;
using pds::UpgResponse;
using pds::UpgStatus;

class UpgSvcImpl final : public UpgSvc::Service {
public:
    Status UpgradeRequest(ServerContext *context, const pds::UpgRequest *req,
                          pds::UpgResponse *rsp) override;
};

#endif    // __UPGRADE_SVC_UPGRADE_HPP__
