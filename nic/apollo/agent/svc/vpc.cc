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
        pds_vpc_proto_to_api_spec(api_spec, proto_spec);
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
        pds_vpc_proto_to_api_spec(api_spec, proto_spec);
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
        pds_vpc_api_info_to_proto(&info, proto_rsp);
    }

    if (proto_req->id_size() == 0) {
        ret = core::vpc_get_all(pds_vpc_api_info_to_proto, proto_rsp);
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
        pds_vpc_peer_proto_to_api_spec(api_spec, proto_spec);
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
        pds_vpc_peer_api_info_to_proto(&info, proto_rsp);
    }

    if (proto_req->id_size() == 0) {
        ret = core::vpc_peer_get_all(pds_vpc_peer_api_info_to_proto, proto_rsp);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    }

    return Status::OK;
}
