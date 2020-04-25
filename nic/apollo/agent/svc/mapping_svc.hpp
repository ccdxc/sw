//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines protobuf API for mapping object
///
//----------------------------------------------------------------------------

#ifndef __AGENT_SVC_MAPPING_SVC_HPP__
#define __AGENT_SVC_MAPPING_SVC_HPP__

#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/api/include/pds_mapping.hpp"
#include "nic/apollo/api/include/pds_debug.hpp"
#include "nic/apollo/agent/core/core.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/svc/mapping.hpp"
#include "nic/apollo/agent/trace.hpp"
#include "nic/apollo/agent/hooks.hpp"
#include "nic/apollo/api/debug.hpp"
#include "nic/apollo/api/utils.hpp"
#include <malloc.h>

static inline sdk_ret_t
pds_remote_mapping_proto_to_api_spec (pds_remote_mapping_spec_t *remote_spec,
                                      const pds::MappingSpec &proto_spec)
{
    pds_obj_key_proto_to_api_spec(&remote_spec->key, proto_spec.id());
    switch (proto_spec.mac_or_ip_case()) {
    case pds::MappingSpec::kMACKey:
        remote_spec->skey.type = PDS_MAPPING_TYPE_L2;
        pds_obj_key_proto_to_api_spec(&remote_spec->skey.subnet,
                                      proto_spec.mackey().subnetid());
        MAC_UINT64_TO_ADDR(remote_spec->skey.mac_addr,
                           proto_spec.mackey().macaddr());
        remote_spec->subnet = remote_spec->skey.subnet;
        break;

    case pds::MappingSpec::kIPKey:
        remote_spec->skey.type = PDS_MAPPING_TYPE_L3;
        pds_obj_key_proto_to_api_spec(&remote_spec->skey.vpc,
                                      proto_spec.ipkey().vpcid());
        ipaddr_proto_spec_to_api_spec(&remote_spec->skey.ip_addr,
                                      proto_spec.ipkey().ipaddr());
        pds_obj_key_proto_to_api_spec(&remote_spec->subnet,
                                      proto_spec.subnetid());
        break;

    default:
        return SDK_RET_INVALID_ARG;
    }

    switch (proto_spec.dstinfo_case()) {
    case pds::MappingSpec::kTunnelId:
        remote_spec->nh_type = PDS_NH_TYPE_OVERLAY;
        pds_obj_key_proto_to_api_spec(&remote_spec->tep,
                                      proto_spec.tunnelid());
        break;

    case pds::MappingSpec::kNexthopGroupId:
        remote_spec->nh_type = PDS_NH_TYPE_OVERLAY_ECMP;
        pds_obj_key_proto_to_api_spec(&remote_spec->nh_group,
                                      proto_spec.nexthopgroupid());
        break;

    case pds::MappingSpec::kVnicId:
    default:
        PDS_TRACE_ERR("Usage of vnic attribute is invalid for remote mappings");
        return SDK_RET_INVALID_ARG;
    }
    MAC_UINT64_TO_ADDR(remote_spec->vnic_mac, proto_spec.macaddr());
    remote_spec->fabric_encap = proto_encap_to_pds_encap(proto_spec.encap());
    if (proto_spec.has_providerip()) {
        if (proto_spec.providerip().af() == types::IP_AF_INET ||
            proto_spec.providerip().af() == types::IP_AF_INET6) {
            remote_spec->provider_ip_valid = true;
            ipaddr_proto_spec_to_api_spec(&remote_spec->provider_ip,
                                          proto_spec.providerip());
        }
    }
    remote_spec->num_tags = proto_spec.tags_size();
    if (remote_spec->num_tags > PDS_MAX_TAGS_PER_MAPPING) {
        PDS_TRACE_ERR("No. of tags {} on remote IP mapping exceeded max. "
                      "supported {}", remote_spec->num_tags,
                      PDS_MAX_TAGS_PER_MAPPING);
        return SDK_RET_INVALID_ARG;
    }
    for (uint32_t i = 0; i < remote_spec->num_tags; i++) {
        remote_spec->tags[i] = proto_spec.tags(i);
    }
    return SDK_RET_OK;
}

