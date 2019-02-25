// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __SVC_SUBNET_HPP__
#define __SVC_SUBNET_HPP__

#include "grpc++/grpc++.h"
#include "gen/proto/types/types.pb.h"
#include "gen/proto/meta/meta.pb.h"
#include "gen/proto/subnet.grpc.pb.h"

using grpc::Status;
using grpc::ServerContext;

using tpc::SubnetSvc;
using tpc::SubnetSpec;
using tpc::SubnetStatus;

class SubnetSvcImpl final : public SubnetSvc::Service {
public:
    Status SubnetCreate(ServerContext *context,
                        const tpc::SubnetSpec *spec,
                        tpc::SubnetStatus *status) override;
};

#endif    // __SVC_SUBNET_HPP__
