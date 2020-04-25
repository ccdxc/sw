//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines protobuf API for policer object
///
//----------------------------------------------------------------------------

#ifndef __AGENT_SVC_POLICER_SVC_HPP__
#define __AGENT_SVC_POLICER_SVC_HPP__

#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/svc/policer.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/api/include/pds_policer.hpp"

static inline pds::PolicerDir
pds_policer_dir_api_spec_to_proto (pds_policer_dir_t dir)
{
    switch (dir) {
    case PDS_POLICER_DIR_INGRESS:
        return pds::POLICER_DIR_INGRESS;
    case PDS_POLICER_DIR_EGRESS:
        return pds::POLICER_DIR_EGRESS;
    default:
        return pds::POLICER_DIR_NONE;
    }
}

static inline pds_policer_dir_t
pds_policer_dir_proto_to_api_spec (pds::PolicerDir dir)
{
    switch (dir) {
    case pds::POLICER_DIR_INGRESS:
        return PDS_POLICER_DIR_INGRESS;
    case pds::POLICER_DIR_EGRESS:
        return PDS_POLICER_DIR_EGRESS;
    default:
        return PDS_POLICER_DIR_NONE;
    }
}

// build policer api spec from proto buf spec
static inline sdk_ret_t
pds_policer_proto_to_api_spec (pds_policer_spec_t *api_spec,
                               const pds::PolicerSpec &proto_spec)
{
    pds_obj_key_proto_to_api_spec(&api_spec->key, proto_spec.id());
    api_spec->dir = pds_policer_dir_proto_to_api_spec(proto_spec.direction());
    if (proto_spec.has_ppspolicer()) {
        api_spec->type = sdk::qos::POLICER_TYPE_PPS;
        api_spec->pps = proto_spec.ppspolicer().packetspersecond();
        api_spec->pps_burst = proto_spec.ppspolicer().burst();
    } else if (proto_spec.has_bpspolicer()) {
        api_spec->type = sdk::qos::POLICER_TYPE_BPS;
        api_spec->bps = proto_spec.bpspolicer().bytespersecond();
        api_spec->bps_burst = proto_spec.bpspolicer().burst();
    }
    return SDK_RET_OK;
}

// populate proto buf spec from policer API spec
static inline void
pds_policer_api_spec_to_proto (pds::PolicerSpec *proto_spec,
                               const pds_policer_spec_t *api_spec)
{
    if (!api_spec || !proto_spec) {
        return;
    }
    proto_spec->set_id(api_spec->key.id, PDS_MAX_KEY_LEN);
    proto_spec->set_direction(pds_policer_dir_api_spec_to_proto(api_spec->dir));
    switch (api_spec->type) {
    case sdk::qos::POLICER_TYPE_PPS:
        {
            auto pps = proto_spec->mutable_ppspolicer();
            pps->set_packetspersecond(api_spec->pps);
            pps->set_burst(api_spec->pps_burst);
        }
        break;
    case sdk::qos::POLICER_TYPE_BPS:
        {
            auto bps = proto_spec->mutable_bpspolicer();
            bps->set_bytespersecond(api_spec->bps);
            bps->set_burst(api_spec->bps_burst);
        }
        break;
    default:
        break;
    }
}

// populate proto buf status from policer API status
static inline void
pds_policer_api_status_to_proto (pds::PolicerStatus *proto_status,
                                 const pds_policer_status_t *api_status)
{
}

// populate proto buf stats from policer API stats
static inline void
pds_policer_api_stats_to_proto (pds::PolicerStats *proto_stats,
                                const pds_policer_stats_t *api_stats)
{
}

// populate proto buf from policer API info
static inline void
pds_policer_api_info_to_proto (pds_policer_info_t *api_info, void *ctxt)
{
    pds::PolicerGetResponse *proto_rsp = (pds::PolicerGetResponse *)ctxt;
    auto policer = proto_rsp->add_response();
    pds::PolicerSpec *proto_spec = policer->mutable_spec();
    pds::PolicerStatus *proto_status = policer->mutable_status();
    pds::PolicerStats *proto_stats = policer->mutable_stats();

    pds_policer_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_policer_api_status_to_proto(proto_status, &api_info->status);
    pds_policer_api_stats_to_proto(proto_stats, &api_info->stats);
}

