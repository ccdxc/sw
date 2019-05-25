//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_vnic.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/vnic.hpp"
#include "nic/apollo/agent/svc/util.hpp"
#include "nic/apollo/agent/svc/vnic.hpp"
#include "nic/apollo/agent/svc/specs.hpp"

// build VNIC api spec from proto buf spec
static inline sdk_ret_t
pds_vnic_proto_spec_to_api_spec (pds_vnic_spec_t *api_spec,
                                 const pds::VnicSpec &proto_spec)
{
    uint32_t msid;

    api_spec->key.id = proto_spec.vnicid();
    api_spec->vpc.id = proto_spec.vpcid();
    api_spec->subnet.id = proto_spec.subnetid();
    api_spec->vnic_encap = proto_encap_to_pds_encap(proto_spec.vnicencap());
    api_spec->fabric_encap = proto_encap_to_pds_encap(proto_spec.fabricencap());
    MAC_UINT64_TO_ADDR(api_spec->mac_addr, proto_spec.macaddress());
    //MAC_UINT64_TO_ADDR(api_spec->provider_mac_addr,
    //                   proto_spec.providermacaddress());
    api_spec->rsc_pool_id = proto_spec.resourcepoolid();
    api_spec->src_dst_check = proto_spec.sourceguardenable();
    for (int i = 0; i < proto_spec.txmirrorsessionid_size(); i++) {
        msid = proto_spec.txmirrorsessionid(i);
        if ((msid < 1) || (msid > 8)) {
            PDS_TRACE_ERR("Invalid tx mirror session id {} in vnic {} spec, "
                          "mirror session ids must be in the range [1-8]",
                          msid, api_spec->key.id);
            return SDK_RET_INVALID_ARG;
        }
        api_spec->tx_mirror_session_bmap |= (1 << (msid - 1));
    }
    for (int i = 0; i < proto_spec.rxmirrorsessionid_size(); i++) {
        msid = proto_spec.rxmirrorsessionid(i);
        if ((msid < 1) || (msid > 8)) {
            PDS_TRACE_ERR("Invalid rx mirror session id {} in vnic {} spec",
                          "mirror session ids must be in the range [1-8]",
                          msid, api_spec->key.id);
            return SDK_RET_INVALID_ARG;
        }
        api_spec->rx_mirror_session_bmap |= (1 << (msid - 1));
    }
    api_spec->v4_meter.id = proto_spec.v4meterid();
    api_spec->v4_meter.id = proto_spec.v6meterid();
    return SDK_RET_OK;
}

Status
VnicSvcImpl::VnicCreate(ServerContext *context,
                        const pds::VnicRequest *proto_req,
                        pds::VnicResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_vnic_key_t key = {0};
    pds_vnic_spec_t *api_spec = NULL;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    for (int i = 0; i < proto_req->request_size(); i ++) {
        api_spec = (pds_vnic_spec_t *)
                    core::agent_state::state()->vnic_slab()->alloc();
        if (api_spec == NULL) {
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OUT_OF_MEM);
            break;
        }
        auto request = proto_req->request(i);
        key.id = request.vnicid();
        ret = pds_vnic_proto_spec_to_api_spec(api_spec, request);
        if (ret != SDK_RET_OK) {
            core::agent_state::state()->vnic_slab()->free(api_spec);
            break;
        }
        ret = core::vnic_create(&key, api_spec);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        if (ret != sdk::SDK_RET_OK) {
            break;
        }
    }
    return Status::OK;
}

Status
VnicSvcImpl::VnicUpdate(ServerContext *context,
                        const pds::VnicRequest *proto_req,
                        pds::VnicResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_vnic_key_t key = {0};
    pds_vnic_spec_t *api_spec = NULL;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    for (int i = 0; i < proto_req->request_size(); i ++) {
        api_spec = (pds_vnic_spec_t *)
                    core::agent_state::state()->vnic_slab()->alloc();
        if (api_spec == NULL) {
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OUT_OF_MEM);
            break;
        }
        auto request = proto_req->request(i);
        key.id = request.vnicid();
        ret = pds_vnic_proto_spec_to_api_spec(api_spec, request);
        if (ret != SDK_RET_OK) {
            core::agent_state::state()->vnic_slab()->free(api_spec);
            break;
        }
        ret = core::vnic_update(&key, api_spec);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        if (ret != sdk::SDK_RET_OK) {
            break;
        }
    }
    return Status::OK;
}

Status
VnicSvcImpl::VnicDelete(ServerContext *context,
                        const pds::VnicDeleteRequest *proto_req,
                        pds::VnicDeleteResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_vnic_key_t key = {0};

    if (proto_req == NULL) {
        proto_rsp->add_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    for (int i = 0; i < proto_req->vnicid_size(); i++) {
        key.id = proto_req->vnicid(i);
        ret = core::vnic_delete(&key);
        proto_rsp->add_apistatus(sdk_ret_to_api_status(ret));
    }
    return Status::OK;
}

// Populate proto buf status from vnic API status
static inline void
vnic_api_status_to_proto_status (const pds_vnic_status_t *api_status,
                                 pds::VnicStatus *proto_status)
{
}

// Populate proto buf stats from vnic API stats
static inline void
vnic_api_stats_to_proto_stats (const pds_vnic_stats_t *api_stats,
                               pds::VnicStats *proto_stats)
{
}

// Populate proto buf from vnic API info
static inline void
vnic_api_info_to_proto (const pds_vnic_info_t *api_info, void *ctxt)
{
    pds::VnicGetResponse *proto_rsp = (pds::VnicGetResponse *)ctxt;
    auto vnic = proto_rsp->add_response();
    pds::VnicSpec *proto_spec = vnic->mutable_spec();
    pds::VnicStatus *proto_status = vnic->mutable_status();
    pds::VnicStats *proto_stats = vnic->mutable_stats();

    vnic_api_spec_to_proto_spec(&api_info->spec, proto_spec);
    vnic_api_status_to_proto_status(&api_info->status, proto_status);
    vnic_api_stats_to_proto_stats(&api_info->stats, proto_stats);
}

Status
VnicSvcImpl::VnicGet(ServerContext *context,
                     const pds::VnicGetRequest *proto_req,
                     pds::VnicGetResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_vnic_key_t key = {0};
    pds_vnic_info_t info = {0};

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }

    for (int i = 0; i < proto_req->vnicid_size(); i++) {
        key.id = proto_req->vnicid(i);
        ret = core::vnic_get(&key, &info);
        if (ret != SDK_RET_OK) {
            proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
            break;
        }
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OK);
        vnic_api_info_to_proto(&info, proto_rsp);
    }

    if (proto_req->vnicid_size() == 0) {
        ret = core::vnic_get_all(vnic_api_info_to_proto, proto_rsp);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    }
    return Status::OK;
}
