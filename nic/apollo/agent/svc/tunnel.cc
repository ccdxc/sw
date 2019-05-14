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
tep_api_spec_to_proto_spec (pds::TunnelSpec *proto_spec,
                            const pds_tep_spec_t *api_spec)
{
    if (api_spec->key.ip_addr != 0) {
        proto_spec->mutable_remoteip()->set_af(types::IP_AF_INET);
        proto_spec->mutable_remoteip()->set_v4addr(api_spec->key.ip_addr);
    }
    switch (api_spec->type) {
    case PDS_TEP_TYPE_WORKLOAD:
        proto_spec->set_type(pds::TUNNEL_TYPE_WORKLOAD);
        break;
    case PDS_TEP_TYPE_IGW:
        proto_spec->set_type(pds::TUNNEL_TYPE_IGW);
        break;
    case PDS_TEP_TYPE_NONE:
    default:
        proto_spec->set_type(pds::TUNNEL_TYPE_NONE);
        break;
    }
    pds_encap_to_proto_encap(proto_spec->mutable_encap(),
                             &api_spec->encap);
    // TODO: fill Nat here
    proto_spec->set_nat(api_spec->nat);
}

// Populate proto buf status from tep API status
static inline void
tep_api_status_to_proto_status (pds::TunnelStatus *proto_status,
                                const pds_tep_status_t *api_status)
{
}

// Populate proto buf stats from tep API stats
static inline void
tep_api_stats_to_proto_stats (pds::TunnelStats *proto_stats,
                              const pds_tep_stats_t *api_stats)
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

    tep_api_spec_to_proto_spec(proto_spec, &api_info->spec);
    tep_api_status_to_proto_status(proto_status, &api_info->status);
    tep_api_stats_to_proto_stats(proto_stats, &api_info->stats);
}

// Build TEP API spec from protobuf spec
static inline void
tep_proto_spec_to_api_spec (pds_tep_spec_t *api_spec,
                            const pds::TunnelSpec &proto_spec)
{
    types::IPAddress remoteip = proto_spec.remoteip();

    memset(api_spec, 0, sizeof(pds_tep_spec_t));
    switch (proto_spec.type()) {
    case pds::TUNNEL_TYPE_IGW:
        api_spec->type = PDS_TEP_TYPE_IGW;
        break;
    case pds::TUNNEL_TYPE_WORKLOAD:
        api_spec->type = PDS_TEP_TYPE_WORKLOAD;
        break;
    default:
        api_spec->type = PDS_TEP_TYPE_NONE;
        break;
    }
    switch (proto_spec.encap().type()) {
    case types::ENCAP_TYPE_VXLAN:
        api_spec->encap.type = PDS_ENCAP_TYPE_VXLAN;
        api_spec->encap.val.vnid = proto_spec.encap().value().vnid();
        break;
    case types::ENCAP_TYPE_MPLSoUDP:
        api_spec->encap.type = PDS_ENCAP_TYPE_MPLSoUDP;
        api_spec->encap.val.mpls_tag = proto_spec.encap().value().mplstag();
        break;
    default:
        break;
    }
    if (types::IP_AF_INET == remoteip.af()) {
        api_spec->key.ip_addr = remoteip.v4addr();
    }
    api_spec->nat = proto_spec.nat();
}

// Create Tunnel Object
Status
TunnelSvcImpl::TunnelCreate(ServerContext *context,
                            const pds::TunnelRequest *proto_req,
                            pds::TunnelResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_tep_spec_t *api_spec = NULL;

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
        tep_proto_spec_to_api_spec(api_spec, request);
        ret = core::tep_create(request.id(), api_spec);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        if (ret != sdk::SDK_RET_OK) {
            break;
        }
    }
    return Status::OK;
}

// Update Tunnel Object
Status
TunnelSvcImpl::TunnelUpdate(ServerContext *context,
                            const pds::TunnelRequest *proto_req,
                            pds::TunnelResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_tep_spec_t *api_spec = NULL;

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
        tep_proto_spec_to_api_spec(api_spec, request);
        ret = core::tep_update(request.id(), api_spec);
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
    pds_tep_info_t info = {0};

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
        tep_api_spec_to_proto_spec(response->mutable_spec(), &info.spec);
        tep_api_status_to_proto_status(response->mutable_status(), &info.status);
        tep_api_stats_to_proto_stats(response->mutable_stats(), &info.stats);
    }
    return Status::OK;
}
