#ifndef __SYSTEM_SVC_HPP__
#define __SYSTEM_SVC_HPP__

#include "nic/include/base.hpp"
#include <grpc++/grpc++.h>
#include "gen/proto/types.pb.h"
#include "gen/proto/system.grpc.pb.h"

using grpc::ServerContext;
using grpc::Status;

using sys::System;
using sys::SystemGetRequest;
using sys::SystemResponse;
using sys::ApiStatsResponse;
using sys::FeatureProfileResponse;
using sys::ForwardingModeResponse;
using sys::MicroSegRequestMsg;
using sys::MicroSegResponseMsg;
using sys::MicroSegSpec;
using sys::MicroSegResponse;
using sys::MicroSegUpdateRequestMsg;
using sys::MicroSegUpdateResponseMsg;
using sys::MicroSegUpdateRequest;
using sys::MicroSegUpdateResponse;
using types::Empty;

class SystemServiceImpl final : public System::Service {
public:
    Status APIStatsGet(ServerContext *context,
                       const Empty *request,
                       ApiStatsResponse *rsp) override;
    Status SystemGet(ServerContext *context,
                     const SystemGetRequest *request,
                     SystemResponse *rsp) override;
    Status SystemUUIDGet(ServerContext *context,
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
    Status ForwardingModeGet(ServerContext *context,
                             const Empty *request,
                             ForwardingModeResponse *rsp) override;
    Status FeatureProfileGet(ServerContext *context,
                             const Empty *request,
                             FeatureProfileResponse *rsp) override;
    Status MicroSegStatusUpdate(ServerContext *context,
                                const MicroSegRequestMsg *req,
                                MicroSegResponseMsg *rsp) override;
    Status MicroSegUpdate(ServerContext *context,
                          const MicroSegUpdateRequestMsg *req,
                          MicroSegUpdateResponseMsg *rsp) override;
};
#endif    // __SYSTEM_SVC_HPP__

