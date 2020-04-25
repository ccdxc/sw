//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines protobuf API for service object
///
//----------------------------------------------------------------------------

#ifndef __AGENT_SVC_SERVICE_SVC_HPP__
#define __AGENT_SVC_SERVICE_SVC_HPP__

#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/api/include/pds_service.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/service.hpp"
#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/svc/service.hpp"
#include "nic/apollo/agent/hooks.hpp"

// populate proto buf spec from service API spec
static inline void
pds_service_api_spec_to_proto (pds::SvcMappingSpec *proto_spec,
                               const pds_svc_mapping_spec_t *api_spec)
{
    if (!proto_spec || !api_spec) {
        return;
    }

    proto_spec->set_id(api_spec->key.id, PDS_MAX_KEY_LEN);
    auto proto_key = proto_spec->mutable_key();
    proto_key->set_vpcid(api_spec->skey.vpc.id, PDS_MAX_KEY_LEN);
    proto_key->set_backendport(api_spec->skey.backend_port);
    ipaddr_api_spec_to_proto_spec(
                proto_key->mutable_backendip(), &api_spec->skey.backend_ip);
    ipaddr_api_spec_to_proto_spec(
                proto_spec->mutable_ipaddr(), &api_spec->vip);
    // provider IP is optional
    if (api_spec->backend_provider_ip.af != IP_AF_NIL) {
        ipaddr_api_spec_to_proto_spec(
                    proto_spec->mutable_providerip(),
                    &api_spec->backend_provider_ip);
    }
    proto_spec->set_svcport(api_spec->svc_port);
}

// populate proto buf status from service API status
static inline void
pds_service_api_status_to_proto (pds::SvcMappingStatus *proto_status,
                                 const pds_svc_mapping_status_t *api_status)
{
}

// populate proto buf stats from service API stats
static inline void
pds_service_api_stats_to_proto (pds::SvcMappingStats *proto_stats,
                                const pds_svc_mapping_stats_t *api_stats)
{
}

// populate proto buf from service API info
static inline void
pds_service_api_info_to_proto (pds_svc_mapping_info_t *api_info,
                               void *ctxt)
{
    pds::SvcMappingGetResponse *proto_rsp = (pds::SvcMappingGetResponse *)ctxt;
    auto service = proto_rsp->add_response();
    pds::SvcMappingSpec *proto_spec = service->mutable_spec();
    pds::SvcMappingStatus *proto_status = service->mutable_status();
    pds::SvcMappingStats *proto_stats = service->mutable_stats();

    pds_service_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_service_api_status_to_proto(proto_status, &api_info->status);
    pds_service_api_stats_to_proto(proto_stats, &api_info->stats);
}

// build service API spec from proto buf spec
static inline void
pds_service_proto_to_api_spec (pds_svc_mapping_spec_t *api_spec,
                               const pds::SvcMappingSpec &proto_spec)
{
    pds_obj_key_proto_to_api_spec(&api_spec->key, proto_spec.id());
    pds_obj_key_proto_to_api_spec(&api_spec->skey.vpc,
                                  proto_spec.key().vpcid());
    api_spec->skey.backend_port = proto_spec.key().backendport();
    ipaddr_proto_spec_to_api_spec(&api_spec->skey.backend_ip,
                                  proto_spec.key().backendip());
    ipaddr_proto_spec_to_api_spec(&api_spec->vip,
                                  proto_spec.ipaddr());
    ipaddr_proto_spec_to_api_spec(&api_spec->backend_provider_ip,
                                  proto_spec.providerip());
    api_spec->svc_port = proto_spec.svcport();
}

