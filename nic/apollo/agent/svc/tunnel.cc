//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/include/api/pds_tep.hpp"
#include "nic/apollo/agent/svc/tunnel.hpp"

// Build TEP API spec from protobuf spec
static inline void
pds_agent_tep_api_spec_fill (const tpc::TunnelSpec *proto_spec,
                             pds_tep_spec_t *api_spec)
{
    types::IPAddress remoteip = proto_spec->remoteip();

    memset(api_spec, 0, sizeof(pds_tep_spec_t));
    switch (proto_spec->encap()) {
    case tpc::TUNNEL_ENCAP_VXLAN:
        api_spec->encap_type = PDS_TEP_ENCAP_TYPE_VXLAN;
        break;
    case tpc::TUNNEL_ENCAP_MPLSoUDP_TAGS_1:
        api_spec->encap_type = PDS_TEP_ENCAP_TYPE_GW_ENCAP;
        break;
    case tpc::TUNNEL_ENCAP_MPLSoUDP_TAGS_2:
        api_spec->encap_type = PDS_TEP_ENCAP_TYPE_VNIC;
        break;
    default:
        api_spec->encap_type = PDS_TEP_ENCAP_TYPE_NONE;
        break;
    }
    if (types::IP_AF_INET == remoteip.af()) {
        api_spec->key.ip_addr = remoteip.v4addr();
    }
}

// Create Tunnel Object
Status
TunnelSvcImpl::TunnelCreate(ServerContext *context,
                            const tpc::TunnelSpec *proto_spec,
                            tpc::TunnelStatus *proto_status) {
    pds_tep_spec_t api_spec;

    pds_agent_tep_api_spec_fill(proto_spec, &api_spec);
    if (pds_tep_create(&api_spec) != SDK_RET_OK) {
        return Status::CANCELLED;
    }

    return Status::OK;
}
