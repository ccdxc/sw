//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines protobuf API for nh object
///
//----------------------------------------------------------------------------

#ifndef __AGENT_SVC_NH_SVC_HPP__
#define __AGENT_SVC_NH_SVC_HPP__

#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/api/include/pds_nexthop.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/nh.hpp"
#include "nic/apollo/agent/core/nh_group.hpp"
#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/svc/nh.hpp"
#include "nic/apollo/agent/trace.hpp"

static inline pds_nh_type_t
proto_nh_type_to_pds_nh_type (pds::NexthopType type)
{
    switch (type) {
    case pds::NEXTHOP_TYPE_IP:
        return PDS_NH_TYPE_IP;
    case pds::NEXTHOP_TYPE_UNDERLAY:
        return PDS_NH_TYPE_UNDERLAY;
    case pds::NEXTHOP_TYPE_OVERLAY:
        return PDS_NH_TYPE_OVERLAY;
    default:
        return PDS_NH_TYPE_NONE;
    }
}

// build nh API spec from protobuf spec
static inline void
pds_nh_proto_to_api_spec (pds_nexthop_spec_t *api_spec,
                          const pds::NexthopSpec &proto_spec)
{
    pds_obj_key_proto_to_api_spec(&api_spec->key, proto_spec.id());
    switch (proto_spec.nhinfo_case()) {
    case pds::NexthopSpec::kIPNhInfo:
        api_spec->type = PDS_NH_TYPE_IP;
        pds_obj_key_proto_to_api_spec(&api_spec->vpc,
                                      proto_spec.ipnhinfo().vpcid());
        ipaddr_proto_spec_to_api_spec(&api_spec->ip,
                                      proto_spec.ipnhinfo().ip());
        api_spec->vlan = proto_spec.ipnhinfo().vlan();
        if (proto_spec.ipnhinfo().mac() != 0) {
            MAC_UINT64_TO_ADDR(api_spec->mac, proto_spec.ipnhinfo().mac());
        }
        break;

    case pds::NexthopSpec::kOverlayNhInfo:
        api_spec->type = PDS_NH_TYPE_OVERLAY;
        pds_obj_key_proto_to_api_spec(&api_spec->tep, proto_spec.overlaynhinfo().tunnelid());
        break;

    case pds::NexthopSpec::kUnderlayNhInfo:
        api_spec->type = PDS_NH_TYPE_UNDERLAY;
        pds_obj_key_proto_to_api_spec(&api_spec->l3_if,
                                      proto_spec.underlaynhinfo().l3interface());
        MAC_UINT64_TO_ADDR(api_spec->underlay_mac,
                           proto_spec.underlaynhinfo().underlaymac());
        break;

    default:
         api_spec->type = PDS_NH_TYPE_NONE;
         break;
    }
}

// populate proto buf spec from nh API spec
static inline void
pds_nh_api_spec_to_proto (pds::NexthopSpec *proto_spec,
                          const pds_nexthop_spec_t *api_spec)
{
    proto_spec->set_id(api_spec->key.id, PDS_MAX_KEY_LEN);
    if (api_spec->type == PDS_NH_TYPE_IP) {
        auto ipnhinfo = proto_spec->mutable_ipnhinfo();
        ipnhinfo->set_vpcid(api_spec->vpc.id, PDS_MAX_KEY_LEN);
        ipaddr_api_spec_to_proto_spec(ipnhinfo->mutable_ip(), &api_spec->ip);
        ipnhinfo->set_vlan(api_spec->vlan);
        ipnhinfo->set_mac(MAC_TO_UINT64(api_spec->mac));
    } else if (api_spec->type == PDS_NH_TYPE_OVERLAY) {
        auto overlayinfo = proto_spec->mutable_overlaynhinfo();
        overlayinfo->set_tunnelid(api_spec->tep.id, PDS_MAX_KEY_LEN);
    } else if (api_spec->type == PDS_NH_TYPE_UNDERLAY) {
        auto underlayinfo = proto_spec->mutable_underlaynhinfo();
        underlayinfo->set_l3interface(api_spec->l3_if.id, PDS_MAX_KEY_LEN);
        underlayinfo->set_underlaymac(MAC_TO_UINT64(api_spec->underlay_mac));
    }
}

