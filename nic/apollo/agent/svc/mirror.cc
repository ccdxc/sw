//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_mirror.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/mirror.hpp"
#include "nic/apollo/agent/svc/mirror.hpp"
#include "nic/apollo/agent/svc/util.hpp"

// populate proto buf spec from mirror session API spec
static inline sdk_ret_t
mirror_session_api_to_proto_spec (pds::MirrorSessionSpec *proto_spec,
                                  const pds_mirror_session_spec_t *api_spec)
{
    proto_spec->set_id(api_spec->key.id);
    proto_spec->set_snaplen(api_spec->snap_len);
    switch (api_spec->type) {
    case PDS_MIRROR_SESSION_TYPE_RSPAN:
        {
            pds::RSpanSpec *proto_rspan = proto_spec->mutable_rspanspec();
            pds_encap_to_proto_encap(proto_rspan->mutable_encap(),
                                     &api_spec->rspan_spec.encap);
            proto_rspan->set_interfaceid(api_spec->rspan_spec.interface);
            proto_spec->set_allocated_rspanspec(proto_rspan);
        }
        break;

    case PDS_MIRROR_SESSION_TYPE_ERSPAN:
        {
            pds::ERSpanSpec *proto_erspan = proto_spec->mutable_erspanspec();
            proto_erspan->mutable_dstip()->set_af(types::IP_AF_INET);
            ipaddr_api_spec_to_proto_spec(proto_erspan->mutable_dstip(),
                                          &api_spec->erspan_spec.dst_ip);
            ipaddr_api_spec_to_proto_spec(proto_erspan->mutable_srcip(),
                                          &api_spec->erspan_spec.src_ip);
            proto_erspan->set_dscp(api_spec->erspan_spec.dscp);
            proto_erspan->set_spanid(api_spec->erspan_spec.span_id);
            proto_spec->set_allocated_erspanspec(proto_erspan);
        }
        break;

    default:
        PDS_TRACE_ERR("Unknown mirror session type {}", api_spec->type);
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

// populate proto buf status from mirror session API status
static inline void
mirror_session_api_status_to_proto_status (pds::MirrorSessionStatus *proto_status,
                                           const pds_mirror_session_status_t *api_status)
{
}

// populate proto buf stats from mirror session API stats
static inline void
mirror_session_api_stats_to_proto_stats (pds::MirrorSessionStats *proto_stats,
                                         const pds_mirror_session_stats_t *api_stats)
{
}

// populate proto buf from mirror session API info
static inline void
mirror_session_api_info_to_proto (const pds_mirror_session_info_t *api_info,
                                  void *ctxt)
{
    pds::MirrorSessionGetResponse *proto_rsp =
        (pds::MirrorSessionGetResponse *)ctxt;
    auto mirror_session = proto_rsp->add_response();
    pds::MirrorSessionSpec *proto_spec = mirror_session->mutable_spec();
    pds::MirrorSessionStatus *proto_status = mirror_session->mutable_status();
    pds::MirrorSessionStats *proto_stats = mirror_session->mutable_stats();

    mirror_session_api_to_proto_spec(proto_spec, &api_info->spec);
    mirror_session_api_status_to_proto_status(proto_status, &api_info->status);
    mirror_session_api_stats_to_proto_stats(proto_stats, &api_info->stats);
}

// build mirror session API spec from protobuf spec
static inline sdk_ret_t
mirror_session_proto_to_api_spec (pds_mirror_session_spec_t *api_spec,
                                  const pds::MirrorSessionSpec &proto_spec)
{
    api_spec->key.id = proto_spec.id();
    api_spec->snap_len = proto_spec.snaplen();
    if (proto_spec.has_rspanspec()) {
        api_spec->type = PDS_MIRROR_SESSION_TYPE_RSPAN;
        api_spec->rspan_spec.encap =
            proto_encap_to_pds_encap(proto_spec.rspanspec().encap());
        if (api_spec->rspan_spec.encap.type != PDS_ENCAP_TYPE_DOT1Q) {
            PDS_TRACE_ERR("Invalid encap type {} in RSPAN mirror session {} "
                          "spec, only PDS_ENCAP_TYPE_DOT1Q encap type is valid",
                          api_spec->rspan_spec.encap.type, api_spec->key.id);
            return SDK_RET_INVALID_ARG;
        }
        if (api_spec->rspan_spec.encap.val.vlan_tag == 0) {
            PDS_TRACE_ERR("Invalid vlan tag 0 in RSPAN mirror session {} spec",
                          api_spec->key.id);
            return SDK_RET_INVALID_ARG;
        }
        api_spec->rspan_spec.interface =
            proto_spec.rspanspec().interfaceid();
    } else if (proto_spec.has_erspanspec()) {
        if (!proto_spec.erspanspec().has_dstip() ||
            !proto_spec.erspanspec().has_srcip()) {
            PDS_TRACE_ERR("src IP or dst IP missing in mirror session {} spec",
                          api_spec->key.id);
            return SDK_RET_INVALID_ARG;
        }
        types::IPAddress dstip = proto_spec.erspanspec().dstip();
        types::IPAddress srcip = proto_spec.erspanspec().srcip();
        api_spec->type = PDS_MIRROR_SESSION_TYPE_ERSPAN;
        ipaddr_proto_spec_to_api_spec(&api_spec->erspan_spec.dst_ip, dstip);
        ipaddr_proto_spec_to_api_spec(&api_spec->erspan_spec.src_ip, srcip);
        api_spec->erspan_spec.dscp = proto_spec.erspanspec().dscp();
        api_spec->erspan_spec.span_id = proto_spec.erspanspec().spanid();
    } else {
        PDS_TRACE_ERR("rspan & erspan config missing in mirror session {} spec",
                      api_spec->key.id);
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

// create mirror session object
Status
MirrorSvcImpl::MirrorSessionCreate(ServerContext *context,
                                   const pds::MirrorSessionRequest *proto_req,
                                   pds::MirrorSessionResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_mirror_session_key_t key;
    pds_mirror_session_spec_t *api_spec;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    for (int i = 0; i < proto_req->request_size(); i ++) {
        api_spec = (pds_mirror_session_spec_t *)
                       core::agent_state::state()->mirror_session_slab()->alloc();
        if (api_spec == NULL) {
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OUT_OF_MEM);
            break;
        }
        auto request = proto_req->request(i);
        key.id = request.id();
        ret = mirror_session_proto_to_api_spec(api_spec, request);
        if (unlikely(ret != SDK_RET_OK)) {
            core::agent_state::state()->mirror_session_slab()->free(api_spec);
            break;
        }
        ret = core::mirror_session_create(&key, api_spec);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        if (ret != sdk::SDK_RET_OK) {
            break;
        }
    }
    return Status::OK;
}

Status
MirrorSvcImpl::MirrorSessionDelete(ServerContext *context,
                                   const pds::MirrorSessionDeleteRequest *proto_req,
                                   pds::MirrorSessionDeleteResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_mirror_session_key_t key;

    if (proto_req == NULL) {
        proto_rsp->add_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    for (int i = 0; i < proto_req->id_size(); i++) {
        key.id = proto_req->id(i);
        ret = core::mirror_session_delete(&key);
        proto_rsp->add_apistatus(sdk_ret_to_api_status(ret));
    }
    return Status::OK;
}

Status
MirrorSvcImpl::MirrorSessionGet(ServerContext *context,
                                const pds::MirrorSessionGetRequest *proto_req,
                                pds::MirrorSessionGetResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_mirror_session_key_t key;
    pds_mirror_session_info_t info;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    if (proto_req->id_size() == 0) {
        ret = core::mirror_session_get_all(mirror_session_api_info_to_proto,
                                           proto_rsp);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    }
    for (int i = 0; i < proto_req->id_size(); i++) {
        key.id = proto_req->id(i);
        ret = core::mirror_session_get(&key, &info);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        if (ret != sdk::SDK_RET_OK) {
            break;
        }
        auto response = proto_rsp->add_response();
        mirror_session_api_to_proto_spec(response->mutable_spec(), &info.spec);
        mirror_session_api_status_to_proto_status(response->mutable_status(),
                                                  &info.status);
        mirror_session_api_stats_to_proto_stats(response->mutable_stats(),
                                                &info.stats);
    }
    return Status::OK;
}
