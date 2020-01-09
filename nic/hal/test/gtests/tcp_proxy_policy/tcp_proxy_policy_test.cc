#include "nic/hal/plugins/cfg/tcp_proxy/tcp_proxy.hpp"
#include "nic/hal/hal.hpp"
#include "lib/list/list.hpp"
#include "gen/proto/vrf.pb.h"
#include "gen/proto/tcp_proxy.pb.h"
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
#include <google/protobuf/util/json_util.h>

using vrf::VrfSpec;
using vrf::VrfResponse;
using tcp_proxy::TcpProxyRuleSpec;
using types::IPProtocol;
using namespace hal;
using namespace fte;
using namespace nwsec;

static inline void
tcp_proxy_test_memleak_checks (void)
{
    // Pre-reqs (might not be complete)
    EXPECT_EQ(g_hal_state->vrf_slab()->num_in_use(), 0);

    // TcpProxyTest policy config
    EXPECT_EQ(g_hal_state->tcp_proxy_cfg_pol_slab()->num_in_use(), 0);
    EXPECT_EQ(g_hal_state->tcp_proxy_policy_ht()->num_entries(), 0);

    // TcpProxyTest rules config
    EXPECT_EQ(g_hal_state->tcp_proxy_cfg_rule_slab()->num_in_use(), 0);
    EXPECT_EQ(g_hal_state->v4addr_list_elem_slab()->num_in_use(), 0);
    EXPECT_EQ(g_hal_state->v6addr_list_elem_slab()->num_in_use(), 0);
    EXPECT_EQ(g_hal_state->mac_addr_list_elem_slab()->num_in_use(), 0);
    EXPECT_EQ(g_hal_state->port_list_elem_slab()->num_in_use(), 0);

    // TcpProxyTest rules oper -- acl etc
    //EXPECT_EQ(g_hal_state->rule_data_slab()->num_in_use(), 0);
    EXPECT_EQ(g_hal_state->ipv4_rule_slab()->num_in_use(), 0);
    // todo - stuff from acl lib
}

class tcp_proxy_policy_test : public hal_base_test {
protected:
    tcp_proxy_policy_test() { }
    virtual ~tcp_proxy_policy_test() { }

    // will be called immediately after the constructor before each test
    virtual void SetUp() { }

    // will be called immediately after each test before the destructor
    virtual void TearDown() {
        tcp_proxy_test_memleak_checks();
    }

    // Will be called at the beginning of all test cases in this class
    static void SetUpTestCase() {
        hal_base_test::SetUpTestCase();
        hal_test_utils_slab_disable_delete();
    }
};

#define  TEST_VRF_ID                 1
//-----------------------------------------------------------------------------
// dependency routines
//-----------------------------------------------------------------------------

static inline void
tcp_proxy_test_vrf_create (int vrf_id, hal_handle_t *vrf_hdl)
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
tcp_proxy_test_vrf_delete (hal_handle_t vrf_hdl)
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

//-----------------------------------------------------------------------------
// tcp_proxy routines
//-----------------------------------------------------------------------------

#define  TEST_CFG_PROXY_CFG_MAX_ENTRIES  10
#define  TEST_POL_ID                 10

typedef struct tcp_proxy_test_rule_match_s {
    ipvx_range_t       src_ip[TEST_CFG_PROXY_CFG_MAX_ENTRIES];
    int                num_src_ip;
    ipvx_range_t       dst_ip[TEST_CFG_PROXY_CFG_MAX_ENTRIES];
    int                num_dst_ip;
    port_range_t       src_port[TEST_CFG_PROXY_CFG_MAX_ENTRIES];
    int                num_src_port;
    port_range_t       dst_port[TEST_CFG_PROXY_CFG_MAX_ENTRIES];
    int                num_dst_port;
    uint16_t           proto;
} tcp_proxy_test_rule_match_t;

typedef struct tcp_proxy_test_rule_action_s {
    tcp_proxy::TcpProxyActionType tcp_proxy_action_type;
} tcp_proxy_test_rule_action_t;

