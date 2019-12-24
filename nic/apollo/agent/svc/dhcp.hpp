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
using pds::DHCPRelayRequest;
using pds::DHCPRelayResponse;
using pds::DHCPRelayDeleteRequest;
using pds::DHCPRelayDeleteResponse;
using pds::DHCPRelayGetRequest;
using pds::DHCPRelayGetResponse;
using pds::DHCPPolicyRequest;
using pds::DHCPPolicyResponse;
using pds::DHCPPolicyDeleteRequest;
using pds::DHCPPolicyDeleteResponse;
using pds::DHCPPolicyGetRequest;
using pds::DHCPPolicyGetResponse;

class DHCPSvcImpl final : public DHCPSvc::Service {
public:
    Status DHCPRelayCreate(ServerContext *context,
                           const pds::DHCPRelayRequest *req,
                           pds::DHCPRelayResponse *rsp) override;
    Status DHCPRelayUpdate(ServerContext *context,
                           const pds::DHCPRelayRequest *req,
                           pds::DHCPRelayResponse *rsp) override;
    Status DHCPRelayGet(ServerContext *context,
                        const pds::DHCPRelayGetRequest *req,
                        pds::DHCPRelayGetResponse *rsp) override;
    Status DHCPRelayDelete(ServerContext *context,
                           const pds::DHCPRelayDeleteRequest *proto_req,
                           pds::DHCPRelayDeleteResponse *proto_rsp) override;
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
