//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines protobuf API for policy object
///
//----------------------------------------------------------------------------

#ifndef __AGENT_SVC_POLICY_SVC_HPP__
#define __AGENT_SVC_POLICY_SVC_HPP__

#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/svc/policy.hpp"
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/api/utils.hpp"
#include "nic/apollo/api/include/pds_policy.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/trace.hpp"
#include <malloc.h>

static inline fw_action_t
pds_proto_action_to_rule_action (types::SecurityRuleAction action)
{
    switch (action) {
    case types::SECURITY_RULE_ACTION_ALLOW:
        return SECURITY_RULE_ACTION_ALLOW;
    case types::SECURITY_RULE_ACTION_DENY:
        return SECURITY_RULE_ACTION_DENY;
    default:
        return SECURITY_RULE_ACTION_NONE;
    }
}

static inline types::SecurityRuleAction
pds_rule_action_to_proto_action (const rule_action_data_t *action_data)
{
    switch (action_data->fw_action.action) {
    case SECURITY_RULE_ACTION_ALLOW:
        return types::SECURITY_RULE_ACTION_ALLOW;
    case SECURITY_RULE_ACTION_DENY:
        return types::SECURITY_RULE_ACTION_DENY;
    default:
        return types::SECURITY_RULE_ACTION_NONE;
    }
}

