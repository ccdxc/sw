//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_vcn.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/pcn.hpp"
#include "nic/apollo/agent/svc/util.hpp"
#include "nic/apollo/agent/svc/pcn.hpp"

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