// populate proto buf status from nh API status
static inline void
pds_nh_api_status_to_proto (pds::NexthopStatus *proto_status,
                            const pds_nexthop_status_t *api_status,
                            const pds_nexthop_spec_t *api_spec)
{
    proto_status->set_hwid(api_status->hw_id);
    switch (api_spec->type) {
    case PDS_NH_TYPE_UNDERLAY:
        {
            auto status = proto_status->mutable_underlaynhinfo();
            status->set_port(api_status->port);
            status->set_vlan(api_status->vlan);
        }
        break;
    case PDS_NH_TYPE_OVERLAY:
        {
            auto status = proto_status->mutable_overlaynhinfo();
            ipaddr_api_spec_to_proto_spec(status->mutable_tunnelip(), &api_status->tep_ip);
        }
    default:
        break;
    }
}

// populate proto buf stats from nh API stats
static inline void
pds_nh_api_stats_to_proto (pds::NexthopStats *proto_stats,
                           const pds_nexthop_stats_t *api_stats)
{
}

// populate proto buf from nh API info
static inline void
pds_nh_api_info_to_proto (pds_nexthop_info_t *api_info, void *ctxt)
{
    nh_get_all_args_t *args = (nh_get_all_args_t *)ctxt;
    pds::NexthopGetResponse *proto_rsp = (pds::NexthopGetResponse *)(args->ctxt);

    if (api_info->spec.type != args->type) {
        return;
    }

    auto nh = proto_rsp->add_response();
    pds::NexthopSpec *proto_spec = nh->mutable_spec();
    pds::NexthopStatus *proto_status = nh->mutable_status();
    pds::NexthopStats *proto_stats = nh->mutable_stats();

    pds_nh_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_nh_api_status_to_proto(proto_status, &api_info->status, &api_info->spec);
    pds_nh_api_stats_to_proto(proto_stats, &api_info->stats);
}

static inline pds_nexthop_group_type_t
proto_nh_group_type_to_pds_nh_group_type (pds::NhGroupType type)
{
    switch (type) {
    case pds::NEXTHOP_GROUP_TYPE_OVERLAY_ECMP:
        return PDS_NHGROUP_TYPE_OVERLAY_ECMP;
    case pds::NEXTHOP_GROUP_TYPE_UNDERLAY_ECMP:
        return PDS_NHGROUP_TYPE_UNDERLAY_ECMP;
    default:
        return PDS_NHGROUP_TYPE_NONE;
    }
}

// build nh group API spec from protobuf spec
static inline sdk_ret_t
pds_nh_group_proto_to_api_spec (pds_nexthop_group_spec_t *api_spec,
                                const pds::NhGroupSpec &proto_spec)
{
    pds_obj_key_proto_to_api_spec(&api_spec->key, proto_spec.id());
    api_spec->num_nexthops = proto_spec.members_size();
    switch (proto_spec.type()) {
    case pds::NEXTHOP_GROUP_TYPE_OVERLAY_ECMP:
        if (api_spec->num_nexthops > PDS_MAX_OVERLAY_ECMP_TEP) {
            return SDK_RET_INVALID_ARG;
        }
        api_spec->type = PDS_NHGROUP_TYPE_OVERLAY_ECMP;
        break;

    case pds::NEXTHOP_GROUP_TYPE_UNDERLAY_ECMP:
        if (api_spec->num_nexthops > PDS_MAX_ECMP_NEXTHOP) {
            return SDK_RET_INVALID_ARG;
        }
        api_spec->type = PDS_NHGROUP_TYPE_UNDERLAY_ECMP;
        break;

    default:
        return SDK_RET_INVALID_ARG;
    }
    for (uint32_t i = 0; i < api_spec->num_nexthops; i++) {
        pds_nh_proto_to_api_spec(&api_spec->nexthops[i],
                                 proto_spec.members(i));
    }
    return SDK_RET_OK;
}

