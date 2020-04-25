//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/svc/dhcp_svc.hpp"

Status
DHCPSvcImpl::DHCPPolicyCreate(ServerContext *context,
                              const pds::DHCPPolicyRequest *proto_req,
                              pds::DHCPPolicyResponse *proto_rsp) {
    pds_svc_dhcp_policy_create(proto_req, proto_rsp);
    return Status::OK;
}

Status
DHCPSvcImpl::DHCPPolicyUpdate(ServerContext *context,
                              const pds::DHCPPolicyRequest *proto_req,
                              pds::DHCPPolicyResponse *proto_rsp) {
    pds_svc_dhcp_policy_update(proto_req, proto_rsp);
    return Status::OK;
}

Status
DHCPSvcImpl::DHCPPolicyDelete(ServerContext *context,
                              const pds::DHCPPolicyDeleteRequest *proto_req,
                              pds::DHCPPolicyDeleteResponse *proto_rsp) {
    pds_svc_dhcp_policy_delete(proto_req, proto_rsp);
    return Status::OK;
}

Status
DHCPSvcImpl::DHCPPolicyGet(ServerContext *context,
                           const pds::DHCPPolicyGetRequest *proto_req,
                           pds::DHCPPolicyGetResponse *proto_rsp) {
    pds_svc_dhcp_policy_get(proto_req, proto_rsp);
    return Status::OK;
}
