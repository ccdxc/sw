// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __AGENT_SVC_DHCP_HPP__
#define __AGENT_SVC_DHCP_HPP__

#include "grpc++/grpc++.h"
#include "gen/proto/types.pb.h"
#include "gen/proto/meta/meta.pb.h"
#include "gen/proto/dhcp.grpc.pb.h"

using grpc::Status;
using grpc::ServerContext;

using pds::DHCPSvc;
using pds::DHCPRelayAgentRequest;
using pds::DHCPRelayAgentResponse;
using pds::DHCPRelayAgentDeleteRequest;
using pds::DHCPRelayAgentDeleteResponse;
using pds::DHCPRelayAgentGetRequest;
using pds::DHCPRelayAgentGetResponse;
using pds::DHCPPolicyRequest;
using pds::DHCPPolicyResponse;
using pds::DHCPPolicyDeleteRequest;
using pds::DHCPPolicyDeleteResponse;
using pds::DHCPPolicyGetRequest;
using pds::DHCPPolicyGetResponse;

class DHCPSvcImpl final : public DHCPSvc::Service {
public:
    Status DHCPRelayAgentCreate(ServerContext *context,
                                const pds::DHCPRelayAgentRequest *req,
                                pds::DHCPRelayAgentResponse *rsp) override;
    Status DHCPRelayAgentUpdate(ServerContext *context,
                                const pds::DHCPRelayAgentRequest *req,
                                pds::DHCPRelayAgentResponse *rsp) override;
    Status DHCPRelayAgentGet(ServerContext *context,
                             const pds::DHCPRelayAgentGetRequest *req,
                             pds::DHCPRelayAgentGetResponse *rsp) override;
    Status DHCPRelayAgentDelete(ServerContext *context,
                                const pds::DHCPRelayAgentDeleteRequest *proto_req,
                                pds::DHCPRelayAgentDeleteResponse *proto_rsp) override;
    Status DHCPPolicyCreate(ServerContext *context,
                            const pds::DHCPPolicyRequest *req,
                            pds::DHCPPolicyResponse *rsp) override;
    Status DHCPPolicyUpdate(ServerContext *context,
                            const pds::DHCPPolicyRequest *req,
                            pds::DHCPPolicyResponse *rsp) override;
    Status DHCPPolicyGet(ServerContext *context,
                         const pds::DHCPPolicyGetRequest *req,
                         pds::DHCPPolicyGetResponse *rsp) override;
    Status DHCPPolicyDelete(ServerContext *context,
                            const pds::DHCPPolicyDeleteRequest *proto_req,
                            pds::DHCPPolicyDeleteResponse *proto_rsp) override;
};

#endif    // __AGENT_SVC_DHCP_HPP__
