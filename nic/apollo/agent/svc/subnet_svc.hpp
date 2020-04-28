//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines protobuf API for subnet object
///
//----------------------------------------------------------------------------

#ifndef __AGENT_SVC_SUBNET_SVC_HPP__
#define __AGENT_SVC_SUBNET_SVC_HPP__

#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/api/include/pds_subnet.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/svc/subnet.hpp"
#include "nic/apollo/agent/trace.hpp"
#include "nic/apollo/agent/hooks.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_subnet.hpp"

// build subnet API spec from proto buf spec
static inline sdk_ret_t
pds_subnet_proto_to_api_spec (pds_subnet_spec_t *api_spec,
                              const pds::SubnetSpec &proto_spec)
{
    pds_obj_key_proto_to_api_spec(&api_spec->key, proto_spec.id());
    pds_obj_key_proto_to_api_spec(&api_spec->vpc, proto_spec.vpcid());
    ipv4pfx_proto_spec_to_api_spec(&api_spec->v4_prefix, proto_spec.v4prefix());
    ipv6pfx_proto_spec_to_ippfx_api_spec(&api_spec->v6_prefix,
                                         proto_spec.v6prefix());
    api_spec->v4_vr_ip = proto_spec.ipv4virtualrouterip();
    api_spec->v6_vr_ip.af = IP_AF_IPV6;
    memcpy(api_spec->v6_vr_ip.addr.v6_addr.addr8,
           proto_spec.ipv6virtualrouterip().c_str(), IP6_ADDR8_LEN);
    MAC_UINT64_TO_ADDR(api_spec->vr_mac, proto_spec.virtualroutermac());
    pds_obj_key_proto_to_api_spec(&api_spec->v4_route_table,
                                  proto_spec.v4routetableid());
    pds_obj_key_proto_to_api_spec(&api_spec->v6_route_table,
                                  proto_spec.v6routetableid());

    if (proto_spec.ingv4securitypolicyid_size() > PDS_MAX_SUBNET_POLICY) {
        PDS_TRACE_ERR("No. of IPv4 ingress security policies on subnet can't "
                      "exceed {}", PDS_MAX_SUBNET_POLICY);
        return SDK_RET_INVALID_ARG;
    }
    if (proto_spec.dhcppolicyid_size() > PDS_MAX_SUBNET_POLICY) {
        PDS_TRACE_ERR("No. of DHCP policies on subnet can't "
                      "exceed {}", PDS_MAX_SUBNET_DHCP_POLICY);
        return SDK_RET_INVALID_ARG;
    }
    api_spec->num_ing_v4_policy = proto_spec.ingv4securitypolicyid_size();
    for (uint8_t i = 0; i < api_spec->num_ing_v4_policy; i++) {
        pds_obj_key_proto_to_api_spec(&api_spec->ing_v4_policy[i],
                                      proto_spec.ingv4securitypolicyid(i));
    }
    if (proto_spec.ingv6securitypolicyid_size() > PDS_MAX_SUBNET_POLICY) {
        PDS_TRACE_ERR("No. of IPv6 ingress security policies on subnet can't "
                      "exceed {}", PDS_MAX_SUBNET_POLICY);
        return SDK_RET_INVALID_ARG;
    }
    api_spec->num_ing_v6_policy = proto_spec.ingv6securitypolicyid_size();
    for (uint8_t i = 0; i < api_spec->num_ing_v6_policy; i++) {
        pds_obj_key_proto_to_api_spec(&api_spec->ing_v6_policy[i],
                                      proto_spec.ingv6securitypolicyid(i));
    }
    if (proto_spec.egv4securitypolicyid_size() > PDS_MAX_SUBNET_POLICY) {
        PDS_TRACE_ERR("No. of IPv4 egress security policies on subnet can't "
                      "exceed {}", PDS_MAX_SUBNET_POLICY);
        return SDK_RET_INVALID_ARG;
    }
    api_spec->num_egr_v4_policy = proto_spec.egv4securitypolicyid_size();
    for (uint8_t i = 0; i < api_spec->num_egr_v4_policy; i++) {
        pds_obj_key_proto_to_api_spec(&api_spec->egr_v4_policy[i],
                                      proto_spec.egv4securitypolicyid(i));
    }
    if (proto_spec.egv6securitypolicyid_size() > PDS_MAX_SUBNET_POLICY) {
        PDS_TRACE_ERR("No. of IPv6 egress security policies on subnet can't "
                      "exceed {}", PDS_MAX_SUBNET_POLICY);
        return SDK_RET_INVALID_ARG;
    }
    api_spec->num_egr_v6_policy = proto_spec.egv6securitypolicyid_size();
    for (uint8_t i = 0; i < api_spec->num_egr_v6_policy; i++) {
        pds_obj_key_proto_to_api_spec(&api_spec->egr_v6_policy[i],
                                      proto_spec.egv6securitypolicyid(i));
    }
    api_spec->fabric_encap = proto_encap_to_pds_encap(proto_spec.fabricencap());
    // currently one host interface per subnet is supported
    if (proto_spec.hostif_size() > 0) {
        pds_obj_key_proto_to_api_spec(&api_spec->host_if, proto_spec.hostif(0));
    }
    api_spec->num_dhcp_policy = proto_spec.dhcppolicyid_size();
    for (uint8_t i = 0; i < api_spec->num_dhcp_policy; i++) {
        pds_obj_key_proto_to_api_spec(&api_spec->dhcp_policy[i],
                                      proto_spec.dhcppolicyid(i));
    }
    api_spec->tos = proto_spec.tos();
    return SDK_RET_OK;
}

