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
subnet_api_spec_to_proto_spec (pds::SubnetSpec *proto_spec,
                               const pds_subnet_spec_t *api_spec)
{
    proto_spec->set_id(api_spec->key.id);
    proto_spec->set_vpcid(api_spec->vcn.id);
    ipv4pfx_api_spec_to_proto_spec(
                    proto_spec->mutable_v4prefix(), &api_spec->v4_pfx);
    ippfx_api_spec_to_proto_spec(
                    proto_spec->mutable_v6prefix(), &api_spec->v6_pfx);
    proto_spec->set_ipv4virtualrouterip(api_spec->v4_vr_ip);
    proto_spec->set_ipv6virtualrouterip(&api_spec->v6_vr_ip.addr.v6_addr.addr8,
                                        IP6_ADDR8_LEN);
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
subnet_api_status_to_proto_status (pds::SubnetStatus *proto_status,
                                   const pds_subnet_status_t *api_status)
{
}

// Populate proto buf stats from subnet API stats
static inline void
subnet_api_stats_to_proto_stats (pds::SubnetStats *proto_stats,
                                 const pds_subnet_stats_t *api_stats)
{
}

// Populate proto buf from subnet API info
static inline void
subnet_api_info_to_proto (const pds_subnet_info_t *api_info, void *ctxt)
{
    pds::SubnetGetResponse *proto_rsp = (pds::SubnetGetResponse *)ctxt;
    auto subnet = proto_rsp->add_response();
    pds::SubnetSpec *proto_spec = subnet->mutable_spec();
    pds::SubnetStatus *proto_status = subnet->mutable_status();
    pds::SubnetStats *proto_stats = subnet->mutable_stats();

    subnet_api_spec_to_proto_spec(proto_spec, &api_info->spec);
    subnet_api_status_to_proto_status(proto_status, &api_info->status);
    subnet_api_stats_to_proto_stats(proto_stats, &api_info->stats);
}

// Build subnet API spec from proto buf spec
static inline void
subnet_proto_spec_to_api_spec (pds_subnet_spec_t *api_spec,
                               const pds::SubnetSpec &proto_spec)
{
    api_spec->key.id = proto_spec.id();
    api_spec->vcn.id = proto_spec.vpcid();
    ipv4pfx_proto_spec_to_api_spec(&api_spec->v4_pfx, proto_spec.v4prefix());
    ippfx_proto_spec_to_api_spec(&api_spec->v6_pfx, proto_spec.v6prefix());
    api_spec->v4_vr_ip = proto_spec.ipv4virtualrouterip();
    api_spec->v6_vr_ip.af = IP_AF_IPV6;
    memcpy(api_spec->v6_vr_ip.addr.v6_addr.addr8,
           proto_spec.ipv6virtualrouterip().c_str(), IP6_ADDR8_LEN);  
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
        subnet_proto_spec_to_api_spec(api_spec, request);
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
    if (proto_req->id_size() == 0) {
        // get all
        ret = core::subnet_get_all(subnet_api_info_to_proto, proto_rsp);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    }
    for (int i = 0; i < proto_req->id_size(); i++) {
        memset(&key, 0, sizeof(pds_subnet_key_t));
        key.id = proto_req->id(i);
        ret = core::subnet_get(&key, &info);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        if (ret != sdk::SDK_RET_OK) {
            break;
        }
        auto response = proto_rsp->add_response();
        subnet_api_spec_to_proto_spec(
                response->mutable_spec(), &info.spec);
        subnet_api_status_to_proto_status(
                response->mutable_status(), &info.status);
        subnet_api_stats_to_proto_stats(
                response->mutable_stats(), &info.stats);
    }
    return Status::OK;
}