static inline void
pds_remote_mapping_api_spec_to_proto (pds::MappingSpec *proto_spec,
                                      const pds_remote_mapping_spec_t *remote_spec)
{
    if (!proto_spec || !remote_spec) {
        return;
    }

    proto_spec->set_id(remote_spec->key.id, PDS_MAX_KEY_LEN);
    switch (remote_spec->skey.type) {
    case PDS_MAPPING_TYPE_L2:
        {
            auto key = proto_spec->mutable_mackey();
            key->set_macaddr(MAC_TO_UINT64(remote_spec->skey.mac_addr));
            key->set_subnetid(remote_spec->skey.subnet.id, PDS_MAX_KEY_LEN);
        }
        break;
    case PDS_MAPPING_TYPE_L3:
        {
            auto key = proto_spec->mutable_ipkey();
            ipaddr_api_spec_to_proto_spec(key->mutable_ipaddr(),
                                          &remote_spec->skey.ip_addr);
            key->set_vpcid(remote_spec->skey.vpc.id, PDS_MAX_KEY_LEN);
        }
        break;
    default:
        return;
    }
    switch (remote_spec->nh_type) {
    case PDS_NH_TYPE_OVERLAY:
        proto_spec->set_tunnelid(remote_spec->tep.id, PDS_MAX_KEY_LEN);
        break;
    case PDS_NH_TYPE_OVERLAY_ECMP:
        proto_spec->set_nexthopgroupid(remote_spec->nh_group.id,
                                       PDS_MAX_KEY_LEN);
        break;
    default:
        return;
    }
    proto_spec->set_subnetid(remote_spec->subnet.id, PDS_MAX_KEY_LEN);
    proto_spec->set_macaddr(MAC_TO_UINT64(remote_spec->vnic_mac));
    pds_encap_to_proto_encap(proto_spec->mutable_encap(),
                             &remote_spec->fabric_encap);
    if (remote_spec->provider_ip_valid) {
        ipaddr_api_spec_to_proto_spec(proto_spec->mutable_providerip(),
                                      &remote_spec->provider_ip);
    }
    for (uint32_t i = 0; i < remote_spec->num_tags; i++) {
        proto_spec->add_tags(remote_spec->tags[i]);
    }
}