typedef struct tcp_proxy_test_rule_s {
    int                       rule_id;
    tcp_proxy_test_rule_match_t   match;
    tcp_proxy_test_rule_action_t  action;
} tcp_proxy_test_rule_t;

typedef struct tcp_proxy_test_pol_s {
    int                pol_id;
    int                vrf_id;
    hal_handle_t       hal_hdl;
    tcp_proxy_test_rule_t  rules[TEST_CFG_PROXY_CFG_MAX_ENTRIES];
    int                num_rules;
} tcp_proxy_test_pol_t;

static inline void
tcp_proxy_test_rule_action_fill (tcp_proxy_test_rule_action_t *action, tcp_proxy::TcpProxyActionType tcp_proxy_action_type)
{
    //action->tcp_proxy_action_type = tcp_proxy::TcpProxyActionType::TCP_PROXY_ACTION_TYPE_ENABLE; 
    action->tcp_proxy_action_type = tcp_proxy_action_type; 
}

static inline void
rule_match_test_fill (tcp_proxy_test_rule_match_t *rule_match, int num_src_ip,
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
tcp_proxy_test_pol_fill (tcp_proxy_test_pol_t *pol, int num_rules, int num_src_ip,
                     int num_dst_ip, int num_src_port, int num_dst_port,
                     tcp_proxy::TcpProxyActionType tcp_proxy_action_type)
{
    int i;

    memset(pol, 0, sizeof(tcp_proxy_test_pol_t));
    pol->vrf_id = TEST_VRF_ID; pol->pol_id = TEST_POL_ID;
    pol->hal_hdl = HAL_HANDLE_INVALID;

    pol->num_rules = num_rules;
    for (i = 0; i < num_rules; i++) {
        pol->rules[i].rule_id = i+1;
        rule_match_test_fill(&pol->rules[i].match, num_src_ip, num_dst_ip,
                             num_src_port, num_dst_port, i*100);
        tcp_proxy_test_rule_action_fill(&pol->rules[i].action, tcp_proxy_action_type);
    }
}

static inline void
tcp_proxy_test_rule_dump (tcp_proxy_test_rule_t *rule, int indent)
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

}

static inline void
tcp_proxy_test_pol_dump (tcp_proxy_test_pol_t *pol, const char *header, int indent)
{
    printf("%s\n", header);
    printf("%*sKey: vrf_id %d pol_id %d handle %lu\n", indent, "",
           pol->vrf_id, pol->pol_id, pol->hal_hdl);
    printf("%*sData: %d rules\n", indent, "", pol->num_rules);
    for (int i = 0; i < pol->num_rules; i++)
        tcp_proxy_test_rule_dump(&pol->rules[i], indent+2);
}

static inline bool
tcp_proxy_test_pol_cmp (tcp_proxy_test_pol_t *a, tcp_proxy_test_pol_t *b)
{
    if (a->num_rules == b->num_rules) {
        return true;
    }

    if ((a->num_rules != b->num_rules) ||
        (memcmp(a->rules, b->rules, sizeof(a->rules)) != 0))
        return false;

    return true;
}

//-----------------------------------------------------------------------------
// tcp_proxy policy CREATE routines
//-----------------------------------------------------------------------------

static inline void
tcp_proxy_test_rule_action_spec_build (tcp_proxy_test_rule_action_t *action,
                                   tcp_proxy::TcpProxyAction spec)
{
    //spec->set_tcp_proxy_action_type(tcp_proxy::TcpProxyActionType::TCP_PROXY_ACTION_TYPE_ENABLE);
    spec.set_tcp_proxy_action_type(action->tcp_proxy_action_type);
}

static inline void
tcp_proxy_test_l4portrange_spec_build (port_range_t *port_range,
                                   types::L4PortRange *spec)
{
    spec->set_port_high(port_range->port_hi);
    spec->set_port_low(port_range->port_lo);
}

