//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines protobuf API for tunnel object
///
//----------------------------------------------------------------------------

#ifndef __AGENT_SVC_TUNNEL_SVC_HPP__
#define __AGENT_SVC_TUNNEL_SVC_HPP__

#include "nic/apollo/api/include/pds_tep.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/svc/tunnel.hpp"
#include "nic/apollo/agent/hooks.hpp"

// populate proto buf spec from tep API spec
static inline sdk_ret_t
pds_tep_api_spec_to_proto (pds::TunnelSpec *proto_spec,
                           const pds_tep_spec_t *api_spec)
{
    if (!api_spec || !proto_spec) {
        return SDK_RET_INVALID_ARG;
    }
    proto_spec->set_id(api_spec->key.id, PDS_MAX_KEY_LEN);
    ipaddr_api_spec_to_proto_spec(proto_spec->mutable_remoteip(),
                                  &api_spec->remote_ip);
    if (api_spec->ip_addr.af != IP_AF_NIL) {
        // local IP is optional
        ipaddr_api_spec_to_proto_spec(proto_spec->mutable_localip(),
                                      &api_spec->ip_addr);
    }
    proto_spec->set_macaddress(MAC_TO_UINT64(api_spec->mac));
    pds_encap_to_proto_encap(proto_spec->mutable_encap(),
                             &api_spec->encap);
    proto_spec->set_nat(api_spec->nat);
    proto_spec->set_vpcid(api_spec->vpc.id, PDS_MAX_KEY_LEN);
    switch (api_spec->type) {
    case PDS_TEP_TYPE_WORKLOAD:
        proto_spec->set_type(pds::TUNNEL_TYPE_WORKLOAD);
        break;
    case PDS_TEP_TYPE_IGW:
        proto_spec->set_type(pds::TUNNEL_TYPE_IGW);
        break;
    case PDS_TEP_TYPE_INTER_DC:
        proto_spec->set_type(pds::TUNNEL_TYPE_INTER_DC);
        break;
    case PDS_TEP_TYPE_SERVICE:
        proto_spec->set_type(pds::TUNNEL_TYPE_SERVICE);
        break;
    case PDS_TEP_TYPE_NONE:
    default:
        proto_spec->set_type(pds::TUNNEL_TYPE_NONE);
        break;
    }

    proto_spec->set_remoteservice(api_spec->remote_svc);
    if (api_spec->remote_svc) {
        pds_encap_to_proto_encap(proto_spec->mutable_remoteserviceencap(),
                                 &api_spec->remote_svc_encap);
        ipaddr_api_spec_to_proto_spec(
            proto_spec->mutable_remoteservicepublicip(),
            &api_spec->remote_svc_public_ip);
    }
    switch (api_spec->nh_type) {
    case PDS_NH_TYPE_UNDERLAY:
        proto_spec->set_nexthopid(api_spec->nh.id, PDS_MAX_KEY_LEN);
        break;
    case PDS_NH_TYPE_UNDERLAY_ECMP:
        proto_spec->set_nexthopid(api_spec->nh_group.id, PDS_MAX_KEY_LEN);
        break;
    case PDS_NH_TYPE_OVERLAY:
        proto_spec->set_tunnelid(api_spec->tep.id, PDS_MAX_KEY_LEN);
        break;
    default:
        //PDS_TRACE_ERR("Unsupported nexthop type {} in TEP {} spec",
                      //api_spec->nh_type, api_spec->key.id);
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

// populate proto buf status from tep API status
static inline void
pds_tep_api_status_to_proto (pds::TunnelStatus *proto_status,
                             const pds_tep_status_t *api_status)
{
    proto_status->set_hwid(api_status->hw_id);
}

// populate proto buf stats from tep API stats
static inline void
pds_tep_api_stats_to_proto (pds::TunnelStats *proto_stats,
                            const pds_tep_stats_t *api_stats)
{
}

// populate proto buf from tep API info
static inline void
pds_tep_api_info_to_proto (pds_tep_info_t *api_info, void *ctxt)
{
    pds::TunnelGetResponse *proto_rsp = (pds::TunnelGetResponse *)ctxt;
    auto tep = proto_rsp->add_response();
    pds::TunnelSpec *proto_spec = tep->mutable_spec();
    pds::TunnelStatus *proto_status = tep->mutable_status();
    pds::TunnelStats *proto_stats = tep->mutable_stats();

    pds_tep_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_tep_api_status_to_proto(proto_status, &api_info->status);
    pds_tep_api_stats_to_proto(proto_stats, &api_info->stats);
}

// build TEP API spec from protobuf spec
static inline sdk_ret_t
pds_tep_proto_to_api_spec (pds_tep_spec_t *api_spec,
                           const pds::TunnelSpec &proto_spec)
{
    memset(api_spec, 0, sizeof(pds_tep_spec_t));
    pds_obj_key_proto_to_api_spec(&api_spec->key, proto_spec.id());
    ipaddr_proto_spec_to_api_spec(&api_spec->remote_ip,
                                  proto_spec.remoteip());
    ipaddr_proto_spec_to_api_spec(&api_spec->ip_addr,
                                  proto_spec.localip());
    MAC_UINT64_TO_ADDR(api_spec->mac, proto_spec.macaddress());
    pds_obj_key_proto_to_api_spec(&api_spec->vpc, proto_spec.vpcid());

    switch (proto_spec.type()) {
    case pds::TUNNEL_TYPE_IGW:
        api_spec->type = PDS_TEP_TYPE_IGW;
        break;
    case pds::TUNNEL_TYPE_WORKLOAD:
        api_spec->type = PDS_TEP_TYPE_WORKLOAD;
        break;
    case pds::TUNNEL_TYPE_INTER_DC:
        api_spec->type = PDS_TEP_TYPE_INTER_DC;
        break;
    case pds::TUNNEL_TYPE_SERVICE:
        api_spec->type = PDS_TEP_TYPE_SERVICE;
        break;
    default:
        api_spec->type = PDS_TEP_TYPE_NONE;
        break;
    }
    api_spec->encap = proto_encap_to_pds_encap(proto_spec.encap());
    api_spec->nat = proto_spec.nat();
    api_spec->remote_svc = proto_spec.remoteservice();
    if (api_spec->remote_svc) {
        api_spec->remote_svc_encap =
            proto_encap_to_pds_encap(proto_spec.remoteserviceencap());
        ipaddr_proto_spec_to_api_spec(&api_spec->remote_svc_public_ip,
                                      proto_spec.remoteservicepublicip());
    }
    switch (proto_spec.nh_case()) {
    case pds::TunnelSpec::kNexthopId:
        api_spec->nh_type = PDS_NH_TYPE_UNDERLAY;
        pds_obj_key_proto_to_api_spec(&api_spec->nh, proto_spec.nexthopid());
        break;
    case pds::TunnelSpec::kNexthopGroupId:
        api_spec->nh_type = PDS_NH_TYPE_UNDERLAY_ECMP;
        pds_obj_key_proto_to_api_spec(&api_spec->nh_group,
                                      proto_spec.nexthopgroupid());
        break;
    case pds::TunnelSpec::kTunnelId:
        api_spec->nh_type = PDS_NH_TYPE_OVERLAY;
        pds_obj_key_proto_to_api_spec(&api_spec->tep,
                                      proto_spec.tunnelid());
        break;
    default:
        PDS_TRACE_ERR("Unsupported nexthop type {} in TEP {} spec",
                      proto_spec.nh_case(), api_spec->key.id);
        return SDK_RET_INVALID_ARG;
        break;
    }
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_svc_tunnel_create (const pds::TunnelRequest *proto_req,
                       pds::TunnelResponse *proto_rsp)
{
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    pds_tep_spec_t api_spec;
    bool batched_internally = false;
    pds_batch_params_t batch_params;

    if ((proto_req == NULL) || (proto_req->request_size() == 0)) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return SDK_RET_INVALID_ARG;
    }

    // create an internal batch, if this is not part of an existing API batch
    bctxt = proto_req->batchctxt().batchcookie();
    if (bctxt == PDS_BATCH_CTXT_INVALID) {
        batch_params.epoch = core::agent_state::state()->new_epoch();
        batch_params.async = false;
        bctxt = pds_batch_start(&batch_params);
        if (bctxt == PDS_BATCH_CTXT_INVALID) {
            PDS_TRACE_ERR("Failed to create a new batch, tunnel creation "
                          "failed");
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return SDK_RET_ERR;
        }
        batched_internally = true;
    }
    for (int i = 0; i < proto_req->request_size(); i ++) {
        memset(&api_spec, 0, sizeof(pds_tep_spec_t));
        auto request = proto_req->request(i);
        pds_tep_proto_to_api_spec(&api_spec, request);
        ret = pds_tep_create(&api_spec, bctxt);
        if (ret != SDK_RET_OK) {
            goto end;
        }
    }

    if (batched_internally) {
        // commit the internal batch
        ret = pds_batch_commit(bctxt);
    }
    proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    return ret;

end:

    if (batched_internally) {
        // destroy the internal batch
        pds_batch_destroy(bctxt);
    }
    proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    return ret;
}

static inline sdk_ret_t
pds_svc_tunnel_update (const pds::TunnelRequest *proto_req,
                       pds::TunnelResponse *proto_rsp)
{
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    pds_tep_spec_t api_spec;
    bool batched_internally = false;
    pds_batch_params_t batch_params;

    if ((proto_req == NULL) || (proto_req->request_size() == 0)) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return SDK_RET_INVALID_ARG;
    }

    // create an internal batch, if this is not part of an existing API batch
    bctxt = proto_req->batchctxt().batchcookie();
    if (bctxt == PDS_BATCH_CTXT_INVALID) {
        batch_params.epoch = core::agent_state::state()->new_epoch();
        batch_params.async = false;
        bctxt = pds_batch_start(&batch_params);
        if (bctxt == PDS_BATCH_CTXT_INVALID) {
            PDS_TRACE_ERR("Failed to create a new batch, tunnel update failed");
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return SDK_RET_ERR;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->request_size(); i ++) {
        memset(&api_spec, 0, sizeof(pds_tep_spec_t));
        auto request = proto_req->request(i);
        pds_tep_proto_to_api_spec(&api_spec, request);
        ret = pds_tep_update(&api_spec, bctxt);
        if (ret != SDK_RET_OK) {
            goto end;
        }
    }

    if (batched_internally) {
        // commit the internal batch
        ret = pds_batch_commit(bctxt);
    }
    proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    return ret;

end:

    if (batched_internally) {
        // destroy the internal batch
        pds_batch_destroy(bctxt);
    }
    proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    return ret;
}

static inline sdk_ret_t
pds_svc_tunnel_delete (const pds::TunnelDeleteRequest *proto_req,
                       pds::TunnelDeleteResponse *proto_rsp)
{
    sdk_ret_t ret;
    pds_obj_key_t key;
    pds_batch_ctxt_t bctxt;
    bool batched_internally = false;
    pds_batch_params_t batch_params;

    if ((proto_req == NULL) || (proto_req->id_size() == 0)) {
        proto_rsp->add_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return SDK_RET_INVALID_ARG;
    }

    // create an internal batch, if this is not part of an existing API batch
    bctxt = proto_req->batchctxt().batchcookie();
    if (bctxt == PDS_BATCH_CTXT_INVALID) {
        batch_params.epoch = core::agent_state::state()->new_epoch();
        batch_params.async = false;
        bctxt = pds_batch_start(&batch_params);
        if (bctxt == PDS_BATCH_CTXT_INVALID) {
            PDS_TRACE_ERR("Failed to create a new batch, vpc delete failed");
            proto_rsp->add_apistatus(types::ApiStatus::API_STATUS_ERR);
            return SDK_RET_ERR;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->id_size(); i++) {
        pds_obj_key_proto_to_api_spec(&key, proto_req->id(i));
        ret = pds_tep_delete(&key, bctxt);
        if (ret != SDK_RET_OK) {
            goto end;
        }
    }

    if (batched_internally) {
        // commit the internal batch
        ret = pds_batch_commit(bctxt);
    }
    proto_rsp->add_apistatus(sdk_ret_to_api_status(ret));
    return ret;

end:

    if (batched_internally) {
        // destroy the internal batch
        pds_batch_destroy(bctxt);
    }
    proto_rsp->add_apistatus(sdk_ret_to_api_status(ret));
    return ret;
}

static inline sdk_ret_t
pds_svc_tunnel_get (const pds::TunnelGetRequest *proto_req,
                    pds::TunnelGetResponse *proto_rsp)
{
    sdk_ret_t ret;
    pds_obj_key_t key;
    pds_tep_info_t info = {0};

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return SDK_RET_INVALID_ARG;
    }
    if (proto_req->id_size() == 0) {
        ret = pds_tep_read_all(pds_tep_api_info_to_proto, proto_rsp);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    }
    for (int i = 0; i < proto_req->id_size(); i++) {
        pds_obj_key_proto_to_api_spec(&key, proto_req->id(i));
        ret = pds_tep_read(&key, &info);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        if (ret != SDK_RET_OK) {
            break;
        }
        pds_tep_api_info_to_proto(&info, proto_rsp);
    }
    return ret;
}

static inline sdk_ret_t
pds_svc_tunnel_handle_cfg (cfg_ctxt_t *ctxt, google::protobuf::Any *any_resp)
{
    sdk_ret_t ret;
    google::protobuf::Any *any_req = (google::protobuf::Any *)ctxt->req;

    switch (ctxt->cfg) {
    case CFG_MSG_TUNNEL_CREATE:
        {
            pds::TunnelRequest req;
            pds::TunnelResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_tunnel_create(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    case CFG_MSG_TUNNEL_UPDATE:
        {
            pds::TunnelRequest req;
            pds::TunnelResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_tunnel_update(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    case CFG_MSG_TUNNEL_DELETE:
        {
            pds::TunnelDeleteRequest req;
            pds::TunnelDeleteResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_tunnel_delete(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    case CFG_MSG_TUNNEL_GET:
        {
            pds::TunnelGetRequest req;
            pds::TunnelGetResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_tunnel_get(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    default:
        ret = SDK_RET_INVALID_ARG;
        break;
    }

    return ret;
}

#endif    //__AGENT_SVC_TUNNEL_SVC_HPP__
