// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __DEBUG_SVC_HPP__
#define __DEBUG_SVC_HPP__

#include "nic/include/base.h"
#include "grpc++/grpc++.h"
#include "nic/gen/proto/hal/types.pb.h"
#include "nic/gen/proto/hal/debug.grpc.pb.h"

using grpc::ServerContext;
using grpc::Status;

using debug::Debug;
using debug::DebugSpec;
using debug::DebugRequestMsg;
using debug::DebugResponse;
using debug::DebugResponseMsg;
using debug::MemTrackGetRequest;
using debug::MemTrackGetRequestMsg;
using debug::MemTrackGetResponse;
using debug::MemTrackGetResponseMsg;

class DebugServiceImpl final : public Debug::Service {
public:
    Status DebugInvoke(ServerContext *context,
                       const DebugRequestMsg *req,
                       DebugResponseMsg *rsp) override;
    Status MemTrackGet(ServerContext *context,
                       const MemTrackGetRequestMsg *req,
                       MemTrackGetResponseMsg *rsp) override;
};

#endif  // __DEBUG_SVC_HPP__

