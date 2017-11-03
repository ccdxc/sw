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

class DebugServiceImpl final : public Debug::Service {
public:
    Status DebugInvoke(ServerContext *context,
                       const DebugRequestMsg *req,
                       DebugResponseMsg *rsp) override;
};

#endif  // __DEBUG_SVC_HPP__
