//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_mapping.hpp"
#include "nic/apollo/agent/svc/util.hpp"
#include "nic/apollo/agent/svc/mapping.hpp"
#include "nic/apollo/agent/trace.hpp"

#if 0
#include "nic/apollo/test/flow_test/flow_test.hpp"
extern flow_test *g_flow_test_obj;
#endif

extern bool g_pds_mock_mode;

// Build VPC API spec from protobuf spec
static inline void
pds_agent_mapping_api_spec_fill (pds_mapping_spec_t *api_spec,
                                 const pds::MappingSpec &proto_spec)
{
    pds::MappingKey key;

    key = proto_spec.id();
    api_spec->key.vcn.id = key.vpcid();
    ipaddr_proto_spec_to_api_spec(&api_spec->key.ip_addr, key.ipaddr());
    api_spec->subnet.id = proto_spec.subnetid();
    api_spec->vnic.id = proto_spec.vnicid();
    if (proto_spec.has_publicip()) {
        if (proto_spec.publicip().af() == types::IP_AF_INET ||
            proto_spec.publicip().af() == types::IP_AF_INET6) {
            api_spec->public_ip_valid = true;
            ipaddr_proto_spec_to_api_spec(&api_spec->public_ip,
                                          proto_spec.publicip());
        }
    }
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
        for (int i = 0; i < proto_req->request_size(); i++) {
            pds_mapping_spec_t api_spec = {0};
            pds_agent_mapping_api_spec_fill(&api_spec, proto_req->request(i));
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
            if (!g_pds_mock_mode) {
                if (pds_mapping_create(&api_spec) != sdk::SDK_RET_OK)
                    return Status::CANCELLED;
            }
        }
    }
    return Status::OK;
}
