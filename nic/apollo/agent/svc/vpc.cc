//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_vpc.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/vpc.hpp"
#include "nic/apollo/agent/svc/util.hpp"
#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/svc/vpc.hpp"
#include "nic/apollo/agent/trace.hpp"

// Build VPC API spec from protobuf spec
static inline void
pds_agent_vpc_api_spec_fill (pds_vpc_spec_t *api_spec,
                             const pds::VPCSpec &proto_spec)
{
    pds::VPCType type;

    api_spec->key.id = proto_spec.id();
    type = proto_spec.type();
    if (type == pds::VPC_TYPE_TENANT) {
        api_spec->type = PDS_VPC_TYPE_TENANT;
    } else if (type == pds::VPC_TYPE_SUBSTRATE) {
        api_spec->type = PDS_VPC_TYPE_SUBSTRATE;
    }
    ipv4pfx_proto_spec_to_api_spec(&api_spec->v4_pfx, proto_spec.v4prefix());
    ippfx_proto_spec_to_api_spec(&api_spec->v6_pfx, proto_spec.v6prefix());
}

Status
VPCSvcImpl::VPCCreate(ServerContext *context,
                      const pds::VPCRequest *proto_req,
                      pds::VPCResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_vpc_key_t key = {0};
    pds_vpc_spec_t *api_spec = {0};

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    for (int i = 0; i < proto_req->request_size(); i ++) {
        api_spec = (pds_vpc_spec_t *)
                    core::agent_state::state()->vpc_slab()->alloc();
        if (api_spec == NULL) {
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OUT_OF_MEM);
            break;
        }
        auto proto_spec = proto_req->request(i);
        key.id = proto_spec.id();
        pds_agent_vpc_api_spec_fill(api_spec, proto_spec);
        ret = core::vpc_create(&key, api_spec);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        if (ret != sdk::SDK_RET_OK) {
            break;
        }
    }
    return Status::OK;
}

Status
VPCSvcImpl::VPCUpdate(ServerContext *context,
                      const pds::VPCRequest *proto_req,
                      pds::VPCResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_vpc_key_t key = {0};
    pds_vpc_spec_t *api_spec = {0};

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    for (int i = 0; i < proto_req->request_size(); i ++) {
        api_spec = (pds_vpc_spec_t *)
                    core::agent_state::state()->vpc_slab()->alloc();
        if (api_spec == NULL) {
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OUT_OF_MEM);
            break;
        }
        auto proto_spec = proto_req->request(i);
        key.id = proto_spec.id();
        pds_agent_vpc_api_spec_fill(api_spec, proto_spec);
        ret = core::vpc_update(&key, api_spec);
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
    pds_vpc_key_t key = {0};

    if (proto_req == NULL) {
        proto_rsp->add_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }

    for (int i = 0; i < proto_req->id_size(); i++) {
        key.id = proto_req->id(i);
        ret = core::vpc_delete(&key);
        proto_rsp->add_apistatus(sdk_ret_to_api_status(ret));
    }
    return Status::OK;
}

// Populate proto buf spec from vpc API spec
static inline void
vpc_api_spec_to_proto_spec (pds::VPCSpec *proto_spec,
                            const pds_vpc_spec_t *api_spec)
{
    proto_spec->set_id(api_spec->key.id);
    if (api_spec->type == PDS_VPC_TYPE_TENANT) {
        proto_spec->set_type(pds::VPC_TYPE_TENANT);
    } else if (api_spec->type == PDS_VPC_TYPE_SUBSTRATE) {
        proto_spec->set_type(pds::VPC_TYPE_SUBSTRATE);
    }
    ipv4pfx_api_spec_to_proto_spec(proto_spec->mutable_v4prefix(), &api_spec->v4_pfx);
    ippfx_api_spec_to_proto_spec(proto_spec->mutable_v6prefix(), &api_spec->v6_pfx);
}

// Populate proto buf status from vpc API status
static inline void
vpc_api_status_to_proto_status (pds::VPCStatus *proto_status,
                                const pds_vpc_status_t *api_status)
{
}

// Populate proto buf stats from vpc API stats
static inline void
vpc_api_stats_to_proto_stats (pds::VPCStats *proto_stats,
                              const pds_vpc_stats_t *api_stats)
{
}

// Populate proto buf from vpc API info
static inline void
vpc_api_info_to_proto (const pds_vpc_info_t *api_info, void *ctxt)
{
    pds::VPCGetResponse *proto_rsp = (pds::VPCGetResponse *)ctxt;
    auto vpc = proto_rsp->add_response();
    pds::VPCSpec *proto_spec = vpc->mutable_spec();
    pds::VPCStatus *proto_status = vpc->mutable_status();
    pds::VPCStats *proto_stats = vpc->mutable_stats();

    vpc_api_spec_to_proto_spec(proto_spec, &api_info->spec);
    vpc_api_status_to_proto_status(proto_status, &api_info->status);
    vpc_api_stats_to_proto_stats(proto_stats, &api_info->stats);
}

