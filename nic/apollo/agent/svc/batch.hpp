// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __AGENT_SVC_BATCH_HPP__
#define __AGENT_SVC_BATCH_HPP__

#include "grpc++/grpc++.h"
#include "gen/proto/types/types.pb.h"
#include "gen/proto/meta/meta.pb.h"
#include "gen/proto/batch.grpc.pb.h"

using grpc::Status;
using grpc::ServerContext;

using types::Empty;
using tpc::BatchSvc;
using tpc::BatchSpec;
using tpc::BatchStatus;

class BatchSvcImpl final : public BatchSvc::Service {
public:
    Status BatchStart(ServerContext *context, const tpc::BatchSpec *spec,
                      tpc::BatchStatus *status) override;
    Status BatchCommit(ServerContext *context, const Empty *spec,
                       Empty *status) override;
    Status BatchAbort(ServerContext *context, const Empty *spec,
                      Empty *status) override;
};

#endif    // __AGENT_SVC_BATCH_HPP__
