//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines protobuf API for vnic object
///
//----------------------------------------------------------------------------

#ifndef __AGENT_SVC_VNIC_SVC_HPP__
#define __AGENT_SVC_VNIC_SVC_HPP__

#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/api/include/pds_vnic.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/svc/vnic.hpp"
#include "nic/apollo/agent/trace.hpp"

// populate proto buf spec from vnic API spec
static inline void
pds_vnic_api_spec_to_proto (pds::VnicSpec *proto_spec,
                            const pds_vnic_spec_t *api_spec)
{
    if (!api_spec || !proto_spec) {
        return;
    }
    proto_spec->set_id(api_spec->key.id, PDS_MAX_KEY_LEN);
    proto_spec->set_hostname(api_spec->hostname);
    proto_spec->set_subnetid(api_spec->subnet.id, PDS_MAX_KEY_LEN);
    pds_encap_to_proto_encap(proto_spec->mutable_vnicencap(),
                             &api_spec->vnic_encap);
    proto_spec->set_macaddress(MAC_TO_UINT64(api_spec->mac_addr));
    //proto_spec->set_providermacaddress(
                        //MAC_TO_UINT64(api_spec->provider_mac_addr));
    pds_encap_to_proto_encap(proto_spec->mutable_fabricencap(),
                             &api_spec->fabric_encap);
    proto_spec->set_sourceguardenable(api_spec->binding_checks_en);
    proto_spec->set_v4meterid(api_spec->v4_meter.id, PDS_MAX_KEY_LEN);
    proto_spec->set_v6meterid(api_spec->v6_meter.id, PDS_MAX_KEY_LEN);
    if (api_spec->tx_mirror_session_bmap) {
        for (uint8_t i = 0; i < 8; i++) {
            if (api_spec->tx_mirror_session_bmap & (1 << i)) {
                proto_spec->add_txmirrorsessionid(i + 1);
            }
        }
    }
    if (api_spec->rx_mirror_session_bmap) {
        for (uint8_t i = 0; i < 8; i++) {
            if (api_spec->rx_mirror_session_bmap & (1 << i)) {
                proto_spec->add_rxmirrorsessionid(i + 1);
            }
        }
    }
    proto_spec->set_switchvnic(api_spec->switch_vnic);
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
    proto_spec->set_hostif(api_spec->host_if.id, PDS_MAX_KEY_LEN);
    proto_spec->set_txpolicerid(api_spec->tx_policer.id, PDS_MAX_KEY_LEN);
    proto_spec->set_rxpolicerid(api_spec->rx_policer.id, PDS_MAX_KEY_LEN);
    proto_spec->set_primary(api_spec->primary);
    proto_spec->set_maxsessions(api_spec->max_sessions);
    proto_spec->set_flowlearnen(api_spec->flow_learn_en);
    proto_spec->set_meteren(api_spec->meter_en);
}

// populate proto buf status from vnic API status
static inline void
pds_vnic_api_status_to_proto (pds::VnicStatus *proto_status,
                              const pds_vnic_status_t *api_status)
{
    proto_status->set_hwid(api_status->hw_id);
}

// populate proto buf stats from vnic API stats
static inline void
pds_vnic_api_stats_to_proto (pds::VnicStats *proto_stats,
                             const pds_vnic_stats_t *api_stats)
{
    proto_stats->set_txbytes(api_stats->tx_bytes);
    proto_stats->set_txpackets(api_stats->tx_pkts);
    proto_stats->set_rxbytes(api_stats->rx_bytes);
    proto_stats->set_rxpackets(api_stats->rx_pkts);
    proto_stats->set_metertxbytes(api_stats->meter_tx_bytes);
    proto_stats->set_metertxpackets(api_stats->meter_tx_pkts);
    proto_stats->set_meterrxbytes(api_stats->meter_rx_bytes);
    proto_stats->set_meterrxpackets(api_stats->meter_rx_pkts);
}

// populate proto buf from vnic API info
static inline void
pds_vnic_api_info_to_proto (pds_vnic_info_t *api_info, void *ctxt)
{
    pds::VnicGetResponse *proto_rsp = (pds::VnicGetResponse *)ctxt;
    auto vnic = proto_rsp->add_response();
    pds::VnicSpec *proto_spec = vnic->mutable_spec();
    pds::VnicStatus *proto_status = vnic->mutable_status();
    pds::VnicStats *proto_stats = vnic->mutable_stats();

    pds_vnic_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_vnic_api_status_to_proto(proto_status, &api_info->status);
    pds_vnic_api_stats_to_proto(proto_stats, &api_info->stats);
}