Status
VPCSvcImpl::VPCGet(ServerContext *context,
                   const pds::VPCGetRequest *proto_req,
                   pds::VPCGetResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_vpc_key_t key = {0};
    pds_vpc_info_t info = {0};

    PDS_TRACE_VERBOSE("VPC Get Received")

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }

    for (int i = 0; i < proto_req->id_size(); i ++) {
        key.id = proto_req->id(i);
        ret = core::vpc_get(&key, &info);
        if (ret != SDK_RET_OK) {
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_NOT_FOUND);
            break;
        }
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OK);
        vpc_api_info_to_proto(&info, proto_rsp);
    }

    if (proto_req->id_size() == 0) {
        ret = core::vpc_get_all(vpc_api_info_to_proto, proto_rsp);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    }

    return Status::OK;
}

Status
VPCSvcImpl::VPCPeerCreate(ServerContext *context,
                          const pds::VPCPeerRequest *proto_req,
                          pds::VPCPeerResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_vpc_peer_key_t key = {0};
    pds_vpc_peer_spec_t *api_spec = {0};

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    for (int i = 0; i < proto_req->request_size(); i ++) {
        api_spec = (pds_vpc_peer_spec_t *)
                    core::agent_state::state()->vpc_peer_slab()->alloc();
        if (api_spec == NULL) {
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OUT_OF_MEM);
            break;
        }
        auto proto_spec = proto_req->request(i);
        key.id = proto_spec.id();
        pds_agent_vpc_peer_api_spec_fill(api_spec, proto_spec);
        ret = core::vpc_peer_create(&key, api_spec);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        if (ret != sdk::SDK_RET_OK) {
            break;
        }
    }
    return Status::OK;
}

Status
VPCSvcImpl::VPCPeerDelete(ServerContext *context,
                          const pds::VPCPeerDeleteRequest *proto_req,
                          pds::VPCPeerDeleteResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_vpc_peer_key_t key = {0};

    if (proto_req == NULL) {
        proto_rsp->add_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }

    for (int i = 0; i < proto_req->id_size(); i++) {
        key.id = proto_req->id(i);
        ret = core::vpc_peer_delete(&key);
        proto_rsp->add_apistatus(sdk_ret_to_api_status(ret));
    }
    return Status::OK;
}

// Populate proto buf status from vpc API status
static inline void
vpc_peer_api_status_to_proto_status (pds::VPCPeerStatus *proto_status,
                                     const pds_vpc_peer_status_t *api_status)
{
}

// Populate proto buf stats from vpc API stats
static inline void
vpc_peer_api_stats_to_proto_stats (pds::VPCPeerStats *proto_stats,
                                   const pds_vpc_peer_stats_t *api_stats)
{
}

// Populate proto buf from vpc API info
static inline void
vpc_peer_api_info_to_proto (const pds_vpc_peer_info_t *api_info, void *ctxt)
{
    pds::VPCPeerGetResponse *proto_rsp = (pds::VPCPeerGetResponse *)ctxt;
    auto vpc = proto_rsp->add_response();
    pds::VPCPeerSpec *proto_spec = vpc->mutable_spec();
    pds::VPCPeerStatus *proto_status = vpc->mutable_status();
    pds::VPCPeerStats *proto_stats = vpc->mutable_stats();

    vpc_peer_api_spec_to_proto_spec(proto_spec, &api_info->spec);
    vpc_peer_api_status_to_proto_status(proto_status, &api_info->status);
    vpc_peer_api_stats_to_proto_stats(proto_stats, &api_info->stats);
}

Status
VPCSvcImpl::VPCPeerGet(ServerContext *context,
                       const pds::VPCPeerGetRequest *proto_req,
                       pds::VPCPeerGetResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_vpc_peer_key_t key = {0};
    pds_vpc_peer_info_t info = {0};

    PDS_TRACE_VERBOSE("VPCPeer Get Received")

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }

    for (int i = 0; i < proto_req->id_size(); i ++) {
        key.id = proto_req->id(i);
        ret = core::vpc_peer_get(&key, &info);
        if (ret != SDK_RET_OK) {
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_NOT_FOUND);
            break;
        }
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OK);
        vpc_peer_api_info_to_proto(&info, proto_rsp);
    }

    if (proto_req->id_size() == 0) {
        ret = core::vpc_peer_get_all(vpc_peer_api_info_to_proto, proto_rsp);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    }

    return Status::OK;
}