// build API spec from protobuf spec
static inline sdk_ret_t
pds_local_mapping_proto_to_api_spec (pds_local_mapping_spec_t *local_spec,
                                     const pds::MappingSpec &proto_spec)
{
    pds_obj_key_proto_to_api_spec(&local_spec->key, proto_spec.id());
    switch (proto_spec.mac_or_ip_case()) {
    case pds::MappingSpec::kIPKey:
        local_spec->skey.type = PDS_MAPPING_TYPE_L3;
        pds_obj_key_proto_to_api_spec(&local_spec->skey.vpc,
                                      proto_spec.ipkey().vpcid());
        ipaddr_proto_spec_to_api_spec(&local_spec->skey.ip_addr,
                                      proto_spec.ipkey().ipaddr());
        pds_obj_key_proto_to_api_spec(&local_spec->subnet,
                                      proto_spec.subnetid());
        break;

    case pds::MappingSpec::kMACKey:
    default:
        PDS_TRACE_ERR("Unsupported local mapping key type %u, local mappings "
                      "can only be L3 mappings", proto_spec.mac_or_ip_case());
        return SDK_RET_INVALID_ARG;
    }
    if (proto_spec.dstinfo_case() != pds::MappingSpec::kVnicId) {
        PDS_TRACE_ERR("Mandatory vnic attribute not set for local mapping");
        return SDK_RET_INVALID_ARG;
    }
    pds_obj_key_proto_to_api_spec(&local_spec->vnic, proto_spec.vnicid());
    if (proto_spec.has_publicip()) {
        if (proto_spec.publicip().af() == types::IP_AF_INET ||
            proto_spec.publicip().af() == types::IP_AF_INET6) {
            local_spec->public_ip_valid = true;
            ipaddr_proto_spec_to_api_spec(&local_spec->public_ip,
                                          proto_spec.publicip());
        }
    }
    if (proto_spec.has_providerip()) {
        if (proto_spec.providerip().af() == types::IP_AF_INET ||
            proto_spec.providerip().af() == types::IP_AF_INET6) {
            local_spec->provider_ip_valid = true;
            ipaddr_proto_spec_to_api_spec(&local_spec->provider_ip,
                                          proto_spec.providerip());
        }
    }
    MAC_UINT64_TO_ADDR(local_spec->vnic_mac, proto_spec.macaddr());
    local_spec->fabric_encap = proto_encap_to_pds_encap(proto_spec.encap());
    local_spec->num_tags = proto_spec.tags_size();
    if (local_spec->num_tags > PDS_MAX_TAGS_PER_MAPPING) {
        PDS_TRACE_ERR("No. of tags {} on local IP mapping exceeded max. "
                      "supported {}", local_spec->num_tags,
                      PDS_MAX_TAGS_PER_MAPPING);
        return SDK_RET_INVALID_ARG;
    }
    for (uint32_t i = 0; i < local_spec->num_tags; i++) {
        local_spec->tags[i] = proto_spec.tags(i);
    }
    return SDK_RET_OK;
}

static inline void
pds_local_mapping_api_spec_to_proto (pds::MappingSpec *proto_spec,
                                     const pds_local_mapping_spec_t *local_spec)
{
    if (!proto_spec || !local_spec) {
        return;
    }

    proto_spec->set_id(local_spec->key.id, PDS_MAX_KEY_LEN);
    switch (local_spec->skey.type) {
    case PDS_MAPPING_TYPE_L2:
        {
            auto key = proto_spec->mutable_mackey();
            key->set_macaddr(MAC_TO_UINT64(local_spec->skey.mac_addr));
            key->set_subnetid(local_spec->skey.subnet.id, PDS_MAX_KEY_LEN);
        }
        break;
    case PDS_MAPPING_TYPE_L3:
        {
            auto key = proto_spec->mutable_ipkey();
            ipaddr_api_spec_to_proto_spec(key->mutable_ipaddr(),
                                          &local_spec->skey.ip_addr);
            key->set_vpcid(local_spec->skey.vpc.id, PDS_MAX_KEY_LEN);
        }
        break;
    default:
        return;
    }
    proto_spec->set_subnetid(local_spec->subnet.id, PDS_MAX_KEY_LEN);
    proto_spec->set_macaddr(MAC_TO_UINT64(local_spec->vnic_mac));
    pds_encap_to_proto_encap(proto_spec->mutable_encap(),
                             &local_spec->fabric_encap);
    proto_spec->set_vnicid(local_spec->vnic.id, PDS_MAX_KEY_LEN);
    if (local_spec->public_ip_valid) {
        ipaddr_api_spec_to_proto_spec(proto_spec->mutable_publicip(),
                                      &local_spec->public_ip);
    }
    if (local_spec->provider_ip_valid) {
        ipaddr_api_spec_to_proto_spec(proto_spec->mutable_providerip(),
                                      &local_spec->provider_ip);
    }
    for (uint32_t i = 0; i < local_spec->num_tags; i++) {
        proto_spec->add_tags(local_spec->tags[i]);
    }
}

// populate proto buf status from mapping API status
static inline void
pds_mapping_api_status_to_proto (pds::MappingStatus *proto_status,
                                 const pds_mapping_status_t *api_status)
{
}

// populate proto buf stats from mapping API stats
static inline void
pds_mapping_api_stats_to_proto (pds::MappingStats *proto_stats,
                                const pds_mapping_stats_t *api_stats)
{
}

