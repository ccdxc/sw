//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines protobuf API for vpc object
///
//----------------------------------------------------------------------------

#ifndef __AGENT_SVC_VPC_SVC_HPP__
#define __AGENT_SVC_VPC_SVC_HPP__

#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/api/include/pds_vpc.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/svc/vpc.hpp"
#include "nic/apollo/agent/trace.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_vpc.hpp"

// build VPC API spec from protobuf spec
static inline void
pds_vpc_proto_to_api_spec (pds_vpc_spec_t *api_spec,
                           const pds::VPCSpec &proto_spec)
{
    pds::VPCType type;

    pds_obj_key_proto_to_api_spec(&api_spec->key, proto_spec.id());
    type = proto_spec.type();
    if (type == pds::VPC_TYPE_TENANT) {
        api_spec->type = PDS_VPC_TYPE_TENANT;
    } else if (type == pds::VPC_TYPE_UNDERLAY) {
        api_spec->type = PDS_VPC_TYPE_UNDERLAY;
    } else if (type == pds::VPC_TYPE_CONTROL) {
        api_spec->type = PDS_VPC_TYPE_CONTROL;
    }
    ipv6pfx_proto_spec_to_ippfx_api_spec(&api_spec->nat46_prefix,
                                         proto_spec.nat46prefix());
    api_spec->fabric_encap = proto_encap_to_pds_encap(proto_spec.fabricencap());
    MAC_UINT64_TO_ADDR(api_spec->vr_mac, proto_spec.virtualroutermac());
    pds_obj_key_proto_to_api_spec(&api_spec->v4_route_table,
                                  proto_spec.v4routetableid());
    pds_obj_key_proto_to_api_spec(&api_spec->v6_route_table,
                                  proto_spec.v6routetableid());
    api_spec->tos = proto_spec.tos();
}

// populate proto buf spec from vpc API spec
static inline void
pds_vpc_api_spec_to_proto (pds::VPCSpec *proto_spec,
                           const pds_vpc_spec_t *api_spec)
{
    if (!api_spec || !proto_spec) {
        return;
    }
    proto_spec->set_id(api_spec->key.id, PDS_MAX_KEY_LEN);
    if (api_spec->type == PDS_VPC_TYPE_TENANT) {
        proto_spec->set_type(pds::VPC_TYPE_TENANT);
    } else if (api_spec->type == PDS_VPC_TYPE_UNDERLAY) {
        proto_spec->set_type(pds::VPC_TYPE_UNDERLAY);
    } else if (api_spec->type == PDS_VPC_TYPE_CONTROL) {
        proto_spec->set_type(pds::VPC_TYPE_CONTROL);
    }
    ippfx_api_spec_to_ipv6pfx_proto_spec(proto_spec->mutable_nat46prefix(),
                                         &api_spec->nat46_prefix);
    pds_encap_to_proto_encap(proto_spec->mutable_fabricencap(),
                             &api_spec->fabric_encap);
    proto_spec->set_virtualroutermac(MAC_TO_UINT64(api_spec->vr_mac));
    proto_spec->set_v4routetableid(api_spec->v4_route_table.id,
                                   PDS_MAX_KEY_LEN);
    proto_spec->set_v6routetableid(api_spec->v6_route_table.id,
                                   PDS_MAX_KEY_LEN);
    proto_spec->set_tos(api_spec->tos);
}

// populate proto buf status from vpc API status
static inline void
pds_vpc_api_status_to_proto (pds::VPCStatus *proto_status,
                             const pds_vpc_status_t *api_status)
{
    proto_status->set_hwid(api_status->hw_id);
}

// populate proto buf stats from vpc API stats
static inline void
pds_vpc_api_stats_to_proto (pds::VPCStats *proto_stats,
                            const pds_vpc_stats_t *api_stats)
{
}