static inline void
tcp_proxy_test_rule_match_dst_port_spec_build (tcp_proxy_test_rule_match_t *match,
                                           types::RuleMatch_L4PortAppInfo *spec)
{
    for (int i = 0; i < match->num_dst_port; i++) {
        auto port_range = spec->add_dst_port_range();
        tcp_proxy_test_l4portrange_spec_build(&match->dst_port[i], port_range);
    }
}

static inline void
tcp_proxy_test_rule_match_src_port_spec_build (tcp_proxy_test_rule_match_t *match,
                                           types::RuleMatch_L4PortAppInfo *spec)
{
    for (int i = 0; i < match->num_src_port; i++) {
        auto port_range = spec->add_src_port_range();
        tcp_proxy_test_l4portrange_spec_build(&match->src_port[i], port_range);
    }
}

static inline void
tcp_proxy_test_rule_match_port_app_spec_build (tcp_proxy_test_rule_match_t *match,
                                           types::RuleMatch *spec)
{
    tcp_proxy_test_rule_match_src_port_spec_build(
        match, spec->mutable_app_match()->mutable_port_info());
    tcp_proxy_test_rule_match_dst_port_spec_build(
        match, spec->mutable_app_match()->mutable_port_info());
}

static inline void
tcp_proxy_test_rule_match_app_spec_build (tcp_proxy_test_rule_match_t *match,
                                      types::RuleMatch *spec)
{
    tcp_proxy_test_rule_match_port_app_spec_build(match, spec);
}

static inline void
tcp_proxy_test_rule_match_proto_spec_build (tcp_proxy_test_rule_match_t *match,
                                        types::RuleMatch *spec)
{
    spec->set_protocol(match->proto);
}

static inline void
tcp_proxy_test_ipvx_range_to_spec (types::AddressRange *spec, ipvx_range_t *range)
{
    auto v4_range = spec->mutable_ipv4_range();
    v4_range->mutable_low_ipaddr()->set_v4_addr(range->ip_lo.v4_addr);
    v4_range->mutable_high_ipaddr()->set_v4_addr(range->ip_hi.v4_addr);
}

static inline void
tcp_proxy_test_addr_list_elem_ipaddrobj_spec_build (ipvx_range_t *addr_elem,
                                                types::IPAddressObj *spec)
{
    spec->set_negate(false);
    tcp_proxy_test_ipvx_range_to_spec(spec->mutable_address()->mutable_range(),
                                  addr_elem);
}

static inline void
tcp_proxy_test_rule_match_dst_addr_spec_build (tcp_proxy_test_rule_match_t *match,
                                           types::RuleMatch *spec)
{
    for (int i = 0; i < match->num_dst_ip; i++) {
        auto addr_spec = spec->add_dst_address();
        tcp_proxy_test_addr_list_elem_ipaddrobj_spec_build(&match->dst_ip[i],
                                                       addr_spec);
    }
}

static inline void
tcp_proxy_test_rule_match_src_addr_spec_build (tcp_proxy_test_rule_match_t *match,
                                           types::RuleMatch *spec)
{
    for (int i = 0; i < match->num_src_ip; i++) {
        auto addr_spec = spec->add_src_address();
        tcp_proxy_test_addr_list_elem_ipaddrobj_spec_build(&match->src_ip[i],
                                                       addr_spec);
    }
}

static inline void
tcp_proxy_test_rule_match_addr_spec_build (tcp_proxy_test_rule_match_t *match,
                                       types::RuleMatch *spec)
{
    tcp_proxy_test_rule_match_src_addr_spec_build(match, spec);
    tcp_proxy_test_rule_match_dst_addr_spec_build(match, spec);
}

static inline void
tcp_proxy_test_rule_match_spec_build (tcp_proxy_test_rule_match_t *match,
                                  types::RuleMatch *spec)
{
    tcp_proxy_test_rule_match_addr_spec_build(match, spec);
    tcp_proxy_test_rule_match_proto_spec_build(match, spec);
    tcp_proxy_test_rule_match_app_spec_build(match, spec);
}