// build VNIC api spec from proto buf spec
static inline sdk_ret_t
pds_vnic_proto_to_api_spec (pds_vnic_spec_t *api_spec,
                            const pds::VnicSpec &proto_spec)
{
    uint32_t msid;

    pds_obj_key_proto_to_api_spec(&api_spec->key, proto_spec.id());
    if (proto_spec.hostname().empty()) {
        api_spec->hostname[0] = '\0';
    } else {
        strncpy(api_spec->hostname, proto_spec.hostname().c_str(),
                PDS_MAX_HOST_NAME_LEN);
         api_spec->hostname[PDS_MAX_HOST_NAME_LEN] = '\0';
    }
    pds_obj_key_proto_to_api_spec(&api_spec->subnet, proto_spec.subnetid());
    api_spec->vnic_encap = proto_encap_to_pds_encap(proto_spec.vnicencap());
    api_spec->fabric_encap = proto_encap_to_pds_encap(proto_spec.fabricencap());
    MAC_UINT64_TO_ADDR(api_spec->mac_addr, proto_spec.macaddress());
    api_spec->binding_checks_en = proto_spec.sourceguardenable();
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
    pds_obj_key_proto_to_api_spec(&api_spec->v4_meter, proto_spec.v4meterid());
    pds_obj_key_proto_to_api_spec(&api_spec->v6_meter, proto_spec.v6meterid());
    api_spec->switch_vnic = proto_spec.switchvnic();
    if (proto_spec.ingv4securitypolicyid_size() > PDS_MAX_VNIC_POLICY) {
        PDS_TRACE_ERR("No. of IPv4 ingress security policies on vnic can't "
                      "exceed {}", PDS_MAX_VNIC_POLICY);
        return SDK_RET_INVALID_ARG;
    }
    api_spec->num_ing_v4_policy = proto_spec.ingv4securitypolicyid_size();
    for (uint8_t i = 0; i < api_spec->num_ing_v4_policy; i++) {
        pds_obj_key_proto_to_api_spec(&api_spec->ing_v4_policy[i],
                                      proto_spec.ingv4securitypolicyid(i));
    }
    if (proto_spec.ingv6securitypolicyid_size() > PDS_MAX_VNIC_POLICY) {
        PDS_TRACE_ERR("No. of IPv6 ingress security policies on vnic can't "
                      "exceed {}", PDS_MAX_VNIC_POLICY);
        return SDK_RET_INVALID_ARG;
    }
    api_spec->num_ing_v6_policy = proto_spec.ingv6securitypolicyid_size();
    for (uint8_t i = 0; i < api_spec->num_ing_v6_policy; i++) {
        pds_obj_key_proto_to_api_spec(&api_spec->ing_v6_policy[i],
                                      proto_spec.ingv6securitypolicyid(i));
    }
    if (proto_spec.egv4securitypolicyid_size() > PDS_MAX_VNIC_POLICY) {
        PDS_TRACE_ERR("No. of IPv4 egress security policies on vnic can't "
                      "exceed {}", PDS_MAX_VNIC_POLICY);
        return SDK_RET_INVALID_ARG;
    }
    api_spec->num_egr_v4_policy = proto_spec.egv4securitypolicyid_size();
    for (uint8_t i = 0; i < api_spec->num_egr_v4_policy; i++) {
        pds_obj_key_proto_to_api_spec(&api_spec->egr_v4_policy[i],
                                      proto_spec.egv4securitypolicyid(i));
    }
    if (proto_spec.egv6securitypolicyid_size() > PDS_MAX_VNIC_POLICY) {
        PDS_TRACE_ERR("No. of IPv6 egress security policies on vnic can't "
                      "exceed {}", PDS_MAX_VNIC_POLICY);
        return SDK_RET_INVALID_ARG;
    }
    api_spec->num_egr_v6_policy = proto_spec.egv6securitypolicyid_size();
    for (uint8_t i = 0; i < api_spec->num_egr_v6_policy; i++) {
        pds_obj_key_proto_to_api_spec(&api_spec->egr_v6_policy[i],
                                      proto_spec.egv6securitypolicyid(i));
    }
    pds_obj_key_proto_to_api_spec(&api_spec->host_if, proto_spec.hostif());
    pds_obj_key_proto_to_api_spec(&api_spec->tx_policer,
                                  proto_spec.txpolicerid());
    pds_obj_key_proto_to_api_spec(&api_spec->rx_policer,
                                  proto_spec.rxpolicerid());
    api_spec->primary = proto_spec.primary();
    api_spec->max_sessions = proto_spec.maxsessions();
    api_spec->flow_learn_en = proto_spec.flowlearnen();
    api_spec->meter_en = proto_spec.meteren();
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_svc_vnic_create (const pds::VnicRequest *proto_req,
                     pds::VnicResponse *proto_rsp)
{
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    pds_vnic_spec_t api_spec;
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
            PDS_TRACE_ERR("Failed to create a new batch, vnic creation failed");
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return SDK_RET_ERR;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->request_size(); i ++) {
        memset(&api_spec, 0, sizeof(pds_vnic_spec_t));
        auto request = proto_req->request(i);
        ret = pds_vnic_proto_to_api_spec(&api_spec, request);
        if (ret != SDK_RET_OK) {
            goto end;
        }
        ret = pds_vnic_create(&api_spec, bctxt);
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
pds_svc_vnic_update (const pds::VnicRequest *proto_req,
                     pds::VnicResponse *proto_rsp)
{
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    pds_vnic_spec_t api_spec;
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
            PDS_TRACE_ERR("Failed to create a new batch, vnic update failed");
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return SDK_RET_ERR;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->request_size(); i ++) {
        memset(&api_spec, 0, sizeof(pds_vnic_spec_t));
        auto request = proto_req->request(i);
        ret = pds_vnic_proto_to_api_spec(&api_spec, request);
        if (ret != SDK_RET_OK) {
            goto end;
        }
        ret = pds_vnic_update(&api_spec, bctxt);
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
pds_svc_vnic_delete (const pds::VnicDeleteRequest *proto_req,
                     pds::VnicDeleteResponse *proto_rsp)
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
            PDS_TRACE_ERR("Failed to create a new batch, vnic delete failed");
            proto_rsp->add_apistatus(types::ApiStatus::API_STATUS_ERR);
            return SDK_RET_ERR;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->id_size(); i++) {
        pds_obj_key_proto_to_api_spec(&key, proto_req->id(i));
        ret = pds_vnic_delete(&key, bctxt);
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
pds_svc_vnic_get (const pds::VnicGetRequest *proto_req,
                  pds::VnicGetResponse *proto_rsp)
{
    sdk_ret_t ret;
    pds_obj_key_t key = { 0 };
    pds_vnic_info_t info = { 0 };

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return SDK_RET_INVALID_ARG;
    }

    for (int i = 0; i < proto_req->id_size(); i++) {
        pds_obj_key_proto_to_api_spec(&key, proto_req->id(i));
        ret = pds_vnic_read(&key, &info);
        if (ret != SDK_RET_OK) {
            proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
            break;
        }
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OK);
        pds_vnic_api_info_to_proto(&info, proto_rsp);
    }

    if (proto_req->id_size() == 0) {
        ret = pds_vnic_read_all(pds_vnic_api_info_to_proto, proto_rsp);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    }
    return ret;
}

static inline sdk_ret_t
pds_svc_vnic_handle_cfg (cfg_ctxt_t *ctxt,
                         google::protobuf::Any *any_resp)
{
    sdk_ret_t ret;
    google::protobuf::Any *any_req = (google::protobuf::Any *)ctxt->req;

    switch (ctxt->cfg) {
        case CFG_MSG_VNIC_CREATE:
        {
            pds::VnicRequest req;
            pds::VnicResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_vnic_create(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    case CFG_MSG_VNIC_UPDATE:
        {
            pds::VnicRequest req;
            pds::VnicResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_vnic_update(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    case CFG_MSG_VNIC_DELETE:
        {
            pds::VnicDeleteRequest req;
            pds::VnicDeleteResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_vnic_delete(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    case CFG_MSG_VNIC_GET:
        {
            pds::VnicGetRequest req;
            pds::VnicGetResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_vnic_get(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    default:
        ret = SDK_RET_INVALID_ARG;
        break;
    }

    return ret;
}

#endif    // __AGENT_SVC_VNIC_HPP__