// populate proto buf spec from nh API spec
static inline sdk_ret_t
pds_nh_group_api_spec_to_proto (pds::NhGroupSpec *proto_spec,
                                const pds_nexthop_group_spec_t *api_spec)
{
    proto_spec->set_id(api_spec->key.id, PDS_MAX_KEY_LEN);
    switch (api_spec->type) {
    case PDS_NHGROUP_TYPE_OVERLAY_ECMP:
        proto_spec->set_type(pds::NEXTHOP_GROUP_TYPE_OVERLAY_ECMP);
        break;

    case PDS_NHGROUP_TYPE_UNDERLAY_ECMP:
        proto_spec->set_type(pds::NEXTHOP_GROUP_TYPE_UNDERLAY_ECMP);
        break;

    default:
        return SDK_RET_INVALID_ARG;
    }

    for (uint32_t i = 0; i < api_spec->num_nexthops; i++) {
        pds_nh_api_spec_to_proto(proto_spec->add_members(),
                                 &api_spec->nexthops[i]);
    }
    return SDK_RET_OK;
}

// populate proto buf status from nh group API status
static inline void
pds_nh_group_api_status_to_proto (pds::NhGroupStatus *proto_status,
                                  const pds_nexthop_group_status_t *api_status,
                                  const pds_nexthop_group_spec_t *api_spec)
{
    proto_status->set_hwid(api_status->hw_id);

    for (uint32_t i = 0; i < api_spec->num_nexthops; i++) {
        pds_nh_api_status_to_proto(proto_status->add_members(),
                                   &api_status->nexthops[i],
                                   &api_spec->nexthops[i]);
    }
}

// populate proto buf stats from nh group API stats
static inline void
pds_nh_group_api_stats_to_proto (pds::NhGroupStats *proto_stats,
                                 const pds_nexthop_group_stats_t *api_stats)
{
}

// populate proto buf from nh API info
static inline void
pds_nh_group_api_info_to_proto (pds_nexthop_group_info_t *api_info,
                                void *ctxt)
{
    nh_group_get_all_args_t *args = (nh_group_get_all_args_t *)ctxt;
    pds::NhGroupGetResponse *proto_rsp = (pds::NhGroupGetResponse *)(args->ctxt);

    if (api_info->spec.type != args->type) {
        return;
    }

    auto nh = proto_rsp->add_response();
    pds::NhGroupSpec *proto_spec = nh->mutable_spec();
    pds::NhGroupStatus *proto_status = nh->mutable_status();
    pds::NhGroupStats *proto_stats = nh->mutable_stats();

    pds_nh_group_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_nh_group_api_status_to_proto(proto_status, &api_info->status, &api_info->spec);
    pds_nh_group_api_stats_to_proto(proto_stats, &api_info->stats);
}