static inline void
tcp_proxy_test_rule_data_spec_build (tcp_proxy_test_rule_t *rule, tcp_proxy::TcpProxyRuleMatchSpec *spec)
{
    tcp_proxy_test_rule_match_spec_build(&rule->match, spec->mutable_match());
    tcp_proxy_test_rule_action_spec_build(&rule->action, spec->tcp_proxy_action());
}

static inline void
tcp_proxy_test_rule_key_spec_build (tcp_proxy_test_rule_t *rule, tcp_proxy::TcpProxyRuleMatchSpec *spec)
{
    spec->set_rule_id(rule->rule_id);
}

static inline void
tcp_proxy_test_rule_spec_build (tcp_proxy_test_rule_t *rule, tcp_proxy::TcpProxyRuleMatchSpec *spec)
{
    tcp_proxy_test_rule_key_spec_build(rule, spec);
    tcp_proxy_test_rule_data_spec_build(rule, spec);
}

static inline void
tcp_proxy_test_pol_rule_spec_build (tcp_proxy_test_pol_t *pol, tcp_proxy::TcpProxyRuleSpec *spec)
{
    for (int i = 0; i < pol->num_rules; i++) {
        auto rule_spec = spec->add_rules();
        tcp_proxy_test_rule_spec_build(&pol->rules[i], rule_spec);
    }
}

static inline void
tcp_proxy_test_pol_data_spec_build (tcp_proxy_test_pol_t *pol, tcp_proxy::TcpProxyRuleSpec *spec)
{
    tcp_proxy_test_pol_rule_spec_build(pol, spec);
}

static inline void
tcp_proxy_test_pol_key_spec_build (tcp_proxy_test_pol_t *pol, kh::TcpProxyRuleKeyHandle *spec)
{
    if (pol->hal_hdl != HAL_HANDLE_INVALID) {
        spec->set_rule_handle(pol->hal_hdl);
    } else {  
        if (pol->vrf_id != -1) {
            spec->mutable_rule_key()->mutable_vrf_key_or_handle()->
                set_vrf_id(pol->vrf_id);
        }
        if (pol->pol_id != -1) {
            spec->mutable_rule_key()->set_tcp_proxy_rule_id(pol->pol_id);
        }
    }
}

static inline void
tcp_proxy_test_pol_spec_build (tcp_proxy_test_pol_t *pol, tcp_proxy::TcpProxyRuleSpec *spec)
{
    tcp_proxy_test_pol_key_spec_build(pol, spec->mutable_key_or_handle());
    tcp_proxy_test_pol_data_spec_build(pol, spec);
}

static inline hal_ret_t
tcp_proxy_test_pol_create (tcp_proxy::TcpProxyRuleSpec& spec, tcp_proxy::TcpProxyRuleResponse *rsp)
{
    hal_ret_t ret;

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::tcp_proxy_rule_create(spec, rsp);
    hal::hal_cfg_db_close();
    return ret;
}

//-----------------------------------------------------------------------------
// tcp_proxy policy GET routines
//-----------------------------------------------------------------------------

static inline void
tcp_proxy_test_rule_action_spec_extract (const tcp_proxy::TcpProxyAction& spec,
                                     tcp_proxy_test_rule_action_t *action)
{
    action->tcp_proxy_action_type = spec.tcp_proxy_action_type();
}

static inline void
tcp_proxy_test_l4portrange_spec_extract (const types::L4PortRange& port,
                                   port_range_t *port_range)
{
    port_range->port_hi = port.port_high();
    port_range->port_lo = port.port_low();
}

