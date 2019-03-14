//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/include/api/pds_subnet.hpp"
#include "nic/apollo/agent/svc/util.hpp"
#include "nic/apollo/agent/svc/subnet.hpp"

// Build subnet API spec from proto buf spec
static inline void
pds_agent_subnet_api_spec_fill (const pds::SubnetSpec &proto_spec,
                                pds_subnet_spec_t *api_spec)
{
    api_spec->key.id = proto_spec.id();
    api_spec->vcn.id = proto_spec.pcnid();
    pds_agent_util_ip_pfx_fill(proto_spec.prefix(), &api_spec->pfx);
    pds_agent_util_ipaddr_fill(proto_spec.virtualrouterip(), &api_spec->vr_ip);
    MAC_UINT64_TO_ADDR(api_spec->vr_mac, proto_spec.virtualroutermac());
    api_spec->v4_route_table.id = proto_spec.v4routetableid();
    api_spec->v6_route_table.id = proto_spec.v6routetableid();
    api_spec->ing_v4_policy.id = proto_spec.ingv4securitypolicyid();
    api_spec->ing_v6_policy.id = proto_spec.ingv6securitypolicyid();
    api_spec->egr_v4_policy.id = proto_spec.egv4securitypolicyid();
    api_spec->egr_v6_policy.id = proto_spec.egv6securitypolicyid();
}

Status
SubnetSvcImpl::SubnetCreate(ServerContext *context,
                            const pds::SubnetRequest *proto_req,
                            pds::SubnetResponse *proto_rsp) {
    if (proto_req) {
        for (int i = 0; i < proto_req->request_size(); i ++) {
            pds_subnet_spec_t api_spec = {0};
            pds_agent_subnet_api_spec_fill(proto_req->request(i), &api_spec);
            if (pds_subnet_create(&api_spec) == sdk::SDK_RET_OK)
                return Status::OK;
        }
    }

    return Status::CANCELLED;
}
