//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_tep.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/tunnel.hpp"
#include "nic/apollo/agent/svc/tunnel.hpp"
#include "nic/apollo/agent/svc/util.hpp"

// Build TEP API spec from protobuf spec
static inline void
pds_agent_tep_api_spec_fill (const pds::TunnelSpec &proto_spec,
                             pds_tep_spec_t *api_spec)
{
    types::IPAddress remoteip = proto_spec.remoteip();

    memset(api_spec, 0, sizeof(pds_tep_spec_t));
    switch (proto_spec.encap()) {
    case pds::TUNNEL_ENCAP_VXLAN:
        api_spec->encap_type = PDS_TEP_ENCAP_TYPE_VXLAN;
        break;
    case pds::TUNNEL_ENCAP_MPLSoUDP_TAGS_1:
        api_spec->encap_type = PDS_TEP_ENCAP_TYPE_GW_ENCAP;
        break;
    case pds::TUNNEL_ENCAP_MPLSoUDP_TAGS_2:
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
                            const pds::TunnelRequest *proto_req,
                            pds::TunnelResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_tep_spec_t *api_spec;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    for (int i = 0; i < proto_req->request_size(); i ++) {
        api_spec = (pds_tep_spec_t *)
                    core::agent_state::state()->tep_slab()->alloc();
        if (api_spec == NULL) {
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OUT_OF_MEM);
            break;
        }
        auto request = proto_req->request(i);
        pds_agent_tep_api_spec_fill(request, api_spec);
        ret = core::tep_create(request.id(), api_spec);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        if (ret != sdk::SDK_RET_OK) {
            break;
        }
    }
    return Status::OK;
}

Status
TunnelSvcImpl::TunnelDelete(ServerContext *context,
                            const pds::TunnelDeleteRequest *proto_req,
                            pds::TunnelDeleteResponse *proto_rsp) {
    sdk_ret_t ret;

    if (proto_req == NULL) {
        proto_rsp->add_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    for (int i = 0; i < proto_req->id_size(); i++) {
        ret = core::tep_delete(proto_req->id(i));
        proto_rsp->add_apistatus(sdk_ret_to_api_status(ret));
    }
    return Status::OK;
}
