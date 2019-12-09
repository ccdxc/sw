#include "nic/hal/plugins/cfg/nat/nat.hpp"
#include "nic/hal/hal.hpp"
#include "lib/list/list.hpp"
#include "gen/proto/vrf.pb.h"
#include "gen/proto/nat.pb.h"
#include "nic/hal/hal.hpp"
#include "gen/proto/types.pb.h"
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include "nic/hal/test/utils/hal_test_utils.hpp"
#include "nic/hal/test/utils/hal_base_test.hpp"
#include "nic/fte/fte_ctx.hpp"
#include "nic/fte/acl/acl_ctx.hpp"
#include "nic/fte/acl/list.hpp"

using vrf::VrfSpec;
using vrf::VrfResponse;
using nat::NatPolicySpec;
using types::IPProtocol;
using namespace hal;
using namespace fte;
using namespace nwsec;

static inline void
nat_test_memleak_checks (void)
{
    // Pre-reqs (might not be complete)
    EXPECT_EQ(g_hal_state->nat_pool_slab()->num_in_use(), 0);
    EXPECT_EQ(g_hal_state->vrf_slab()->num_in_use(), 0);

    // NAT policy config
    EXPECT_EQ(g_hal_state->nat_cfg_pol_slab()->num_in_use(), 0);
    EXPECT_EQ(g_hal_state->nat_policy_ht()->num_entries(), 0);

    // NAT rules config
    EXPECT_EQ(g_hal_state->nat_cfg_rule_slab()->num_in_use(), 0);
    EXPECT_EQ(g_hal_state->v4addr_list_elem_slab()->num_in_use(), 0);
    EXPECT_EQ(g_hal_state->v6addr_list_elem_slab()->num_in_use(), 0);
    EXPECT_EQ(g_hal_state->mac_addr_list_elem_slab()->num_in_use(), 0);
    EXPECT_EQ(g_hal_state->port_list_elem_slab()->num_in_use(), 0);

    // NAT rules oper -- acl etc
    //EXPECT_EQ(g_hal_state->rule_data_slab()->num_in_use(), 0);
    EXPECT_EQ(g_hal_state->ipv4_rule_slab()->num_in_use(), 0);
    // todo - stuff from acl lib
}

class nat_policy_test : public hal_base_test {
protected:
    nat_policy_test() { }
    virtual ~nat_policy_test() { }

    // will be called immediately after the constructor before each test
    virtual void SetUp() { }

    // will be called immediately after each test before the destructor
    virtual void TearDown() {
        nat_test_memleak_checks();
    }

    // Will be called at the beginning of all test cases in this class
    static void SetUpTestCase() {
        hal_base_test::SetUpTestCase();
        hal_test_utils_slab_disable_delete();
    }
};

//-----------------------------------------------------------------------------
// dependency routines
//-----------------------------------------------------------------------------

static inline void
nat_test_vrf_create (int vrf_id, hal_handle_t *vrf_hdl)
{
    hal_ret_t ret;
    VrfSpec spec;
    VrfResponse rsp;

    spec.mutable_key_or_handle()->set_vrf_id(vrf_id);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    *vrf_hdl = rsp.vrf_status().key_or_handle().vrf_handle();
}

