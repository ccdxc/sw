// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __SVC_VNIC_HPP__
#define __SVC_VNIC_HPP__

#include "grpc++/grpc++.h"
#include "gen/proto/types/types.pb.h"
#include "gen/proto/meta/meta.pb.h"
#include "gen/proto/vnic.grpc.pb.h"

using grpc::Status;
using grpc::ServerContext;

using tpc::VnicSvc;
using tpc::VnicSpec;
using tpc::VnicStatus;

class VnicSvcImpl final : public VnicSvc::Service {
public:
    Status VnicCreate(ServerContext *context,
                      const tpc::VnicSpec *spec,
                      tpc::VnicStatus *status) override;
};

#endif    // __SVC_VNIC_HPP__