static inline sdk_ret_t
pds_svc_policer_create (const pds::PolicerRequest *proto_req,
                        pds::PolicerResponse *proto_rsp)
{
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    pds_policer_spec_t api_spec;
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
            PDS_TRACE_ERR("Failed to create a new batch, policer creation failed");
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return SDK_RET_ERR;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->request_size(); i ++) {
        memset(&api_spec, 0, sizeof(api_spec));
        auto request = proto_req->request(i);
        pds_policer_proto_to_api_spec(&api_spec, request);
        if (!core::agent_state::state()->pds_mock_mode()) {
            ret = pds_policer_create(&api_spec, bctxt);
            if (ret != SDK_RET_OK) {
                goto end;
            }
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
pds_svc_policer_update (const pds::PolicerRequest *proto_req,
                        pds::PolicerResponse *proto_rsp)
{
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    pds_policer_spec_t api_spec;
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
            PDS_TRACE_ERR("Failed to create a new batch, policer update failed");
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return SDK_RET_ERR;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->request_size(); i ++) {
        memset(&api_spec, 0, sizeof(api_spec));
        auto request = proto_req->request(i);
        pds_policer_proto_to_api_spec(&api_spec, request);
        if (!core::agent_state::state()->pds_mock_mode()) {
            ret = pds_policer_update(&api_spec, bctxt);
            if (ret != SDK_RET_OK) {
                goto end;
            }
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
pds_svc_policer_delete (const pds::PolicerDeleteRequest *proto_req,
                        pds::PolicerDeleteResponse *proto_rsp)
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
            PDS_TRACE_ERR("Failed to create a new batch, policer deletion failed");
            return SDK_RET_ERR;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->id_size(); i++) {
        pds_obj_key_proto_to_api_spec(&key, proto_req->id(i));
        ret = pds_policer_delete(&key, bctxt);
        proto_rsp->add_apistatus(sdk_ret_to_api_status(ret));
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
pds_svc_policer_get (const pds::PolicerGetRequest *proto_req,
                     pds::PolicerGetResponse *proto_rsp)
{
    sdk_ret_t ret;
    pds_obj_key_t key;
    pds_policer_info_t info = { 0 };

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return SDK_RET_INVALID_ARG;
    }
    if (proto_req->id_size() == 0) {
        // get all
        ret = pds_policer_read_all(pds_policer_api_info_to_proto, proto_rsp);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    }
    for (int i = 0; i < proto_req->id_size(); i++) {
        pds_obj_key_proto_to_api_spec(&key, proto_req->id(i));
        ret = pds_policer_read(&key, &info);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        if (ret != SDK_RET_OK) {
            break;
        }
        pds_policer_api_info_to_proto(&info, proto_rsp);
    }
    return ret;
}

static inline sdk_ret_t
pds_svc_policer_handle_cfg (cfg_ctxt_t *ctxt, google::protobuf::Any *any_resp)
{
    sdk_ret_t ret;
    google::protobuf::Any *any_req = (google::protobuf::Any *)ctxt->req;

    switch (ctxt->cfg) {
    case CFG_MSG_POLICER_CREATE:
        {
            pds::PolicerRequest req;
            pds::PolicerResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_policer_create(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    case CFG_MSG_POLICER_UPDATE:
        {
            pds::PolicerRequest req;
            pds::PolicerResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_policer_update(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    case CFG_MSG_POLICER_DELETE:
        {
            pds::PolicerDeleteRequest req;
            pds::PolicerDeleteResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_policer_delete(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    case CFG_MSG_POLICER_GET:
        {
            pds::PolicerGetRequest req;
            pds::PolicerGetResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_policer_get(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    default:
        ret = SDK_RET_INVALID_ARG;
        break;
    }

    return ret;
}

#endif    //__AGENT_SVC_POLICER_SVC_HPP__
