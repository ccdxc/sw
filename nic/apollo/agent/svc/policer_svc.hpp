//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines protobuf API for policer object
///
//----------------------------------------------------------------------------

#ifndef __AGENT_SVC_POLICER_SVC_HPP__
#define __AGENT_SVC_POLICER_SVC_HPP__

#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/svc/policer.hpp"

static inline pds::PolicerDir
pds_policer_dir_api_spec_to_proto (pds_policer_dir_t dir)
{
    switch (dir) {
    case PDS_POLICER_DIR_INGRESS:
        return pds::POLICER_DIR_INGRESS;
    case PDS_POLICER_DIR_EGRESS:
        return pds::POLICER_DIR_EGRESS;
    default:
        return pds::POLICER_DIR_NONE;
    }
}

static inline pds_policer_dir_t
pds_policer_dir_proto_to_api_spec (pds::PolicerDir dir)
{
    switch (dir) {
    case pds::POLICER_DIR_INGRESS:
        return PDS_POLICER_DIR_INGRESS;
    case pds::POLICER_DIR_EGRESS:
        return PDS_POLICER_DIR_EGRESS;
    default:
        return PDS_POLICER_DIR_NONE;
    }
}

// build policer api spec from proto buf spec
static inline sdk_ret_t
pds_policer_proto_to_api_spec (pds_policer_spec_t *api_spec,
                               const pds::PolicerSpec &proto_spec)
{
    pds_obj_key_proto_to_api_spec(&api_spec->key, proto_spec.id());
    api_spec->dir = pds_policer_dir_proto_to_api_spec(proto_spec.direction());
    if (proto_spec.has_ppspolicer()) {
        api_spec->type = sdk::POLICER_TYPE_PPS;
        api_spec->pps = proto_spec.ppspolicer().packetspersecond();
        api_spec->pps_burst = proto_spec.ppspolicer().burst();
    } else if (proto_spec.has_bpspolicer()) {
        api_spec->type = sdk::POLICER_TYPE_BPS;
        api_spec->bps = proto_spec.bpspolicer().bytespersecond();
        api_spec->bps_burst = proto_spec.bpspolicer().burst();
    }
    return SDK_RET_OK;
}

// populate proto buf spec from policer API spec
static inline void
pds_policer_api_spec_to_proto (pds::PolicerSpec *proto_spec,
                               const pds_policer_spec_t *api_spec)
{
    if (!api_spec || !proto_spec) {
        return;
    }
    proto_spec->set_id(api_spec->key.id, PDS_MAX_KEY_LEN);
    proto_spec->set_direction(pds_policer_dir_api_spec_to_proto(api_spec->dir));
    switch (api_spec->type) {
    case sdk::POLICER_TYPE_PPS:
        {
            auto pps = proto_spec->mutable_ppspolicer();
            pps->set_packetspersecond(api_spec->pps);
            pps->set_burst(api_spec->pps_burst);
        }
        break;
    case sdk::POLICER_TYPE_BPS:
        {
            auto bps = proto_spec->mutable_bpspolicer();
            bps->set_bytespersecond(api_spec->bps);
            bps->set_burst(api_spec->bps_burst);
        }
        break;
    default:
        break;
    }
}

// populate proto buf status from policer API status
static inline void
pds_policer_api_status_to_proto (pds::PolicerStatus *proto_status,
                                 const pds_policer_status_t *api_status)
{
}

// populate proto buf stats from policer API stats
static inline void
pds_policer_api_stats_to_proto (pds::PolicerStats *proto_stats,
                                const pds_policer_stats_t *api_stats)
{
}

// populate proto buf from policer API info
static inline void
pds_policer_api_info_to_proto (pds_policer_info_t *api_info, void *ctxt)
{
    pds::PolicerGetResponse *proto_rsp = (pds::PolicerGetResponse *)ctxt;
    auto policer = proto_rsp->add_response();
    pds::PolicerSpec *proto_spec = policer->mutable_spec();
    pds::PolicerStatus *proto_status = policer->mutable_status();
    pds::PolicerStats *proto_stats = policer->mutable_stats();

    pds_policer_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_policer_api_status_to_proto(proto_status, &api_info->status);
    pds_policer_api_stats_to_proto(proto_stats, &api_info->stats);
}

#endif    //__AGENT_SVC_POLICER_SVC_HPP__
