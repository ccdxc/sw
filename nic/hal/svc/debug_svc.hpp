// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __DEBUG_SVC_HPP__
#define __DEBUG_SVC_HPP__

#include "nic/include/base.h"
#include "grpc++/grpc++.h"
#include "nic/gen/proto/hal/types.pb.h"
#include "nic/gen/proto/hal/debug.grpc.pb.h"

using grpc::ServerContext;
using grpc::Status;

using types::Empty;
using debug::Debug;
using debug::DebugSpec;
using debug::DebugRequestMsg;
using debug::DebugResponse;
using debug::DebugResponseMsg;
using debug::MemTrackGetRequest;
using debug::MemTrackGetRequestMsg;
using debug::MemTrackGetResponse;
using debug::MemTrackGetResponseMsg;
using debug::SlabGetRequestMsg;
using debug::SlabGetResponseMsg;
using debug::MpuTraceRequestMsg;
using debug::MpuTraceRequest;
using debug::MpuTraceResponseMsg;
using debug::MpuTraceResponse;
using debug::TraceSpec;
using debug::TraceRequestMsg;
using debug::TraceResponse;
using debug::TraceResponseMsg;

class DebugServiceImpl final : public Debug::Service {
public:
    Status DebugInvoke(ServerContext *context,
                       const DebugRequestMsg *req,
                       DebugResponseMsg *rsp) override;
    Status MemTrackGet(ServerContext *context,
                       const MemTrackGetRequestMsg *req,
                       MemTrackGetResponseMsg *rsp) override;
    Status SlabGet(ServerContext *context,
                   const SlabGetRequestMsg *req,
                   SlabGetResponseMsg *rsp) override;
    Status MpuTraceOpn(ServerContext *context,
                       const MpuTraceRequestMsg *req,
                       MpuTraceResponseMsg *rsp) override;
    Status TraceUpdate(ServerContext *context,
                       const TraceRequestMsg *req,
                       TraceResponseMsg *rsp) override;
    Status TraceGet(ServerContext *context,
                    const Empty *req,
                    TraceResponseMsg *rsp) override;
};

#endif  // __DEBUG_SVC_HPP__

