#ifndef __INTERNAL_SVC_HPP__
#define __INTERNAL_SVC_HPP__

#include <base.h>
#include <grpc++/grpc++.h>
#include <types.pb.h>
#include <internal.grpc.pb.h>

using grpc::ServerContext;
using grpc::Status;

using internal::Internal;
using internal::GetProgramAddressRequestMsg;
using internal::ProgramAddressResponseMsg;

class InternalServiceImpl final : public Internal::Service {
public:

    Status GetProgramAddress(ServerContext *context,
                             const GetProgramAddressRequestMsg *reqs,
                             ProgramAddressResponseMsg *resps) override;
                
};

#endif    // __INTERNAL_SVC_HPP__

