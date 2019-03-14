//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/include/api/pds_mapping.hpp"
#include "nic/apollo/agent/svc/util.hpp"
#include "nic/apollo/agent/svc/mapping.hpp"

#if 0
#include "nic/apollo/test/flow_test/flow_test.hpp"
extern flow_test *g_flow_test_obj;
#endif

// Build PCN API spec from protobuf spec
static inline void
pds_agent_mapping_api_spec_fill (const pds::MappingSpec &proto_spec,
                                 pds_mapping_spec_t *api_spec)
{
    pds::MappingKey key;

    key = proto_spec.id();
    api_spec->key.vcn.id = key.pcnid();
    pds_agent_util_ipaddr_fill(key.ipaddr(), &api_spec->key.ip_addr);
    api_spec->subnet.id = proto_spec.subnetid();
    api_spec->vnic.id = proto_spec.vnicid();
    pds_agent_util_ipaddr_fill(proto_spec.publicip(), &api_spec->public_ip);
    MAC_UINT64_TO_ADDR(api_spec->overlay_mac, proto_spec.macaddr());
    api_spec->tep.ip_addr = proto_spec.tunnelid();
    api_spec->subnet.id = proto_spec.subnetid();
    api_spec->vnic.id = proto_spec.vnicid();
    api_spec->fabric_encap = proto_encap_to_pds_encap(proto_spec.encap());
}

Status
MappingSvcImpl::MappingCreate(ServerContext *context,
                              const pds::MappingRequest *proto_req,
                              pds::MappingResponse *proto_rsp) {
    if (proto_req) {
        for (int i = 0; i < proto_req->request_size(); i ++) {
            pds_mapping_spec_t api_spec = {0};
            pds_agent_mapping_api_spec_fill(proto_req->request(i), &api_spec);
#if 0
        // TODO: Adding this here since there is no proto defs for
        // flows. This needs to be cleaned up
        if (api_spec.tep.ip_addr == 0x01000001u) {
            g_flow_test_obj->add_local_ep(api_spec.key.vcn.id,
                                          api_spec.key.ip_addr);
        } else {
            g_flow_test_obj->add_remote_ep(api_spec.key.vcn.id,
                                           api_spec.key.ip_addr);
        }
#endif
            if (pds_mapping_create(&api_spec) == sdk::SDK_RET_OK)
                return Status::OK;
        }
    }
    return Status::CANCELLED;
}
