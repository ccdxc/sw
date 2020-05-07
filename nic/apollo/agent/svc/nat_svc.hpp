//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines protobuf API for nat object
///
//----------------------------------------------------------------------------

#ifndef __AGENT_SVC_NAT_SVC_HPP__
#define __AGENT_SVC_NAT_SVC_HPP__

#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/api/include/pds_nat.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/svc/nat.hpp"
#include "nic/apollo/agent/trace.hpp"

static inline sdk_ret_t
pds_nat_port_block_proto_to_api_spec (pds_nat_port_block_spec_t *api_spec,
                                      const pds::NatPortBlockSpec &proto_spec)
{
    pds_obj_key_proto_to_api_spec(&api_spec->key, proto_spec.id());
    pds_obj_key_proto_to_api_spec(&api_spec->vpc, proto_spec.vpcid());
    api_spec->ip_proto = proto_spec.protocol();
    if (proto_spec.nataddress().has_prefix()) {
        ipsubnet_proto_spec_to_ipvx_range(&api_spec->nat_ip_range,
                                          proto_spec.nataddress().prefix());
    } else if (proto_spec.nataddress().has_range()) {
        iprange_proto_spec_to_api_spec(&api_spec->nat_ip_range,
                                       proto_spec.nataddress().range());
    }
    api_spec->nat_port_range.port_lo = proto_spec.ports().portlow();
    api_spec->nat_port_range.port_hi = proto_spec.ports().porthigh();
    switch (proto_spec.addresstype()) {
    case types::ADDR_TYPE_PUBLIC:
        api_spec->address_type = ADDR_TYPE_PUBLIC;
        break;
    case types::ADDR_TYPE_SERVICE:
        api_spec->address_type = ADDR_TYPE_SERVICE;
        break;
    default:
        SDK_ASSERT(FALSE);
        return SDK_RET_INVALID_ARG;
    }
    api_spec->threshold = proto_spec.threshold();
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_nat_port_block_api_spec_to_proto (pds::NatPortBlockSpec *proto_spec,
                                      const pds_nat_port_block_spec_t *api_spec)
{
    proto_spec->set_id(api_spec->key.id, PDS_MAX_KEY_LEN);
    proto_spec->set_vpcid(api_spec->vpc.id, PDS_MAX_KEY_LEN);
    proto_spec->set_protocol(api_spec->ip_proto);
    auto range_spec = proto_spec->mutable_nataddress()->mutable_range();
    iprange_api_spec_to_proto_spec(range_spec, &api_spec->nat_ip_range);
    proto_spec->mutable_ports()->set_portlow(api_spec->nat_port_range.port_lo);
    proto_spec->mutable_ports()->set_porthigh(api_spec->nat_port_range.port_hi);
    switch (api_spec->address_type) {
    case ADDR_TYPE_PUBLIC:
        proto_spec->set_addresstype(types::ADDR_TYPE_PUBLIC);
        break;
    case ADDR_TYPE_SERVICE:
        proto_spec->set_addresstype(types::ADDR_TYPE_SERVICE);
        break;
    default:
        SDK_ASSERT(FALSE);
        return SDK_RET_INVALID_ARG;
    }
    proto_spec->set_threshold(api_spec->threshold);
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_nat_port_block_api_status_to_proto (pds::NatPortBlockStatus *proto_status,
                                        const pds_nat_port_block_status_t *api_status)
{
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_nat_port_block_api_stats_to_proto (pds::NatPortBlockStats *proto_stats,
                                       const pds_nat_port_block_stats_t *api_stats)
{
    proto_stats->set_inusecount(api_stats->in_use_count);
    proto_stats->set_sessioncount(api_stats->session_count);
    return SDK_RET_OK;
}

// populate proto buf from route table API info
static inline void
pds_nat_port_block_api_info_to_proto (const pds_nat_port_block_info_t *api_info,
                                      void *ctxt)
{
    pds::NatPortBlockGetResponse *proto_rsp =
        (pds::NatPortBlockGetResponse *)ctxt;
    auto nat = proto_rsp->add_response();
    pds::NatPortBlockSpec *proto_spec = nat->mutable_spec();
    pds::NatPortBlockStatus *proto_status = nat->mutable_status();
    pds::NatPortBlockStats *proto_stats = nat->mutable_stats();

    pds_nat_port_block_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_nat_port_block_api_status_to_proto(proto_status, &api_info->status);
    pds_nat_port_block_api_stats_to_proto(proto_stats, &api_info->stats);
}

static inline sdk_ret_t
pds_svc_nat_port_block_create (const pds::NatPortBlockRequest *proto_req,
                               pds::NatPortBlockResponse *proto_rsp)
{
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    bool batched_internally = false;
    pds_batch_params_t batch_params;
    pds_nat_port_block_spec_t api_spec;

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
            PDS_TRACE_ERR("Failed to create a new batch, NAT port block "
                          "creation failed");
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return SDK_RET_ERR;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->request_size(); i ++) {
        memset(&api_spec, 0, sizeof(api_spec));
        auto request = proto_req->request(i);
        pds_nat_port_block_proto_to_api_spec(&api_spec, request);
        if (!core::agent_state::state()->pds_mock_mode()) {
            ret = pds_nat_port_block_create(&api_spec, bctxt);
            if (ret != SDK_RET_OK) {
                goto end;
            }
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
pds_svc_nat_port_block_delete (const pds::NatPortBlockDeleteRequest *proto_req,
                               pds::NatPortBlockDeleteResponse *proto_rsp)
{
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    pds_obj_key_t key = { 0 };
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
            PDS_TRACE_ERR("Failed to create a new batch, NAT port block delete "
                          "failed");
            proto_rsp->add_apistatus(types::ApiStatus::API_STATUS_ERR);
            return SDK_RET_ERR;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->id_size(); i++) {
        pds_obj_key_proto_to_api_spec(&key, proto_req->id(i));
        ret = pds_nat_port_block_delete(&key, bctxt);
        proto_rsp->add_apistatus(sdk_ret_to_api_status(ret));
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
pds_svc_nat_port_block_get (const pds::NatPortBlockGetRequest *proto_req,
                            pds::NatPortBlockGetResponse *proto_rsp)
{
    sdk_ret_t ret;
    pds_obj_key_t key = { 0 };
    pds_nat_port_block_info_t info = { 0 };

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return SDK_RET_INVALID_ARG;
    }
    if (proto_req->id_size() == 0) {
        // get all
        ret = pds_nat_port_block_read_all(pds_nat_port_block_api_info_to_proto,
                                          proto_rsp);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    }

    for (int i = 0; i < proto_req->id_size(); i ++) {
        pds_obj_key_proto_to_api_spec(&key, proto_req->id(i));
        ret = pds_nat_port_block_read(&key, &info);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
        if (ret != SDK_RET_OK) {
            break;
        }
        pds_nat_port_block_api_info_to_proto(&info, proto_rsp);
    }
    return ret;
}

static inline sdk_ret_t
pds_svc_nat_port_block_handle_cfg (cfg_ctxt_t *ctxt, google::protobuf::Any *any_resp)
{
    sdk_ret_t ret;
    google::protobuf::Any *any_req = (google::protobuf::Any *)ctxt->req;

    switch (ctxt->cfg) {
    case CFG_MSG_NAT_PORT_BLOCK_CREATE:
        {
            pds::NatPortBlockRequest req;
            pds::NatPortBlockResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_nat_port_block_create(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    case CFG_MSG_NAT_PORT_BLOCK_DELETE:
        {
            pds::NatPortBlockDeleteRequest req;
            pds::NatPortBlockDeleteResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_nat_port_block_delete(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    case CFG_MSG_NAT_PORT_BLOCK_GET:
        {
            pds::NatPortBlockGetRequest req;
            pds::NatPortBlockGetResponse rsp;

            any_req->UnpackTo(&req);
            ret = pds_svc_nat_port_block_get(&req, &rsp);
            any_resp->PackFrom(rsp);
        }
        break;
    default:
        ret = SDK_RET_INVALID_ARG;
        break;
    }

    return ret;
}

#endif    //__AGENT_SVC_NAT_SVC_HPP__