static inline sdk_ret_t
pds_svc_nexthop_create (const pds::NexthopRequest *proto_req,
                        pds::NexthopResponse *proto_rsp)
{
    sdk_ret_t ret;
    pds_obj_key_t key;
    pds_batch_ctxt_t bctxt;
    pds_nexthop_spec_t api_spec;
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
            PDS_TRACE_ERR("Failed to create a new batch, nexthop creation "
                          "failed");
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return SDK_RET_ERR;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->request_size(); i ++) {
        memset(&api_spec, 0, sizeof(pds_nexthop_spec_t));
        auto proto_spec = proto_req->request(i);
        pds_obj_key_proto_to_api_spec(&key, proto_spec.id());
        pds_nh_proto_to_api_spec(&api_spec, proto_spec);
        ret = core::nh_create(&key, &api_spec, bctxt);
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
pds_svc_nexthop_update (const pds::NexthopRequest *proto_req,
                        pds::NexthopResponse *proto_rsp)
{
    sdk_ret_t ret;
    pds_obj_key_t key;
    pds_batch_ctxt_t bctxt;
    pds_nexthop_spec_t api_spec;
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
            PDS_TRACE_ERR("Failed to create a new batch, nexthop update "
                          "failed");
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return SDK_RET_ERR;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->request_size(); i ++) {
        memset(&api_spec, 0, sizeof(pds_nexthop_spec_t));
        auto proto_spec = proto_req->request(i);
        pds_obj_key_proto_to_api_spec(&key, proto_spec.id());
        pds_nh_proto_to_api_spec(&api_spec, proto_spec);
        ret = core::nh_update(&key, &api_spec, bctxt);
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
pds_svc_nexthop_delete (const pds::NexthopDeleteRequest *proto_req,
                        pds::NexthopDeleteResponse *proto_rsp)
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
            PDS_TRACE_ERR("Failed to create a new batch, nexthop delete "
                          "failed");
            proto_rsp->add_apistatus(types::ApiStatus::API_STATUS_ERR);
            return SDK_RET_ERR;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->id_size(); i++) {
        pds_obj_key_proto_to_api_spec(&key, proto_req->id(i));
        ret = core::nh_delete(&key, bctxt);
        proto_rsp->add_apistatus(sdk_ret_to_api_status(ret));
        if (ret != SDK_RET_OK) {
            goto end;
        }
    }

    if (batched_internally) {
        // commit the internal batch
        ret = pds_batch_commit(bctxt);
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
pds_svc_nexthop_get (const pds::NexthopGetRequest *proto_req,
                        pds::NexthopGetResponse *proto_rsp)
{
    sdk_ret_t ret;
    pds_obj_key_t key = { 0 };
    pds_nexthop_info_t info = { 0 };

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return SDK_RET_INVALID_ARG;
    }

    switch (proto_req->gettype_case()) {
    case pds::NexthopGetRequest::kType:
        {
            nh_get_all_args_t args;
            args.type = proto_nh_type_to_pds_nh_type(proto_req->type());
            args.ctxt = proto_rsp;
            ret = core::nh_get_all(pds_nh_api_info_to_proto, &args);
            proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        }
        break;
    case pds::NexthopGetRequest::kId:
        pds_obj_key_proto_to_api_spec(&key, proto_req->id());
        ret = core::nh_get(&key, &info);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        if (ret != SDK_RET_OK) {
            break;
        }
        pds_nh_api_info_to_proto(&info, proto_rsp);
        break;
    default:
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        break;
    }

    return ret;
}

static inline sdk_ret_t
pds_svc_nexthop_handle_cfg (cfg_ctxt_t *ctxt, google::protobuf::Any *any_resp)
{
    sdk_ret_t ret;
    google::protobuf::Any *any_req = (google::protobuf::Any *)ctxt->req;

    switch (ctxt->cfg) {
    case CFG_MSG_NEXTHOP_CREATE:
        {
            pds::NexthopRequest req;
            pds::NexthopResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_nexthop_create(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    case CFG_MSG_NEXTHOP_UPDATE:
        {
            pds::NexthopRequest req;
            pds::NexthopResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_nexthop_update(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    case CFG_MSG_NEXTHOP_DELETE:
        {
            pds::NexthopDeleteRequest req;
            pds::NexthopDeleteResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_nexthop_delete(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    case CFG_MSG_NEXTHOP_GET:
        {
            pds::NexthopGetRequest req;
            pds::NexthopGetResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_nexthop_get(&req, &rsp);
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
pds_svc_nhgroup_create (const pds::NhGroupRequest *proto_req,
                        pds::NhGroupResponse *proto_rsp)
{
    sdk_ret_t ret = SDK_RET_OK;
    pds_batch_ctxt_t bctxt;
    pds_obj_key_t key = { 0 };
    pds_nexthop_group_spec_t api_spec = { 0 };
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
            PDS_TRACE_ERR("Failed to create a new batch %u",
                          batch_params.epoch);
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return SDK_RET_ERR;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->request_size(); i ++) {
        memset(&api_spec, 0, sizeof(pds_nexthop_group_spec_t));
        auto proto_spec = proto_req->request(i);
        pds_obj_key_proto_to_api_spec(&key, proto_spec.id());
        pds_nh_group_proto_to_api_spec(&api_spec, proto_spec);
        ret = core::nh_group_create(&key, &api_spec, bctxt);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        if (ret != SDK_RET_OK) {
            goto end;
        }
    }

    if (batched_internally) {
        // commit the internal batch
        ret = pds_batch_commit(bctxt);
    }

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
pds_svc_nhgroup_update (const pds::NhGroupRequest *proto_req,
                        pds::NhGroupResponse *proto_rsp)
{
    sdk_ret_t ret = SDK_RET_OK;
    pds_batch_ctxt_t bctxt;
    pds_obj_key_t key = { 0 };
    pds_nexthop_group_spec_t api_spec = { 0 };
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
            PDS_TRACE_ERR("Failed to create a new batch %u",
                          batch_params.epoch);
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return SDK_RET_ERR;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->request_size(); i ++) {
        memset(&api_spec, 0, sizeof(pds_nexthop_group_spec_t));
        auto proto_spec = proto_req->request(i);
        pds_obj_key_proto_to_api_spec(&key, proto_spec.id());
        ret = pds_nh_group_proto_to_api_spec(&api_spec, proto_spec);
        if (ret != SDK_RET_OK) {
            goto end;
        }
        ret = core::nh_group_update(&key, &api_spec, bctxt);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        if (ret != SDK_RET_OK) {
            goto end;
        }
    }

    if (batched_internally) {
        // commit the internal batch
        ret = pds_batch_commit(bctxt);
    }

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
pds_svc_nhgroup_delete (const pds::NhGroupDeleteRequest *proto_req,
                        pds::NhGroupDeleteResponse *proto_rsp)
{
    sdk_ret_t ret = SDK_RET_OK;
    pds_batch_ctxt_t bctxt;
    pds_obj_key_t key = { 0 };
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
            PDS_TRACE_ERR("Failed to create a new batch %u",
                          batch_params.epoch);
            proto_rsp->add_apistatus(types::ApiStatus::API_STATUS_ERR);
            return SDK_RET_ERR;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->id_size(); i++) {
        pds_obj_key_proto_to_api_spec(&key, proto_req->id(i));
        ret = core::nh_group_delete(&key, bctxt);
        proto_rsp->add_apistatus(sdk_ret_to_api_status(ret));
        if (ret != SDK_RET_OK) {
            goto end;
        }
    }

    if (batched_internally) {
        // commit the internal batch
        ret = pds_batch_commit(bctxt);
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
pds_svc_nhgroup_get (const pds::NhGroupGetRequest *proto_req,
                        pds::NhGroupGetResponse *proto_rsp)
{
    sdk_ret_t ret = SDK_RET_OK;
    pds_obj_key_t key = { 0 };
    pds_nexthop_group_info_t info = { 0 };

    PDS_TRACE_VERBOSE("NhGroup Get Received")

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return SDK_RET_INVALID_ARG;
    }

    switch (proto_req->gettype_case()) {
    case pds::NhGroupGetRequest::kType:
        {
            nh_group_get_all_args_t args;
            args.type = proto_nh_group_type_to_pds_nh_group_type(proto_req->type());
            args.ctxt = proto_rsp;
            ret = core::nh_group_get_all(pds_nh_group_api_info_to_proto, &args);
            proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        }
        break;
    case pds::NexthopGetRequest::kId:
        pds_obj_key_proto_to_api_spec(&key, proto_req->id());
        ret = core::nh_group_get(&key, &info);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        if (ret != SDK_RET_OK) {
            break;
        }
        pds_nh_group_api_info_to_proto(&info, proto_rsp);
        break;
    default:
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        break;
    }

    return ret;
}

static inline sdk_ret_t
pds_svc_nhgroup_handle_cfg (cfg_ctxt_t *ctxt, google::protobuf::Any *any_resp)
{
    sdk_ret_t ret;
    google::protobuf::Any *any_req = (google::protobuf::Any *)ctxt->req;

    switch (ctxt->cfg) {
    case CFG_MSG_NHGROUP_CREATE:
        {
            pds::NhGroupRequest req;
            pds::NhGroupResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_nhgroup_create(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    case CFG_MSG_NHGROUP_UPDATE:
        {
            pds::NhGroupRequest req;
            pds::NhGroupResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_nhgroup_update(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    case CFG_MSG_NHGROUP_DELETE:
        {
            pds::NhGroupDeleteRequest req;
            pds::NhGroupDeleteResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_nhgroup_delete(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    case CFG_MSG_NHGROUP_GET:
        {
            pds::NhGroupGetRequest req;
            pds::NhGroupGetResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_nhgroup_get(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    default:
        ret = SDK_RET_INVALID_ARG;
        break;
    }

    return ret;
}

#endif    //__AGENT_SVC_NH_SVC_HPP__