static inline void
tcp_proxy_test_rule_match_app_match_spec_extract (
    const types::RuleMatch_AppMatch spec, tcp_proxy_test_rule_match_t *rule_match)
{
    int i;
    const types::RuleMatch_L4PortAppInfo port_info = spec.port_info();

    rule_match->num_src_port = port_info.src_port_range_size();
    for (i = 0; i < rule_match->num_src_port; i++)
        tcp_proxy_test_l4portrange_spec_extract(port_info.src_port_range(i),
                                          &rule_match->src_port[i]);

    rule_match->num_dst_port = port_info.dst_port_range_size();
    for (i = 0; i < rule_match->num_dst_port; i++)
        tcp_proxy_test_l4portrange_spec_extract(port_info.dst_port_range(i),
                                          &rule_match->dst_port[i]);
}

static inline void
tcp_proxy_test_spec_to_ipvx_range (const types::AddressRange& spec,
                             ipvx_range_t *range)
{
    range->ip_lo.v4_addr = spec.ipv4_range().low_ipaddr().v4_addr();
    range->ip_hi.v4_addr = spec.ipv4_range().high_ipaddr().v4_addr();
}

static inline void
tcp_proxy_test_ipaddressobj_spec_extract (const types::IPAddressObj& spec,
                                    ipvx_range_t *ip_range)
{
    ASSERT_TRUE(spec.negate() == false);
    tcp_proxy_test_spec_to_ipvx_range(spec.address().range(), ip_range); 
}

static inline void
tcp_proxy_test_rule_match_spec_extract (const types::RuleMatch& spec,
                                  tcp_proxy_test_rule_match_t *rule_match)
{
    int i;

    rule_match->proto = spec.protocol();

    rule_match->num_src_ip = spec.src_address_size();
    for (i = 0; i < rule_match->num_src_ip; i++)
        tcp_proxy_test_ipaddressobj_spec_extract(spec.src_address(i),
                                           &rule_match->src_ip[i]);

    rule_match->num_dst_ip = spec.dst_address_size();
    for (i = 0; i < rule_match->num_dst_ip; i++)
        tcp_proxy_test_ipaddressobj_spec_extract(spec.dst_address(i),
                                           &rule_match->dst_ip[i]);

    tcp_proxy_test_rule_match_app_match_spec_extract(spec.app_match(),
                                                   rule_match);
}

static inline void
tcp_proxy_test_rule_data_spec_extract (const tcp_proxy::TcpProxyRuleMatchSpec& spec,
                                 tcp_proxy_test_rule_t *rule)
{
    tcp_proxy_test_rule_match_spec_extract(spec.match(), &rule->match);
    tcp_proxy_test_rule_action_spec_extract(spec.tcp_proxy_action(), &rule->action);
}

static inline void
tcp_proxy_test_rule_key_spec_extract (const tcp_proxy::TcpProxyRuleMatchSpec& spec,
                                tcp_proxy_test_rule_t *rule)
{
    rule->rule_id = spec.rule_id();
}

static inline void
tcp_proxy_test_rule_spec_extract (const tcp_proxy::TcpProxyRuleMatchSpec& spec, tcp_proxy_test_rule_t *rule)
{
    tcp_proxy_test_rule_key_spec_extract(spec, rule);
    tcp_proxy_test_rule_data_spec_extract(spec, rule);
}

static inline void
tcp_proxy_test_pol_rule_spec_extract (const tcp_proxy::TcpProxyRuleSpec& spec,
                                tcp_proxy_test_pol_t *pol)
{
    pol->num_rules = spec.rules_size();
    for (int i = 0; i < pol->num_rules; i++)
        tcp_proxy_test_rule_spec_extract(spec.rules(i), &pol->rules[i]);
}

static inline void
tcp_proxy_test_pol_data_spec_extract (const tcp_proxy::TcpProxyRuleSpec& spec,
                                  tcp_proxy_test_pol_t *pol)
{
    tcp_proxy_test_pol_rule_spec_extract(spec, pol);
}

static inline void
tcp_proxy_test_pol_key_spec_extract (const tcp_proxy::TcpProxyRuleSpec& spec,
                                 tcp_proxy_test_pol_t *pol)
{
    pol->pol_id = spec.key_or_handle().rule_key().tcp_proxy_rule_id();
    pol->vrf_id = spec.key_or_handle().rule_key().
        vrf_key_or_handle().vrf_id();
    pol->hal_hdl = spec.key_or_handle().rule_handle();
}

