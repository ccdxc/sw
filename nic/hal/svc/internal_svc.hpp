#ifndef __INTERNAL_SVC_HPP__
#define __INTERNAL_SVC_HPP__

#include "nic/include/base.h"
#include <grpc++/grpc++.h>
#include "nic/proto/types.pb.h"
#include "nic/proto/hal/internal.grpc.pb.h"

using grpc::ServerContext;
using grpc::Status;

using internal::Internal;
using internal::GetProgramAddressRequestMsg;
using internal::ProgramAddressResponseMsg;
using internal::AllocHbmAddressRequestMsg;
using internal::AllocHbmAddressResponseMsg;

class InternalServiceImpl final : public Internal::Service {
public:

    Status GetProgramAddress(ServerContext *context,
                             const GetProgramAddressRequestMsg *reqs,
                             ProgramAddressResponseMsg *resps) override;
                
    Status AllocHbmAddress(ServerContext *context,
                           const AllocHbmAddressRequestMsg *reqs,
                           AllocHbmAddressResponseMsg *resps) override;
};

#endif    // __INTERNAL_SVC_HPP__

