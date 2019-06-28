// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __AGENT_SVC_SESSION_HPP__
#define __AGENT_SVC_SESSION_HPP__

#include "grpc++/grpc++.h"
#include "gen/proto/types.pb.h"
#include "gen/proto/meta/meta.pb.h"
#include "gen/proto/session.grpc.pb.h"

using grpc::Status;
using grpc::ServerContext;
using grpc::ServerWriter;

using types::Empty;
using pds::SessionSvc;
using pds::SessionGetResponse;
using pds::SessionClearResponse;
using pds::SessionClearRequest;
using pds::FlowGetResponse;
using pds::FlowClearResponse;
using pds::FlowClearRequest;

class SessionSvcImpl final : public SessionSvc::Service {
public:
    Status FlowGet(ServerContext *context, const Empty *req,
                   grpc::ServerWriter<pds::FlowGetResponse> *writer) override;
    Status SessionGet(ServerContext *context, const Empty *req,
                      grpc::ServerWriter<pds::SessionGetResponse> *writer) override;
    Status FlowClear(ServerContext *context, const pds::FlowClearRequest *req,
                     pds::FlowClearResponse *rsp) override;
    Status SessionClear(ServerContext *context, const pds::SessionClearRequest *req,
                        pds::SessionClearResponse *rsp) override;
};

#endif    // __AGENT_SVC_SESSION_HPP__