// populate proto buf spec from subnet API spec
static inline void
pds_subnet_api_spec_to_proto (pds::SubnetSpec *proto_spec,
                              const pds_subnet_spec_t *api_spec)
{
    proto_spec->set_id(api_spec->key.id, PDS_MAX_KEY_LEN);
    proto_spec->set_vpcid(api_spec->vpc.id, PDS_MAX_KEY_LEN);
    ipv4pfx_api_spec_to_proto_spec(
                    proto_spec->mutable_v4prefix(), &api_spec->v4_prefix);
    ippfx_api_spec_to_ipv6pfx_proto_spec(
                    proto_spec->mutable_v6prefix(), &api_spec->v6_prefix);
    proto_spec->set_ipv4virtualrouterip(api_spec->v4_vr_ip);
    proto_spec->set_ipv6virtualrouterip(&api_spec->v6_vr_ip.addr.v6_addr.addr8,
                                        IP6_ADDR8_LEN);
    proto_spec->set_virtualroutermac(MAC_TO_UINT64(api_spec->vr_mac));
    proto_spec->set_v4routetableid(api_spec->v4_route_table.id,
                                   PDS_MAX_KEY_LEN);
    proto_spec->set_v6routetableid(api_spec->v6_route_table.id,
                                   PDS_MAX_KEY_LEN);

    for (uint8_t i = 0; i < api_spec->num_ing_v4_policy; i++) {
        proto_spec->add_ingv4securitypolicyid(api_spec->ing_v4_policy[i].id,
                                              PDS_MAX_KEY_LEN);
    }
    for (uint8_t i = 0; i < api_spec->num_ing_v6_policy; i++) {
        proto_spec->add_ingv6securitypolicyid(api_spec->ing_v6_policy[i].id,
                                              PDS_MAX_KEY_LEN);
    }
    for (uint8_t i = 0; i < api_spec->num_egr_v4_policy; i++) {
        proto_spec->add_egv4securitypolicyid(api_spec->egr_v4_policy[i].id,
                                             PDS_MAX_KEY_LEN);
    }
    for (uint8_t i = 0; i < api_spec->num_egr_v6_policy; i++) {
        proto_spec->add_egv6securitypolicyid(api_spec->egr_v6_policy[i].id,
                                             PDS_MAX_KEY_LEN);
    }
    pds_encap_to_proto_encap(proto_spec->mutable_fabricencap(),
                             &api_spec->fabric_encap);
    proto_spec->add_hostif(api_spec->host_if.id, PDS_MAX_KEY_LEN);
    for (uint8_t i = 0; i < api_spec->num_dhcp_policy; i++) {
        proto_spec->add_dhcppolicyid(api_spec->dhcp_policy[i].id,
                                     PDS_MAX_KEY_LEN);
    }
    proto_spec->set_tos(api_spec->tos);
}

// populate proto buf status from subnet API status
static inline void
pds_subnet_api_status_to_proto (pds::SubnetStatus *proto_status,
                                const pds_subnet_status_t *api_status)
{
    proto_status->set_hwid(api_status->hw_id);
}

// populate proto buf stats from subnet API stats
static inline void
pds_subnet_api_stats_to_proto (pds::SubnetStats *proto_stats,
                               const pds_subnet_stats_t *api_stats)
{
}

// populate proto buf from subnet API info
static inline void
pds_subnet_api_info_to_proto (pds_subnet_info_t *api_info, void *ctxt)
{
    pds::SubnetGetResponse *proto_rsp = (pds::SubnetGetResponse *)ctxt;
    auto subnet = proto_rsp->add_response();
    pds::SubnetSpec *proto_spec = subnet->mutable_spec();
    pds::SubnetStatus *proto_status = subnet->mutable_status();
    pds::SubnetStats *proto_stats = subnet->mutable_stats();

    pds_subnet_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_subnet_api_status_to_proto(proto_status, &api_info->status);
    pds_subnet_api_stats_to_proto(proto_stats, &api_info->stats);
}

