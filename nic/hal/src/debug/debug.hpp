//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __DEBUG_HPP__
#define __DEBUG_HPP__

#include "nic/include/base.h"
#include "nic/gen/proto/hal/debug.pb.h"

using debug::MemTrackGetRequest;
using debug::MemTrackGetResponseMsg;
using debug::SlabGetRequest;
using debug::SlabGetResponseMsg;
using debug::TraceSpec;
using debug::TraceResponse;
using debug::TraceResponseMsg;

namespace hal {

hal_ret_t mtrack_get(debug::MemTrackGetRequest& req,
                     debug::MemTrackGetResponseMsg *rsp);

hal_ret_t slab_get_from_req(debug::SlabGetRequest& req,
                            debug::SlabGetResponseMsg *rsp);

hal_ret_t mpu_trace_enable(debug::MpuTraceRequest& req,
                           debug::MpuTraceResponseMsg *rsp);

hal_ret_t trace_update(debug::TraceSpec& spec, debug::TraceResponse *rsp);
hal_ret_t trace_get(debug::TraceResponseMsg *rsp);

}    // namespace hal

#endif    // __DEBUG_HPP__

