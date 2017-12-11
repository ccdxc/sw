#ifndef __INTERNAL_SVC_HPP__
#define __INTERNAL_SVC_HPP__

#include "nic/include/base.h"
#include <grpc++/grpc++.h>
#include "nic/gen/proto/hal/types.pb.h"
#include "nic/gen/proto/hal/internal.grpc.pb.h"

using grpc::ServerContext;
using grpc::Status;

using internal::Internal;
using internal::GetProgramAddressRequestMsg;
using internal::ProgramAddressResponseMsg;
using internal::AllocHbmAddressRequestMsg;
using internal::AllocHbmAddressResponseMsg;
using internal::ConfigureLifBdfRequestMsg;
using internal::ConfigureLifBdfResponseMsg;

class InternalServiceImpl final : public Internal::Service {
public:

    Status GetProgramAddress(ServerContext *context,
                             const GetProgramAddressRequestMsg *reqs,
                             ProgramAddressResponseMsg *resps) override;
                
    Status AllocHbmAddress(ServerContext *context,
                           const AllocHbmAddressRequestMsg *reqs,
                           AllocHbmAddressResponseMsg *resps) override;

    Status ConfigureLifBdf(ServerContext *context,
                           const ConfigureLifBdfRequestMsg *reqs,
                           ConfigureLifBdfResponseMsg *resps) override;
};

#endif    // __INTERNAL_SVC_HPP__

