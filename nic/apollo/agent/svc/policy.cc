//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_policy.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/trace.hpp"
#include "nic/apollo/agent/svc/util.hpp"
#include "nic/apollo/agent/svc/policy.hpp"

#if 0
#include "nic/apollo/test/flow_test/flow_test.hpp"
extern flow_test *g_flow_test_obj;
#endif

static inline void
pds_agent_direction_fill (const pds::SecurityPolicySpec &proto_spec,
                          rule_dir_t *dir)
{
    if (proto_spec.direction() == types::RULE_DIR_INGRESS)
        *dir = RULE_DIR_INGRESS;
    else if (proto_spec.direction() == types::RULE_DIR_EGRESS)
        *dir = RULE_DIR_EGRESS;
}

static inline void
pds_agent_rule_match_fill (const pds::SecurityRule &proto_rule,
                           rule_match_t *match)
{
    types::RuleMatch proto_match = proto_rule.match();
    types::RuleL3Match proto_l3_match = proto_match.l3match();
    types::RuleL4Match proto_l4_match = proto_match.l4match();

    match->l3_match.ip_proto = proto_l3_match.protocol();
    ippfx_proto_spec_to_api_spec(&match->l3_match.ip_pfx, proto_l3_match.prefix());
    if (proto_l4_match.has_ports()) {
        types::PortMatch ports = proto_l4_match.ports();
        types::PortRange sport_range = ports.srcportrange();
        types::PortRange dport_range = ports.dstportrange();

        match->l4_match.sport_range.port_lo = sport_range.portlow();
        match->l4_match.sport_range.port_hi = sport_range.porthigh();
        match->l4_match.dport_range.port_lo = dport_range.portlow();
        match->l4_match.dport_range.port_hi = dport_range.porthigh();
    } else if (proto_l4_match.has_typecode()) {
        types::ICMPMatch typecode = proto_l4_match.typecode();
        match->l4_match.icmp_type = typecode.type();
        match->l4_match.icmp_code = typecode.code();
    } else {
        PDS_TRACE_ERR("Invalid l4_match type");
    }
}

// Build policy API spec from protobuf spec
static inline void
pds_agent_policy_api_spec_fill (const pds::SecurityPolicySpec &proto_spec,
                                 pds_policy_spec_t *api_spec)
{
    uint32_t num_rules = 0;

    api_spec->key.id = proto_spec.id();
    // TODO : policy type missing in proto
    api_spec->policy_type = POLICY_TYPE_FIREWALL;
    pds_af_proto_spec_to_api_spec(proto_spec.addrfamily(), &api_spec->af);
    num_rules = proto_spec.rules_size();
    api_spec->num_rules = num_rules;
    api_spec->rules = (rule_t *)SDK_CALLOC(PDS_MEM_ALLOC_SECURITY_POLICY,
                                           sizeof(rule_t) * num_rules);
    pds_agent_direction_fill(proto_spec, &api_spec->direction);
    for (uint32_t i = 0; i < num_rules; i++) {
        const pds::SecurityRule &proto_rule = proto_spec.rules(i);
        api_spec->rules[i].stateful = proto_rule.stateful();
        api_spec->rules[i].action_data.fw_action.action =
                                                 SECURITY_RULE_ACTION_ALLOW;
        pds_agent_rule_match_fill(proto_rule, &api_spec->rules[i].match);
    }
}

Status
SecurityPolicySvcImpl::SecurityPolicyCreate(ServerContext *context,
                              const pds::SecurityPolicyRequest *proto_req,
                              pds::SecurityPolicyResponse *proto_rsp) {
    if (proto_req) {
        for (int i = 0; i < proto_req->request_size(); i ++) {
            pds_policy_spec_t api_spec;
            pds_agent_policy_api_spec_fill(proto_req->request(i), &api_spec);
            if (!core::agent_state::state()->pds_mock_mode()) {
                if (pds_policy_create(&api_spec) != sdk::SDK_RET_OK)
                    return Status::CANCELLED;
            }
        }
    }
    return Status::OK;
}