// populate proto buf from vpc API info
static inline void
pds_vpc_api_info_to_proto (pds_vpc_info_t *api_info, void *ctxt)
{
    pds::VPCGetResponse *proto_rsp = (pds::VPCGetResponse *)ctxt;
    auto vpc = proto_rsp->add_response();
    pds::VPCSpec *proto_spec = vpc->mutable_spec();
    pds::VPCStatus *proto_status = vpc->mutable_status();
    pds::VPCStats *proto_stats = vpc->mutable_stats();

    pds_vpc_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_vpc_api_status_to_proto(proto_status, &api_info->status);
    pds_vpc_api_stats_to_proto(proto_stats, &api_info->stats);
}

// populate VPCPeer protobuf spec from VPCPeer API spec
static inline void
pds_vpc_peer_api_spec_to_proto (pds::VPCPeerSpec *proto_spec,
                                const pds_vpc_peer_spec_t *api_spec)
{
    if (!api_spec || !proto_spec) {
        return;
    }
    proto_spec->set_id(api_spec->key.id, PDS_MAX_KEY_LEN);
    proto_spec->set_vpc1(api_spec->vpc1.id, PDS_MAX_KEY_LEN);
    proto_spec->set_vpc2(api_spec->vpc2.id, PDS_MAX_KEY_LEN);
}

// populate proto buf status from vpc API status
static inline void
pds_vpc_peer_api_status_to_proto (pds::VPCPeerStatus *proto_status,
                                  const pds_vpc_peer_status_t *api_status)
{
}

// populate proto buf stats from vpc API stats
static inline void
pds_vpc_peer_api_stats_to_proto (pds::VPCPeerStats *proto_stats,
                                 const pds_vpc_peer_stats_t *api_stats)
{
}

// populate proto buf from vpc API info
static inline void
pds_vpc_peer_api_info_to_proto (pds_vpc_peer_info_t *api_info, void *ctxt)
{
    pds::VPCPeerGetResponse *proto_rsp = (pds::VPCPeerGetResponse *)ctxt;
    auto vpc = proto_rsp->add_response();
    pds::VPCPeerSpec *proto_spec = vpc->mutable_spec();
    pds::VPCPeerStatus *proto_status = vpc->mutable_status();
    pds::VPCPeerStats *proto_stats = vpc->mutable_stats();

    pds_vpc_peer_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_vpc_peer_api_status_to_proto(proto_status, &api_info->status);
    pds_vpc_peer_api_stats_to_proto(proto_stats, &api_info->stats);
}

// populate VPCPeer api spec from VPCPeer proto spec
static inline void
pds_vpc_peer_proto_to_api_spec (pds_vpc_peer_spec_t *api_spec,
                                const pds::VPCPeerSpec &proto_spec)
{
    if (!api_spec) {
        return;
    }
    pds_obj_key_proto_to_api_spec(&api_spec->key, proto_spec.id());
    pds_obj_key_proto_to_api_spec(&api_spec->vpc1, proto_spec.vpc1());
    pds_obj_key_proto_to_api_spec(&api_spec->vpc2, proto_spec.vpc2());
}

