
//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines protobuf API for meter object
///
//----------------------------------------------------------------------------

#ifndef __AGENT_SVC_METER_SVC_HPP__
#define __AGENT_SVC_METER_SVC_HPP__

#include "nic/apollo/agent/svc/specs.hpp"

static inline void
pds_meter_debug_stats_to_proto (pds_meter_debug_stats_t *stats, void *ctxt)
{
    pds::MeterStatsGetResponse *rsp = (pds::MeterStatsGetResponse *)ctxt;
    auto proto_stats = rsp->add_stats();

    proto_stats->set_statsindex(stats->idx);
    proto_stats->set_rxbytes(stats->rx_bytes);
    proto_stats->set_txbytes(stats->tx_bytes);
}

// build Meter api spec from proto buf spec
static inline sdk_ret_t
pds_meter_proto_to_api_spec (pds_meter_spec_t *api_spec,
                             const pds::MeterSpec &proto_spec)
{
    sdk_ret_t ret;

    pds_obj_key_proto_to_api_spec(&api_spec->key, proto_spec.id());
    ret = pds_af_proto_spec_to_api_spec(&api_spec->af, proto_spec.af());
    if (ret != SDK_RET_OK) {
        return SDK_RET_INVALID_ARG;
    }

    api_spec->num_rules = proto_spec.rules_size();
    api_spec->rules =
        (pds_meter_rule_t *)SDK_MALLOC(PDS_MEM_ALLOC_ID_METER,
                        api_spec->num_rules * sizeof(pds_meter_rule_t));
    for (uint32_t rule = 0; rule < api_spec->num_rules; rule++) {
        const pds::MeterRuleSpec &proto_rule_spec = proto_spec.rules(rule);
        pds_meter_rule_t *api_rule_spec = &api_spec->rules[rule];
        if (proto_rule_spec.has_ppspolicer()) {
            api_rule_spec->type = PDS_METER_TYPE_PPS_POLICER;
            api_rule_spec->pps =
                            proto_rule_spec.ppspolicer().packetspersecond();
            api_rule_spec->pkt_burst = proto_rule_spec.ppspolicer().burst();
        } else if (proto_rule_spec.has_bpspolicer()) {
            api_rule_spec->type = PDS_METER_TYPE_BPS_POLICER;
            api_rule_spec->bps =
                            proto_rule_spec.bpspolicer().bytespersecond();
            api_rule_spec->byte_burst = proto_rule_spec.bpspolicer().burst();
        } else {
            api_rule_spec->type = PDS_METER_TYPE_ACCOUNTING;
        }

        api_rule_spec->priority = proto_rule_spec.priority();
        api_rule_spec->num_prefixes = proto_rule_spec.prefix_size();
        api_rule_spec->prefixes =
                 (ip_prefix_t *)SDK_MALLOC(PDS_MEM_ALLOC_ID_METER,
                            api_rule_spec->num_prefixes * sizeof(ip_prefix_t));
        for (int pfx = 0; pfx < proto_rule_spec.prefix_size(); pfx++) {
            ippfx_proto_spec_to_api_spec(
                    &api_rule_spec->prefixes[pfx], proto_rule_spec.prefix(pfx));
        }
    }
    return SDK_RET_OK;
}

// populate proto buf spec from meter API spec
static inline void
pds_meter_api_spec_to_proto (pds::MeterSpec *proto_spec,
                             const pds_meter_spec_t *api_spec)
{
    if (!api_spec || !proto_spec) {
        return;
    }
    proto_spec->set_id(api_spec->key.id, PDS_MAX_KEY_LEN);
    if (api_spec->af == IP_AF_IPV4) {
        proto_spec->set_af(types::IP_AF_INET);
    } else if (api_spec->af == IP_AF_IPV6) {
        proto_spec->set_af(types::IP_AF_INET6);
    } else {
        SDK_ASSERT(FALSE);
    }
    for (uint32_t rule = 0; rule < api_spec->num_rules; rule++) {
        pds::MeterRuleSpec *proto_rule_spec = proto_spec->add_rules();
        pds_meter_rule_t *api_rule_spec = &api_spec->rules[rule];
        switch (api_rule_spec->type) {
        case pds_meter_type_t::PDS_METER_TYPE_PPS_POLICER:
            proto_rule_spec->mutable_ppspolicer()->set_packetspersecond(
                                            api_rule_spec->pps);
            proto_rule_spec->mutable_ppspolicer()->set_burst(
                                            api_rule_spec->pkt_burst);
            break;

        case pds_meter_type_t::PDS_METER_TYPE_BPS_POLICER:
            proto_rule_spec->mutable_bpspolicer()->set_bytespersecond(
                                            api_rule_spec->bps);
            proto_rule_spec->mutable_bpspolicer()->set_burst(
                                            api_rule_spec->byte_burst);
            break;

        case pds_meter_type_t::PDS_METER_TYPE_ACCOUNTING:
            break;

        default:
            break;
        }
        proto_rule_spec->set_priority(api_rule_spec->priority);
        for (uint32_t pfx = 0;
                  pfx < api_rule_spec->num_prefixes; pfx++) {
            ippfx_api_spec_to_proto_spec(proto_rule_spec->add_prefix(),
                           &api_rule_spec->prefixes[pfx]);
        }
    }
    return;
}

// populate proto buf status from meter API status
static inline void
pds_meter_api_status_to_proto (pds::MeterStatus *proto_status,
                               const pds_meter_status_t *api_status)
{
}

// populate proto buf stats from meter API stats
static inline void
pds_meter_api_stats_to_proto (pds::MeterStats *proto_stats,
                              const pds_meter_stats_t *api_stats)
{
    proto_stats->set_meterid(api_stats->idx);
    proto_stats->set_rxbytes(api_stats->rx_bytes);
    proto_stats->set_txbytes(api_stats->tx_bytes);
}

// populate proto buf from meter API info
static inline void
pds_meter_api_info_to_proto (pds_meter_info_t *api_info, void *ctxt)
{
    pds::MeterGetResponse *proto_rsp = (pds::MeterGetResponse *)ctxt;
    auto meter = proto_rsp->add_response();
    pds::MeterSpec *proto_spec = meter->mutable_spec();
    pds::MeterStatus *proto_status = meter->mutable_status();
    pds::MeterStats *proto_stats = meter->mutable_stats();

    pds_meter_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_meter_api_status_to_proto(proto_status, &api_info->status);
    pds_meter_api_stats_to_proto(proto_stats, &api_info->stats);
}

#endif //__AGENT_SVC_METER_SVC_HPP__
