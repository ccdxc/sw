// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __AGENT_SVC_VNIC_HPP__
#define __AGENT_SVC_VNIC_HPP__

#include "grpc++/grpc++.h"
#include "gen/proto/types.pb.h"
#include "gen/proto/meta/meta.pb.h"
#include "gen/proto/vnic.grpc.pb.h"

using grpc::Status;
using grpc::ServerContext;

using pds::VnicSvc;
using pds::VnicRequest;
using pds::VnicSpec;
using pds::VnicResponse;
using types::Empty;

class VnicSvcImpl final : public VnicSvc::Service {
public:
    Status VnicCreate(ServerContext *context, const pds::VnicRequest *req,
                      pds::VnicResponse *rsp) override;
    Status VnicUpdate(ServerContext *context, const pds::VnicRequest *req,
                      pds::VnicResponse *rsp) override;
    Status VnicDelete(ServerContext *context,
                      const pds::VnicDeleteRequest *proto_req,
                      pds::VnicDeleteResponse *proto_rsp) override;
    Status VnicGet(ServerContext *context,
                   const pds::VnicGetRequest *proto_req,
                   pds::VnicGetResponse *proto_rsp) override;
    Status VnicStatsReset(ServerContext *context, const types::Id *req,
                          Empty *rsp) override;
};

#endif    // __AGENT_SVC_VNIC_HPP__