static inline sdk_ret_t
pds_svc_vpc_create (const pds::VPCRequest *proto_req,
                    pds::VPCResponse *proto_rsp)
{
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    pds_vpc_spec_t api_spec;
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
            PDS_TRACE_ERR("Failed to create a new batch, vpc creation failed");
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return SDK_RET_ERR;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->request_size(); i ++) {
        memset(&api_spec, 0, sizeof(pds_vpc_spec_t));
        auto proto_spec = proto_req->request(i);
        pds_vpc_proto_to_api_spec(&api_spec, proto_spec);
        // underlay VPC is always sent to control-plane
        if (core::agent_state::state()->device()->overlay_routing_en ||
           (api_spec.type == PDS_VPC_TYPE_UNDERLAY)) {
            // call the metaswitch api
            ret = pds_ms::vpc_create(&api_spec, bctxt);
        } else if (!core::agent_state::state()->pds_mock_mode()) {
            ret = pds_vpc_create(&api_spec, bctxt);
        }
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
pds_svc_vpc_update (const pds::VPCRequest *proto_req,
                    pds::VPCResponse *proto_rsp)
{
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    pds_vpc_spec_t api_spec;
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
            PDS_TRACE_ERR("Failed to create a new batch, vpc update failed");
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return SDK_RET_ERR;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->request_size(); i ++) {
        memset(&api_spec, 0, sizeof(pds_vpc_spec_t));
        auto proto_spec = proto_req->request(i);
        pds_vpc_proto_to_api_spec(&api_spec, proto_spec);
        // underlay VPC is always sent to control-plane
        if (core::agent_state::state()->device()->overlay_routing_en ||
           (api_spec.type == PDS_VPC_TYPE_UNDERLAY)) {
            // call the metaswitch api
            ret = pds_ms::vpc_update(&api_spec, bctxt);
        } else if (!core::agent_state::state()->pds_mock_mode()) {
            ret = pds_vpc_update(&api_spec, bctxt);
        }
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
pds_svc_vpc_delete (const pds::VPCDeleteRequest *proto_req,
                    pds::VPCDeleteResponse *proto_rsp)
{
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    pds_obj_key_t key = { 0 };
    pds_vpc_info_t info = { 0 };
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
            PDS_TRACE_ERR("Failed to create a new batch, vpc deletion failed");
            proto_rsp->add_apistatus(types::ApiStatus::API_STATUS_ERR);
            return SDK_RET_ERR;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->id_size(); i++) {
        pds_obj_key_proto_to_api_spec(&key, proto_req->id(i));
        ret = pds_vpc_read(&key, &info);
        if (ret != SDK_RET_OK) {
            goto end;
        }
        // underlay VPC is always sent to control-plane
        if (core::agent_state::state()->device()->overlay_routing_en ||
           (info.spec.type == PDS_VPC_TYPE_UNDERLAY)) {
            // call the metaswitch api
            ret = pds_ms::vpc_delete(&info.spec, bctxt);
        } else if (!core::agent_state::state()->pds_mock_mode()) {
            ret = pds_vpc_delete(&key, bctxt);
        }
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
pds_svc_vpc_get (const pds::VPCGetRequest *proto_req,
                 pds::VPCGetResponse *proto_rsp)
{
    sdk_ret_t ret;
    pds_obj_key_t key;
    pds_vpc_info_t info = { 0 };

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return SDK_RET_INVALID_ARG;
    }

    for (int i = 0; i < proto_req->id_size(); i ++) {
        pds_obj_key_proto_to_api_spec(&key, proto_req->id(i));
        ret = pds_vpc_read(&key, &info);
        if (ret != SDK_RET_OK) {
            proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
            break;
        }
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OK);
        pds_vpc_api_info_to_proto(&info, proto_rsp);
    }

    if (proto_req->id_size() == 0) {
        ret = pds_vpc_read_all(pds_vpc_api_info_to_proto, proto_rsp);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    }

    return ret;
}

static inline sdk_ret_t
pds_svc_vpc_handle_cfg (cfg_ctxt_t *ctxt, google::protobuf::Any *any_resp)
{
    sdk_ret_t ret;
    google::protobuf::Any *any_req = (google::protobuf::Any *)ctxt->req;

    switch (ctxt->cfg) {
    case CFG_MSG_VPC_CREATE:
        {
            pds::VPCRequest req;
            pds::VPCResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_vpc_create(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    case CFG_MSG_VPC_UPDATE:
        {
            pds::VPCRequest req;
            pds::VPCResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_vpc_update(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    case CFG_MSG_VPC_DELETE:
        {
            pds::VPCDeleteRequest req;
            pds::VPCDeleteResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_vpc_delete(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    case CFG_MSG_VPC_GET:
        {
            pds::VPCGetRequest req;
            pds::VPCGetResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_vpc_get(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    default:
        ret = SDK_RET_INVALID_ARG;
        break;
    }

    return ret;
}

static inline sdk_ret_t
pds_svc_vpc_peer_create (const pds::VPCPeerRequest *proto_req,
                         pds::VPCPeerResponse *proto_rsp)
{
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    pds_vpc_peer_spec_t api_spec;
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
            PDS_TRACE_ERR("Failed to create a new batch, vpc peer creation "
                          "failed");
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return SDK_RET_ERR;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->request_size(); i ++) {
        memset(&api_spec, 0, sizeof(pds_vpc_peer_spec_t));
        auto proto_spec = proto_req->request(i);
        pds_vpc_peer_proto_to_api_spec(&api_spec, proto_spec);
        ret = pds_vpc_peer_create(&api_spec, bctxt);
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
pds_svc_vpc_peer_delete (const pds::VPCPeerDeleteRequest *proto_req,
                         pds::VPCPeerDeleteResponse *proto_rsp)
{
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    pds_obj_key_t key;
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
            PDS_TRACE_ERR("Failed to create a new batch, vpc peer delete "
                          "failed");
            proto_rsp->add_apistatus(types::ApiStatus::API_STATUS_ERR);
            return SDK_RET_ERR;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->id_size(); i++) {
        pds_obj_key_proto_to_api_spec(&key, proto_req->id(i));
        ret = pds_vpc_peer_delete(&key, bctxt);
        proto_rsp->add_apistatus(sdk_ret_to_api_status(ret));
        if (ret != SDK_RET_OK) {
            goto end;
        }
    }

    if (batched_internally) {
        // commit the internal batch
        pds_batch_commit(bctxt);
    }
    return ret;

end:

    if (batched_internally) {
        // destroy the internal batch
        pds_batch_destroy(bctxt);
    }
    return ret;
}

static inline sdk_ret_t
pds_svc_vpc_peer_get (const pds::VPCPeerGetRequest *proto_req,
                      pds::VPCPeerGetResponse *proto_rsp)
{
    sdk_ret_t ret;
    pds_obj_key_t key;
    pds_vpc_peer_info_t info = { 0 };

    PDS_TRACE_VERBOSE("VPCPeer Get Received")

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return SDK_RET_INVALID_ARG;
    }

    for (int i = 0; i < proto_req->id_size(); i ++) {
        pds_obj_key_proto_to_api_spec(&key, proto_req->id(i));
        ret = pds_vpc_peer_read(&key, &info);
        if (ret != SDK_RET_OK) {
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_NOT_FOUND);
            break;
        }
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OK);
        pds_vpc_peer_api_info_to_proto(&info, proto_rsp);
    }

    if (proto_req->id_size() == 0) {
        ret = pds_vpc_peer_read_all(pds_vpc_peer_api_info_to_proto, proto_rsp);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    }

    return ret;
}

static inline sdk_ret_t
pds_svc_vpc_peer_handle_cfg (cfg_ctxt_t *ctxt, google::protobuf::Any *any_resp)
{
    sdk_ret_t ret;
    google::protobuf::Any *any_req = (google::protobuf::Any *)ctxt->req;

    switch (ctxt->cfg) {
    case CFG_MSG_VPC_PEER_CREATE:
        {
            pds::VPCPeerRequest req;
            pds::VPCPeerResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_vpc_peer_create(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    case CFG_MSG_VPC_PEER_DELETE:
        {
            pds::VPCPeerDeleteRequest req;
            pds::VPCPeerDeleteResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_vpc_peer_delete(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    case CFG_MSG_VPC_PEER_GET:
        {
            pds::VPCPeerGetRequest req;
            pds::VPCPeerGetResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_vpc_peer_get(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    default:
        ret = SDK_RET_INVALID_ARG;
        break;
    }

    return ret;
}

#endif    // __AGENT_SVC_VPC_HPP__