static inline sdk_ret_t
pds_policy_rule_match_proto_to_api_spec (pds_obj_key_t policy_key,
                                         pds_obj_key_t rule,
                                         rule_match_t *match,
                                         const types::RuleMatch &proto_match)
{
    const types::RuleL3Match& proto_l3_match = proto_match.l3match();
    if (proto_l3_match.has_srcprefix()) {
        match->l3_match.src_match_type = IP_MATCH_PREFIX;
        ippfx_proto_spec_to_api_spec(&match->l3_match.src_ip_pfx,
                                     proto_l3_match.srcprefix());
    } else if (proto_l3_match.has_srcrange()) {
        match->l3_match.src_match_type = IP_MATCH_RANGE;
        iprange_proto_spec_to_api_spec(&match->l3_match.src_ip_range,
                                       proto_l3_match.srcrange());
    } else if (proto_l3_match.srctag()) {
        match->l3_match.src_match_type = IP_MATCH_TAG;
        match->l3_match.src_tag = proto_l3_match.srctag();
    } else {
        match->l3_match.src_match_type = IP_MATCH_NONE;
    }
    if (proto_l3_match.has_dstprefix()) {
        match->l3_match.dst_match_type = IP_MATCH_PREFIX;
        ippfx_proto_spec_to_api_spec(&match->l3_match.dst_ip_pfx,
                                     proto_l3_match.dstprefix());
    } else if (proto_l3_match.has_dstrange()) {
        match->l3_match.dst_match_type = IP_MATCH_RANGE;
        iprange_proto_spec_to_api_spec(&match->l3_match.dst_ip_range,
                                       proto_l3_match.dstrange());
    } else if (proto_l3_match.dsttag()) {
        match->l3_match.dst_match_type = IP_MATCH_TAG;
        match->l3_match.dst_tag = proto_l3_match.dsttag();
    } else {
        match->l3_match.dst_match_type = IP_MATCH_NONE;
    }

    if (proto_l3_match.protomatch_case() ==
        types::RuleL3Match::ProtomatchCase::kProtoNum) {
        match->l3_match.proto_match_type = MATCH_SPECIFIC;
        match->l3_match.ip_proto = proto_l3_match.protonum();
        if (proto_match.has_l4match() &&
            (proto_match.l4match().has_ports() ||
             proto_match.l4match().has_typecode())) {
            const types::RuleL4Match& proto_l4_match = proto_match.l4match();
            if (proto_l4_match.has_ports()) {
                if ((match->l3_match.ip_proto != IP_PROTO_UDP) &&
                    (match->l3_match.ip_proto != IP_PROTO_TCP)) {
                    PDS_TRACE_ERR("Invalid port config in security policy {}"
                                  ", rule {}", policy_key.str(), rule.str());
                    return SDK_RET_INVALID_ARG;
                }
                if (proto_l4_match.ports().has_srcportrange()) {
                    const types::PortRange& sport_range =
                            proto_l4_match.ports().srcportrange();
                    match->l4_match.sport_range.port_lo = sport_range.portlow();
                    match->l4_match.sport_range.port_hi = sport_range.porthigh();
                    if (unlikely(match->l4_match.sport_range.port_lo >
                                 match->l4_match.sport_range.port_hi)) {
                        PDS_TRACE_ERR("Invalid src port range in security policy {}"
                                      ", rule {}", policy_key.str(), rule.str());
                        return SDK_RET_INVALID_ARG;
                    }
                } else {
                    match->l4_match.sport_range.port_lo = 0;
                    match->l4_match.sport_range.port_hi = 65535;
                }
                if (proto_l4_match.ports().has_dstportrange()) {
                    const types::PortRange& dport_range =
                            proto_l4_match.ports().dstportrange();
                    match->l4_match.dport_range.port_lo = dport_range.portlow();
                    match->l4_match.dport_range.port_hi = dport_range.porthigh();
                    if (unlikely(match->l4_match.dport_range.port_lo >
                                 match->l4_match.dport_range.port_hi)) {
                        PDS_TRACE_ERR("Invalid dst port range in security policy {}"
                                      ", rule {}", policy_key.str(), rule.str());
                        return SDK_RET_INVALID_ARG;
                    }
                } else {
                    match->l4_match.dport_range.port_lo = 0;
                    match->l4_match.dport_range.port_hi = 65535;
                }
            } else if (proto_l4_match.has_typecode()) {
                if ((match->l3_match.ip_proto != IP_PROTO_ICMP) &&
                    (match->l3_match.ip_proto != IP_PROTO_ICMPV6)) {
                    PDS_TRACE_ERR("Invalid ICMP config in security policy {}, "
                                  "rule {}", policy_key.str(), rule.str());
                    return SDK_RET_INVALID_ARG;
                }

                const types::ICMPMatch& typecode = proto_l4_match.typecode();
                if (typecode.typematch_case() == types::ICMPMatch::kTypeNum) {
                    match->l4_match.type_match_type = MATCH_SPECIFIC;
                    match->l4_match.icmp_type = typecode.typenum();
                    if (typecode.codematch_case() == types::ICMPMatch::kCodeNum) {
                        match->l4_match.code_match_type = MATCH_SPECIFIC;
                        match->l4_match.icmp_code = typecode.codenum();
                    } else {
                        match->l4_match.code_match_type = MATCH_ANY;
                    }
                } else {
                    match->l4_match.type_match_type = MATCH_ANY;
                    match->l4_match.code_match_type = MATCH_ANY;
                }
            }
        } else {
            // wildcard L4 match
            if ((match->l3_match.ip_proto == IP_PROTO_UDP) ||
                (match->l3_match.ip_proto == IP_PROTO_TCP)) {
                match->l4_match.sport_range.port_lo = 0;
                match->l4_match.sport_range.port_hi = 65535;
                match->l4_match.dport_range.port_lo = 0;
                match->l4_match.dport_range.port_hi = 65535;
            } else
            if ((match->l3_match.ip_proto == IP_PROTO_ICMP) ||
                (match->l3_match.ip_proto == IP_PROTO_ICMPV6)) {
                match->l4_match.type_match_type = MATCH_ANY;
                match->l4_match.code_match_type = MATCH_ANY;
            }
        }
    } else {
        match->l3_match.proto_match_type = MATCH_ANY;
        if (proto_match.has_l4match()) {
            PDS_TRACE_ERR("L4 match without L3 protocol in security policy {}"
                          ", rule {}", policy_key.str(), rule.str());
            return SDK_RET_INVALID_ARG;
        }
    }
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_policy_rule_attrs_proto_to_api_spec (
    pds_policy_rule_attrs_t *rule_attrs,
    const pds::SecurityRuleAttrs &proto_attrs,
    pds_obj_key_t& rule_key, pds_obj_key_t& policy_key)
{
    sdk_ret_t ret;

    if (unlikely(proto_attrs.priority() > PDS_MAX_RULE_PRIORITY)) {
        PDS_TRACE_ERR("Security policy {}, rule {} priority {} is invalid,"
                      " must be <= {}", policy_key.str(),
                      rule_key.str(), proto_attrs.priority(),
                      PDS_MAX_RULE_PRIORITY);
        return SDK_RET_INVALID_ARG;
    }

    if (unlikely(proto_attrs.has_match() == false)) {
        PDS_TRACE_ERR("Security policy {}, rule {} has no match condition",
                      policy_key.str(), rule_key.str());
        return SDK_RET_INVALID_ARG;
    }

    if (unlikely(proto_attrs.match().has_l3match() == false)) {
        PDS_TRACE_ERR("Security policy {}, rule {} has no L3 match condition, "
                      "IP protocol is a mandatory match condition",
                      policy_key.str(), rule_key.str());
        return SDK_RET_INVALID_ARG;
    }
    rule_attrs->stateful = proto_attrs.stateful();
    rule_attrs->priority = proto_attrs.priority();
    rule_attrs->action_data.fw_action.action =
                pds_proto_action_to_rule_action(proto_attrs.action());
    if (rule_attrs->action_data.fw_action.action == SECURITY_RULE_ACTION_NONE) {
        PDS_TRACE_ERR("Invalid action in security policy {} rule {}",
                      policy_key.str(), rule_key.str());
        return SDK_RET_INVALID_ARG;
    }

    ret = pds_policy_rule_match_proto_to_api_spec(policy_key, rule_key,
                  &rule_attrs->match, proto_attrs.match());
    if (unlikely(ret != SDK_RET_OK)) {
        PDS_TRACE_ERR("Failed converting security policy {} rule {} spec, "
                      "err {}", policy_key.str(), rule_key.str(), ret);
    }
    return ret;
}

// build policy API spec from protobuf spec
static inline sdk_ret_t
pds_policy_proto_to_api_spec (pds_policy_spec_t *api_spec,
                              const pds::SecurityPolicySpec &proto_spec)
{
    sdk_ret_t ret;
    uint32_t num_rules = 0;

    pds_obj_key_proto_to_api_spec(&api_spec->key, proto_spec.id());
    num_rules = proto_spec.rules_size();
    api_spec->rule_info =
        (rule_info_t *)SDK_CALLOC(PDS_MEM_ALLOC_SECURITY_POLICY,
                                  POLICY_RULE_INFO_SIZE(num_rules));
    if (unlikely(api_spec->rule_info == NULL)) {
        PDS_TRACE_ERR("Failed to allocate memory for security policy {}",
                      api_spec->key.id);
        return SDK_RET_OOM;
    }
    ret = pds_af_proto_spec_to_api_spec(&api_spec->rule_info->af,
                                        proto_spec.addrfamily());
    if (unlikely(ret != SDK_RET_OK)) {
        return ret;
    }
    api_spec->rule_info->default_action.fw_action.action =
            pds_proto_action_to_rule_action(proto_spec.defaultfwaction());
    api_spec->rule_info->num_rules = num_rules;
    for (uint32_t i = 0; i < num_rules; i++) {
        pds_obj_key_proto_to_api_spec(&api_spec->rule_info->rules[i].key,
                                      proto_spec.rules(i).id());
        ret = pds_policy_rule_attrs_proto_to_api_spec(
                  &api_spec->rule_info->rules[i].attrs, proto_spec.rules(i).attrs(),
                  api_spec->rule_info->rules[i].key, api_spec->key);
        if (unlikely(ret != SDK_RET_OK)) {
            PDS_TRACE_ERR("Failed converting policy {} spec, err {}",
                          api_spec->key.str(), ret);
            goto cleanup;
        }
    }

    return SDK_RET_OK;

cleanup:

    if (api_spec->rule_info) {
        SDK_FREE(PDS_MEM_ALLOC_SECURITY_POLICY, api_spec->rule_info);
        api_spec->rule_info = NULL;
    }
    return ret;
}

// build policy rule API spec from protobuf spec
static inline sdk_ret_t
pds_security_rule_proto_to_api_spec (pds_policy_rule_spec_t *api_spec,
                                     const pds::SecurityRuleSpec &proto_spec)
{
    pds_obj_key_proto_to_api_spec(&api_spec->key.rule_id, proto_spec.id());
    pds_obj_key_proto_to_api_spec(&api_spec->key.policy_id,
                                  proto_spec.securitypolicyid());
    return pds_policy_rule_attrs_proto_to_api_spec(&api_spec->attrs,
               proto_spec.attrs(), api_spec->key.rule_id,
               api_spec->key.policy_id);
}

static inline void
pds_policy_rule_attrs_api_spec_to_proto (pds::SecurityRuleAttrs *proto_spec,
                                         const pds_policy_rule_attrs_t *api_spec)
{
    proto_spec->set_stateful(api_spec->stateful);
    proto_spec->set_priority(api_spec->priority);
    switch (api_spec->match.l3_match.src_match_type) {
    case IP_MATCH_PREFIX:
        if ((api_spec->match.l3_match.src_ip_pfx.len) &&
            ((api_spec->match.l3_match.src_ip_pfx.addr.af ==
                  IP_AF_IPV4) ||
             (api_spec->match.l3_match.src_ip_pfx.addr.af ==
                  IP_AF_IPV6))) {
            ippfx_api_spec_to_proto_spec(
                proto_spec->mutable_match()->mutable_l3match()->mutable_srcprefix(),
                &api_spec->match.l3_match.src_ip_pfx);
        }
        break;
    case IP_MATCH_RANGE:
        iprange_api_spec_to_proto_spec(
            proto_spec->mutable_match()->mutable_l3match()->mutable_srcrange(),
            &api_spec->match.l3_match.src_ip_range);
        break;
    case IP_MATCH_TAG:
        proto_spec->mutable_match()->mutable_l3match()->set_srctag(
            api_spec->match.l3_match.src_tag);
        break;
    default:
        break;
    }

    switch (api_spec->match.l3_match.dst_match_type) {
    case IP_MATCH_PREFIX:
        if ((api_spec->match.l3_match.dst_ip_pfx.len) &&
            ((api_spec->match.l3_match.dst_ip_pfx.addr.af ==
                  IP_AF_IPV4) ||
             (api_spec->match.l3_match.dst_ip_pfx.addr.af ==
                  IP_AF_IPV6))) {
            ippfx_api_spec_to_proto_spec(
                proto_spec->mutable_match()->mutable_l3match()->mutable_dstprefix(),
                &api_spec->match.l3_match.dst_ip_pfx);
        }
        break;
    case IP_MATCH_RANGE:
        iprange_api_spec_to_proto_spec(
            proto_spec->mutable_match()->mutable_l3match()->mutable_dstrange(),
            &api_spec->match.l3_match.dst_ip_range);
        break;
    case IP_MATCH_TAG:
        proto_spec->mutable_match()->mutable_l3match()->set_dsttag(
            api_spec->match.l3_match.dst_tag);
        break;
    default:
        break;
    }
    if (api_spec->match.l3_match.proto_match_type == MATCH_SPECIFIC) {
        proto_spec->mutable_match()->mutable_l3match()->set_protonum(
                api_spec->match.l3_match.ip_proto);
        if ((api_spec->match.l3_match.ip_proto == IP_PROTO_UDP) ||
            (api_spec->match.l3_match.ip_proto == IP_PROTO_TCP)) {
            proto_spec->mutable_match()->mutable_l4match()->mutable_ports()->
                mutable_srcportrange()->set_portlow(api_spec->
                    match.l4_match.sport_range.port_lo);
            proto_spec->mutable_match()->mutable_l4match()->mutable_ports()->
                mutable_srcportrange()->set_porthigh(api_spec->
                    match.l4_match.sport_range.port_hi);
            proto_spec->mutable_match()->mutable_l4match()->mutable_ports()->
                mutable_dstportrange()->set_portlow(api_spec->
                    match.l4_match.dport_range.port_lo);
            proto_spec->mutable_match()->mutable_l4match()->mutable_ports()->
                mutable_dstportrange()->set_porthigh(api_spec->
                    match.l4_match.dport_range.port_hi);
        } else if ((api_spec->match.l3_match.ip_proto ==
                        IP_PROTO_ICMP) ||
                   (api_spec->match.l3_match.ip_proto ==
                        IP_PROTO_ICMPV6)) {
            if (api_spec->match.l4_match.type_match_type ==
                    MATCH_SPECIFIC) {
                proto_spec->mutable_match()->mutable_l4match()->
                    mutable_typecode()->set_typenum(api_spec->
                        match.l4_match.icmp_type);
            } else {
                proto_spec->mutable_match()->mutable_l4match()->
                    mutable_typecode()->set_typewildcard(
                    types::WildcardMatch::MATCH_ANY);
            }
            if (api_spec->match.l4_match.code_match_type ==
                    MATCH_SPECIFIC) {
                proto_spec->mutable_match()->mutable_l4match()->
                    mutable_typecode()->set_codenum(
                    api_spec->match.l4_match.icmp_code);
            } else {
                proto_spec->mutable_match()->mutable_l4match()->
                    mutable_typecode()->set_codewildcard(
                    types::WildcardMatch::MATCH_ANY);
            }
        }
    } else {
        proto_spec->mutable_match()->mutable_l3match()->set_protowildcard(
            types::WildcardMatch::MATCH_ANY);
    }
    proto_spec->set_action(pds_rule_action_to_proto_action(&api_spec->action_data));
}

// populate proto buf spec from policy API spec
static inline void
pds_policy_api_spec_to_proto (pds::SecurityPolicySpec *proto_spec,
                              const pds_policy_spec_t *api_spec)
{
    if (!api_spec || !proto_spec) {
        return;
    }
    proto_spec->set_id(api_spec->key.id, PDS_MAX_KEY_LEN);
    if (api_spec->rule_info == NULL) {
        return;
    }
    if (api_spec->rule_info->af == IP_AF_IPV4) {
        proto_spec->set_addrfamily(types::IP_AF_INET);
    } else if (api_spec->rule_info->af == IP_AF_IPV6) {
        proto_spec->set_addrfamily(types::IP_AF_INET6);
    } else {
        SDK_ASSERT(FALSE);
    }
    proto_spec->set_defaultfwaction(
        pds_rule_action_to_proto_action(&api_spec->rule_info->default_action));
    for (uint32_t i = 0; i < api_spec->rule_info->num_rules; i++) {
        pds::SecurityRuleInfo *proto_rule = proto_spec->add_rules();
        rule_t *api_rule = &api_spec->rule_info->rules[i];
        proto_rule->set_id(api_rule->key.id, PDS_MAX_KEY_LEN);
        pds_policy_rule_attrs_api_spec_to_proto(proto_rule->mutable_attrs(),
                                                &api_rule->attrs);
    }
    return;
}

// populate proto buf status from policy API status
static inline void
pds_policy_api_status_to_proto (pds::SecurityPolicyStatus *proto_status,
                                const pds_policy_status_t *api_status)
{
}

// populate proto buf stats from policy API stats
static inline void
pds_policy_api_stats_to_proto (pds::SecurityPolicyStats *proto_stats,
                                 const pds_policy_stats_t *api_stats)
{
}

// populate proto buf from policy API info
static inline void
pds_policy_api_info_to_proto (pds_policy_info_t *api_info, void *ctxt)
{
    pds::SecurityPolicyGetResponse *proto_rsp = (pds::SecurityPolicyGetResponse *)ctxt;
    auto policy = proto_rsp->add_response();
    pds::SecurityPolicySpec *proto_spec = policy->mutable_spec();
    pds::SecurityPolicyStatus *proto_status = policy->mutable_status();
    pds::SecurityPolicyStats *proto_stats = policy->mutable_stats();

    pds_policy_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_policy_api_status_to_proto(proto_status, &api_info->status);
    pds_policy_api_stats_to_proto(proto_stats, &api_info->stats);
}

static inline sdk_ret_t
pds_security_rule_api_spec_to_proto (pds::SecurityRuleSpec *proto_spec,
                                     const pds_policy_rule_spec_t *api_spec)
{
    if (!api_spec || !proto_spec) {
        return SDK_RET_INVALID_ARG;
    }
    proto_spec->set_id(api_spec->key.rule_id.id, PDS_MAX_KEY_LEN);
    proto_spec->set_securitypolicyid(api_spec->key.policy_id.id, PDS_MAX_KEY_LEN);
    pds_policy_rule_attrs_api_spec_to_proto(proto_spec->mutable_attrs(),
                                            &api_spec->attrs);
    return SDK_RET_OK;
}

static inline void
pds_security_rule_api_status_to_proto (pds::SecurityRuleStatus *proto_status,
                                       const pds_policy_rule_status_t *api_status)
{
}

static inline void
pds_security_rule_api_stats_to_proto (pds::SecurityRuleStats *proto_stats,
                                      const pds_policy_rule_stats_t *api_stats)
{
}

static inline void
pds_policy_rule_api_info_to_proto (pds_policy_rule_info_t *api_info, void *ctxt)
{
    pds::SecurityRuleGetResponse *proto_rsp = (pds::SecurityRuleGetResponse *)ctxt;
    auto rule = proto_rsp->add_response();
    pds::SecurityRuleSpec *proto_spec = rule->mutable_spec();
    pds::SecurityRuleStatus *proto_status = rule->mutable_status();
    pds::SecurityRuleStats *proto_stats = rule->mutable_stats();

    pds_security_rule_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_security_rule_api_status_to_proto(proto_status, &api_info->status);
    pds_security_rule_api_stats_to_proto(proto_stats, &api_info->stats);
}

// populate proto buf spec from security profile API spec
static inline void
pds_security_profile_api_spec_to_proto (pds::SecurityProfileSpec *proto_spec,
                                        const pds_security_profile_spec_t *api_spec)
{
    if (!api_spec || !proto_spec) {
        return;
    }

    proto_spec->set_id(api_spec->key.id, PDS_MAX_KEY_LEN);
    proto_spec->set_conntracken(api_spec->conn_track_en);
    proto_spec->set_defaultfwaction(
            pds_rule_action_to_proto_action((rule_action_data_t *)&api_spec->default_action));
    proto_spec->set_tcpidletimeout(api_spec->tcp_idle_timeout);
    proto_spec->set_udpidletimeout(api_spec->udp_idle_timeout);
    proto_spec->set_icmpidletimeout(api_spec->icmp_idle_timeout);
    proto_spec->set_otheridletimeout(api_spec->other_idle_timeout);
    proto_spec->set_tcpcnxnsetuptimeout(api_spec->tcp_syn_timeout);
    proto_spec->set_tcphalfclosetimeout(api_spec->tcp_halfclose_timeout);
    proto_spec->set_tcpclosetimeout(api_spec->tcp_close_timeout);
    proto_spec->set_tcpdroptimeout(api_spec->tcp_drop_timeout);
    proto_spec->set_udpdroptimeout(api_spec->udp_drop_timeout);
    proto_spec->set_icmpdroptimeout(api_spec->icmp_drop_timeout);
    proto_spec->set_otherdroptimeout(api_spec->other_drop_timeout);
}

// populate proto buf status from security profile API status
static inline void
pds_security_profile_api_status_to_proto (pds::SecurityProfileStatus *proto_status,
                                          const pds_security_profile_status_t *api_status)
{
}

// populate proto buf stats from security profile API stats
static inline void
pds_security_profile_api_stats_to_proto (pds::SecurityProfileStats *proto_stats,
                                         const pds_security_profile_stats_t *api_stats)
{
}

// populate proto buf from security profile API info
static inline void
pds_security_profile_api_info_to_proto (const pds_security_profile_info_t *api_info, void *ctxt)
{
    pds::SecurityProfileGetResponse *proto_rsp = (pds::SecurityProfileGetResponse *)ctxt;
    auto profile = proto_rsp->add_response();
    pds::SecurityProfileSpec *proto_spec = profile->mutable_spec();
    pds::SecurityProfileStatus *proto_status = profile->mutable_status();
    pds::SecurityProfileStats *proto_stats = profile->mutable_stats();

    pds_security_profile_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_security_profile_api_status_to_proto(proto_status, &api_info->status);
    pds_security_profile_api_stats_to_proto(proto_stats, &api_info->stats);
}

// build policy API spec from protobuf spec
static inline sdk_ret_t
pds_security_profile_proto_to_api_spec (pds_security_profile_spec_t *api_spec,
                                        const pds::SecurityProfileSpec &proto_spec)
{
    pds_obj_key_proto_to_api_spec(&api_spec->key, proto_spec.id());
    api_spec->conn_track_en = proto_spec.conntracken();
    api_spec->default_action.fw_action.action =
            pds_proto_action_to_rule_action(proto_spec.defaultfwaction());
    if (api_spec->default_action.fw_action.action ==
            SECURITY_RULE_ACTION_NONE) {
        // global default action is allow
        api_spec->default_action.fw_action.action = SECURITY_RULE_ACTION_ALLOW;
    }
    api_spec->tcp_idle_timeout = proto_spec.tcpidletimeout();
    api_spec->udp_idle_timeout = proto_spec.udpidletimeout();
    api_spec->icmp_idle_timeout = proto_spec.icmpidletimeout();
    api_spec->other_idle_timeout = proto_spec.otheridletimeout();
    api_spec->tcp_syn_timeout = proto_spec.tcpcnxnsetuptimeout();
    api_spec->tcp_halfclose_timeout = proto_spec.tcphalfclosetimeout();
    api_spec->tcp_close_timeout = proto_spec.tcpclosetimeout();
    api_spec->tcp_drop_timeout = proto_spec.tcpdroptimeout();
    api_spec->udp_drop_timeout = proto_spec.udpdroptimeout();
    api_spec->icmp_drop_timeout = proto_spec.icmpdroptimeout();
    api_spec->other_drop_timeout = proto_spec.otherdroptimeout();
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_svc_security_policy_create (const pds::SecurityPolicyRequest *proto_req,
                                pds::SecurityPolicyResponse *proto_rsp)
{
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    pds_policy_spec_t api_spec;
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
            PDS_TRACE_ERR("Failed to create a new batch, security policy "
                          "creation failed");
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return SDK_RET_ERR;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->request_size(); i ++) {
        memset(&api_spec, 0, sizeof(pds_policy_spec_t));
        ret = pds_policy_proto_to_api_spec(&api_spec,
                                           proto_req->request(i));
        if (unlikely(ret != SDK_RET_OK)) {
            goto end;
        }
        ret = pds_policy_create(&api_spec, bctxt);
        if (api_spec.rule_info != NULL) {
            SDK_FREE(PDS_MEM_ALLOC_SECURITY_POLICY, api_spec.rule_info);
            api_spec.rule_info = NULL;
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
pds_svc_security_policy_update (const pds::SecurityPolicyRequest *proto_req,
                                pds::SecurityPolicyResponse *proto_rsp)
{
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    pds_policy_spec_t api_spec;
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
            PDS_TRACE_ERR("Failed to create a new batch, security policy "
                          "update failed");
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return SDK_RET_ERR;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->request_size(); i ++) {
        memset(&api_spec, 0, sizeof(pds_policy_spec_t));
        ret = pds_policy_proto_to_api_spec(&api_spec,
                                           proto_req->request(i));
        if (unlikely(ret != SDK_RET_OK)) {
            goto end;
        }
        ret = pds_policy_update(&api_spec, bctxt);
        if (api_spec.rule_info != NULL) {
            SDK_FREE(PDS_MEM_ALLOC_SECURITY_POLICY, api_spec.rule_info);
            api_spec.rule_info = NULL;
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
pds_svc_security_policy_delete (const pds::SecurityPolicyDeleteRequest *proto_req,
                                pds::SecurityPolicyDeleteResponse *proto_rsp)
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
            PDS_TRACE_ERR("Failed to create a new batch, security policy "
                          "delete failed");
            proto_rsp->add_apistatus(types::ApiStatus::API_STATUS_ERR);
            return SDK_RET_ERR;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->id_size(); i++) {
        pds_obj_key_proto_to_api_spec(&key, proto_req->id(i));
        ret = pds_policy_delete(&key, bctxt);
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
pds_svc_security_policy_get (const pds::SecurityPolicyGetRequest *proto_req,
                             pds::SecurityPolicyGetResponse *proto_rsp)
{
    sdk_ret_t ret;
    pds_obj_key_t key;
    pds_policy_info_t info;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return SDK_RET_INVALID_ARG;
    }
    for (int i = 0; i < proto_req->id_size(); i++) {
        memset(&info, 0, sizeof(info));
        info.spec.rule_info =
            (rule_info_t *)SDK_CALLOC(PDS_MEM_ALLOC_SECURITY_POLICY,
                                      POLICY_RULE_INFO_SIZE(0));
        pds_obj_key_proto_to_api_spec(&key, proto_req->id(i));
        // get number of rules
        ret = pds_policy_read(&key, &info);
        if (ret == SDK_RET_OK) {
            uint32_t num_rules = info.spec.rule_info->num_rules;
            SDK_FREE(PDS_MEM_ALLOC_SECURITY_POLICY, info.spec.rule_info);
            info.spec.rule_info =
                (rule_info_t *)SDK_CALLOC(PDS_MEM_ALLOC_SECURITY_POLICY,
                                          POLICY_RULE_INFO_SIZE(num_rules));
            info.spec.rule_info->num_rules = num_rules;
            ret = pds_policy_read(&key, &info);
        }
        if (ret != SDK_RET_OK) {
            proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
            SDK_FREE(PDS_MEM_ALLOC_SECURITY_POLICY, info.spec.rule_info);
            info.spec.rule_info = NULL;
            break;
        }
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OK);
        pds_policy_api_info_to_proto(&info, proto_rsp);
        if (info.spec.rule_info) {
            SDK_FREE(PDS_MEM_ALLOC_SECURITY_POLICY, info.spec.rule_info);
            info.spec.rule_info = NULL;
        }
    }

    if (proto_req->id_size() == 0) {
        ret = pds_policy_read_all(pds_policy_api_info_to_proto, proto_rsp);
        proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    }

    PDS_MEMORY_TRIM();
    return ret;
}

static inline sdk_ret_t
pds_svc_security_policy_handle_cfg (cfg_ctxt_t *ctxt, google::protobuf::Any *any_resp)
{
    sdk_ret_t ret;
    google::protobuf::Any *any_req = (google::protobuf::Any *)ctxt->req;
    
    switch (ctxt->cfg) {
    case CFG_MSG_SECURITY_POLICY_CREATE:
        {
            pds::SecurityPolicyRequest req;
            pds::SecurityPolicyResponse rsp;

            ret = pds_svc_security_policy_create(&req, &rsp);
            any_req->UnpackTo(&req);
            any_resp->PackFrom(rsp);
        }
        break;

    case CFG_MSG_SECURITY_POLICY_UPDATE:
        {
            pds::SecurityPolicyRequest req;
            pds::SecurityPolicyResponse rsp;

            ret = pds_svc_security_policy_update(&req, &rsp);
            any_req->UnpackTo(&req);
            any_resp->PackFrom(rsp);
        }
        break;

    case CFG_MSG_SECURITY_POLICY_DELETE:
        {
            pds::SecurityPolicyDeleteRequest req;
            pds::SecurityPolicyDeleteResponse rsp;

            ret = pds_svc_security_policy_delete(&req, &rsp);
            any_req->UnpackTo(&req);
            any_resp->PackFrom(rsp);
        }
        break;

    case CFG_MSG_SECURITY_POLICY_GET:
        {
            pds::SecurityPolicyGetRequest req;
            pds::SecurityPolicyGetResponse rsp;

            ret = pds_svc_security_policy_get(&req, &rsp);
            any_req->UnpackTo(&req);
            any_resp->PackFrom(rsp);
        }
        break;

    default:
        ret = SDK_RET_INVALID_ARG;
        break;
    }

    return ret;
}

static inline sdk_ret_t
pds_svc_security_profile_create (const pds::SecurityProfileRequest *proto_req,
                                 pds::SecurityProfileResponse *proto_rsp)
{
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    bool batched_internally = false;
    pds_batch_params_t batch_params;
    pds_security_profile_spec_t api_spec;

    if ((proto_req == NULL) || (proto_req->request_size() == 0)) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return SDK_RET_INVALID_ARG;
    }

    // only one global security profile is allowed (at this time)
    if (proto_req->request_size() > 1) {
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
            PDS_TRACE_ERR("Failed to create a new batch, security profile"
                          "creation failed");
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return SDK_RET_ERR;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->request_size(); i ++) {
        memset(&api_spec, 0, sizeof(api_spec));
        ret = pds_security_profile_proto_to_api_spec(&api_spec,
                                                     proto_req->request(i));
        if (unlikely(ret != SDK_RET_OK)) {
            goto end;
        }
        ret = pds_security_profile_create(&api_spec, bctxt);
        if (unlikely(ret != SDK_RET_OK)) {
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
pds_svc_security_profile_update (const pds::SecurityProfileRequest *proto_req,
                                 pds::SecurityProfileResponse *proto_rsp)
{
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    bool batched_internally = false;
    pds_batch_params_t batch_params;
    pds_security_profile_spec_t api_spec;

    if ((proto_req == NULL) || (proto_req->request_size() == 0)) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return SDK_RET_INVALID_ARG;
    }

    // only one global security profile is allowed (at this time)
    if (proto_req->request_size() > 1) {
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
            PDS_TRACE_ERR("Failed to create a new batch, security profile "
                          "update failed");
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return SDK_RET_ERR;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->request_size(); i ++) {
        memset(&api_spec, 0, sizeof(api_spec));
        ret = pds_security_profile_proto_to_api_spec(&api_spec,
                                                     proto_req->request(i));
        if (unlikely(ret != SDK_RET_OK)) {
            goto end;
        }
        ret = pds_security_profile_update(&api_spec, bctxt);
        if (unlikely(ret != SDK_RET_OK)) {
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
pds_svc_security_profile_delete (const pds::SecurityProfileDeleteRequest *proto_req,
                                 pds::SecurityProfileDeleteResponse *proto_rsp)
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

    // only one global security profile is allowed (at this time)
    if (proto_req->id_size() > 1) {
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
            PDS_TRACE_ERR("Failed to create a new batch, security profile "
                          "delete failed");
            proto_rsp->add_apistatus(types::ApiStatus::API_STATUS_ERR);
            return SDK_RET_ERR;
        }
        batched_internally = true;
    }

    for (int i = 0; i < proto_req->id_size(); i++) {
        pds_obj_key_proto_to_api_spec(&key, proto_req->id(i));
        ret = pds_security_profile_delete(&key, bctxt);
        if (unlikely(ret != SDK_RET_OK)) {
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
pds_svc_security_profile_get (const pds::SecurityProfileGetRequest *proto_req,
                              pds::SecurityProfileGetResponse *proto_rsp)
{
    // TODO: coming as part of PR 22772
    PDS_TRACE_ERR("SecurityProfile GET not implemented");
    proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OK);
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_svc_security_profile_handle_cfg (cfg_ctxt_t *ctxt, google::protobuf::Any *any_resp)
{
    sdk_ret_t ret;
    google::protobuf::Any *any_req = (google::protobuf::Any *)ctxt->req;
    
    switch (ctxt->cfg) {
    case CFG_MSG_SECURITY_POLICY_CREATE:
        {
            pds::SecurityProfileRequest req;
            pds::SecurityProfileResponse rsp;

            ret = pds_svc_security_profile_create(&req, &rsp);
            any_req->UnpackTo(&req);
            any_resp->PackFrom(rsp);
        }
        break;

    case CFG_MSG_SECURITY_POLICY_UPDATE:
        {
            pds::SecurityProfileRequest req;
            pds::SecurityProfileResponse rsp;

            ret = pds_svc_security_profile_update(&req, &rsp);
            any_req->UnpackTo(&req);
            any_resp->PackFrom(rsp);
        }
        break;

    case CFG_MSG_SECURITY_POLICY_DELETE:
        {
            pds::SecurityProfileDeleteRequest req;
            pds::SecurityProfileDeleteResponse rsp;

            ret = pds_svc_security_profile_delete(&req, &rsp);
            any_req->UnpackTo(&req);
            any_resp->PackFrom(rsp);
        }
        break;

    case CFG_MSG_SECURITY_POLICY_GET:
        {
            pds::SecurityProfileGetRequest req;
            pds::SecurityProfileGetResponse rsp;

            ret = pds_svc_security_profile_get(&req, &rsp);
            any_req->UnpackTo(&req);
            any_resp->PackFrom(rsp);
        }
        break;

    default:
        ret = SDK_RET_INVALID_ARG;
        break;
    }

    return ret;
}

static inline sdk_ret_t
pds_svc_security_rule_create (const pds::SecurityRuleRequest *proto_req,
                              pds::SecurityRuleResponse *proto_rsp)
{
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    pds_policy_rule_spec_t api_spec;
    bool batched_internally = false;
    pds_batch_params_t batch_params;

    if (proto_req == NULL) {
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
            PDS_TRACE_ERR("Failed to create a new batch, security rule "
                          "creation failed");
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return SDK_RET_ERR;
        }
        batched_internally = true;
    }

    memset(&api_spec, 0, sizeof(pds_policy_rule_spec_t));
    ret = pds_security_rule_proto_to_api_spec(&api_spec, proto_req->request());
    if (unlikely(ret != SDK_RET_OK)) {
        goto end;
    }
    ret = pds_policy_rule_create(&api_spec, bctxt);
    if (ret != SDK_RET_OK) {
        goto end;
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
pds_svc_security_rule_update (const pds::SecurityRuleRequest *proto_req,
                              pds::SecurityRuleResponse *proto_rsp)
{
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    pds_policy_rule_spec_t api_spec;
    bool batched_internally = false;
    pds_batch_params_t batch_params;

    if (proto_req == NULL) {
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
            PDS_TRACE_ERR("Failed to create a new batch, security rule "
                          "update failed");
            proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
            return SDK_RET_ERR;
        }
        batched_internally = true;
    }

    memset(&api_spec, 0, sizeof(pds_policy_spec_t));
    ret = pds_security_rule_proto_to_api_spec(&api_spec, proto_req->request());
    if (unlikely(ret != SDK_RET_OK)) {
        goto end;
    }
    ret = pds_policy_rule_update(&api_spec, bctxt);
    if (ret != SDK_RET_OK) {
        goto end;
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
pds_svc_security_rule_delete (const pds::SecurityRuleDeleteRequest *req,
                              pds::SecurityRuleDeleteResponse *rsp)
{
    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;
    pds_policy_rule_key_t key;
    bool batched_internally = false;
    pds_batch_params_t batch_params;

    if ((req == NULL) || (req->id_size() == 0)) {
        rsp->add_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return SDK_RET_INVALID_ARG;
    }

    // create an internal batch, if this is not part of an existing API batch
    bctxt = req->batchctxt().batchcookie();
    if (bctxt == PDS_BATCH_CTXT_INVALID) {
        batch_params.epoch = core::agent_state::state()->new_epoch();
        batch_params.async = false;
        bctxt = pds_batch_start(&batch_params);
        if (bctxt == PDS_BATCH_CTXT_INVALID) {
            PDS_TRACE_ERR("Failed to create a new batch, rule "
                          "delete failed");
            rsp->add_apistatus(types::ApiStatus::API_STATUS_ERR);
            return SDK_RET_ERR;
        }
        batched_internally = true;
    }

    for (int i = 0; i < req->id_size(); i++) {
        pds_obj_key_proto_to_api_spec(&key.rule_id, req->id(i).id());
        pds_obj_key_proto_to_api_spec(&key.policy_id,
                                      req->id(i).securitypolicyid());
        ret = pds_policy_rule_delete(&key, bctxt);
        if (ret != SDK_RET_OK) {
            goto end;
        }
    }

    if (batched_internally) {
        // commit the internal batch
        ret = pds_batch_commit(bctxt);
    }
    rsp->add_apistatus(sdk_ret_to_api_status(ret));
    return ret;

end:

    if (batched_internally) {
        // destroy the internal batch
        pds_batch_destroy(bctxt);
    }
    rsp->add_apistatus(sdk_ret_to_api_status(ret));
    return ret;
}

static inline sdk_ret_t
pds_svc_security_rule_get (const pds::SecurityRuleGetRequest *req,
                           pds::SecurityRuleGetResponse *rsp)
{
    sdk_ret_t ret;
    pds_policy_rule_key_t key;
    pds_policy_rule_info_t info;

    if (req == NULL) {
        rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return SDK_RET_INVALID_ARG;
    }
    for (int i = 0; i < req->id_size(); i++) {
        memset(&info, 0, sizeof(info));
        pds_obj_key_proto_to_api_spec(&key.rule_id, req->id(i).id());
        pds_obj_key_proto_to_api_spec(&key.policy_id,
                                      req->id(i).securitypolicyid());
        ret = pds_policy_rule_read(&key, &info);
        if (ret != SDK_RET_OK) {
            rsp->set_apistatus(sdk_ret_to_api_status(ret));
            break;
        }
        rsp->set_apistatus(types::ApiStatus::API_STATUS_OK);
        //pds_policy_rule_api_info_to_proto(&info, rsp);
    }
    PDS_MEMORY_TRIM();
    return ret;
}

static inline sdk_ret_t
pds_svc_security_rule_handle_cfg (cfg_ctxt_t *ctxt, google::protobuf::Any *any_resp)
{
    sdk_ret_t ret;
    google::protobuf::Any *any_req = (google::protobuf::Any *)ctxt->req;
    
    switch (ctxt->cfg) {
    case CFG_MSG_SECURITY_POLICY_CREATE:
        {
            pds::SecurityRuleRequest req;
            pds::SecurityRuleResponse rsp;

            ret = pds_svc_security_rule_create(&req, &rsp);
            any_req->UnpackTo(&req);
            any_resp->PackFrom(rsp);
        }
        break;

    case CFG_MSG_SECURITY_POLICY_UPDATE:
        {
            pds::SecurityRuleRequest req;
            pds::SecurityRuleResponse rsp;

            ret = pds_svc_security_rule_update(&req, &rsp);
            any_req->UnpackTo(&req);
            any_resp->PackFrom(rsp);
        }
        break;

    case CFG_MSG_SECURITY_POLICY_DELETE:
        {
            pds::SecurityRuleDeleteRequest req;
            pds::SecurityRuleDeleteResponse rsp;

            ret = pds_svc_security_rule_delete(&req, &rsp);
            any_req->UnpackTo(&req);
            any_resp->PackFrom(rsp);
        }
        break;

    case CFG_MSG_SECURITY_POLICY_GET:
        {
            pds::SecurityRuleGetRequest req;
            pds::SecurityRuleGetResponse rsp;

            ret = pds_svc_security_rule_get(&req, &rsp);
            any_req->UnpackTo(&req);
            any_resp->PackFrom(rsp);
        }
        break;

    default:
        ret = SDK_RET_INVALID_ARG;
        break;
    }

    return ret;
}

#endif    //__AGENT_SVC_POLICY_SVC_HPP__
