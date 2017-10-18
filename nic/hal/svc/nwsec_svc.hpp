#ifndef __NWSEC_SVC_HPP__
#define __NWSEC_SVC_HPP__

#include "nic/include/base.h"
#include "grpc++/grpc++.h"
#include "nic/gen/proto/hal/types.pb.h"
#include "nic/gen/proto/hal/nwsec.grpc.pb.h"

using grpc::ServerContext;
using grpc::Status;

using nwsec::NwSecurity;
using nwsec::SecurityProfileKeyHandle;
using nwsec::SecurityProfileSpec;
using nwsec::SecurityProfileRequestMsg;
using nwsec::SecurityProfileStatus;
using nwsec::SecurityProfileResponse;
using nwsec::SecurityProfileResponseMsg;
using nwsec::SecurityProfileDeleteRequest;
using nwsec::SecurityProfileDeleteRequestMsg;
using nwsec::SecurityProfileDeleteResponseMsg;
using nwsec::SecurityProfileGetRequest;
using nwsec::SecurityProfileGetRequestMsg;
using nwsec::SecurityProfileStats;
using nwsec::SecurityProfileGetResponse;
using nwsec::SecurityProfileGetResponseMsg;
using nwsec::SecurityGroupSpec;
using nwsec::SecurityGroupRequestMsg;
using nwsec::SecurityGroupStatus;
using nwsec::SecurityGroupResponse;
using nwsec::SecurityGroupResponseMsg;
using nwsec::SecurityGroupDeleteRequestMsg;
using nwsec::SecurityGroupDeleteResponseMsg;
using nwsec::SecurityGroupGetRequestMsg;
using nwsec::SecurityGroupGetResponseMsg;
using nwsec::SecurityGroupGetResponse;
using nwsec::DoSPolicyRequestMsg;
using nwsec::DoSPolicyResponseMsg;
using nwsec::DoSPolicyDeleteRequestMsg;
using nwsec::DoSPolicyDeleteResponseMsg;
using nwsec::DoSPolicyGetRequestMsg;
using nwsec::DoSPolicyGetResponseMsg;
using nwsec::Service;

class NwSecurityServiceImpl final : public NwSecurity::Service {
public:
    Status SecurityProfileCreate(ServerContext *context,
                                 const SecurityProfileRequestMsg *req,
                                 SecurityProfileResponseMsg *rsp) override;

    Status SecurityProfileUpdate(ServerContext *context,
                                 const SecurityProfileRequestMsg *req,
                                 SecurityProfileResponseMsg *rsp) override;

    Status SecurityProfileDelete(ServerContext *context,
                                 const SecurityProfileDeleteRequestMsg *req,
                                 SecurityProfileDeleteResponseMsg *rsp) override;

    Status SecurityProfileGet(ServerContext *context,
                              const SecurityProfileGetRequestMsg *req,
                              SecurityProfileGetResponseMsg *rsp) override;

    Status SecurityGroupCreate(ServerContext *context,
                               const SecurityGroupRequestMsg *req,
                               SecurityGroupResponseMsg      *rsp) override;

    Status SecurityGroupUpdate(ServerContext *context,
                               const SecurityGroupRequestMsg *req,
                               SecurityGroupResponseMsg      *rsp) override;

    Status SecurityGroupDelete(ServerContext *context,
                               const SecurityGroupDeleteRequestMsg *req,
                               SecurityGroupDeleteResponseMsg      *rsp) override;
    
    Status SecurityGroupGet(ServerContext *context,
                            const SecurityGroupGetRequestMsg *req,
                            SecurityGroupGetResponseMsg      *rsp) override;

    Status DoSPolicyCreate(ServerContext *context,
                           const DoSPolicyRequestMsg *req,
                           DoSPolicyResponseMsg *rsp) override;

    Status DoSPolicyUpdate(ServerContext *context,
                           const DoSPolicyRequestMsg *req,
                           DoSPolicyResponseMsg *rsp) override;

    Status DoSPolicyDelete(ServerContext *context,
                           const DoSPolicyDeleteRequestMsg *req,
                           DoSPolicyDeleteResponseMsg *rsp) override;

    Status DoSPolicyGet(ServerContext *context,
                        const DoSPolicyGetRequestMsg *req,
                        DoSPolicyGetResponseMsg *rsp) override;
};

#endif    // __NWSEC_SVC_HPP__

