//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_subnet.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/subnet.hpp"
#include "nic/apollo/agent/svc/util.hpp"
#include "nic/apollo/agent/svc/subnet.hpp"

// Populate proto buf spec from subnet API spec
static inline void
subnet_api_spec_to_proto_spec_fill (const pds_subnet_spec_t *api_spec,
                                    pds::SubnetSpec *proto_spec)
{
    proto_spec->set_id(api_spec->key.id);
    proto_spec->set_pcnid(api_spec->vcn.id);
    ippfx_api_spec_to_proto_spec_fill(
                    &api_spec->pfx, proto_spec->mutable_prefix());
    ipaddr_api_spec_to_proto_spec_fill(
                    &api_spec->vr_ip, proto_spec->mutable_virtualrouterip());
    proto_spec->set_virtualroutermac(MAC_TO_UINT64(api_spec->vr_mac));
    proto_spec->set_v4routetableid(api_spec->v4_route_table.id);
    proto_spec->set_v6routetableid(api_spec->v6_route_table.id);
    proto_spec->set_ingv4securitypolicyid(api_spec->ing_v4_policy.id);
    proto_spec->set_ingv6securitypolicyid(api_spec->ing_v6_policy.id);
    proto_spec->set_egv4securitypolicyid(api_spec->egr_v4_policy.id);
    proto_spec->set_egv6securitypolicyid(api_spec->egr_v6_policy.id);
}

// Populate proto buf status from subnet API status
static inline void
subnet_api_status_to_proto_status_fill (const pds_subnet_status_t *api_status,
                                        pds::SubnetStatus *proto_status)
{
}

// Populate proto buf stats from subnet API stats
static inline void
subnet_api_stats_to_proto_stats_fill (const pds_subnet_stats_t *api_stats,
                                      pds::SubnetStats *proto_stats)
{
}

// Build subnet API spec from proto buf spec
static inline void
subnet_proto_spec_to_api_spec_fill (const pds::SubnetSpec &proto_spec,
                                    pds_subnet_spec_t *api_spec)
{
    api_spec->key.id = proto_spec.id();
    api_spec->vcn.id = proto_spec.pcnid();
    ippfx_proto_spec_to_api_spec_fill(proto_spec.prefix(), &api_spec->pfx);
    ipaddr_proto_spec_to_api_spec_fill(
                            proto_spec.virtualrouterip(), &api_spec->vr_ip);
    MAC_UINT64_TO_ADDR(api_spec->vr_mac, proto_spec.virtualroutermac());
    api_spec->v4_route_table.id = proto_spec.v4routetableid();
    api_spec->v6_route_table.id = proto_spec.v6routetableid();
    api_spec->ing_v4_policy.id = proto_spec.ingv4securitypolicyid();
    api_spec->ing_v6_policy.id = proto_spec.ingv6securitypolicyid();
    api_spec->egr_v4_policy.id = proto_spec.egv4securitypolicyid();
    api_spec->egr_v6_policy.id = proto_spec.egv6securitypolicyid();
}

Status
SubnetSvcImpl::SubnetCreate(ServerContext *context,
                            const pds::SubnetRequest *proto_req,
                            pds::SubnetResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_subnet_key_t key;
    pds_subnet_spec_t *api_spec;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    for (int i = 0; i < proto_req->request_size(); i ++) {
        api_spec = (pds_subnet_spec_t *)
                    core::agent_state::state()->subnet_slab()->alloc();
        if (api_spec == NULL) {
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OUT_OF_MEM);
            break;
        }
        auto request = proto_req->request(i);
        memset(&key, 0, sizeof(pds_subnet_key_t));
        key.id = request.id();
        subnet_proto_spec_to_api_spec_fill(request, api_spec);
        ret = core::subnet_create(&key, api_spec);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        if (ret != sdk::SDK_RET_OK) {
            break;
        }
    }
    return Status::OK;
}

Status
SubnetSvcImpl::SubnetDelete(ServerContext *context,
                            const pds::SubnetDeleteRequest *proto_req,
                            pds::SubnetDeleteResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_subnet_key_t key;

    if (proto_req == NULL) {
        proto_rsp->add_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    for (int i = 0; i < proto_req->id_size(); i++) {
        memset(&key, 0, sizeof(pds_subnet_key_t));
        key.id = proto_req->id(i);
        ret = core::subnet_delete(&key);
        proto_rsp->add_apistatus(sdk_ret_to_api_status(ret));
    }
    return Status::OK;
}

Status
SubnetSvcImpl::SubnetGet(ServerContext *context,
                         const pds::SubnetGetRequest *proto_req,
                         pds::SubnetGetResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_subnet_key_t key;
    pds_subnet_info_t info;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    for (int i = 0; i < proto_req->id_size(); i++) {
        memset(&key, 0, sizeof(pds_subnet_key_t));
        key.id = proto_req->id(i);
        ret = core::subnet_get(&key, &info);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        if (ret != sdk::SDK_RET_OK) {
            break;
        }
        subnet_api_spec_to_proto_spec_fill(
                &info.spec, proto_rsp->add_response()->mutable_spec());
        subnet_api_status_to_proto_status_fill(
                &info.status, proto_rsp->add_response()->mutable_status());
        subnet_api_stats_to_proto_stats_fill(
                &info.stats, proto_rsp->add_response()->mutable_stats());
    }
    return Status::OK;
}
