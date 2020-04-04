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

static inline fw_action_t
pds_proto_action_to_rule_action (types::SecurityRuleAction action)
{
    switch (action) {
    case types::SECURITY_RULE_ACTION_ALLOW:
        return SECURITY_RULE_ACTION_ALLOW;
    case types::SECURITY_RULE_ACTION_DENY:
        return SECURITY_RULE_ACTION_DENY;
    default:
        return SECURITY_RULE_ACTION_DENY;
    }
}

static inline types::SecurityRuleAction
pds_rule_action_to_proto_action (rule_action_data_t *action_data)
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
    match->l3_match.ip_proto = proto_l3_match.protocol();
    if ((match->l3_match.ip_proto != IP_PROTO_UDP) &&
        (match->l3_match.ip_proto != IP_PROTO_TCP) &&
        (match->l3_match.ip_proto != IP_PROTO_ICMP) &&
        (match->l3_match.ip_proto != IP_PROTO_ICMPV6)) {
        PDS_TRACE_ERR("Security policy {}, rule {} with unsupported IP "
                      "protocol {}", policy_key.str(), rule.str(),
                      match->l3_match.ip_proto);
        return SDK_RET_INVALID_ARG;
    }
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
            match->l4_match.icmp_type = typecode.type();
            match->l4_match.icmp_code = typecode.code();
        }
    } else {
        // wildcard L4 match
        if ((match->l3_match.ip_proto == IP_PROTO_UDP) ||
            (match->l3_match.ip_proto == IP_PROTO_TCP)) {
            match->l4_match.sport_range.port_lo = 0;
            match->l4_match.sport_range.port_hi = 65535;
            match->l4_match.dport_range.port_lo = 0;
            match->l4_match.dport_range.port_hi = 65535;
        } else if ((match->l3_match.ip_proto == IP_PROTO_ICMP) ||
                   (match->l3_match.ip_proto == IP_PROTO_ICMPV6)) {
            // TODO : wildcard ICMP support will come later
        }
    }
    return SDK_RET_OK;
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
pds_policy_rule_proto_to_api_spec_common (rule_t *rule,
                                          const pds::SecurityRuleInfo &proto_rule,
                                          pds_obj_key_t policy_key)
{
    sdk_ret_t ret;

    pds_obj_key_proto_to_api_spec(&rule->key, proto_rule.id());

    if (unlikely(proto_rule.priority() > PDS_MAX_RULE_PRIORITY)) {
        PDS_TRACE_ERR("Security policy {}, rule {} priority {} is invalid,"
                      " must be <= {}", policy_key.str(),
                      rule->key.str(), proto_rule.priority(),
                      PDS_MAX_RULE_PRIORITY);
        return SDK_RET_INVALID_ARG;
    }

    if (unlikely(proto_rule.has_match() == false)) {
        PDS_TRACE_ERR("Security policy {}, rule {} has no match condition",
                      policy_key.str(), rule->key.str());
        return SDK_RET_INVALID_ARG;
    }

    if (unlikely(proto_rule.match().has_l3match() == false)) {
        PDS_TRACE_ERR("Security policy {}, rule {} has no L3 match condition, "
                      "IP protocol is a mandatory match condition",
                      policy_key.str(), rule->key.str());
        return SDK_RET_INVALID_ARG;
    }

    rule->stateful = proto_rule.stateful();
    rule->priority = proto_rule.priority();
    rule->action_data.fw_action.action =
                pds_proto_action_to_rule_action(proto_rule.action());

    ret = pds_policy_rule_match_proto_to_api_spec(policy_key, rule->key,
                  &rule->match, proto_rule.match());
    if (unlikely(ret != SDK_RET_OK)) {
        PDS_TRACE_ERR("Failed converting security policy {} rule {} spec,"
                      " err {}", policy_key.str(), rule->key.str(), ret);
    }

    return ret;
}

// build policy rule API spec from protobuf spec
static inline sdk_ret_t
pds_policy_rule_proto_to_api_spec (pds_policy_rule_spec_t *api_spec,
                                   const pds::SecurityRuleSpec &proto_spec)
{
    pds_obj_key_proto_to_api_spec(&api_spec->key, proto_spec.id());
    pds_obj_key_proto_to_api_spec(&api_spec->policy,
                                  proto_spec.securitypolicyid());
    return pds_policy_rule_proto_to_api_spec_common(&api_spec->rule,
                                                    proto_spec.securityrule(),
                                                    api_spec->policy);
}

