//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/include/api/pds_mapping.hpp"
#include "nic/apollo/agent/svc/util.hpp"
#include "nic/apollo/agent/svc/mapping.hpp"

// Build PCN API spec from protobuf spec
static inline void
pds_agent_mapping_api_spec_fill (const tpc::MappingSpec *proto_spec,
                                 pds_mapping_spec_t *api_spec)
{
    tpc::MappingKey key;

    key = proto_spec->id();
    api_spec->key.vcn.id = key.pcnid();
    pds_agent_util_ipaddr_fill(key.ipaddr(), &api_spec->key.ip_addr);
    api_spec->subnet.id = proto_spec->subnetid();
    api_spec->vnic.id = proto_spec->vnicid();
    pds_agent_util_ipaddr_fill(proto_spec->publicip(), &api_spec->public_ip);
    MAC_UINT64_TO_ADDR(api_spec->overlay_mac, proto_spec->macaddr());
    // TODO tep key - tunnel id
    api_spec->subnet.id = proto_spec->subnetid();
    api_spec->vnic.id = proto_spec->vnicid();
}

Status
MappingSvcImpl::MappingCreate(ServerContext *context,
                              const tpc::MappingSpec *proto_spec,
                              tpc::MappingStatus *proto_status) {
    pds_mapping_spec_t api_spec = {0};

    if (proto_spec) {
        pds_agent_mapping_api_spec_fill(proto_spec, &api_spec);
        if (pds_mapping_create(&api_spec) == sdk::SDK_RET_OK)
            return Status::OK;
    }
    return Status::CANCELLED;
}
