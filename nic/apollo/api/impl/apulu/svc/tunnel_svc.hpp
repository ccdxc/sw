//------------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
///
/// \file
/// apulu pipeline api info to proto and vice versa implementation for tunnel
///
//------------------------------------------------------------------------------

#ifndef __APULU_SVC_TUNNEL_SVC_HPP__
#define __APULU_SVC_TUNNEL_SVC_HPP__

#include "grpc++/grpc++.h"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/tep.hpp"
#include "nic/apollo/api/impl/apulu/svc/specs.hpp"
#include "gen/proto/tunnel.grpc.pb.h"

// populate proto buf spec from tep API spec
static inline sdk_ret_t
pds_tep_api_spec_to_proto (pds::TunnelSpec *proto_spec,
                           const pds_tep_spec_t *api_spec)
{
    if (!api_spec || !proto_spec) {
        return SDK_RET_INVALID_ARG;
    }
    proto_spec->set_id(api_spec->key.id, PDS_MAX_KEY_LEN);
    switch (api_spec->type) {
    case PDS_TEP_TYPE_WORKLOAD:
        proto_spec->set_type(pds::TUNNEL_TYPE_WORKLOAD);
        break;
    case PDS_TEP_TYPE_IGW:
        proto_spec->set_type(pds::TUNNEL_TYPE_IGW);
        break;
    case PDS_TEP_TYPE_INTER_DC:
        proto_spec->set_type(pds::TUNNEL_TYPE_INTER_DC);
        break;
    case PDS_TEP_TYPE_SERVICE:
        proto_spec->set_type(pds::TUNNEL_TYPE_SERVICE);
        break;
    case PDS_TEP_TYPE_NONE:
    default:
        proto_spec->set_type(pds::TUNNEL_TYPE_NONE);
        break;
    }
    return SDK_RET_OK;
}

// populate proto buf status from tep API status
static inline void
pds_tep_api_status_to_proto (pds::TunnelStatus *proto_status,
                             const pds_tep_status_t *api_status)
{
    proto_status->set_hwid(api_status->hw_id1_);
    proto_status->set_hwid2(api_status->hw_id2_);
}

// populate proto buf spec from tep API spec
static inline void
pds_tep_api_info_to_proto (pds_tep_info_t *api_info, void *ctxt)
{
    pds::TunnelGetResponse *proto_rsp = (pds::TunnelGetResponse *)ctxt;
    auto tep = proto_rsp->add_response();
    pds::TunnelSpec *proto_spec = tep->mutable_spec();
    pds::TunnelStatus *proto_status = tep->mutable_status();

    pds_tep_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_tep_api_status_to_proto(proto_status, &api_info->status);
}

// populate tep API spec from proto spec
static inline sdk_ret_t
pds_tep_proto_to_api_spec (pds_tep_spec_t *api_spec,
                           const pds::TunnelSpec &proto_spec)
{
    memset(api_spec, 0, sizeof(pds_tep_spec_t));
    pds_obj_key_proto_to_api_spec(&api_spec->key, proto_spec.id());
    switch (proto_spec.type()) {
    case pds::TUNNEL_TYPE_IGW:
        api_spec->type = PDS_TEP_TYPE_IGW;
        break;
    case pds::TUNNEL_TYPE_WORKLOAD:
        api_spec->type = PDS_TEP_TYPE_WORKLOAD;
        break;
    case pds::TUNNEL_TYPE_INTER_DC:
        api_spec->type = PDS_TEP_TYPE_INTER_DC;
        break;
    case pds::TUNNEL_TYPE_SERVICE:
        api_spec->type = PDS_TEP_TYPE_SERVICE;
        break;
    default:
        api_spec->type = PDS_TEP_TYPE_NONE;
        break;
    }
    return SDK_RET_OK;
}

// populate tep API status from proto status
static inline sdk_ret_t
pds_tep_proto_to_api_status (pds_tep_status_t *api_status,
                             const pds::TunnelStatus &proto_status)
{
    api_status->hw_id1_ = proto_status.hwid();
    api_status->hw_id2_ = proto_status.hwid2();
    return SDK_RET_OK;
}

// populate API info from tep proto response
static inline sdk_ret_t
pds_tep_proto_to_api_info (pds_tep_info_t *api_info,
                           pds::TunnelGetResponse *proto_rsp)
{
    pds_tep_spec_t *api_spec;
    pds_tep_status_t *api_status;
    pds::TunnelSpec proto_spec;
    pds::TunnelStatus proto_status;
    sdk_ret_t ret;

    SDK_ASSERT(proto_rsp->response_size() == 1);
    api_spec = &api_info->spec;
    api_status = &api_info->status;
    proto_spec = proto_rsp->mutable_response(0)->spec();
    proto_status = proto_rsp->mutable_response(0)->status();
    ret = pds_tep_proto_to_api_spec(api_spec, proto_spec);
    if (ret == SDK_RET_OK) {
        ret = pds_tep_proto_to_api_status(api_status, proto_status);
    } else {
        SDK_ASSERT(ret == SDK_RET_OK);
    }
    return ret;
}

#endif    // __APULU_SVC_TUNNEL_SVC_HPP__