// populate proto buf from local mapping API info
static inline void
pds_local_mapping_api_info_to_proto (void *info, void *ctxt)
{
    pds_local_mapping_info_t *api_info = (pds_local_mapping_info_t *)info;
    pds::MappingGetResponse *proto_rsp = (pds::MappingGetResponse *)ctxt;
    auto mapping = proto_rsp->add_response();
    pds::MappingSpec *proto_spec = mapping->mutable_spec();
    pds::MappingStatus *proto_status = mapping->mutable_status();
    pds::MappingStats *proto_stats = mapping->mutable_stats();

    pds_local_mapping_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_mapping_api_status_to_proto(proto_status, &api_info->status);
    pds_mapping_api_stats_to_proto(proto_stats, &api_info->stats);
}

// populate proto buf from remote mapping API info
static inline void
pds_remote_mapping_api_info_to_proto (void *info, void *ctxt)
{
    pds_remote_mapping_info_t *api_info = (pds_remote_mapping_info_t *)info;
    pds::MappingGetResponse *proto_rsp = (pds::MappingGetResponse *)ctxt;
    auto mapping = proto_rsp->add_response();
    pds::MappingSpec *proto_spec = mapping->mutable_spec();
    pds::MappingStatus *proto_status = mapping->mutable_status();
    pds::MappingStats *proto_stats = mapping->mutable_stats();

    pds_remote_mapping_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_mapping_api_status_to_proto(proto_status, &api_info->status);
    pds_mapping_api_stats_to_proto(proto_stats, &api_info->stats);
}

