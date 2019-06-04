//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_policy.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/policy.hpp"
#include "nic/apollo/agent/trace.hpp"
#include "nic/apollo/agent/svc/util.hpp"
#include "nic/apollo/agent/svc/policy.hpp"

#if 0
#include "nic/apollo/test/flow_test/flow_test.hpp"
extern flow_test *g_flow_test_obj;
#endif

static inline sdk_ret_t
pds_policy_dir_proto_to_api_spec (rule_dir_t *dir,
                                  const pds::SecurityPolicySpec &proto_spec)
{
    if (proto_spec.direction() == types::RULE_DIR_INGRESS) {
        *dir = RULE_DIR_INGRESS;
    } else if (proto_spec.direction() == types::RULE_DIR_EGRESS) {
        *dir = RULE_DIR_EGRESS;
    } else {
        PDS_TRACE_ERR("Invalid direction {} in policy spec {}",
                      proto_spec.direction(), proto_spec.id());
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_policy_rule_match_proto_to_api_spec (pds_policy_id_t policy_id,
                                         uint32_t rule_id, rule_match_t *match,
                                         const pds::SecurityRule &proto_rule)
{
    if (unlikely(proto_rule.has_match() == false)) {
        PDS_TRACE_ERR("Security policy {}, rule {} has no match condition, "
                      "IP protocol is a mandatory match condition",
                      policy_id, rule_id);
        return SDK_RET_INVALID_ARG;
    }

    if (unlikely(proto_rule.match().has_l3match() == false)) {
        PDS_TRACE_ERR("Security policy {}, rule {} has no L3 match condition, "
                      "IP protocol is a mandatory match condition",
                      policy_id, rule_id);
        return SDK_RET_INVALID_ARG;
    }

    const types::RuleMatch& proto_match = proto_rule.match();
    const types::RuleL3Match& proto_l3_match = proto_match.l3match();
    match->l3_match.ip_proto = proto_l3_match.protocol();
    if ((match->l3_match.ip_proto != IP_PROTO_UDP) &&
        (match->l3_match.ip_proto != IP_PROTO_TCP) &&
        (match->l3_match.ip_proto != IP_PROTO_ICMP) &&
        (match->l3_match.ip_proto != IP_PROTO_ICMPV6)) {
        PDS_TRACE_ERR("Security policy {}, rule {} with unsupported IP "
                      "protocol {}", policy_id, rule_id,
                      match->l3_match.ip_proto);
        return SDK_RET_INVALID_ARG;
    }
    if (proto_l3_match.has_srcprefix()) {
        ippfx_proto_spec_to_api_spec(&match->l3_match.src_ip_pfx,
                                     proto_l3_match.srcprefix());
    } else {
        // since the memory is zero-ed out, this is 0.0.0.0/0 or 0::0/0
        // TODO: should we set the IP_AF_XXX ?
    }
    if (proto_l3_match.has_dstprefix()) {
        ippfx_proto_spec_to_api_spec(&match->l3_match.dst_ip_pfx,
                                     proto_l3_match.dstprefix());
    } else {
        // since the memory is zero-ed out, this is 0.0.0.0/0 or 0::0/0
        // TODO: should we set the IP_AF_XXX ?
    }

    if (proto_rule.match().has_l4match() &&
        (proto_rule.match().l4match().has_ports() ||
         proto_rule.match().l4match().has_typecode())) {
        const types::RuleL4Match& proto_l4_match = proto_match.l4match();
        if (proto_l4_match.has_ports()) {
            if ((match->l3_match.ip_proto != IP_PROTO_UDP) &&
                (match->l3_match.ip_proto != IP_PROTO_TCP)) {
                PDS_TRACE_ERR("Invalid port config in security policy {}, "
                              "rule {}", policy_id, rule_id);
                return SDK_RET_INVALID_ARG;
            }
            if (proto_l4_match.ports().has_srcportrange()) {
                const types::PortRange& sport_range =
                    proto_l4_match.ports().srcportrange();
                match->l4_match.sport_range.port_lo = sport_range.portlow();
                match->l4_match.sport_range.port_hi = sport_range.porthigh();
                if (unlikely(match->l4_match.sport_range.port_lo >
                             match->l4_match.sport_range.port_hi)) {
                    PDS_TRACE_ERR("Invalid src port range in security "
                                  "policy {}, rule {}", policy_id, rule_id);
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
                    PDS_TRACE_ERR("Invalid dst port range in security "
                                  "policy {}, rule {}", policy_id, rule_id);
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
                              "rule {}", policy_id, rule_id);
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

// build policy API spec from protobuf spec
static inline sdk_ret_t
pds_policy_proto_to_api_spec (const pds::SecurityPolicySpec &proto_spec,
                              pds_policy_spec_t *api_spec)
{
    uint32_t num_rules = 0;
    sdk_ret_t ret;

    api_spec->key.id = proto_spec.id();
    api_spec->policy_type = POLICY_TYPE_FIREWALL;
    ret = pds_af_proto_spec_to_api_spec(&api_spec->af, proto_spec.addrfamily());
    if (unlikely(ret != SDK_RET_OK)) {
        return ret;
    }
    ret = pds_policy_dir_proto_to_api_spec(&api_spec->direction, proto_spec);
    if (unlikely(ret != SDK_RET_OK)) {
        return ret;
    }
    num_rules = proto_spec.rules_size();
    api_spec->num_rules = num_rules;
    api_spec->rules = (rule_t *)SDK_CALLOC(PDS_MEM_ALLOC_SECURITY_POLICY,
                                           sizeof(rule_t) * num_rules);
    if (unlikely(api_spec->rules == NULL)) {
        PDS_TRACE_ERR("Failed to allocate memory for security policy {}",
                      api_spec->key.id);
        return SDK_RET_OOM;
    }
    for (uint32_t i = 0; i < num_rules; i++) {
        const pds::SecurityRule &proto_rule = proto_spec.rules(i);
        api_spec->rules[i].priority = proto_rule.priority();
        api_spec->rules[i].stateful = proto_rule.stateful();
        api_spec->rules[i].action_data.fw_action.action =
                                                 SECURITY_RULE_ACTION_ALLOW;
        ret = pds_policy_rule_match_proto_to_api_spec(api_spec->key.id,
                                                      i+ 1,
                                                      &api_spec->rules[i].match,
                                                      proto_rule);
        if (unlikely(ret != SDK_RET_OK)) {
            PDS_TRACE_ERR("Failed converting policy {} spec, err {}",
                          api_spec->key.id, ret);
            goto cleanup;
        }
    }

    return SDK_RET_OK;

cleanup :

    if (api_spec->rules) {
        SDK_FREE(PDS_MEM_ALLOC_SECURITY_POLICY, api_spec->rules);
        api_spec->rules = NULL;
    }
    return ret;
}

Status
SecurityPolicySvcImpl::SecurityPolicyCreate(ServerContext *context,
                                            const pds::SecurityPolicyRequest *proto_req,
                                            pds::SecurityPolicyResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_policy_spec_t api_spec = {};
    pds_policy_key_t key = {0};

    if (proto_req) {
        for (int i = 0; i < proto_req->request_size(); i ++) {
            ret = pds_policy_proto_to_api_spec(proto_req->request(i),
                                               &api_spec);
            if (unlikely(ret != SDK_RET_OK)) {
                return Status::CANCELLED;
            }
            auto request = proto_req->request(i);
            key.id = request.id();
            ret = core::policy_create(&key, &api_spec);
            proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
            SDK_FREE(PDS_MEM_ALLOC_SECURITY_POLICY, api_spec.rules);
            api_spec.rules = NULL;
            if (ret != SDK_RET_OK) {
                return Status::CANCELLED;
            }
        }
    } else  {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    return Status::OK;
}

Status
SecurityPolicySvcImpl::SecurityPolicyUpdate(ServerContext *context,
                                            const pds::SecurityPolicyRequest *proto_req,
                                            pds::SecurityPolicyResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_policy_spec_t api_spec = {};
    pds_policy_key_t key = {0};

    if (proto_req) {
        for (int i = 0; i < proto_req->request_size(); i ++) {
            ret = pds_policy_proto_to_api_spec(proto_req->request(i),
                                               &api_spec);
            if (unlikely(ret != SDK_RET_OK)) {
                return Status::CANCELLED;
            }
            auto request = proto_req->request(i);
            key.id = request.id();
            ret = core::policy_update(&key, &api_spec);
            proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
            SDK_FREE(PDS_MEM_ALLOC_SECURITY_POLICY, api_spec.rules);
            api_spec.rules = NULL;
            if (ret != SDK_RET_OK) {
                return Status::CANCELLED;
            }
        }
    } else  {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    return Status::OK;
}
