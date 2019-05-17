//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_tag.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/tag.hpp"
#include "nic/apollo/agent/svc/util.hpp"
#include "nic/apollo/agent/svc/tag.hpp"

// build Tag api spec from proto buf spec
static inline sdk_ret_t
pds_tag_proto_spec_to_api_spec (pds_tag_spec_t *api_spec,
                                const pds::TagSpec &proto_spec)
{
    sdk_ret_t ret;

    api_spec->key.id = proto_spec.id();
    ret = pds_af_proto_spec_to_api_spec(&api_spec->af, proto_spec.af());
    if (ret != SDK_RET_OK) {
        return SDK_RET_INVALID_ARG;
    }

#if 0
    api_spec->num_prefixes = proto_spec.prefix_size();
    api_spec->prefixes = (ip_prefix_t *)SDK_MALLOC(PDS_MEM_ALLOC_ID_METER,
                                                   api_spec->num_prefixes * sizeof(ip_prefix_t));
    for (int i = 0; i < proto_spec.prefix_size(); i ++) {
        ippfx_proto_spec_to_api_spec(
                    &api_spec->prefixes[i], proto_spec.prefix(i));
    }
#endif
    return SDK_RET_OK;
}

Status
TagSvcImpl::TagCreate(ServerContext *context,
                      const pds::TagRequest *proto_req,
                      pds::TagResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_tag_key_t key = {0};
    pds_tag_spec_t *api_spec = NULL;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    for (int i = 0; i < proto_req->request_size(); i ++) {
        api_spec = (pds_tag_spec_t *)
                    core::agent_state::state()->tag_slab()->alloc();
        if (api_spec == NULL) {
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OUT_OF_MEM);
            break;
        }
        auto request = proto_req->request(i);
        key.id = request.id();
        ret = pds_tag_proto_spec_to_api_spec(api_spec, request);
        if (ret != SDK_RET_OK) {
            core::agent_state::state()->tag_slab()->free(api_spec);
            break;
        }
        ret = core::tag_create(&key, api_spec);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        if (ret != sdk::SDK_RET_OK) {
            break;
        }
    }
    return Status::OK;
}

Status
TagSvcImpl::TagUpdate(ServerContext *context,
                      const pds::TagRequest *proto_req,
                      pds::TagResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_tag_key_t key = {0};
    pds_tag_spec_t *api_spec = NULL;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    for (int i = 0; i < proto_req->request_size(); i ++) {
        api_spec = (pds_tag_spec_t *)
                    core::agent_state::state()->tag_slab()->alloc();
        if (api_spec == NULL) {
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OUT_OF_MEM);
            break;
        }
        auto request = proto_req->request(i);
        key.id = request.id();
        ret = pds_tag_proto_spec_to_api_spec(api_spec, request);
        if (ret != SDK_RET_OK) {
            core::agent_state::state()->tag_slab()->free(api_spec);
            break;
        }
        ret = core::tag_update(&key, api_spec);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        if (ret != sdk::SDK_RET_OK) {
            break;
        }
    }
    return Status::OK;
}

Status
TagSvcImpl::TagDelete(ServerContext *context,
                      const pds::TagDeleteRequest *proto_req,
                      pds::TagDeleteResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_tag_key_t key = {0};

    if (proto_req == NULL) {
        proto_rsp->add_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    for (int i = 0; i < proto_req->id_size(); i++) {
        key.id = proto_req->id(i);
        ret = core::tag_delete(&key);
        proto_rsp->add_apistatus(sdk_ret_to_api_status(ret));
    }
    return Status::OK;
}

// Populate proto buf spec from tag API spec
static inline void
tag_api_spec_to_proto_spec (const pds_tag_spec_t *api_spec,
                            pds::TagSpec *proto_spec)
{
    proto_spec->set_id(api_spec->key.id);
    proto_spec->set_af(pds_af_api_spec_to_proto_spec(api_spec->af));
#if 0
    for (uint32_t i = 0; i < api_spec->num_prefixes; i ++) {
        ippfx_api_spec_to_proto_spec(
                    proto_spec->add_prefix(), &api_spec->prefixes[i]);
    }
#endif
}

// Populate proto buf status from tag API status
static inline void
tag_api_status_to_proto_status (const pds_tag_status_t *api_status,
                                pds::TagStatus *proto_status)
{
}

// Populate proto buf stats from tag API stats
static inline void
tag_api_stats_to_proto_stats (const pds_tag_stats_t *api_stats,
                              pds::TagStats *proto_stats)
{
}

// Populate proto buf from tag API info
static inline void
tag_api_info_to_proto (const pds_tag_info_t *api_info, void *ctxt)
{
    pds::TagGetResponse *proto_rsp = (pds::TagGetResponse *)ctxt;
    auto tag = proto_rsp->add_response();
    pds::TagSpec *proto_spec = tag->mutable_spec();
    pds::TagStatus *proto_status = tag->mutable_status();
    pds::TagStats *proto_stats = tag->mutable_stats();

    tag_api_spec_to_proto_spec(&api_info->spec, proto_spec);
    tag_api_status_to_proto_status(&api_info->status, proto_status);
    tag_api_stats_to_proto_stats(&api_info->stats, proto_stats);
}

Status
TagSvcImpl::TagGet(ServerContext *context,
                   const pds::TagGetRequest *proto_req,
                   pds::TagGetResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_tag_key_t key = {0};
    pds_tag_info_t info;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }

    for (int i = 0; i < proto_req->id_size(); i++) {
        key.id = proto_req->id(i);
        ret = core::tag_get(&key, &info);
        if (ret != SDK_RET_OK) {
            proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
            break;
        }
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OK);
        tag_api_info_to_proto(&info, proto_rsp);
    }

    if (proto_req->id_size() == 0) {
        ret = core::tag_get_all(tag_api_info_to_proto, proto_rsp);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    }
    return Status::OK;
}