static inline void
tcp_proxy_test_pol_spec_extract (const tcp_proxy::TcpProxyRuleSpec& spec, tcp_proxy_test_pol_t *pol)
{
    tcp_proxy_test_pol_key_spec_extract(spec, pol);
    tcp_proxy_test_pol_data_spec_extract(spec, pol);
}

static inline hal_ret_t
tcp_proxy_test_pol_get (tcp_proxy::TcpProxyRuleGetRequest& req, tcp_proxy_test_pol_t *pol)
{
    hal_ret_t ret;
    tcp_proxy::TcpProxyRuleGetResponseMsg rsp;

    memset(pol, 0, sizeof(tcp_proxy_test_pol_t));
    rsp.Clear();

    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    ret = hal::tcp_proxy_rule_get(req, &rsp);
    hal::hal_cfg_db_close();

    if (rsp.response_size() > 0)
        tcp_proxy_test_pol_spec_extract(rsp.response(0).spec(), pol);
    return ret;
}

//-----------------------------------------------------------------------------
// tcp_proxy policy DELETE routines
//-----------------------------------------------------------------------------

static inline hal_ret_t
tcp_proxy_test_pol_delete (tcp_proxy::TcpProxyRuleDeleteRequest &req)
{
    hal_ret_t ret;
    tcp_proxy::TcpProxyRuleDeleteResponse rsp;

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::tcp_proxy_rule_delete(req, &rsp);
    hal::hal_cfg_db_close();
    return ret;
}

//-----------------------------------------------------------------------------
// workflows - define common workflows to use across different tests
//-----------------------------------------------------------------------------

static void
tcp_proxy_test_wf_create_get_delete_get_tcp_proxy_pol (int num_rules, bool use_hdl)
{
    hal_ret_t ret;
    TcpProxyRuleSpec spec; TcpProxyRuleResponse rsp;
    TcpProxyRuleGetRequest get_req;
    TcpProxyRuleDeleteRequest del_req;
    tcp_proxy_test_pol_t in_pol, out_pol;
    hal_handle_t vrf_hdl; 
    tcp_proxy::TcpProxyActionType tcp_proxy_action_type;

    tcp_proxy_action_type = tcp_proxy::TcpProxyActionType::TCP_PROXY_ACTION_TYPE_ENABLE;

    // pre-requisites
    tcp_proxy_test_vrf_create(TEST_VRF_ID, &vrf_hdl);

    // create tcp_proxy pol
    tcp_proxy_test_pol_fill(&in_pol, num_rules, 2, 2, 2, 2, tcp_proxy_action_type);
    tcp_proxy_test_pol_dump(&in_pol, "Input Policy", 2);
    spec.Clear();
    tcp_proxy_test_pol_spec_build(&in_pol, &spec);
    HAL_TRACE_DEBUG("TcpProxyTest 1:");
    ret = tcp_proxy_test_pol_create(spec, &rsp);
    HAL_TRACE_DEBUG("TcpProxyTest 2:");
    ASSERT_TRUE(ret == HAL_RET_OK);
    out_pol.hal_hdl = rsp.status().handle();
    HAL_TRACE_DEBUG("TcpProxyTest 3:");
    ASSERT_TRUE(out_pol.hal_hdl != HAL_HANDLE_INVALID);

    // get tcp_proxy pol
    get_req.Clear();
    HAL_TRACE_DEBUG("TcpProxyTest 4:");
    tcp_proxy_test_pol_key_spec_build(use_hdl ? &out_pol : &in_pol,
                                  get_req.mutable_key_or_handle());
    HAL_TRACE_DEBUG("TcpProxyTest 5:");
    ret = tcp_proxy_test_pol_get(get_req, &out_pol);
    HAL_TRACE_DEBUG("TcpProxyTest 6:");
    tcp_proxy_test_pol_dump(&out_pol, "Output Policy", 2);
    ASSERT_TRUE(ret == HAL_RET_OK);

    HAL_TRACE_DEBUG("num_rules_in {} num_rules_out {}", in_pol.num_rules, out_pol.num_rules);
    // compare to see if both create & get policies match
    ASSERT_TRUE(tcp_proxy_test_pol_cmp(&in_pol, &out_pol) == true);

    HAL_TRACE_DEBUG("TcpProxyTest 7:");
    // delete tcp_proxy pol
    del_req.Clear();
    tcp_proxy_test_pol_key_spec_build(use_hdl ? &out_pol : &in_pol,
                                  del_req.mutable_key_or_handle());
    HAL_TRACE_DEBUG("TcpProxyTest 8:");
    ret = tcp_proxy_test_pol_delete(del_req);
    ASSERT_TRUE(ret == HAL_RET_OK);
    HAL_TRACE_DEBUG("TcpProxyTest 9:");

    // get tcp_proxy pol
    get_req.Clear();
    tcp_proxy_test_pol_key_spec_build(use_hdl ? &out_pol : &in_pol,
                                  get_req.mutable_key_or_handle());
    ret = tcp_proxy_test_pol_get(get_req, &out_pol);
    ASSERT_TRUE(ret == HAL_RET_TCP_PROXY_RULE_NOT_FOUND);
    HAL_TRACE_DEBUG("TcpProxyTest 10:");
    HAL_TRACE_DEBUG("TcpProxyTest 11:");
    tcp_proxy_test_vrf_delete(vrf_hdl);
    HAL_TRACE_DEBUG("TcpProxyTest 12:");
}

