//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_vcn.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/pcn.hpp"
#include "nic/apollo/agent/svc/util.hpp"
#include "nic/apollo/agent/svc/pcn.hpp"
#include "nic/apollo/agent/trace.hpp"

// Build PCN API spec from protobuf spec
static inline void
pds_agent_pcn_api_spec_fill (const pds::PCNSpec &proto_spec,
                             pds_vcn_spec_t *api_spec)
{
    pds::PCNType type;

    api_spec->key.id = proto_spec.id();
    type = proto_spec.type();
    if (type == pds::PCN_TYPE_TENANT) {
        api_spec->type = PDS_VCN_TYPE_TENANT;
    } else if (type == pds::PCN_TYPE_SUBSTRATE) {
        api_spec->type = PDS_VCN_TYPE_SUBSTRATE;
    }
    ippfx_proto_spec_to_api_spec_fill(proto_spec.prefix(), &api_spec->pfx);
}

Status
PCNSvcImpl::PCNCreate(ServerContext *context,
                      const pds::PCNRequest *proto_req,
                      pds::PCNResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_vcn_key_t key;
    pds_vcn_spec_t *api_spec;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    for (int i = 0; i < proto_req->request_size(); i ++) {
        api_spec = (pds_vcn_spec_t *)
                    core::agent_state::state()->pcn_slab()->alloc();
        if (api_spec == NULL) {
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OUT_OF_MEM);
            break;
        }
        auto proto_spec = proto_req->request(i);
        memset(&key, 0, sizeof(pds_vcn_key_t));
        key.id = proto_spec.id();
        pds_agent_pcn_api_spec_fill(proto_spec, api_spec);
        ret = core::pcn_create(&key, api_spec);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        if (ret != sdk::SDK_RET_OK) {
            break;
        }
    }
    return Status::OK;
}

Status
PCNSvcImpl::PCNDelete(ServerContext *context,
                      const pds::PCNDeleteRequest *proto_req,
                      pds::PCNDeleteResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_vcn_key_t key;

    if (proto_req == NULL) {
        proto_rsp->add_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }

    for (int i = 0; i < proto_req->id_size(); i++) {
        memset(&key, 0, sizeof(pds_vcn_key_t));
        key.id = proto_req->id(i);
        ret = core::pcn_delete(&key);
        proto_rsp->add_apistatus(sdk_ret_to_api_status(ret));
    }
    return Status::OK;
}

// Populate proto buf spec from pcn API spec
static inline void
pcn_api_spec_to_proto_spec_fill (const pds_vcn_spec_t *api_spec,
                                 pds::PCNSpec *proto_spec)
{
    auto proto_pfx = proto_spec->mutable_prefix();

    proto_spec->set_id(api_spec->key.id);
    if (api_spec->type == PDS_VCN_TYPE_TENANT) {
        proto_spec->set_type(pds::PCN_TYPE_TENANT);
    } else if (api_spec->type == PDS_VCN_TYPE_SUBSTRATE) {
        proto_spec->set_type(pds::PCN_TYPE_SUBSTRATE);
    }
    ippfx_api_spec_to_proto_spec_fill(&api_spec->pfx, proto_pfx);
}

// Populate proto buf status from pcn API status
static inline void
pcn_api_status_to_proto_status_fill (const pds_vcn_status_t *api_status,
                                     pds::PCNStatus *proto_status) 
{
}

// Populate proto buf stats from pcn API stats
static inline void
pcn_api_stats_to_proto_stats_fill (const pds_vcn_stats_t *api_stats,
                                   pds::PCNStats *proto_stats)
{
}

// Populate proto buf from pcn API info
static inline void
pcn_api_info_to_proto_fill (const pds_vcn_info_t *api_info,
                            void *ctxt)
{
    pds::PCNGetResponse *proto_rsp = (pds::PCNGetResponse *)ctxt;
    auto pcn = proto_rsp->add_response();
    pds::PCNSpec *proto_spec = pcn->mutable_spec();
    pds::PCNStatus *proto_status = pcn->mutable_status();
    pds::PCNStats *proto_stats = pcn->mutable_stats();

    pcn_api_spec_to_proto_spec_fill(&api_info->spec, proto_spec);
    pcn_api_status_to_proto_status_fill(&api_info->status, proto_status);
    pcn_api_stats_to_proto_stats_fill(&api_info->stats, proto_stats);
}

Status
PCNSvcImpl::PCNGet(ServerContext *context,
                   const pds::PCNGetRequest *proto_req,
                   pds::PCNGetResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_vcn_key_t key;
    pds_vcn_info_t info = {0};

    PDS_TRACE_VERBOSE("PCN Get Received")

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }

    for (int i = 0; i < proto_req->id_size(); i ++) {
        memset(&key, 0, sizeof(pds_vcn_key_t));
        key.id = proto_req->id(i);
        ret = core::pcn_get(&key, &info);
        if (ret != SDK_RET_OK) {
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_NOT_FOUND);
            break;
        }
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OK);
        pcn_api_info_to_proto_fill(&info, proto_rsp);
    }

    if (proto_req->id_size() == 0) {
        // get all
        ret = core::pcn_get_all(pcn_api_info_to_proto_fill, proto_rsp);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    }

    return Status::OK;
}
