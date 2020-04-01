//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// server side methods for Learn service
///
//----------------------------------------------------------------------------

#ifndef __AGENT_SVC_LEARN_HPP__
#define __AGENT_SVC_LEARN_HPP__

#include "grpc++/grpc++.h"
#include "gen/proto/types.pb.h"
#include "gen/proto/meta/meta.pb.h"
#include "gen/proto/learn.grpc.pb.h"

using grpc::Status;
using grpc::ServerContext;

using pds::LearnSvc;

class LearnSvcImpl final : public LearnSvc::Service {
public:
    Status LearnMACGet(ServerContext *context,
                       const pds::LearnMACRequest *request,
                       pds::LearnMACGetResponse *proto_rsp) override;
    Status LearnIPGet(ServerContext* context,
                      const pds::LearnIPGetRequest* request,
                      pds::LearnIPGetResponse* proto_rsp) override;
    Status LearnStatsGet(ServerContext* context,
                         const types::Empty* request,
                         pds::LearnStatsGetResponse* proto_rsp) override;
    Status LearnMACClear(ServerContext *context,
                         const pds::LearnMACRequest *request,
                         pds::LearnClearResponse *proto_rsp) override;
    Status LearnIPClear(ServerContext* context,
                        const pds::LearnIPRequest* request,
                        pds::LearnClearResponse* proto_rsp) override;
    Status LearnStatsClear(ServerContext* context,
                           const types::Empty* request,
                           types::Empty* proto_rsp) override;
};

#endif    // __AGENT_SVC_LEARN_HPP__