// populate proto buf spec from policy API spec
inline void
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
    for (uint32_t i = 0; i < api_spec->rule_info->num_rules; i++) {
        pds::SecurityRuleInfo *proto_rule = proto_spec->add_rules();
        rule_t *api_rule = &api_spec->rule_info->rules[i];
        proto_rule->set_id(api_rule->key.id, PDS_MAX_KEY_LEN);
        proto_rule->set_priority(api_rule->priority);
        proto_rule->set_action(pds_rule_action_to_proto_action(&api_rule->action_data));
        proto_rule->set_stateful(api_rule->stateful);
        if (api_rule->match.l3_match.ip_proto) {
            proto_rule->mutable_match()->mutable_l3match()->set_protocol(
                                            api_rule->match.l3_match.ip_proto);
        }

        switch (api_rule->match.l3_match.src_match_type) {
        case IP_MATCH_PREFIX:
            if ((api_rule->match.l3_match.src_ip_pfx.len) &&
                ((api_rule->match.l3_match.src_ip_pfx.addr.af == IP_AF_IPV4) ||
                 (api_rule->match.l3_match.src_ip_pfx.addr.af == IP_AF_IPV6))) {
                ippfx_api_spec_to_proto_spec(
                    proto_rule->mutable_match()->mutable_l3match()->mutable_srcprefix(),
                    &api_rule->match.l3_match.src_ip_pfx);
            }
            break;
        case IP_MATCH_RANGE:
            iprange_api_spec_to_proto_spec(
                proto_rule->mutable_match()->mutable_l3match()->mutable_srcrange(),
                &api_rule->match.l3_match.src_ip_range);
            break;
        case IP_MATCH_TAG:
            proto_rule->mutable_match()->mutable_l3match()->set_srctag(
                api_rule->match.l3_match.src_tag);
            break;
        default:
            break;
        }

        switch (api_rule->match.l3_match.dst_match_type) {
        case IP_MATCH_PREFIX:
            if ((api_rule->match.l3_match.dst_ip_pfx.len) &&
                ((api_rule->match.l3_match.dst_ip_pfx.addr.af == IP_AF_IPV4) ||
                 (api_rule->match.l3_match.dst_ip_pfx.addr.af == IP_AF_IPV6))) {
                ippfx_api_spec_to_proto_spec(
                    proto_rule->mutable_match()->mutable_l3match()->mutable_dstprefix(),
                    &api_rule->match.l3_match.dst_ip_pfx);
            }
            break;
        case IP_MATCH_RANGE:
            iprange_api_spec_to_proto_spec(
                proto_rule->mutable_match()->mutable_l3match()->mutable_dstrange(),
                &api_rule->match.l3_match.dst_ip_range);
            break;
        case IP_MATCH_TAG:
            proto_rule->mutable_match()->mutable_l3match()->set_dsttag(
                api_rule->match.l3_match.dst_tag);
            break;
        default:
            break;
        }
        proto_rule->mutable_match()->mutable_l4match()->mutable_ports()->mutable_srcportrange()->set_portlow(api_rule->match.l4_match.sport_range.port_lo);
        proto_rule->mutable_match()->mutable_l4match()->mutable_ports()->mutable_srcportrange()->set_porthigh(api_rule->match.l4_match.sport_range.port_hi);
        proto_rule->mutable_match()->mutable_l4match()->mutable_ports()->mutable_dstportrange()->set_portlow(api_rule->match.l4_match.dport_range.port_lo);
        proto_rule->mutable_match()->mutable_l4match()->mutable_ports()->mutable_dstportrange()->set_porthigh(api_rule->match.l4_match.dport_range.port_hi);
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
    api_spec->rule_info->num_rules = num_rules;
    for (uint32_t i = 0; i < num_rules; i++) {
        ret = pds_policy_rule_proto_to_api_spec_common(&api_spec->rule_info->rules[i],
                                                       proto_spec.rules(i),
                                                       api_spec->key);
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

#endif //__AGENT_SVC_SPECS_HPP__