static inline sdk_ret_t
pds_svc_subnet_create (const pds::SubnetRequest *proto_req,
                     pds::SubnetResponse *proto_rsp)
{
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    pds_subnet_spec_t api_spec;
    bool batched_internally = false;
    pds_batch_params_t batch_params;

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
            PDS_TRACE_ERR("Failed to create a new batch, subnet creation "
                          "failed");
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return SDK_RET_ERR;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->request_size(); i ++) {
        memset(&api_spec, 0, sizeof(pds_subnet_spec_t));
        auto request = proto_req->request(i);
        pds_subnet_proto_to_api_spec(&api_spec, request);
        if (core::agent_state::state()->device()->overlay_routing_en) {
            // call the metaswitch api
            ret = pds_ms::subnet_create(&api_spec, bctxt);
        } else if (!core::agent_state::state()->pds_mock_mode()) {
            ret = pds_subnet_create(&api_spec, bctxt);
        }
        if (ret != SDK_RET_OK) {
            goto end;
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
pds_svc_subnet_update (const pds::SubnetRequest *proto_req,
                     pds::SubnetResponse *proto_rsp)
{
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    pds_subnet_spec_t api_spec;
    bool batched_internally = false;
    pds_batch_params_t batch_params;

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
            PDS_TRACE_ERR("Failed to create a new batch, subnet update failed");
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return SDK_RET_ERR;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->request_size(); i ++) {
        memset(&api_spec, 0, sizeof(pds_subnet_spec_t));
        auto request = proto_req->request(i);
        pds_subnet_proto_to_api_spec(&api_spec, request);
        if (core::agent_state::state()->device()->overlay_routing_en) {
            // call the metaswitch api
            ret = pds_ms::subnet_update(&api_spec, bctxt);
        } else if (!core::agent_state::state()->pds_mock_mode()) {
            ret = pds_subnet_update(&api_spec, bctxt);
        }
        if (ret != SDK_RET_OK) {
            goto end;
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
pds_svc_subnet_delete (const pds::SubnetDeleteRequest *proto_req,
                     pds::SubnetDeleteResponse *proto_rsp)
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
            PDS_TRACE_ERR("Failed to create a new batch, subnet delete failed");
            proto_rsp->add_apistatus(types::ApiStatus::API_STATUS_ERR);
            return SDK_RET_ERR;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->id_size(); i++) {
        pds_obj_key_proto_to_api_spec(&key, proto_req->id(i));
        if (core::agent_state::state()->device()->overlay_routing_en) {
            // call the metaswitch api
            ret = pds_ms::subnet_delete(key, bctxt);
        } else if (!core::agent_state::state()->pds_mock_mode()) {
            ret = pds_subnet_delete(&key, bctxt);
        }
        if (ret != SDK_RET_OK) {
            goto end;
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
pds_svc_subnet_get (const pds::SubnetGetRequest *proto_req,
                  pds::SubnetGetResponse *proto_rsp)
{
    sdk_ret_t ret;
    pds_obj_key_t key;
    pds_subnet_info_t info = { 0 };

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return SDK_RET_INVALID_ARG;
    }
    if (proto_req->id_size() == 0) {
        // read all
        ret = pds_subnet_read_all(pds_subnet_api_info_to_proto, proto_rsp);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    }
    for (int i = 0; i < proto_req->id_size(); i++) {
        pds_obj_key_proto_to_api_spec(&key, proto_req->id(i));
        ret = pds_subnet_read(&key, &info);
        if (ret != SDK_RET_OK) {
            proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
            break;
        }
        pds_subnet_api_info_to_proto(&info, proto_rsp);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    }
    return ret;
}

static inline sdk_ret_t
pds_svc_subnet_handle_cfg (cfg_ctxt_t *ctxt, google::protobuf::Any *any_resp)
{
    sdk_ret_t ret;
    google::protobuf::Any *any_req = (google::protobuf::Any *)ctxt->req;

    switch (ctxt->cfg) {
    case CFG_MSG_SUBNET_CREATE:
        {
            pds::SubnetRequest req;
            pds::SubnetResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_subnet_create(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    case CFG_MSG_SUBNET_UPDATE:
        {
            pds::SubnetRequest req;
            pds::SubnetResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_subnet_update(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    case CFG_MSG_SUBNET_DELETE:
        {
            pds::SubnetDeleteRequest req;
            pds::SubnetDeleteResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_subnet_delete(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    case CFG_MSG_SUBNET_GET:
        {
            pds::SubnetGetRequest req;
            pds::SubnetGetResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_subnet_get(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    default:
        ret = SDK_RET_INVALID_ARG;
        break;
    }

    return ret;
}

#endif    // __AGENT_SVC_SUBNET_HPP__