// crgd = create/get/delete
enum Oper { CREATE = 0, GET, DEL };
enum UnknownData { HANDLE = 0, VRF, ID  };
static void
tcp_proxy_test_wf_crgd_tcp_proxy_pol_with_unknown_data (Oper oper, UnknownData ud)
{
    hal_ret_t ret;
    tcp_proxy_test_pol_t pol;
    hal_handle_t vrf_hdl;

    // assign some random values for unknown data
    memset(&pol, 0, sizeof(tcp_proxy_test_pol_t));
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
    tcp_proxy_test_vrf_create(TEST_VRF_ID, &vrf_hdl);

    switch (oper) {
    case CREATE:
    {
        TcpProxyRuleSpec spec; TcpProxyRuleResponse rsp;

        tcp_proxy_test_pol_key_spec_build(&pol, spec.mutable_key_or_handle());
        ret = tcp_proxy_test_pol_create(spec, &rsp);
        if (ud == VRF)
            ASSERT_TRUE(ret == HAL_RET_VRF_NOT_FOUND);
        else
            ASSERT_TRUE(ret == HAL_RET_INVALID_ARG);
        ASSERT_TRUE(rsp.status().handle() == 
                    HAL_HANDLE_INVALID);
        break;
    }
    case GET:
    {
        TcpProxyRuleGetRequest get_req;
        tcp_proxy_test_pol_key_spec_build(&pol, get_req.mutable_key_or_handle());
        ret = tcp_proxy_test_pol_get(get_req, &pol);
        ASSERT_TRUE(ret == HAL_RET_TCP_PROXY_RULE_NOT_FOUND);
        break;
    }
    case DEL:
    {
        TcpProxyRuleDeleteRequest del_req;
        ret = tcp_proxy_test_pol_delete(del_req);
        ASSERT_TRUE(ret == HAL_RET_TCP_PROXY_RULE_NOT_FOUND);
        break;
    }
    }

    tcp_proxy_test_vrf_delete(vrf_hdl);
}

//-----------------------------------------------------------------------------
// tests
//-----------------------------------------------------------------------------

TEST_F(tcp_proxy_policy_test, create_get_delete_get_pol_using_keys)
{
    tcp_proxy_test_wf_create_get_delete_get_tcp_proxy_pol(2, false);
}

