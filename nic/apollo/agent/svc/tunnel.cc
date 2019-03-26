//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_tep.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/tunnel.hpp"
#include "nic/apollo/agent/svc/tunnel.hpp"
#include "nic/apollo/agent/svc/util.hpp"

// Populate proto buf spec from tep API spec
static inline void
tep_api_spec_to_proto_spec (const pds_tep_spec_t *api_spec,
                            pds::TunnelSpec *proto_spec)
{
    switch (api_spec->encap_type) {
    case PDS_TEP_ENCAP_TYPE_VXLAN:
        proto_spec->set_encap(pds::TUNNEL_ENCAP_VXLAN);
        break;

    case PDS_TEP_ENCAP_TYPE_GW_ENCAP:
        proto_spec->set_encap(pds::TUNNEL_ENCAP_MPLSoUDP_TAGS_1);
        break;

    case PDS_TEP_ENCAP_TYPE_VNIC:
        proto_spec->set_encap(pds::TUNNEL_ENCAP_MPLSoUDP_TAGS_2);
        break;

    default:
        proto_spec->set_encap(pds::TUNNEL_ENCAP_VXLAN);
        break;
    }
    if (api_spec->key.ip_addr != 0) {
        proto_spec->mutable_remoteip()->set_af(types::IP_AF_INET);
        proto_spec->mutable_remoteip()->set_v4addr(api_spec->key.ip_addr);
    }
}

// Populate proto buf status from tep API status
static inline void
tep_api_status_to_proto_status (const pds_tep_status_t *api_status,
                                pds::TunnelStatus *proto_status)
{
}

// Populate proto buf stats from tep API stats
static inline void
tep_api_stats_to_proto_stats (const pds_tep_stats_t *api_stats,
                              pds::TunnelStats *proto_stats)
{
}

// Populate proto buf from tep API info
static inline void
tep_api_info_to_proto (const pds_tep_info_t *api_info, void *ctxt)
{
    pds::TunnelGetResponse *proto_rsp = (pds::TunnelGetResponse *)ctxt;
    auto tep = proto_rsp->add_response();
    pds::TunnelSpec *proto_spec = tep->mutable_spec();
    pds::TunnelStatus *proto_status = tep->mutable_status();
    pds::TunnelStats *proto_stats = tep->mutable_stats();

    tep_api_spec_to_proto_spec(&api_info->spec, proto_spec);
    tep_api_status_to_proto_status(&api_info->status, proto_status);
    tep_api_stats_to_proto_stats(&api_info->stats, proto_stats);
}

// Build TEP API spec from protobuf spec
static inline void
tep_proto_spec_to_api_spec (const pds::TunnelSpec &proto_spec,
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
        tep_proto_spec_to_api_spec(request, api_spec);
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

Status
TunnelSvcImpl::TunnelGet(ServerContext *context,
                         const pds::TunnelGetRequest *proto_req,
                         pds::TunnelGetResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_tep_info_t info;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    if (proto_req->id_size() == 0) {
        // get all
        ret = core::tep_get_all(tep_api_info_to_proto, proto_rsp);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    }
    for (int i = 0; i < proto_req->id_size(); i++) {
        ret = core::tep_get(proto_req->id(i), &info);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        if (ret != sdk::SDK_RET_OK) {
            break;
        }
        auto response = proto_rsp->add_response();
        tep_api_spec_to_proto_spec(&info.spec, response->mutable_spec());
        tep_api_status_to_proto_status(&info.status,
                                       response->mutable_status());
        tep_api_stats_to_proto_stats(&info.stats, response->mutable_stats());
    }
    return Status::OK;
}
