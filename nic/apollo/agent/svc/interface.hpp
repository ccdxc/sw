// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __AGENT_SVC_LIF_HPP__
#define __AGENT_SVC_LIF_HPP__

#include "grpc++/grpc++.h"
#include "gen/proto/types.pb.h"
#include "gen/proto/meta/meta.pb.h"
#include "gen/proto/interface.grpc.pb.h"
#include "gen/proto/types.pb.h"
#include "gen/proto/interface.pb.h"

using grpc::Status;
using grpc::ServerContext;

using pds::IfSvc;
using pds::LifGetRequest;
using pds::LifGetResponse;
using pds::Lif;
using types::LifType;
using pds::InterfaceRequest;
using pds::InterfaceResponse;
using pds::InterfaceDeleteRequest;
using pds::InterfaceDeleteResponse;
using pds::InterfaceGetRequest;
using pds::InterfaceGetResponse;
using types::Empty;

class IfSvcImpl final : public IfSvc::Service {
public:
    Status InterfaceCreate(ServerContext *context, const pds::InterfaceRequest *req,
                           pds::InterfaceResponse *rsp) override;
    Status InterfaceUpdate(ServerContext *context, const pds::InterfaceRequest *req,
                           pds::InterfaceResponse *rsp) override;
    Status InterfaceDelete(ServerContext *context, const pds::InterfaceDeleteRequest *req,
                           pds::InterfaceDeleteResponse *rsp) override;
    Status InterfaceGet(ServerContext *context, const pds::InterfaceGetRequest *req,
                        pds::InterfaceGetResponse *rsp) override;
    Status LifGet(ServerContext *context, const pds::LifGetRequest *req,
                  pds::LifGetResponse *rsp) override;
    Status LifStatsReset(ServerContext *context, const types::Id *req,
                         Empty *rsp) override;
};

#endif    // __AGENT_SVC_LIF_HPP__