TEST_F(tcp_proxy_policy_test, create_get_delete_get_pol_using_hal_hdl)
{
    tcp_proxy_test_wf_create_get_delete_get_tcp_proxy_pol(2, true);
}

TEST_F(tcp_proxy_policy_test, create_pol_without_rules)
{
    tcp_proxy_test_wf_create_get_delete_get_tcp_proxy_pol(0, false);
}

TEST_F(tcp_proxy_policy_test, create_pol_without_key_or_handle)
{
    hal_ret_t ret;
    hal_handle_t vrf_hdl;
    TcpProxyRuleSpec spec; TcpProxyRuleResponse rsp;

    // pre-requisites
    tcp_proxy_test_vrf_create(TEST_VRF_ID, &vrf_hdl);

    ret = tcp_proxy_test_pol_create(spec, &rsp);
    ASSERT_TRUE(ret == HAL_RET_INVALID_ARG);
    ASSERT_TRUE(rsp.status().handle() == HAL_HANDLE_INVALID);

    tcp_proxy_test_vrf_delete(vrf_hdl);
}

TEST_F(tcp_proxy_policy_test, create_pol_with_unknown_vrf)
{
    tcp_proxy_test_wf_crgd_tcp_proxy_pol_with_unknown_data(CREATE, VRF);
}

TEST_F(tcp_proxy_policy_test, create_pol_with_handle)
{
    tcp_proxy_test_wf_crgd_tcp_proxy_pol_with_unknown_data(CREATE, HANDLE);
}

TEST_F(tcp_proxy_policy_test, get_pol_without_key_or_handle)
{
    hal_ret_t ret;
    tcp_proxy_test_pol_t pol;
    TcpProxyRuleGetRequest get_req;
    hal_handle_t vrf_hdl;

    // pre-requisites
    tcp_proxy_test_vrf_create(TEST_VRF_ID, &vrf_hdl);

    get_req.Clear();
    ret = tcp_proxy_test_pol_get(get_req, &pol);
    ASSERT_TRUE(ret == HAL_RET_OK);

    tcp_proxy_test_vrf_delete(vrf_hdl);
}

TEST_F(tcp_proxy_policy_test, get_pol_with_unknown_vrf)
{
    tcp_proxy_test_wf_crgd_tcp_proxy_pol_with_unknown_data(GET, VRF);
}

TEST_F(tcp_proxy_policy_test, get_pol_with_unknown_id)
{
    tcp_proxy_test_wf_crgd_tcp_proxy_pol_with_unknown_data(GET, ID);
}

TEST_F(tcp_proxy_policy_test, get_pol_with_unknown_handle)
{
    tcp_proxy_test_wf_crgd_tcp_proxy_pol_with_unknown_data(GET, HANDLE);
}

TEST_F(tcp_proxy_policy_test, del_pol_without_key_or_handle)
{
    hal_ret_t ret;
    TcpProxyRuleDeleteRequest del_req;
    hal_handle_t vrf_hdl;

    // pre-requisites
    tcp_proxy_test_vrf_create(TEST_VRF_ID, &vrf_hdl);

    ret = tcp_proxy_test_pol_delete(del_req);
    ASSERT_TRUE(ret == HAL_RET_TCP_PROXY_RULE_NOT_FOUND);

    tcp_proxy_test_vrf_delete(vrf_hdl);
}

TEST_F(tcp_proxy_policy_test, del_pol_with_unknown_vrf)
{
    tcp_proxy_test_wf_crgd_tcp_proxy_pol_with_unknown_data(DEL, VRF);
}

TEST_F(tcp_proxy_policy_test, del_pol_with_unknown_id)
{
    tcp_proxy_test_wf_crgd_tcp_proxy_pol_with_unknown_data(DEL, ID);
}

TEST_F(tcp_proxy_policy_test, del_pol_with_unknown_handle)
{
    tcp_proxy_test_wf_crgd_tcp_proxy_pol_with_unknown_data(DEL, HANDLE);
}

int main (int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
