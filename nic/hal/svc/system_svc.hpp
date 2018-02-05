#ifndef __SYSTEM_SVC_HPP__
#define __SYSTEM_SVC_HPP__

#include "nic/include/base.h"
#include <grpc++/grpc++.h>
#include "nic/gen/proto/hal/types.pb.h"
#include "nic/gen/proto/hal/system.grpc.pb.h"

using grpc::ServerContext;
using grpc::Status;

using sys::System;
using sys::SystemResponse;
using types::Empty;
using sys::SystemConfigMsg;
using sys::SystemConfigResponseMsg;


class SystemServiceImpl final : public System::Service {
public:
    Status SystemGet(ServerContext *context,
                    const Empty *request,
                     SystemResponse *rsp) override;
    Status SystemConfig(ServerContext *context,
                             const  SystemConfigMsg *request,
                             SystemConfigResponseMsg *rsp) override;
};
#endif    // __SYSTEM_SVC_HPP__

