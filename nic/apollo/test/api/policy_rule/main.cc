//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains all policy rule test cases
///
//----------------------------------------------------------------------------

#include "nic/apollo/test/api/utils/policy.hpp"
#include "nic/apollo/test/api/utils/vpc.hpp"
#include "nic/apollo/test/api/utils/workflow.hpp"

namespace test {
namespace api {

// globals
static constexpr uint16_t g_num_stateful_rules = 64;

//----------------------------------------------------------------------------
// Policy test class
//----------------------------------------------------------------------------

class policy_rule_test : public ::pds_test_base {
protected:
    policy_rule_test() {}
    virtual ~policy_rule_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        if (!agent_mode()) {
            pds_test_base::SetUpTestCase(g_tc_params);
        }
        g_trace_level = sdk::lib::SDK_TRACE_LEVEL_VERBOSE;
        pds_batch_ctxt_t bctxt = batch_start();
        sample_vpc_setup(bctxt, PDS_VPC_TYPE_TENANT);
        batch_commit(bctxt);
    }
    static void TearDownTestCase() {
        g_trace_level = sdk::lib::SDK_TRACE_LEVEL_DEBUG;
        pds_batch_ctxt_t bctxt = batch_start();
        sample_vpc_teardown(bctxt, PDS_VPC_TYPE_TENANT);
        batch_commit(bctxt);
        if (!agent_mode()) {
            pds_test_base::TearDownTestCase();
        }
    }
};

static uint32_t k_num_init_rules = 10;
static uint32_t k_num_rule_add = 1;

//----------------------------------------------------------------------------
// Policy rule test cases implementation
//----------------------------------------------------------------------------
static void
policy_setup(pds_batch_ctxt_t bctxt) {
    policy_feeder pol_feeder;
    pds_obj_key_t pol_key = int2pdsobjkey(TEST_POLICY_ID_BASE + 1);

    // setup and teardown parameters should be in sync
    pol_feeder.init(pol_key, 512, IP_AF_IPV4, "10.0.0.1/16",
                    1, k_num_init_rules);
    many_create(bctxt, pol_feeder);
}

static void
policy_teardown(pds_batch_ctxt_t bctxt) {
    policy_feeder pol_feeder;
    pds_obj_key_t pol_key = int2pdsobjkey(TEST_POLICY_ID_BASE + 1);

    // this feeder base values doesn't matter in case of deletes
    pol_feeder.init(pol_key, 512, IP_AF_IPV4, "10.0.0.1/16",
                    1, k_num_init_rules);
    many_delete(bctxt, pol_feeder);
}

static void
policy_add_rules (pds_batch_ctxt_t bctxt, std::string cidr_str)
{
    uint32_t add_rule_count = k_num_rule_add;
    pds_policy_rule_spec_t spec;
    uint32_t rule_id = 513; // default rules 1-512
    ip_prefix_t ip_pfx;
    sdk_ret_t ret;

    test::extract_ip_pfx((char *)cidr_str.c_str(), &ip_pfx);
    for (uint32_t i = 0; i < add_rule_count; i ++) {
        spec.key = int2pdsobjkey(rule_id + i);
        spec.policy = int2pdsobjkey(TEST_POLICY_ID_BASE + 1);
        spec.rule.key = int2pdsobjkey(rule_id + i);
        spec.rule.match.l4_match.sport_range.port_lo = 0;
        spec.rule.match.l4_match.sport_range.port_hi = 65535;
        spec.rule.match.l4_match.dport_range.port_lo = 0;
        spec.rule.match.l4_match.dport_range.port_hi = 65535;
        spec.rule.match.l3_match.ip_proto = IP_PROTO_TCP;
        spec.rule.match.l3_match.src_match_type = IP_MATCH_RANGE;
        spec.rule.match.l3_match.dst_match_type = IP_MATCH_RANGE;
        spec.rule.match.l3_match.src_ip_range.af = ip_pfx.addr.af;
        memcpy(&spec.rule.match.l3_match.src_ip_range.ip_lo,
               &ip_pfx.addr.addr, sizeof(ipvx_addr_t));
        memcpy(&spec.rule.match.l3_match.dst_ip_range.ip_lo,
               &ip_pfx.addr.addr, sizeof(ipvx_addr_t));
        test::increment_ip_addr(&ip_pfx.addr, 2);
        memcpy(&spec.rule.match.l3_match.src_ip_range.ip_hi,
               &ip_pfx.addr.addr, sizeof(ipvx_addr_t));
        memcpy(&spec.rule.match.l3_match.dst_ip_range.ip_hi,
               &ip_pfx.addr.addr, sizeof(ipvx_addr_t));
        increment_ip_addr(&ip_pfx.addr);
        spec.rule.action_data.fw_action.action = SECURITY_RULE_ACTION_DENY;
        ret = pds_policy_rule_create(&spec, bctxt);
        ASSERT_TRUE(ret == SDK_RET_OK);
    }
}

static void
policy_add_rules_verify (void)
{
    pds_policy_info_t info;
    pds_obj_key_t key;
    sdk_ret_t ret;

    memset(&info, 0, sizeof(pds_policy_info_t));
    info.spec.rule_info =
        (rule_info_t *)SDK_CALLOC(PDS_MEM_ALLOC_SECURITY_POLICY,
                                  POLICY_RULE_INFO_SIZE(0));
    key = int2pdsobjkey(TEST_POLICY_ID_BASE + 1);
    ret = pds_policy_read(&key, &info);
    ASSERT_TRUE(ret == SDK_RET_OK);
    ASSERT_TRUE(info.spec.rule_info->num_rules ==
                k_num_init_rules + k_num_rule_add);
    SDK_FREE(PDS_MEM_ALLOC_SECURITY_POLICY, info.spec.rule_info);
    info.spec.rule_info = NULL;
}

static void
policy_rule_update (pds_batch_ctxt_t bctxt, std::string cidr_str)
{
    pds_policy_rule_spec_t spec;
    ip_prefix_t ip_pfx;
    sdk_ret_t ret;

    test::extract_ip_pfx((char *)cidr_str.c_str(), &ip_pfx);
    spec.key = int2pdsobjkey(1);
    spec.policy = int2pdsobjkey(TEST_POLICY_ID_BASE + 1);
    spec.rule.key = int2pdsobjkey(1);
    spec.rule.match.l4_match.sport_range.port_lo = 0;
    spec.rule.match.l4_match.sport_range.port_hi = 65535;
    spec.rule.match.l4_match.dport_range.port_lo = 0;
    spec.rule.match.l4_match.dport_range.port_hi = 65535;
    spec.rule.match.l3_match.ip_proto = IP_PROTO_TCP;
    spec.rule.match.l3_match.src_match_type = IP_MATCH_RANGE;
    spec.rule.match.l3_match.dst_match_type = IP_MATCH_RANGE;
    spec.rule.match.l3_match.src_ip_range.af = ip_pfx.addr.af;
    memcpy(&spec.rule.match.l3_match.src_ip_range.ip_lo,
           &ip_pfx.addr.addr, sizeof(ipvx_addr_t));
    memcpy(&spec.rule.match.l3_match.dst_ip_range.ip_lo,
           &ip_pfx.addr.addr, sizeof(ipvx_addr_t));
    test::increment_ip_addr(&ip_pfx.addr, 2);
    memcpy(&spec.rule.match.l3_match.src_ip_range.ip_hi,
           &ip_pfx.addr.addr, sizeof(ipvx_addr_t));
    memcpy(&spec.rule.match.l3_match.dst_ip_range.ip_hi,
           &ip_pfx.addr.addr, sizeof(ipvx_addr_t));
    increment_ip_addr(&ip_pfx.addr);
    spec.rule.action_data.fw_action.action = SECURITY_RULE_ACTION_DENY;
    ret = pds_policy_rule_update(&spec, bctxt);
    ASSERT_TRUE(ret == SDK_RET_OK);
}

static void
policy_rule_update_verify (std::string cidr_str)
{
    sdk_ret_t ret;
    pds_obj_key_t key;
    ip_prefix_t ip_pfx;
    pds_policy_info_t info;

    memset(&info, 0, sizeof(pds_policy_info_t));
    info.spec.rule_info =
        (rule_info_t *)SDK_CALLOC(PDS_MEM_ALLOC_SECURITY_POLICY,
                                  POLICY_RULE_INFO_SIZE(0));
    key = int2pdsobjkey(TEST_POLICY_ID_BASE + 1);
    ret = pds_policy_read(&key, &info);
    ASSERT_TRUE(ret == SDK_RET_OK);
    ASSERT_TRUE(info.spec.rule_info->num_rules == k_num_init_rules);
    SDK_FREE(PDS_MEM_ALLOC_SECURITY_POLICY, info.spec.rule_info);

    info.spec.rule_info =
            (rule_info_t *)SDK_CALLOC(PDS_MEM_ALLOC_SECURITY_POLICY,
                                      POLICY_RULE_INFO_SIZE(k_num_init_rules));
    info.spec.rule_info->num_rules = k_num_init_rules;
    ret = pds_policy_read(&key, &info);
    ASSERT_TRUE(ret == SDK_RET_OK);
    test::extract_ip_pfx((char *)cidr_str.c_str(), &ip_pfx);
    ASSERT_TRUE(memcmp(&info.spec.rule_info->rules[0].match.l3_match.src_ip_range.ip_lo,
                       &ip_pfx.addr.addr, sizeof(ipvx_addr_t)) == 0);
    SDK_FREE(PDS_MEM_ALLOC_ID_ROUTE_TABLE, info.spec.rule_info);

    info.spec.rule_info = NULL;
}

/// \defgroup POLICY_TEST Policy rule tests
/// @{

/// do policy and individual rule(s) add in separate batches
TEST_F(policy_rule_test, rule_add_1) {
    pds_batch_ctxt_t bctxt;

    bctxt = batch_start();
    policy_setup(bctxt);
    batch_commit(bctxt);

    bctxt = batch_start();
    policy_add_rules(bctxt, "30.0.0.1/16");
    batch_commit(bctxt);

    policy_add_rules_verify();

    bctxt = batch_start();
    policy_teardown(bctxt);
    batch_commit(bctxt);
}

#if 0
/// do policy and individual rule(s) add in same batch
TEST_F(policy_rule_test, rule_add_2) {
    pds_batch_ctxt_t bctxt;

    bctxt = batch_start();
    policy_setup(bctxt);
    policy_add_rules(bctxt, "30.0.0.1/16");
    batch_commit(bctxt);

    policy_add_rules_verify();

    bctxt = batch_start();
    policy_teardown(bctxt);
    batch_commit(bctxt);
}

/// do policy add and then individual rule(s) add/del in same batch
TEST_F(policy_rule_test, rule_add_del_1) {
    pds_batch_ctxt_t bctxt;

    bctxt = batch_start();
    policy_setup(bctxt);
    policy_add_rules(bctxt, "30.0.0.1/16");
    // @rsrinkanth, can u make this API generic to take API op, num rules,
    // policy key etc. ?
    policy_del_rules(...);
    batch_commit(bctxt);

    policy_add_rules_verify();

    bctxt = batch_start();
    policy_teardown(bctxt);
    batch_commit(bctxt);
}
#endif

/// add policy and update individual rule(s) in separate batches
TEST_F(policy_rule_test, rule_upd_1) {
    pds_batch_ctxt_t bctxt;

    bctxt = batch_start();
    policy_setup(bctxt);
    batch_commit(bctxt);

    bctxt = batch_start();
    policy_rule_update(bctxt, "30.0.0.1/16");
    batch_commit(bctxt);

    policy_rule_update_verify("30.0.0.1/16");

    bctxt = batch_start();
    policy_teardown(bctxt);
    batch_commit(bctxt);
}

#if 0
/// add policy and update individual rule(s) in same batch
TEST_F(policy_rule_test, rule_upd_2) {
    pds_batch_ctxt_t bctxt;

    bctxt = batch_start();
    policy_setup(bctxt);
    policy_rule_update(bctxt, "30.0.0.1/16");
    batch_commit(bctxt);

    policy_rule_update_verify("30.0.0.1/16");

    bctxt = batch_start();
    policy_teardown(bctxt);
    batch_commit(bctxt);
}
#endif

/// @}

}    // namespace api
}    // namespace test

//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------

/// @private
int
main (int argc, char **argv)
{
    return api_test_program_run(argc, argv);
}