static inline void
nat_test_vrf_delete (hal_handle_t vrf_hdl)
{
    hal_ret_t ret;
    VrfDeleteRequest req;
    VrfDeleteResponse rsp;

    req.mutable_key_or_handle()->set_vrf_handle(vrf_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_delete(req, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
}

static inline void
nat_test_natpool_create (int vrf_id, hal_handle_t *pool_hdl)
{
    hal_ret_t ret;
    NatPoolSpec req;
    NatPoolResponse rsp;

    req.mutable_key_or_handle()->mutable_pool_key()->mutable_vrf_kh()->
        set_vrf_id(vrf_id);
    req.mutable_key_or_handle()->mutable_pool_key()->set_pool_id(1);
    req.add_address();
    req.mutable_address(0)->mutable_range()->mutable_ipv4_range()->
        mutable_low_ipaddr()->set_ip_af(types::IP_AF_INET);
    req.mutable_address(0)->mutable_range()->mutable_ipv4_range()->
        mutable_low_ipaddr()->set_v4_addr(0x0a000001);
    req.mutable_address(0)->mutable_range()->mutable_ipv4_range()->
        mutable_high_ipaddr()->set_ip_af(types::IP_AF_INET);
    req.mutable_address(0)->mutable_range()->mutable_ipv4_range()->
        mutable_high_ipaddr()->set_v4_addr(0x0a000003);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::nat_pool_create(req, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    *pool_hdl = rsp.pool_status().pool_handle();
}

static inline void
nat_test_natpool_delete (hal_handle_t pool_hdl)
{
    hal_ret_t ret;
    NatPoolDeleteRequest req;
    NatPoolDeleteResponse rsp;

    req.mutable_key_or_handle()->set_pool_handle(pool_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::nat_pool_delete(req, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
}

//-----------------------------------------------------------------------------
// nat routines
//-----------------------------------------------------------------------------

#define  TEST_NAT_CFG_MAX_ENTRIES  10
#define  TEST_VRF_ID                1
#define  TEST_POL_ID               10

typedef struct nat_test_rule_match_s {
    ipvx_range_t       src_ip[TEST_NAT_CFG_MAX_ENTRIES];
    int                num_src_ip;
    ipvx_range_t       dst_ip[TEST_NAT_CFG_MAX_ENTRIES];
    int                num_dst_ip;
    port_range_t       src_port[TEST_NAT_CFG_MAX_ENTRIES];
    int                num_src_port;
    port_range_t       dst_port[TEST_NAT_CFG_MAX_ENTRIES];
    int                num_dst_port;
    types::IPProtocol  proto;
} nat_test_rule_match_t;

typedef struct nat_test_rule_action_s {
    types::NatAction     src_nat_action;
    hal_handle_t         src_nat_pool;
    types::NatAction     dst_nat_action;
    hal_handle_t         dst_nat_pool;
} nat_test_rule_action_t;

typedef struct nat_test_rule_s {
    int                       rule_id;
    nat_test_rule_match_t     match;
    nat_test_rule_action_t    action;
} nat_test_rule_t;

typedef struct nat_test_pol_s {
    int                pol_id;
    int                vrf_id;
    hal_handle_t       hal_hdl;
    nat_test_rule_t    rules[TEST_NAT_CFG_MAX_ENTRIES];
    int                num_rules;
} nat_test_pol_t;

static inline void
nat_test_rule_action_fill (nat_test_rule_action_t *action, hal_handle_t pool_hdl)
{
    action->src_nat_action = action->dst_nat_action =
        types::NatAction::NAT_TYPE_DYNAMIC_ADDRESS;
    action->src_nat_pool = pool_hdl;
    action->dst_nat_pool = pool_hdl;
}

static inline void
rule_match_test_fill (nat_test_rule_match_t *rule_match, int num_src_ip,
                      int num_dst_ip, int num_src_port, int num_dst_port,
                      int offset)
{
    int i;

    rule_match->num_src_ip = num_src_ip;
    for (i = 0; i < num_src_ip; i++) {
        rule_match->src_ip[i].ip_lo.v4_addr = 0xbade1000 + offset + i;
        rule_match->src_ip[i].ip_hi.v4_addr = 0xbade2000 + offset + i;
    }

    rule_match->num_dst_ip = num_dst_ip;
    for (i = 0; i < num_dst_ip; i++) {
        rule_match->dst_ip[i].ip_lo.v4_addr = 0xbade3000 + offset + i;
        rule_match->dst_ip[i].ip_hi.v4_addr = 0xbade4000 + offset + i;
    }

    rule_match->num_src_port = num_src_port;
    for (i = 0; i < num_src_port; i++) {
        rule_match->src_port[i].port_lo = 0x1000 + offset + i;
        rule_match->src_port[i].port_hi = 0x2000 + offset + i;
    }

    rule_match->num_dst_port = num_dst_port;
    for (i = 0; i < num_dst_port; i++) {
        rule_match->dst_port[i].port_lo = 0x3000 + offset + i;
        rule_match->dst_port[i].port_hi = 0x4000 + offset + i;
    }
}

static inline void
nat_test_pol_fill (nat_test_pol_t *pol, int num_rules, int num_src_ip,
                   int num_dst_ip, int num_src_port, int num_dst_port,
                   hal_handle_t pool_hdl)
{
    int i;

    memset(pol, 0, sizeof(nat_test_pol_t));
    pol->vrf_id = TEST_VRF_ID; pol->pol_id = TEST_POL_ID;
    pol->hal_hdl = HAL_HANDLE_INVALID;

    pol->num_rules = num_rules;
    for (i = 0; i < num_rules; i++) {
        pol->rules[i].rule_id = i+1;
        rule_match_test_fill(&pol->rules[i].match, num_src_ip, num_dst_ip,
                             num_src_port, num_dst_port, i*100);
        nat_test_rule_action_fill(&pol->rules[i].action, pool_hdl);
    }
}

static inline void
nat_test_rule_dump (nat_test_rule_t *rule, int indent)
{
    int i;

    printf("%*sRule%d:\n", indent, "", rule->rule_id);

    indent += 2;
    printf("%*s%d Source IP Addresses\n",
           indent, "", rule->match.num_src_ip);
    for (i = 0; i < rule->match.num_src_ip; i++) {
        printf("%*sLow: %x ", indent+2, "",
               rule->match.src_ip[i].ip_lo.v4_addr);
        printf("High: %x\n", rule->match.src_ip[i].ip_hi.v4_addr);
    }

    printf("%*s%d Destination IP Addresses\n",
           indent, "", rule->match.num_dst_ip);
    for (i = 0; i < rule->match.num_dst_ip; i++) {
        printf("%*sLow: %x ", indent+2, "",
               rule->match.dst_ip[i].ip_lo.v4_addr);
        printf("High: %x\n", rule->match.dst_ip[i].ip_hi.v4_addr);
    }

    printf("%*s%d Source Ports\n",
           indent, "", rule->match.num_src_port);
    for (i = 0; i < rule->match.num_src_port; i++) {
        printf("%*sLow: %d ", indent+2, "", rule->match.src_port[i].port_lo);
        printf("High: %d\n", rule->match.src_port[i].port_hi);
    }

    printf("%*s%d Destination Ports\n",
           indent, "", rule->match.num_dst_port);
    for (i = 0; i < rule->match.num_dst_port; i++) {
        printf("%*sLow: %d ", indent+2, "", rule->match.dst_port[i].port_lo);
        printf("High: %d\n", rule->match.dst_port[i].port_hi);
    }

    printf("%*sSrc NAT Action: %d, Dst NAT Action: %d\n",
           indent, "", rule->action.src_nat_action,
           rule->action.dst_nat_action);
    printf("%*sSrc NAT Pool: %lu, Dst NAT Pool: %lu\n",
           indent, "", rule->action.src_nat_pool, rule->action.dst_nat_pool);
}

static inline void
nat_test_pol_dump (nat_test_pol_t *pol, const char *header, int indent)
{
    printf("%s\n", header);
    printf("%*sKey: vrf_id %d pol_id %d handle %lu\n", indent, "",
           pol->vrf_id, pol->pol_id, pol->hal_hdl);
    printf("%*sData: %d rules\n", indent, "", pol->num_rules);
    for (int i = 0; i < pol->num_rules; i++)
        nat_test_rule_dump(&pol->rules[i], indent+2);
}

static inline bool
nat_test_pol_cmp (nat_test_pol_t *a, nat_test_pol_t *b)
{
    if ((a->num_rules != b->num_rules) ||
        (memcmp(a->rules, b->rules, sizeof(a->rules)) != 0))
        return false;

    return true;
}

//-----------------------------------------------------------------------------
// nat policy CREATE routines
//-----------------------------------------------------------------------------

static inline void
nat_test_rule_action_spec_build (nat_test_rule_action_t *action,
                                 nat::NatRuleAction* spec)
{
    spec->set_src_nat_action(action->src_nat_action);
    spec->set_dst_nat_action(action->dst_nat_action);
    spec->mutable_src_nat_pool()->set_pool_handle(action->src_nat_pool);
    spec->mutable_dst_nat_pool()->set_pool_handle(action->dst_nat_pool);
}

static inline void
nat_test_l4portrange_spec_build (port_range_t *port_range,
                                 types::L4PortRange *spec)
{
    spec->set_port_high(port_range->port_hi);
    spec->set_port_low(port_range->port_lo);
}

static inline void
nat_test_rule_match_dst_port_spec_build (nat_test_rule_match_t *match,
                                         types::RuleMatch_L4PortAppInfo *spec)
{
    for (int i = 0; i < match->num_dst_port; i++) {
        auto port_range = spec->add_dst_port_range();
        nat_test_l4portrange_spec_build(&match->dst_port[i], port_range);
    }
}

static inline void
nat_test_rule_match_src_port_spec_build (nat_test_rule_match_t *match,
                                         types::RuleMatch_L4PortAppInfo *spec)
{
    for (int i = 0; i < match->num_src_port; i++) {
        auto port_range = spec->add_src_port_range();
        nat_test_l4portrange_spec_build(&match->src_port[i], port_range);
    }
}

static inline void
nat_test_rule_match_port_app_spec_build (nat_test_rule_match_t *match,
                                         types::RuleMatch *spec)
{
    nat_test_rule_match_src_port_spec_build(
        match, spec->mutable_app_match()->mutable_port_info());
    nat_test_rule_match_dst_port_spec_build(
        match, spec->mutable_app_match()->mutable_port_info());
}

static inline void
nat_test_rule_match_app_spec_build (nat_test_rule_match_t *match,
                                    types::RuleMatch *spec)
{
    nat_test_rule_match_port_app_spec_build(match, spec);
}

static inline void
nat_test_rule_match_proto_spec_build (nat_test_rule_match_t *match,
                                      types::RuleMatch *spec)
{
    spec->set_protocol(match->proto);
}

static inline void
nat_test_ipvx_range_to_spec (types::AddressRange *spec, ipvx_range_t *range)
{
    auto v4_range = spec->mutable_ipv4_range();
    v4_range->mutable_low_ipaddr()->set_v4_addr(range->ip_lo.v4_addr);
    v4_range->mutable_high_ipaddr()->set_v4_addr(range->ip_hi.v4_addr);
}

static inline void
nat_test_addr_list_elem_ipaddrobj_spec_build (ipvx_range_t *addr_elem,
                                              types::IPAddressObj *spec)
{
    spec->set_negate(false);
    nat_test_ipvx_range_to_spec(spec->mutable_address()->mutable_range(),
                                addr_elem);
}

static inline void
nat_test_rule_match_dst_addr_spec_build (nat_test_rule_match_t *match,
                                         types::RuleMatch *spec)
{
    for (int i = 0; i < match->num_dst_ip; i++) {
        auto addr_spec = spec->add_dst_address();
        nat_test_addr_list_elem_ipaddrobj_spec_build(&match->dst_ip[i],
                                                     addr_spec);
    }
}

static inline void
nat_test_rule_match_src_addr_spec_build (nat_test_rule_match_t *match,
                                         types::RuleMatch *spec)
{
    for (int i = 0; i < match->num_src_ip; i++) {
        auto addr_spec = spec->add_src_address();
        nat_test_addr_list_elem_ipaddrobj_spec_build(&match->src_ip[i],
                                                     addr_spec);
    }
}

static inline void
nat_test_rule_match_addr_spec_build (nat_test_rule_match_t *match,
                                     types::RuleMatch *spec)
{
    nat_test_rule_match_src_addr_spec_build(match, spec);
    nat_test_rule_match_dst_addr_spec_build(match, spec);
}

static inline void
nat_test_rule_match_spec_build (nat_test_rule_match_t *match,
                                types::RuleMatch *spec)
{
    nat_test_rule_match_addr_spec_build(match, spec);
    nat_test_rule_match_proto_spec_build(match, spec);
    nat_test_rule_match_app_spec_build(match, spec);
}

static inline void
nat_test_rule_data_spec_build (nat_test_rule_t *rule, nat::NatRuleSpec *spec)
{
    nat_test_rule_match_spec_build(&rule->match, spec->mutable_match());
    nat_test_rule_action_spec_build(&rule->action, spec->mutable_action());
}

static inline void
nat_test_rule_key_spec_build (nat_test_rule_t *rule, nat::NatRuleSpec *spec)
{
    spec->set_rule_id(rule->rule_id);
}

static inline void
nat_test_rule_spec_build (nat_test_rule_t *rule, nat::NatRuleSpec *spec)
{
    nat_test_rule_key_spec_build(rule, spec);
    nat_test_rule_data_spec_build(rule, spec);
}

static inline void
nat_test_pol_rule_spec_build (nat_test_pol_t *pol, nat::NatPolicySpec *spec)
{
    for (int i = 0; i < pol->num_rules; i++) {
        auto rule_spec = spec->add_rules();
        nat_test_rule_spec_build(&pol->rules[i], rule_spec);
    }
}

static inline void
nat_test_pol_data_spec_build (nat_test_pol_t *pol, nat::NatPolicySpec *spec)
{
    nat_test_pol_rule_spec_build(pol, spec);
}

static inline void
nat_test_pol_key_spec_build (nat_test_pol_t *pol, kh::NatPolicyKeyHandle *spec)
{
    if (pol->hal_hdl != HAL_HANDLE_INVALID) {
        spec->set_policy_handle(pol->hal_hdl);
    } else {  
        if (pol->vrf_id != -1) {
            spec->mutable_policy_key()->mutable_vrf_key_or_handle()->
                set_vrf_id(pol->vrf_id);
        }
        if (pol->pol_id != -1) {
            spec->mutable_policy_key()->set_nat_policy_id(pol->pol_id);
        }
    }
}

static inline void
nat_test_pol_spec_build (nat_test_pol_t *pol, nat::NatPolicySpec *spec)
{
    nat_test_pol_key_spec_build(pol, spec->mutable_key_or_handle());
    nat_test_pol_data_spec_build(pol, spec);
}

static inline hal_ret_t
nat_test_pol_create (nat::NatPolicySpec& spec, nat::NatPolicyResponse *rsp)
{
    hal_ret_t ret;

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::nat_policy_create(spec, rsp);
    hal::hal_cfg_db_close();
    return ret;
}

//-----------------------------------------------------------------------------
// nat policy GET routines
//-----------------------------------------------------------------------------

static inline void
nat_test_rule_action_spec_extract (const nat::NatRuleAction& spec,
                                   nat_test_rule_action_t *action)
{
    action->src_nat_action = spec.src_nat_action();
    action->dst_nat_action = spec.dst_nat_action();
    action->src_nat_pool = spec.src_nat_pool().pool_handle();
    action->dst_nat_pool = spec.dst_nat_pool().pool_handle();
}

static inline void
nat_test_l4portrange_spec_extract (const types::L4PortRange& port,
                                   port_range_t *port_range)
{
    port_range->port_hi = port.port_high();
    port_range->port_lo = port.port_low();
}

static inline void
nat_test_rule_match_app_match_spec_extract (
    const types::RuleMatch_AppMatch spec, nat_test_rule_match_t *rule_match)
{
    int i;
    const types::RuleMatch_L4PortAppInfo port_info = spec.port_info();

    rule_match->num_src_port = port_info.src_port_range_size();
    for (i = 0; i < rule_match->num_src_port; i++)
        nat_test_l4portrange_spec_extract(port_info.src_port_range(i),
                                          &rule_match->src_port[i]);

    rule_match->num_dst_port = port_info.dst_port_range_size();
    for (i = 0; i < rule_match->num_dst_port; i++)
        nat_test_l4portrange_spec_extract(port_info.dst_port_range(i),
                                          &rule_match->dst_port[i]);
}

static inline void
nat_test_spec_to_ipvx_range (const types::AddressRange& spec,
                             ipvx_range_t *range)
{
    range->ip_lo.v4_addr = spec.ipv4_range().low_ipaddr().v4_addr();
    range->ip_hi.v4_addr = spec.ipv4_range().high_ipaddr().v4_addr();
}

static inline void
nat_test_ipaddressobj_spec_extract (const types::IPAddressObj& spec,
                                    ipvx_range_t *ip_range)
{
    ASSERT_TRUE(spec.negate() == false);
    nat_test_spec_to_ipvx_range(spec.address().range(), ip_range); 
}

static inline void
nat_test_rule_match_spec_extract (const types::RuleMatch& spec,
                                  nat_test_rule_match_t *rule_match)
{
    int i;

    rule_match->proto = spec.protocol();

    rule_match->num_src_ip = spec.src_address_size();
    for (i = 0; i < rule_match->num_src_ip; i++)
        nat_test_ipaddressobj_spec_extract(spec.src_address(i),
                                           &rule_match->src_ip[i]);

    rule_match->num_dst_ip = spec.dst_address_size();
    for (i = 0; i < rule_match->num_dst_ip; i++)
        nat_test_ipaddressobj_spec_extract(spec.dst_address(i),
                                           &rule_match->dst_ip[i]);

    nat_test_rule_match_app_match_spec_extract(spec.app_match(),
                                               rule_match);
}

static inline void
nat_test_rule_data_spec_extract (const nat::NatRuleSpec& spec,
                                 nat_test_rule_t *rule)
{
    nat_test_rule_match_spec_extract(spec.match(), &rule->match);
    nat_test_rule_action_spec_extract(spec.action(), &rule->action);
}

static inline void
nat_test_rule_key_spec_extract (const nat::NatRuleSpec& spec,
                                nat_test_rule_t *rule)
{
    rule->rule_id = spec.rule_id();
}

static inline void
nat_test_rule_spec_extract (const nat::NatRuleSpec& spec, nat_test_rule_t *rule)
{
    nat_test_rule_key_spec_extract(spec, rule);
    nat_test_rule_data_spec_extract(spec, rule);
}

static inline void
nat_test_pol_rule_spec_extract (const nat::NatPolicySpec& spec,
                                nat_test_pol_t *pol)
{
    pol->num_rules = spec.rules_size();
    for (int i = 0; i < pol->num_rules; i++)
        nat_test_rule_spec_extract(spec.rules(i), &pol->rules[i]);
}

static inline void
nat_test_pol_data_spec_extract (const nat::NatPolicySpec& spec,
                                nat_test_pol_t *pol)
{
    nat_test_pol_rule_spec_extract(spec, pol);
}

static inline void
nat_test_pol_key_spec_extract (const nat::NatPolicySpec& spec,
                               nat_test_pol_t *pol)
{
    pol->pol_id = spec.key_or_handle().policy_key().nat_policy_id();
    pol->vrf_id = spec.key_or_handle().policy_key().
        vrf_key_or_handle().vrf_id();
    pol->hal_hdl = spec.key_or_handle().policy_handle();
}

static inline void
nat_test_pol_spec_extract (const nat::NatPolicySpec& spec, nat_test_pol_t *pol)
{
    nat_test_pol_key_spec_extract(spec, pol);
    nat_test_pol_data_spec_extract(spec, pol);
}

static inline hal_ret_t
nat_test_pol_get (nat::NatPolicyGetRequest& req, nat_test_pol_t *pol)
{
    hal_ret_t ret;
    nat::NatPolicyGetResponseMsg rsp;

    memset(pol, 0, sizeof(nat_test_pol_t));
    rsp.Clear();

    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    ret = hal::nat_policy_get(req, &rsp);
    hal::hal_cfg_db_close();

    if (rsp.response_size() > 0)
        nat_test_pol_spec_extract(rsp.response(0).spec(), pol);
    return ret;
}

//-----------------------------------------------------------------------------
// nat policy DELETE routines
//-----------------------------------------------------------------------------

static inline hal_ret_t
nat_test_pol_delete (nat::NatPolicyDeleteRequest &req)
{
    hal_ret_t ret;
    nat::NatPolicyDeleteResponse rsp;

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::nat_policy_delete(req, &rsp);
    hal::hal_cfg_db_close();
    return ret;
}

//-----------------------------------------------------------------------------
// workflows - define common workflows to use across different tests
//-----------------------------------------------------------------------------

static void
nat_test_wf_create_get_delete_get_nat_pol (int num_rules, bool use_hdl)
{
    hal_ret_t ret;
    NatPolicySpec spec; NatPolicyResponse rsp;
    NatPolicyGetRequest get_req;
    NatPolicyDeleteRequest del_req;
    nat_test_pol_t in_pol, out_pol;
    hal_handle_t vrf_hdl, pool_hdl;

    // pre-requisites
    nat_test_vrf_create(TEST_VRF_ID, &vrf_hdl);
    nat_test_natpool_create(TEST_VRF_ID, &pool_hdl);

    // create nat pol
    nat_test_pol_fill(&in_pol, num_rules, 2, 2, 2, 2, pool_hdl);
    //nat_test_pol_dump(&in_pol, "Input Policy", 2);
    spec.Clear();
    nat_test_pol_spec_build(&in_pol, &spec);
    ret = nat_test_pol_create(spec, &rsp);
    ASSERT_TRUE(ret == HAL_RET_OK);
    out_pol.hal_hdl = rsp.policy_status().nat_policy_handle();
    ASSERT_TRUE(out_pol.hal_hdl != HAL_HANDLE_INVALID);

    // get nat pol
    get_req.Clear();
    nat_test_pol_key_spec_build(use_hdl ? &out_pol : &in_pol,
                                get_req.mutable_key_or_handle());
    ret = nat_test_pol_get(get_req, &out_pol);
    //nat_test_pol_dump(&out_pol, "Output Policy", 2);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // compare to see if both create & get policies match
    ASSERT_TRUE(nat_test_pol_cmp(&in_pol, &out_pol) == true);

    // delete nat pol
    del_req.Clear();
    nat_test_pol_key_spec_build(use_hdl ? &out_pol : &in_pol,
                                del_req.mutable_key_or_handle());
    ret = nat_test_pol_delete(del_req);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // get nat pol
    get_req.Clear();
    nat_test_pol_key_spec_build(use_hdl ? &out_pol : &in_pol,
                                get_req.mutable_key_or_handle());
    ret = nat_test_pol_get(get_req, &out_pol);
    ASSERT_TRUE(ret == HAL_RET_NAT_POLICY_NOT_FOUND);

    nat_test_natpool_delete(pool_hdl);
    nat_test_vrf_delete(vrf_hdl);
}

// crgd = create/get/delete
enum Oper { CREATE = 0, GET, DEL };
enum UnknownData { HANDLE = 0, VRF, ID  };
static void
nat_test_wf_crgd_nat_pol_with_unknown_data (Oper oper, UnknownData ud)
{
    hal_ret_t ret;
    nat_test_pol_t pol;
    hal_handle_t vrf_hdl, pool_hdl;

    // assign some random values for unknown data
    memset(&pol, 0, sizeof(nat_test_pol_t));
    switch (ud) {
    case HANDLE:
        pol.hal_hdl = 0x10000; pol.vrf_id = -1; pol.pol_id = -1;
        break;
    case VRF:
        pol.hal_hdl = HAL_HANDLE_INVALID;
        pol.vrf_id = TEST_VRF_ID + 1; pol.pol_id = 0x10;
        break;
    case ID:
        pol.hal_hdl = HAL_HANDLE_INVALID;
        pol.vrf_id = TEST_VRF_ID; pol.pol_id = 0x10;
    }

    // pre-requisites
    nat_test_vrf_create(TEST_VRF_ID, &vrf_hdl);
    nat_test_natpool_create(TEST_VRF_ID, &pool_hdl);

    switch (oper) {
    case CREATE:
    {
        NatPolicySpec spec; NatPolicyResponse rsp;

        nat_test_pol_key_spec_build(&pol, spec.mutable_key_or_handle());
        ret = nat_test_pol_create(spec, &rsp);
        if (ud == VRF)
            ASSERT_TRUE(ret == HAL_RET_VRF_NOT_FOUND);
        else
            ASSERT_TRUE(ret == HAL_RET_INVALID_ARG);
        ASSERT_TRUE(rsp.policy_status().nat_policy_handle() == 
                    HAL_HANDLE_INVALID);
        break;
    }
    case GET:
    {
        NatPolicyGetRequest get_req;
        nat_test_pol_key_spec_build(&pol, get_req.mutable_key_or_handle());
        ret = nat_test_pol_get(get_req, &pol);
        ASSERT_TRUE(ret == HAL_RET_NAT_POLICY_NOT_FOUND);
        break;
    }
    case DEL:
    {
        NatPolicyDeleteRequest del_req;
        ret = nat_test_pol_delete(del_req);
        ASSERT_TRUE(ret == HAL_RET_NAT_POLICY_NOT_FOUND);
        break;
    }
    }

    nat_test_natpool_delete(pool_hdl);
    nat_test_vrf_delete(vrf_hdl);
}

//-----------------------------------------------------------------------------
// tests
//-----------------------------------------------------------------------------

TEST_F(nat_policy_test, create_get_delete_get_pol_using_keys)
{
    nat_test_wf_create_get_delete_get_nat_pol(2, false);
}

TEST_F(nat_policy_test, create_get_delete_get_pol_using_hal_hdl)
{
    nat_test_wf_create_get_delete_get_nat_pol(2, true);
}

TEST_F(nat_policy_test, create_pol_without_rules)
{
    nat_test_wf_create_get_delete_get_nat_pol(0, false);
}

TEST_F(nat_policy_test, create_pol_without_key_or_handle)
{
    hal_ret_t ret;
    hal_handle_t vrf_hdl, pool_hdl;
    NatPolicySpec spec; NatPolicyResponse rsp;

    // pre-requisites
    nat_test_vrf_create(TEST_VRF_ID, &vrf_hdl);
    nat_test_natpool_create(TEST_VRF_ID, &pool_hdl);

    ret = nat_test_pol_create(spec, &rsp);
    ASSERT_TRUE(ret == HAL_RET_INVALID_ARG);
    ASSERT_TRUE(rsp.policy_status().nat_policy_handle() == HAL_HANDLE_INVALID);

    nat_test_natpool_delete(pool_hdl);
    nat_test_vrf_delete(vrf_hdl);
}

TEST_F(nat_policy_test, create_pol_with_unknown_vrf)
{
    nat_test_wf_crgd_nat_pol_with_unknown_data(CREATE, VRF);
}

TEST_F(nat_policy_test, create_pol_with_handle)
{
    nat_test_wf_crgd_nat_pol_with_unknown_data(CREATE, HANDLE);
}

TEST_F(nat_policy_test, get_pol_without_key_or_handle)
{
    hal_ret_t ret;
    nat_test_pol_t pol;
    NatPolicyGetRequest get_req;
    hal_handle_t vrf_hdl, pool_hdl;

    // pre-requisites
    nat_test_vrf_create(TEST_VRF_ID, &vrf_hdl);
    nat_test_natpool_create(TEST_VRF_ID, &pool_hdl);

    get_req.Clear();
    ret = nat_test_pol_get(get_req, &pol);
    ASSERT_TRUE(ret == HAL_RET_OK);

    nat_test_natpool_delete(pool_hdl);
    nat_test_vrf_delete(vrf_hdl);
}

TEST_F(nat_policy_test, get_pol_with_unknown_vrf)
{
    nat_test_wf_crgd_nat_pol_with_unknown_data(GET, VRF);
}

TEST_F(nat_policy_test, get_pol_with_unknown_id)
{
    nat_test_wf_crgd_nat_pol_with_unknown_data(GET, ID);
}

TEST_F(nat_policy_test, get_pol_with_unknown_handle)
{
    nat_test_wf_crgd_nat_pol_with_unknown_data(GET, HANDLE);
}

TEST_F(nat_policy_test, del_pol_without_key_or_handle)
{
    hal_ret_t ret;
    NatPolicyDeleteRequest del_req;
    hal_handle_t vrf_hdl, pool_hdl;

    // pre-requisites
    nat_test_vrf_create(TEST_VRF_ID, &vrf_hdl);
    nat_test_natpool_create(TEST_VRF_ID, &pool_hdl);

    ret = nat_test_pol_delete(del_req);
    ASSERT_TRUE(ret == HAL_RET_NAT_POLICY_NOT_FOUND);

    nat_test_natpool_delete(pool_hdl);
    nat_test_vrf_delete(vrf_hdl);
}

TEST_F(nat_policy_test, del_pol_with_unknown_vrf)
{
    nat_test_wf_crgd_nat_pol_with_unknown_data(DEL, VRF);
}

TEST_F(nat_policy_test, del_pol_with_unknown_id)
{
    nat_test_wf_crgd_nat_pol_with_unknown_data(DEL, ID);
}

TEST_F(nat_policy_test, del_pol_with_unknown_handle)
{
    nat_test_wf_crgd_nat_pol_with_unknown_data(DEL, HANDLE);
}

int main (int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
