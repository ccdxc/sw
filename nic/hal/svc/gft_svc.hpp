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

using kh::GftExactMatchProfileKeyHandle;
using kh::GftHeaderTranspositionProfileKeyHandle;
using kh::GftExactMatchFlowEntryKeyHandle;

using gft::GftExactMatchProfileSpec;
using gft::GftExactMatchProfileRequestMsg;
using gft::GftExactMatchProfileStatus;
using gft::GftExactMatchProfileResponse;
using gft::GftExactMatchProfileResponseMsg;

using gft::GftHeaderTranspositionProfileSpec;
using gft::GftHeaderTranspositionProfileRequestMsg;
using gft::GftHeaderTranspositionProfileStatus;
using gft::GftHeaderTranspositionProfileResponse;
using gft::GftHeaderTranspositionProfileResponseMsg;

using gft::GftExactMatchFlowEntrySpec;
using gft::GftExactMatchFlowEntryRequestMsg;
using gft::GftExactMatchFlowEntryStatus;
using gft::GftExactMatchFlowEntryResponse;
using gft::GftExactMatchFlowEntryResponseMsg;

class GftServiceImpl final : public Gft::Service {
public:
    Status GftExactMatchProfileCreate(ServerContext *context,
                           const GftExactMatchProfileRequestMsg *req,
                           GftExactMatchProfileResponseMsg *rsp) override;

    Status GftHeaderTranspositionProfileCreate(ServerContext *context,
                                    const GftHeaderTranspositionProfileRequestMsg *req,
                                    GftHeaderTranspositionProfileResponseMsg *rsp) override;

    Status GftExactMatchFlowEntryCreate(ServerContext *context,
                                 const GftExactMatchFlowEntryRequestMsg *req,
                                 GftExactMatchFlowEntryResponseMsg *rsp) override;
};

#endif    // __GFT_SVC_HPP__

