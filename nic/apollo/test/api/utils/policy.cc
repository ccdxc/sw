//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/test/api/utils/batch.hpp"
#include "nic/apollo/test/api/utils/policy.hpp"

namespace test {
namespace api {

#define MAX_RANGE_RULES_V6 5
#define MAX_RANGE_RULES_V4 20

//----------------------------------------------------------------------------
// Policy feeder class routines
//----------------------------------------------------------------------------

void
policy_feeder::init(pds_obj_key_t key,
                    uint16_t stateful_rules,
                    uint8_t af,
                    std::string cidr_str,
                    uint32_t num_policy,
                    uint32_t num_rules_per_policy) {
    uint32_t max_rules;

    if (num_rules_per_policy) {
        max_rules = num_rules_per_policy;
    } else {
        max_rules = ((af == IP_AF_IPV4) ?
                          PDS_MAX_RULES_PER_IPV4_SECURITY_POLICY :
                          PDS_MAX_RULES_PER_IPV6_SECURITY_POLICY);
    }
    memset(&this->spec, 0, sizeof(pds_policy_spec_t));
    this->spec.key = key;
    this->af = af;
    this->num_rules = max_rules;
    this->stateful_rules = stateful_rules;
    this->spec.rule_info = NULL;
    this->cidr_str = cidr_str;
    create_rules(this->cidr_str, this->af, this->stateful_rules,
                 (rule_info_t **)&(this->spec.rule_info), this->num_rules);
    num_obj = num_policy;
}

void
policy_feeder::iter_next(int width) {
    this->spec.key = int2pdsobjkey(pdsobjkey2int(this->spec.key) + width);
    cur_iter_pos++;
}

static inline bool
is_l3 (layer_t layer)
{
    if (layer == L3 || layer == LAYER_ALL) {
        return true;
    }
    return false;
}

static inline bool
is_l4 (layer_t layer)
{
    if (layer == L4 || layer == LAYER_ALL) {
        return true;
    }
    return false;
}

static inline fw_action_t
action_get (action_t action)
{
    switch (action) {
    case DENY:
        return SECURITY_RULE_ACTION_DENY;
    case RANDOM:
        return rand()%2 ? SECURITY_RULE_ACTION_ALLOW :
                          SECURITY_RULE_ACTION_DENY;
    case ALLOW:
    default:
        return SECURITY_RULE_ACTION_ALLOW;
    }
}

void
policy_feeder::key_build(pds_obj_key_t *key) const {
    memcpy(key, &this->spec.key, sizeof(pds_obj_key_t));
}

void
create_rules(std::string cidr_str, uint8_t af, uint16_t stateful_rules,
             rule_info_t **rule_info, uint16_t num_rules,
             layer_t layer, action_t action, uint32_t priority,
             bool wildcard)
{
    ip_prefix_t ip_pfx;
    uint16_t num_range_rules;
    rule_t *rule;

    *rule_info =
        (rule_info_t *)SDK_CALLOC(PDS_MEM_ALLOC_SECURITY_POLICY,
                                  POLICY_RULE_INFO_SIZE(num_rules));
    (*rule_info)->af = af;
    (*rule_info)->num_rules = num_rules;
    if (num_rules == 0) return;
    test::extract_ip_pfx((char *)cidr_str.c_str(), &ip_pfx);

    if (apulu()) {
        num_range_rules = (ip_pfx.addr.af == IP_AF_IPV6)?
                           MAX_RANGE_RULES_V6 : MAX_RANGE_RULES_V4;
        for (uint32_t i = 0; i < num_rules; i++) {
            rule = &(*rule_info)->rules[i];
            rule->key = int2pdsobjkey(i+1);
            rule->attrs.priority = priority;
            if (is_l4(layer)) {
                rule->attrs.match.l4_match.sport_range.port_lo = 0;
                rule->attrs.match.l4_match.sport_range.port_hi = 65535;
                rule->attrs.match.l4_match.dport_range.port_lo = 0;
                rule->attrs.match.l4_match.dport_range.port_hi = 65535;
            }
            if (is_l3(layer)) {
                rule->attrs.match.l3_match.proto_match_type = MATCH_SPECIFIC;
                rule->attrs.match.l3_match.ip_proto = IP_PROTO_TCP;

                // create few as range match rules and rest as prefix
                if (num_range_rules) {
                    rule->attrs.match.l3_match.src_match_type = IP_MATCH_RANGE;
                    rule->attrs.match.l3_match.dst_match_type = IP_MATCH_RANGE;
                    rule->attrs.match.l3_match.src_ip_range.af = ip_pfx.addr.af;
                    memcpy(&rule->attrs.match.l3_match.src_ip_range.ip_lo,
                           &ip_pfx.addr.addr, sizeof(ipvx_addr_t));
                    memcpy(&rule->attrs.match.l3_match.dst_ip_range.ip_lo,
                           &ip_pfx.addr.addr, sizeof(ipvx_addr_t));
                    test::increment_ip_addr(&ip_pfx.addr, 2);
                    memcpy(&rule->attrs.match.l3_match.src_ip_range.ip_hi,
                           &ip_pfx.addr.addr, sizeof(ipvx_addr_t));
                    memcpy(&rule->attrs.match.l3_match.dst_ip_range.ip_hi,
                           &ip_pfx.addr.addr, sizeof(ipvx_addr_t));
                    increment_ip_addr(&ip_pfx.addr);
                    num_range_rules--;
                } else {
                    rule->attrs.match.l3_match.src_match_type = IP_MATCH_PREFIX;
                    rule->attrs.match.l3_match.dst_match_type = IP_MATCH_PREFIX;
                    memcpy(&rule->attrs.match.l3_match.src_ip_pfx,
                           &ip_pfx, sizeof(ip_prefix_t));
                    // using same ip as dst ip just for testing
                    memcpy(&rule->attrs.match.l3_match.dst_ip_pfx,
                           &ip_pfx, sizeof(ip_prefix_t));
                    test::increment_ip_addr(&ip_pfx.addr);
                }
                cidr_str = ippfx2str(&ip_pfx);
            }
            rule->attrs.action_data.fw_action.action = action_get(action);
        }
    } else {
        for (uint32_t i = 0; i < num_rules; i++) {
            rule = &(*rule_info)->rules[i];
            rule->key = int2pdsobjkey(i+1);
            rule->attrs.priority = priority;
            if (stateful_rules) {
                rule->attrs.stateful = true;
                if (is_l4(layer)) {
                    rule->attrs.match.l4_match.sport_range.port_lo = 0;
                    rule->attrs.match.l4_match.sport_range.port_hi = 65535;
                    rule->attrs.match.l4_match.dport_range.port_lo = 0;
                    rule->attrs.match.l4_match.dport_range.port_hi = 65535;
                }
                if (is_l3(layer)) {
                    rule->attrs.match.l3_match.proto_match_type = MATCH_SPECIFIC;
                    rule->attrs.match.l3_match.ip_proto = IP_PROTO_TCP;
                }
                stateful_rules--;
            } else {
                rule->attrs.stateful = false;
                if (is_l3(layer)) {
                    rule->attrs.match.l3_match.proto_match_type = MATCH_SPECIFIC;
                    rule->attrs.match.l3_match.ip_proto = IP_PROTO_ICMP;
                }
                if (is_l4(layer)) {
                    rule->attrs.match.l4_match.type_match_type = MATCH_SPECIFIC;
                    rule->attrs.match.l4_match.icmp_type = 1;
                    rule->attrs.match.l4_match.code_match_type = MATCH_SPECIFIC;
                    rule->attrs.match.l4_match.icmp_code = 1;
                }
            }
            if (is_l3(layer)) {
                memcpy(&rule->attrs.match.l3_match.src_ip_pfx,
                       &ip_pfx, sizeof(ip_prefix_t));
                test::increment_ip_addr(&ip_pfx.addr);
                cidr_str = ippfx2str(&ip_pfx);
            }
            rule->attrs.action_data.fw_action.action =
                                                action_get(action);
        }
    }
}

void
policy_feeder::spec_build(pds_policy_spec_t *spec) const {
    memcpy(spec, &this->spec, sizeof(pds_policy_spec_t));
    create_rules(this->cidr_str, this->af, this->stateful_rules,
                 (rule_info_t **)&(spec->rule_info), this->num_rules);
}

bool
policy_feeder::key_compare(const pds_obj_key_t *key) const {
    return (memcmp(key, &this->spec.key, sizeof(pds_obj_key_t)) == 0);
}

bool
policy_feeder::spec_compare(const pds_policy_spec_t *spec) const {
    if (spec->rule_info)
        if (spec->rule_info->af != this->af)
            return false;
    return true;
}

bool
policy_feeder::status_compare(const pds_policy_status_t *status1,
                              const pds_policy_status_t *status2) const {
    return true;
}

//----------------------------------------------------------------------------
// Policy CRUD helper routines
//----------------------------------------------------------------------------

void
policy_create (policy_feeder& feeder)
{
    pds_batch_ctxt_t bctxt = batch_start();

    SDK_ASSERT_RETURN_VOID(
        (SDK_RET_OK == many_create<policy_feeder>(bctxt, feeder)));
    batch_commit(bctxt);
}

void
policy_read (policy_feeder& feeder, sdk_ret_t exp_result)
{
    SDK_ASSERT_RETURN_VOID(
        (SDK_RET_OK == many_read<policy_feeder>(feeder, exp_result)));
}

static void
rule_attr_update (policy_feeder& feeder, rule_info_t *info,
                  uint64_t chg_bmap)
{
    SDK_ASSERT(info);
    feeder.spec.rule_info->num_rules = info->num_rules;
    memcpy(feeder.spec.rule_info->rules, info->rules,
           sizeof(rule_t) * info->num_rules);
}

static void
rule_info_attr_update (policy_feeder& feeder, rule_info_t *info,
                       uint64_t chg_bmap)
{
    if (bit_isset(chg_bmap, RULE_INFO_ATTR_AF)) {
        feeder.spec.rule_info->af = info->af;
        feeder.spec.rule_info->num_rules = info->num_rules;
        memcpy(&feeder.spec.rule_info->rules, &info->rules,
               sizeof(rule_t) * info->num_rules);
    }
    if (bit_isset(chg_bmap, RULE_INFO_ATTR_RULE)) {
        memcpy(&feeder.spec.rule_info->rules, &info->rules,
               sizeof(rule_t) * info->num_rules);
    }
    if (bit_isset(chg_bmap, RULE_INFO_ATTR_DEFAULT_ACTION)) {
        feeder.spec.rule_info->default_action.fw_action.action =
                            info->default_action.fw_action.action;
        memcpy(&feeder.spec.rule_info->default_action,
               &info->default_action,
               sizeof(rule_action_data_t));
    }
}

void
policy_rule_update (policy_feeder& feeder, pds_policy_spec_t *spec,
                    uint64_t chg_bmap, sdk_ret_t exp_result)
{
    pds_batch_ctxt_t bctxt = batch_start();
    rule_info_t *rule_info = spec ? spec->rule_info : NULL;

    rule_attr_update(feeder, rule_info, chg_bmap);
    SDK_ASSERT_RETURN_VOID(
        (SDK_RET_OK == many_update<policy_feeder>(bctxt, feeder)));

    // if expected result is err, batch commit should fail
    if (exp_result == SDK_RET_ERR)
        batch_commit_fail(bctxt);
    else
        batch_commit(bctxt);
}

void
policy_rule_info_update (policy_feeder& feeder, pds_policy_spec_t *spec,
                         uint64_t chg_bmap, sdk_ret_t exp_result)
{
    pds_batch_ctxt_t bctxt = batch_start();

    rule_info_attr_update(feeder, spec->rule_info, chg_bmap);
    SDK_ASSERT_RETURN_VOID(
        (SDK_RET_OK == many_update<policy_feeder>(bctxt, feeder)));

    // if expected result is err, batch commit should fail
    if (exp_result == SDK_RET_ERR)
        batch_commit_fail(bctxt);
    else
        batch_commit(bctxt);
}

void
policy_update (policy_feeder& feeder)
{
    pds_batch_ctxt_t bctxt = batch_start();

    SDK_ASSERT_RETURN_VOID(
        (SDK_RET_OK == many_update<policy_feeder>(bctxt, feeder)));
    batch_commit(bctxt);
}

void
policy_delete (policy_feeder& feeder)
{
    pds_batch_ctxt_t bctxt = batch_start();

    SDK_ASSERT_RETURN_VOID(
        (SDK_RET_OK == many_delete<policy_feeder>(bctxt, feeder)));
    batch_commit(bctxt);
}

//----------------------------------------------------------------------------
// Misc routines
//----------------------------------------------------------------------------

// do not modify these sample values as rest of system is sync with these
static policy_feeder k_pol_feeder;

void sample_policy_setup(pds_batch_ctxt_t bctxt) {
    pds_obj_key_t pol_key = int2pdsobjkey(TEST_POLICY_ID_BASE + 1);

    // setup and teardown parameters should be in sync
    k_pol_feeder.init(pol_key, 512, IP_AF_IPV4, "10.0.0.1/16", 5);
    many_create(bctxt, k_pol_feeder);

    pol_key = int2pdsobjkey(pdsobjkey2int(pol_key) + 5);
    k_pol_feeder.init(pol_key, 512, IP_AF_IPV6, "2001::1/64", 5);
    many_create(bctxt, k_pol_feeder);

    pol_key = int2pdsobjkey(pdsobjkey2int(pol_key) + 5);
    k_pol_feeder.init(pol_key, 512, IP_AF_IPV4, "20.0.0.1/16", 5);
    many_create(bctxt, k_pol_feeder);

    pol_key = int2pdsobjkey(pdsobjkey2int(pol_key) + 5);
    k_pol_feeder.init(pol_key, 512, IP_AF_IPV6, "3001::1/64", 5);
    many_create(bctxt, k_pol_feeder);
}

void sample_policy_teardown(pds_batch_ctxt_t bctxt) {
    pds_obj_key_t pol_key = int2pdsobjkey(TEST_POLICY_ID_BASE + 1);

    // this feeder base values doesn't matter in case of deletes
    k_pol_feeder.init(pol_key, 512, IP_AF_IPV4, "10.0.0.1/16", 5);
    many_delete(bctxt, k_pol_feeder);

    pol_key = int2pdsobjkey(pdsobjkey2int(pol_key) + 5);
    k_pol_feeder.init(pol_key, 512, IP_AF_IPV6, "2001::1/64", 5);
    many_delete(bctxt, k_pol_feeder);

    pol_key = int2pdsobjkey(pdsobjkey2int(pol_key) + 5);
    k_pol_feeder.init(pol_key, 512, IP_AF_IPV4, "20.0.0.1/16", 5);
    many_delete(bctxt, k_pol_feeder);

    pol_key = int2pdsobjkey(pdsobjkey2int(pol_key) + 5);
    k_pol_feeder.init(pol_key, 512, IP_AF_IPV6, "3001::1/64", 5);
    many_delete(bctxt, k_pol_feeder);
}

}    // namespace api
}    // namespace test
