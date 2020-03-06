// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __LINKMGR_DEBUG_SVC_HPP__
#define __LINKMGR_DEBUG_SVC_HPP__

#include "grpc++/grpc++.h"

#include "nic/include/base.hpp"
#include "gen/proto/types.pb.h"
#include "gen/proto/debug.grpc.pb.h"

using grpc::ServerContext;
using grpc::Status;

using debug::Debug;
using debug::GenericOpnRequest;
using debug::GenericOpnResponse;
using debug::GenericOpnRequestMsg;
using debug::GenericOpnResponseMsg;

namespace linkmgr {

class DebugServiceImpl final : public Debug::Service {
public:
    Status GenericOpn(ServerContext *context,
                      const GenericOpnRequestMsg *req_msg,
                      GenericOpnResponseMsg *rsp_msg);
};

} // namespace linkmgr

#endif    // __LINKMGR_DEBUG_SVC_HPP__
