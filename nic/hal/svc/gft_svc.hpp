// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __GFT_SVC_HPP__
#define __GFT_SVC_HPP__

#include "nic/include/base.h"
#include "grpc++/grpc++.h"
#include "nic/gen/proto/hal/types.pb.h"
#include "nic/gen/proto/hal/gft.grpc.pb.h"

using grpc::ServerContext;
using grpc::Status;

using gft::Gft;
using gft::GftExactMatchProfileSpec;
using gft::GftExactMatchProfileStatus;
using gft::GftExactMatchProfileResponse;
using kh::GftExactMatchProfileKeyHandle;
using gft::GftExactMatchProfileRequestMsg;
using gft::GftExactMatchProfileResponseMsg;

class GftServiceImpl final : public Gft::Service {
public:
    Status GftExactMatchProfileCreate(ServerContext *context,
                           const GftExactMatchProfileRequestMsg *req,
                           GftExactMatchProfileResponseMsg *rsp) override;

};

#endif    // __GFT_SVC_HPP__

