// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __AGENT_SVC_BATCH_HPP__
#define __AGENT_SVC_BATCH_HPP__

#include "grpc++/grpc++.h"
#include "gen/proto/types.pb.h"
#include "gen/proto/meta/meta.pb.h"
#include "gen/proto/batch.grpc.pb.h"

using grpc::Status;
using grpc::ServerContext;

using types::Empty;
using types::BatchCtxt;
using pds::BatchSvc;
using pds::BatchSpec;
using pds::BatchStatus;

class BatchSvcImpl final : public BatchSvc::Service {
public:
    Status BatchStart(ServerContext *context, const pds::BatchSpec *spec,
                      pds::BatchStatus *status) override;
    Status BatchCommit(ServerContext *context, const types::BatchCtxt *ctxt,
                      pds::BatchStatus *status) override;
    Status BatchAbort(ServerContext *context, const types::BatchCtxt *ctxt,
                      pds::BatchStatus *status) override;
};

#endif    // __AGENT_SVC_BATCH_HPP__
