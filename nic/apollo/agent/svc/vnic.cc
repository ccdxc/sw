//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/include/api/pds_vnic.hpp"
#include "nic/apollo/agent/svc/util.hpp"
#include "nic/apollo/agent/svc/vnic.hpp"

// Build VNIC API spec from proto buf spec
static inline void
pds_agent_vnic_api_spec_fill (const tpc::VnicSpec &proto_spec,
                              pds_vnic_spec_t *api_spec)
{
    api_spec->vcn.id = proto_spec.pcnid();
    api_spec->subnet.id = proto_spec.subnetid();
    api_spec->key.id = proto_spec.vnicid();
    api_spec->wire_vlan = proto_spec.wirevlan();
    api_spec->fabric_encap = proto_encap_to_pds_encap(proto_spec.encap());
    MAC_UINT64_TO_ADDR(api_spec->mac_addr, proto_spec.macaddress());
    api_spec->rsc_pool_id = proto_spec.resourcepoolid();
    api_spec->src_dst_check = proto_spec.sourceguardenable();
}

Status
VnicSvcImpl::VnicCreate(ServerContext *context, const tpc::VnicRequest *proto_req,
                        tpc::VnicResponse *proto_rsp) {
    if (proto_req) {
        for (int i = 0; i < proto_req->request_size(); i ++) {
            pds_vnic_spec_t api_spec = {0};

            pds_agent_vnic_api_spec_fill(proto_req->request(i), &api_spec);
            if (pds_vnic_create(&api_spec) == sdk::SDK_RET_OK)
                return Status::OK;
        }
    }
    return Status::CANCELLED;
}
