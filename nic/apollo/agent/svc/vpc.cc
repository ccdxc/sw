//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_vcn.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/vpc.hpp"
#include "nic/apollo/agent/svc/util.hpp"
#include "nic/apollo/agent/svc/vpc.hpp"
#include "nic/apollo/agent/trace.hpp"

// Build VPC API spec from protobuf spec
static inline void
pds_agent_vpc_api_spec_fill (const pds::VPCSpec &proto_spec,
                             pds_vcn_spec_t *api_spec)
{
    pds::VPCType type;

    api_spec->key.id = proto_spec.id();
    type = proto_spec.type();
    if (type == pds::VPC_TYPE_TENANT) {
        api_spec->type = PDS_VCN_TYPE_TENANT;
    } else if (type == pds::VPC_TYPE_SUBSTRATE) {
        api_spec->type = PDS_VCN_TYPE_SUBSTRATE;
    }
    ippfx_proto_spec_to_api_spec_fill(proto_spec.prefix(), &api_spec->pfx);
}

Status
VPCSvcImpl::VPCCreate(ServerContext *context,
                      const pds::VPCRequest *proto_req,
                      pds::VPCResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_vcn_key_t key;
    pds_vcn_spec_t *api_spec;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    for (int i = 0; i < proto_req->request_size(); i ++) {
        api_spec = (pds_vcn_spec_t *)
                    core::agent_state::state()->vpc_slab()->alloc();
        if (api_spec == NULL) {
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OUT_OF_MEM);
            break;
        }
        auto proto_spec = proto_req->request(i);
        memset(&key, 0, sizeof(pds_vcn_key_t));
        key.id = proto_spec.id();
        pds_agent_vpc_api_spec_fill(proto_spec, api_spec);
        ret = core::vpc_create(&key, api_spec);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        if (ret != sdk::SDK_RET_OK) {
            break;
        }
    }
    return Status::OK;
}

Status
VPCSvcImpl::VPCDelete(ServerContext *context,
                      const pds::VPCDeleteRequest *proto_req,
                      pds::VPCDeleteResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_vcn_key_t key;

    if (proto_req == NULL) {
        proto_rsp->add_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }

    for (int i = 0; i < proto_req->id_size(); i++) {
        memset(&key, 0, sizeof(pds_vcn_key_t));
        key.id = proto_req->id(i);
        ret = core::vpc_delete(&key);
        proto_rsp->add_apistatus(sdk_ret_to_api_status(ret));
    }
    return Status::OK;
}

// Populate proto buf spec from vpc API spec
static inline void
vpc_api_spec_to_proto_spec_fill (const pds_vcn_spec_t *api_spec,
                                 pds::VPCSpec *proto_spec)
{
    auto proto_pfx = proto_spec->mutable_prefix();

    proto_spec->set_id(api_spec->key.id);
    if (api_spec->type == PDS_VCN_TYPE_TENANT) {
        proto_spec->set_type(pds::VPC_TYPE_TENANT);
    } else if (api_spec->type == PDS_VCN_TYPE_SUBSTRATE) {
        proto_spec->set_type(pds::VPC_TYPE_SUBSTRATE);
    }
    ippfx_api_spec_to_proto_spec_fill(&api_spec->pfx, proto_pfx);
}

// Populate proto buf status from vpc API status
static inline void
vpc_api_status_to_proto_status_fill (const pds_vcn_status_t *api_status,
                                     pds::VPCStatus *proto_status) 
{
}

// Populate proto buf stats from vpc API stats
static inline void
vpc_api_stats_to_proto_stats_fill (const pds_vcn_stats_t *api_stats,
                                   pds::VPCStats *proto_stats)
{
}

// Populate proto buf from vpc API info
static inline void
vpc_api_info_to_proto_fill (const pds_vcn_info_t *api_info,
                            void *ctxt)
{
    pds::VPCGetResponse *proto_rsp = (pds::VPCGetResponse *)ctxt;
    auto vpc = proto_rsp->add_response();
    pds::VPCSpec *proto_spec = vpc->mutable_spec();
    pds::VPCStatus *proto_status = vpc->mutable_status();
    pds::VPCStats *proto_stats = vpc->mutable_stats();

    vpc_api_spec_to_proto_spec_fill(&api_info->spec, proto_spec);
    vpc_api_status_to_proto_status_fill(&api_info->status, proto_status);
    vpc_api_stats_to_proto_stats_fill(&api_info->stats, proto_stats);
}

Status
VPCSvcImpl::VPCGet(ServerContext *context,
                   const pds::VPCGetRequest *proto_req,
                   pds::VPCGetResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_vcn_key_t key;
    pds_vcn_info_t info = {0};

    PDS_TRACE_VERBOSE("VPC Get Received")

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }

    for (int i = 0; i < proto_req->id_size(); i ++) {
        memset(&key, 0, sizeof(pds_vcn_key_t));
        key.id = proto_req->id(i);
        ret = core::vpc_get(&key, &info);
        if (ret != SDK_RET_OK) {
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_NOT_FOUND);
            break;
        }
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OK);
        vpc_api_info_to_proto_fill(&info, proto_rsp);
    }

    if (proto_req->id_size() == 0) {
        // get all
        ret = core::vpc_get_all(vpc_api_info_to_proto_fill, proto_rsp);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    }

    return Status::OK;
}