static inline sdk_ret_t
pds_svc_mapping_create (const pds::MappingRequest *proto_req,
                        pds::MappingResponse *proto_rsp)
{
    pds_batch_ctxt_t bctxt;
    sdk_ret_t ret = SDK_RET_OK;
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
            PDS_TRACE_ERR("Failed to create a new batch, mapping creation "
                          "failed");
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return SDK_RET_ERR;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->request_size(); i++) {
        pds_local_mapping_spec_t local_spec;
        pds_remote_mapping_spec_t remote_spec;
        if (proto_req->request(i).tunnelid().empty()) {
            memset(&local_spec, 0, sizeof(local_spec));
            pds_local_mapping_proto_to_api_spec(&local_spec,
                                                proto_req->request(i));
            hooks::local_mapping_create(&local_spec);
        } else {
            memset(&remote_spec, 0, sizeof(remote_spec));
            pds_remote_mapping_proto_to_api_spec(&remote_spec,
                                                 proto_req->request(i));
            hooks::remote_mapping_create(&remote_spec);
        }

        if (!core::agent_state::state()->pds_mock_mode()) {
            if (proto_req->request(i).tunnelid().empty()) {
                ret = pds_local_mapping_create(&local_spec, bctxt);
                if (ret != SDK_RET_OK) {
                    goto end;
                }
            } else {
                ret = pds_remote_mapping_create(&remote_spec, bctxt);
                if (ret != SDK_RET_OK) {
                    goto end;
                }
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
pds_svc_mapping_update (const pds::MappingRequest *proto_req,
                        pds::MappingResponse *proto_rsp)
{
    pds_batch_ctxt_t bctxt;
    sdk_ret_t ret = SDK_RET_OK;
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
            PDS_TRACE_ERR("Failed to create a new batch, mapping update "
                          "failed");
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return SDK_RET_ERR;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->request_size(); i++) {
        pds_local_mapping_spec_t local_spec;
        pds_remote_mapping_spec_t remote_spec;
        if (proto_req->request(i).tunnelid().empty()) {
            memset(&local_spec, 0, sizeof(local_spec));
            pds_local_mapping_proto_to_api_spec(&local_spec,
                                                proto_req->request(i));
        } else {
            memset(&remote_spec, 0, sizeof(remote_spec));
            pds_remote_mapping_proto_to_api_spec(&remote_spec,
                                                 proto_req->request(i));
        }
        if (!core::agent_state::state()->pds_mock_mode()) {
            if (proto_req->request(i).tunnelid().empty()) {
                ret = pds_local_mapping_update(&local_spec, bctxt);
                if (ret != SDK_RET_OK) {
                    goto end;
                }
            } else {
                ret = pds_remote_mapping_update(&remote_spec, bctxt);
                if (ret != SDK_RET_OK) {
                    goto end;
                }
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
        // commit the internal batch
        pds_batch_destroy(bctxt);
    }
    proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    return ret;
}

static inline sdk_ret_t
pds_svc_mapping_delete (const pds::MappingDeleteRequest *proto_req,
                        pds::MappingDeleteResponse *proto_rsp)
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
            PDS_TRACE_ERR("Failed to create new batch, mapping delete failed");
            proto_rsp->add_apistatus(types::ApiStatus::API_STATUS_ERR);
            return SDK_RET_ERR;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->id_size(); i++) {
        pds_obj_key_proto_to_api_spec(&key, proto_req->id(i));
        if (!core::agent_state::state()->pds_mock_mode()) {
            ret = pds_local_mapping_delete(&key, bctxt);
            if (ret != SDK_RET_OK) {
                goto end;
            }
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
        // commit the internal batch
        pds_batch_destroy(bctxt);
    }
    proto_rsp->add_apistatus(sdk_ret_to_api_status(ret));
    return ret;
}

static inline sdk_ret_t
pds_svc_mapping_get (const pds::MappingGetRequest *proto_req,
                     pds::MappingGetResponse *proto_rsp)
{
    sdk_ret_t ret;
    pds_obj_key_t key;
    pds_local_mapping_info_t local_info;
    pds_remote_mapping_info_t remote_info;

    if (proto_req == NULL || proto_req->id_size() == 0) {
        ret = pds_local_mapping_read_all(pds_local_mapping_api_info_to_proto,
                                         proto_rsp);
        if (ret == SDK_RET_OK) {
            ret = pds_remote_mapping_read_all(pds_remote_mapping_api_info_to_proto,
                                              proto_rsp);
        }
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        PDS_MEMORY_TRIM();
    }

    for (int i = 0; i < proto_req->id_size(); i++) {
        pds_obj_key_proto_to_api_spec(&key, proto_req->id(i));
        if (!core::agent_state::state()->pds_mock_mode()) {
            ret = pds_local_mapping_read(&key, &local_info);
            if (ret == SDK_RET_OK) {
                pds_local_mapping_api_info_to_proto(&local_info, proto_rsp);
                continue;
            }
            ret = pds_remote_mapping_read(&key, &remote_info);
            if (ret != SDK_RET_OK) {
                proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
                break;
            }
            pds_remote_mapping_api_info_to_proto(&remote_info, proto_rsp);
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OK);
        }
    }
    return ret;
}

static inline sdk_ret_t
pds_svc_mapping_handle_cfg (cfg_ctxt_t *ctxt, google::protobuf::Any *any_resp)
{
    sdk_ret_t ret;
    google::protobuf::Any *any_req = (google::protobuf::Any *)ctxt->req;

    switch (ctxt->cfg) {
    case CFG_MSG_MAPPING_CREATE:
        {
            pds::MappingRequest req;
            pds::MappingResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_mapping_create(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    case CFG_MSG_MAPPING_UPDATE:
        {
            pds::MappingRequest req;
            pds::MappingResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_mapping_update(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    case CFG_MSG_MAPPING_DELETE:
        {
            pds::MappingDeleteRequest req;
            pds::MappingDeleteResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_mapping_delete(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    case CFG_MSG_MAPPING_GET:
        {
            pds::MappingGetRequest req;
            pds::MappingGetResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_mapping_get(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    default:
        ret = SDK_RET_INVALID_ARG;
        break;
    }

    return ret;
}

#endif    //__AGENT_SVC_MAPPING_SVC_HPP__
