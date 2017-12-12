// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __LINKMGR_SVC_HPP__
#define __LINKMGR_SVC_HPP__

#include "grpc++/grpc++.h"

#include "nic/include/base.h"

#include "nic/gen/proto/hal/types.pb.h"
#include "nic/gen/proto/hal/port.grpc.pb.h"

using grpc::ServerContext;
using grpc::Status;

using port::Port;
using port::PortOperStatus;
using port::PortResponse;
using port::PortRequestMsg;
using port::PortResponseMsg;
using port::PortDeleteRequestMsg;
using port::PortDeleteResponseMsg;
using port::PortGetRequest;
using port::PortGetRequestMsg;
using port::PortGetResponse;
using port::PortGetResponseMsg;

class PortServiceImpl final : public Port::Service {
public:
    Status PortCreate(ServerContext *context,
                      const PortRequestMsg *req,
                      PortResponseMsg *rsp) override;

    Status PortUpdate(ServerContext *context,
                      const PortRequestMsg *req,
                      PortResponseMsg *rsp) override;

    Status PortDelete(ServerContext *context,
                      const PortDeleteRequestMsg *req,
                      PortDeleteResponseMsg *rsp) override;

    Status PortGet(ServerContext *context,
                   const PortGetRequestMsg *req,
                   PortGetResponseMsg *rsp) override;
};

#endif    // __LINKMGR_SVC_HPP__
