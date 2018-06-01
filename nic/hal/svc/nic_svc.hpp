// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __HAL_SVC_NIC_HPP__
#define __HAL_SVC_NIC_HPP__

#include "nic/include/base.h"
#include "nic/gen/hal/include/hal_api_stats.hpp"
#include "grpc++/grpc++.h"
#include "nic/gen/proto/hal/types.pb.h"
#include "nic/gen/proto/hal/nic.grpc.pb.h"

using grpc::ServerContext;
using grpc::Status;
using device::Nic;

class NicServiceImpl final : public Nic::Service {
public:
    Status DeviceCreate(ServerContext *context,
                        const device::DeviceRequestMsg *req,
                        device::DeviceResponseMsg *rsp) override;

    Status DeviceUpdate(ServerContext *context,
                        const device::DeviceRequestMsg *req,
                        device::DeviceResponseMsg *rsp) override;

    Status DeviceDelete(ServerContext *context,
                        const device::DeviceDeleteRequestMsg *req,
                        device::DeviceDeleteResponseMsg *rsp) override;

    Status DeviceGet(ServerContext *context,
                        const device::DeviceGetRequestMsg *req,
                        device::DeviceGetResponseMsg *rsp) override;
};


#endif   // __HAL_SVC_NIC_HPP__
