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
using debug::RegisterRequestMsg;
using debug::RegisterResponseMsg;
using debug::MemoryRequestMsg;
using debug::MemoryRequest;
using debug::MemoryResponseMsg;
using debug::MemoryResponse;

class DebugServiceImpl final : public Debug::Service {
public:
    Status RegisterGet(ServerContext *context,
                       const RegisterRequestMsg *req,
                       RegisterResponseMsg *rsp);
    Status RegisterUpdate(ServerContext *context,
                          const RegisterRequestMsg *req,
                          RegisterResponseMsg *rsp);
    Status MemoryGet(ServerContext *context,
                     const MemoryRequestMsg *req,
                     MemoryResponseMsg *rsp);
    Status MemoryUpdate(ServerContext *context,
                        const MemoryRequestMsg *req,
                        MemoryResponseMsg *rsp);
    Status MemTrackGet(ServerContext *context,
                       const MemTrackGetRequestMsg *req,
                       MemTrackGetResponseMsg *rsp) override;
    Status SlabGet(ServerContext *context,
                   const SlabGetRequestMsg *req,
                   SlabGetResponseMsg *rsp) override;
    Status MpuTraceUpdate(ServerContext *context,
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

