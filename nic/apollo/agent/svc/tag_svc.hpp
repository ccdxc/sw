
//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines protobuf API for tag object
///
//----------------------------------------------------------------------------

#ifndef __AGENT_SVC_TAG_SVC_HPP__
#define __AGENT_SVC_TAG_SVC_HPP__

#include "nic/apollo/agent/svc/specs.hpp"

// populate proto buf spec from tag API spec
static inline void
pds_tag_api_spec_to_proto (pds::TagSpec *proto_spec,
                           const pds_tag_spec_t *api_spec)
{
    proto_spec->set_id(api_spec->key.id, PDS_MAX_KEY_LEN);
    proto_spec->set_af(pds_af_api_spec_to_proto_spec(api_spec->af));
#if 0 // We don't store rules in db for now
    for (uint32_t i = 0; i < api_spec->num_rules; i ++) {
        auto rule = proto_spec->add_rules();
        rule->set_tag(api_spec->rules[i].tag);
        rule->set_priority(api_spec->rules[i].priority);
        for (uint32_t j = 0; j < api_spec->rules[i].num_prefixes; j ++) {
            ippfx_api_spec_to_proto_spec(
                    rule->add_prefix(), &api_spec->rules[i].prefixes[j]);
        }
    }
#endif
}

// populate proto buf status from tag API status
static inline void
pds_tag_api_status_to_proto (const pds_tag_status_t *api_status,
                             pds::TagStatus *proto_status)
{
}

// populate proto buf stats from tag API stats
static inline void
pds_tag_api_stats_to_proto (const pds_tag_stats_t *api_stats,
                            pds::TagStats *proto_stats)
{
}

// populate proto buf from tag API info
static inline void
pds_tag_api_info_to_proto (pds_tag_info_t *api_info, void *ctxt)
{
    pds::TagGetResponse *proto_rsp = (pds::TagGetResponse *)ctxt;
    auto tag = proto_rsp->add_response();
    pds::TagSpec *proto_spec = tag->mutable_spec();
    pds::TagStatus *proto_status = tag->mutable_status();
    pds::TagStats *proto_stats = tag->mutable_stats();

    pds_tag_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_tag_api_status_to_proto(&api_info->status, proto_status);
    pds_tag_api_stats_to_proto(&api_info->stats, proto_stats);
}

// build Tag api spec from proto buf spec
static inline sdk_ret_t
pds_tag_proto_to_api_spec (pds_tag_spec_t *api_spec,
                           const pds::TagSpec &proto_spec)
{
    sdk_ret_t ret;

    pds_obj_key_proto_to_api_spec(&api_spec->key, proto_spec.id());
    ret = pds_af_proto_spec_to_api_spec(&api_spec->af, proto_spec.af());
    if (ret != SDK_RET_OK) {
        return SDK_RET_INVALID_ARG;
    }

    api_spec->num_rules = proto_spec.rules_size();
    api_spec->rules = (pds_tag_rule_t *)SDK_MALLOC(PDS_MEM_ALLOC_ID_TAG,
                                                   api_spec->num_rules *
                                                   sizeof(pds_tag_rule_t));
    for (int i = 0; i < proto_spec.rules_size(); i ++) {
        auto proto_rule = proto_spec.rules(i);

        api_spec->rules[i].tag = proto_rule.tag();
        api_spec->rules[i].priority = proto_rule.priority();
        api_spec->rules[i].num_prefixes = proto_rule.prefix_size();
        api_spec->rules[i].prefixes =
            (ip_prefix_t *)SDK_MALLOC(PDS_MEM_ALLOC_ID_TAG,
                                      api_spec->rules[i].num_prefixes *
                                          sizeof(ip_prefix_t));
        for (int j = 0; j < proto_rule.prefix_size(); j ++) {
            ippfx_proto_spec_to_api_spec(
                        &api_spec->rules[i].prefixes[j], proto_rule.prefix(j));
        }
    }

    return SDK_RET_OK;
}

#endif    //__AGENT_SVC_TAG_SVC_HPP__
