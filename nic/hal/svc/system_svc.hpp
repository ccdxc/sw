#ifndef __SYSTEM_SVC_HPP__
#define __SYSTEM_SVC_HPP__

#include "nic/include/base.hpp"
#include <grpc++/grpc++.h>
#include "gen/proto/types.pb.h"
#include "gen/proto/system.grpc.pb.h"

using grpc::ServerContext;
using grpc::Status;

using sys::System;
using sys::SystemResponse;
using sys::ApiStatsResponse;
using types::Empty;

class SystemServiceImpl final : public System::Service {
public:
    Status ApiStatsGet(ServerContext *context,
                       const Empty *request,
                       ApiStatsResponse *rsp) override;
    Status SystemGet(ServerContext *context,
                     const Empty *request,
                     SystemResponse *rsp) override;
    Status SystemUuidGet(ServerContext *context,
                         const Empty *request,
                         SystemResponse *rsp) override;
    Status ClearIngressDropStats(ServerContext *context,
                                 const Empty *request,
                                 Empty *rsp) override;
    Status ClearEgressDropStats(ServerContext *context,
                                const Empty *request,
                                Empty *rsp) override;
    Status ClearPbDropStats(ServerContext *context,
                            const Empty *request,
                            Empty *rsp) override;
    Status ClearFteStats(ServerContext *context,
                         const Empty *request,
                         Empty *rsp) override; 
    Status ClearFteTxRxStats(ServerContext *context,
                             const Empty *request,
                             Empty *rsp) override;
    Status ClearTableStats(ServerContext *context,
                           const Empty *request,
                           Empty *rsp) override;
    Status ClearPbStats(ServerContext *context,
                        const Empty *request,
                        Empty *rsp) override;
};
#endif    // __SYSTEM_SVC_HPP__