static inline sdk_ret_t
pds_svc_service_mapping_create (const pds::SvcMappingRequest *proto_req,
                                pds::SvcMappingResponse *proto_rsp)
{
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    bool batched_internally = false;
    pds_batch_params_t batch_params;
    pds_svc_mapping_spec_t api_spec;

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
            PDS_TRACE_ERR("Failed to create a new batch, service mapping "
                          "creation failed");
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return SDK_RET_ERR;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->request_size(); i ++) {
        memset(&api_spec, 0, sizeof(api_spec));
        pds_service_proto_to_api_spec(&api_spec, proto_req->request(i));
        hooks::svc_mapping_create(&api_spec);
        if (!core::agent_state::state()->pds_mock_mode()) {
            ret = pds_svc_mapping_create(&api_spec, bctxt);
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
pds_svc_service_mapping_update (const pds::SvcMappingRequest *proto_req,
                                pds::SvcMappingResponse *proto_rsp)
{
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    bool batched_internally = false;
    pds_batch_params_t batch_params;
    pds_svc_mapping_spec_t api_spec;

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
            PDS_TRACE_ERR("Failed to create a new batch, service mapping "
                          "update failed");
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return SDK_RET_ERR;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->request_size(); i ++) {
        memset(&api_spec, 0, sizeof(api_spec));
        pds_service_proto_to_api_spec(&api_spec, proto_req->request(i));
        if (!core::agent_state::state()->pds_mock_mode()) {
            ret = pds_svc_mapping_update(&api_spec, bctxt);
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
pds_svc_service_mapping_delete (const pds::SvcMappingDeleteRequest *proto_req,
                                pds::SvcMappingDeleteResponse *proto_rsp)
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
            PDS_TRACE_ERR("Failed to create a new batch, service mapping "
                          "delete failed");
            proto_rsp->add_apistatus(types::ApiStatus::API_STATUS_ERR);
            return SDK_RET_ERR;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->id_size(); i++) {
        pds_obj_key_proto_to_api_spec(&key, proto_req->id(i));
        if (!core::agent_state::state()->pds_mock_mode()) {
            ret = pds_svc_mapping_delete(&key, bctxt);
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
        // destroy the internal batch
        pds_batch_destroy(bctxt);
    }
    proto_rsp->add_apistatus(sdk_ret_to_api_status(ret));
    return ret;
}

static inline sdk_ret_t
pds_svc_service_mapping_get (const pds::SvcMappingGetRequest *proto_req,
                                pds::SvcMappingGetResponse *proto_rsp)
{
    sdk_ret_t ret;
    pds_obj_key_t key;
    pds_svc_mapping_info_t info;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return SDK_RET_INVALID_ARG;
    }
    if (proto_req->id_size() == 0) {
        ret = pds_svc_mapping_read_all(pds_service_api_info_to_proto, proto_rsp);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    }
    for (int i = 0; i < proto_req->id_size(); i++) {
        pds_obj_key_proto_to_api_spec(&key, proto_req->id(i));
        if (!core::agent_state::state()->pds_mock_mode()) {
            ret = pds_svc_mapping_read(&key, &info);
            if (ret == SDK_RET_OK) {
                pds_service_api_info_to_proto(&info, proto_rsp);
                proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OK);
            } else {
                proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
                break;
            }
        }
    }
    return ret;
}

static inline sdk_ret_t
pds_svc_service_mapping_handle_cfg (cfg_ctxt_t *ctxt, google::protobuf::Any *any_resp)
{
    sdk_ret_t ret;
    google::protobuf::Any *any_req = (google::protobuf::Any *)ctxt->req;

    switch (ctxt->cfg) {
    case CFG_MSG_SVC_MAPPING_CREATE:
        {
            pds::SvcMappingRequest req;
            pds::SvcMappingResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_service_mapping_create(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    case CFG_MSG_SVC_MAPPING_UPDATE:
        {
            pds::SvcMappingRequest req;
            pds::SvcMappingResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_service_mapping_update(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    case CFG_MSG_SVC_MAPPING_DELETE:
        {
            pds::SvcMappingDeleteRequest req;
            pds::SvcMappingDeleteResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_service_mapping_delete(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    case CFG_MSG_SVC_MAPPING_GET:
        {
            pds::SvcMappingGetRequest req;
            pds::SvcMappingGetResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_service_mapping_get(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    default:
        ret = SDK_RET_INVALID_ARG;
        break;
    }

    return ret;
}

#endif    //__AGENT_SVC_SERVICE_SVC_HPP__
