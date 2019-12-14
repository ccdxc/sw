#ifdef __x86_64__
#include "nic/hal/plugins/cfg/ipsec/ipsec.hpp"
#endif
#include "nic/hal/hal.hpp"
#include "lib/list/list.hpp"
#include "gen/proto/vrf.pb.h"
#include "gen/proto/ipsec.pb.h"
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
using ipsec::IpsecRuleSpec;
using types::IPProtocol;
using namespace hal;
using namespace fte;
using namespace nwsec;

static inline void
ipsec_test_memleak_checks (void)
{
    // Pre-reqs (might not be complete)
    EXPECT_EQ(g_hal_state->vrf_slab()->num_in_use(), 0);

    // IPSec policy config
    EXPECT_EQ(g_hal_state->ipsec_cfg_pol_slab()->num_in_use(), 0);
    EXPECT_EQ(g_hal_state->ipsec_policy_ht()->num_entries(), 0);

    // IPSec rules config
    EXPECT_EQ(g_hal_state->ipsec_cfg_rule_slab()->num_in_use(), 0);
    EXPECT_EQ(g_hal_state->v4addr_list_elem_slab()->num_in_use(), 0);
    EXPECT_EQ(g_hal_state->v6addr_list_elem_slab()->num_in_use(), 0);
    EXPECT_EQ(g_hal_state->mac_addr_list_elem_slab()->num_in_use(), 0);
    EXPECT_EQ(g_hal_state->port_list_elem_slab()->num_in_use(), 0);

    // IPSec rules oper -- acl etc
    //EXPECT_EQ(g_hal_state->rule_data_slab()->num_in_use(), 0);
    EXPECT_EQ(g_hal_state->ipv4_rule_slab()->num_in_use(), 0);
    // todo - stuff from acl lib
}

class ipsec_policy_test : public hal_base_test {
protected:
    ipsec_policy_test() { }
    virtual ~ipsec_policy_test() { }

    // will be called immediately after the constructor before each test
    virtual void SetUp() { }

    // will be called immediately after each test before the destructor
    virtual void TearDown() {
        ipsec_test_memleak_checks();
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
ipsec_test_vrf_create (int vrf_id, hal_handle_t *vrf_hdl)
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
ipsec_test_vrf_delete (hal_handle_t vrf_hdl)
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
ipsec_test_sa_obj_create (int vrf_id, hal_handle_t *sa_hdl)
{
    IpsecSAEncrypt              encrypt_spec;
    IpsecSAEncryptResponse      encrypt_resp;
    ::google::protobuf::uint32  ip1 = 0x0a010001;
    ::google::protobuf::uint32  ip2 = 0x0a010002;
    hal_ret_t                   ret;

    //Encrypt
    encrypt_spec.mutable_key_or_handle()->set_cb_id(1);
    encrypt_spec.mutable_tep_vrf()->set_vrf_id(TEST_VRF_ID);
    encrypt_spec.set_protocol(ipsec::IpsecProtocol::IPSEC_PROTOCOL_ESP);
    encrypt_spec.set_authentication_algorithm(ipsec::AuthenticationAlgorithm::AUTHENTICATION_AES_GCM);
    encrypt_spec.set_encryption_algorithm(ipsec::EncryptionAlgorithm::ENCRYPTION_ALGORITHM_AES_GCM_256);
    encrypt_spec.mutable_authentication_key()->set_key("cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc");
    encrypt_spec.mutable_encryption_key()->set_key("cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc");
    encrypt_spec.mutable_remote_gateway_ip()->set_ip_af(types::IP_AF_INET);
    encrypt_spec.mutable_remote_gateway_ip()->set_v4_addr(ip2);
    encrypt_spec.mutable_local_gateway_ip()->set_ip_af(types::IP_AF_INET);
    encrypt_spec.mutable_local_gateway_ip()->set_v4_addr(ip1);
    encrypt_spec.set_salt(0xbbbbbbbb);
    encrypt_spec.set_iv(0xaaaaaaaaaaaaaaaa);
    encrypt_spec.set_spi(0);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::ipsec_saencrypt_create(encrypt_spec, &encrypt_resp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    *sa_hdl = encrypt_resp.mutable_ipsec_sa_status()->ipsec_sa_handle();
}

static inline void
ipsec_test_sa_obj_delete (hal_handle_t sa_hdl)
{
    hal_ret_t ret;
    IpsecSAEncryptDeleteRequest del_enc_req;
    IpsecSAEncryptDeleteResponse del_enc_rsp;

    del_enc_req.mutable_key_or_handle()->set_cb_id(1);
    del_enc_req.mutable_key_or_handle()->set_cb_handle(sa_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::ipsec_saencrypt_delete(del_enc_req, &del_enc_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
}

//-----------------------------------------------------------------------------
// ipsec routines
//-----------------------------------------------------------------------------

#define  TEST_IPSEC_CFG_MAX_ENTRIES  10
#define  TEST_POL_ID                 10

typedef struct ipsec_test_rule_match_s {
    ipvx_range_t       src_ip[TEST_IPSEC_CFG_MAX_ENTRIES];
    int                num_src_ip;
    ipvx_range_t       dst_ip[TEST_IPSEC_CFG_MAX_ENTRIES];
    int                num_dst_ip;
    port_range_t       src_port[TEST_IPSEC_CFG_MAX_ENTRIES];
    int                num_src_port;
    port_range_t       dst_port[TEST_IPSEC_CFG_MAX_ENTRIES];
    int                num_dst_port;
    types::IPProtocol  proto;
} ipsec_test_rule_match_t;

typedef struct ipsec_test_rule_action_s {
    ipsec::IpsecSAActionType   sa_action;
    hal_handle_t               sa_handle;
} ipsec_test_rule_action_t;

typedef struct ipsec_test_rule_s {
    int                       rule_id;
    ipsec_test_rule_match_t   match;
    ipsec_test_rule_action_t  action;
} ipsec_test_rule_t;

typedef struct ipsec_test_pol_s {
    int                pol_id;
    int                vrf_id;
    hal_handle_t       hal_hdl;
    ipsec_test_rule_t  rules[TEST_IPSEC_CFG_MAX_ENTRIES];
    int                num_rules;
} ipsec_test_pol_t;

static inline void
ipsec_test_rule_action_fill (ipsec_test_rule_action_t *action, hal_handle_t sa_hdl)
{
  //rule_req.mutable_rules(0)->mutable_sa_action()->set_sa_action_type(ipsec::IpsecSAActionType::IPSEC_SA_ACTION_TYPE_ENCRYPT);
  //rule_req.mutable_rules(0)->mutable_sa_action()->mutable_enc_handle()->set_cb_id(1);

    action->sa_action = ipsec::IpsecSAActionType::IPSEC_SA_ACTION_TYPE_ENCRYPT;
    action->sa_handle = 1; // sa_hdl; CB-ID
}

static inline void
rule_match_test_fill (ipsec_test_rule_match_t *rule_match, int num_src_ip,
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
ipsec_test_pol_fill (ipsec_test_pol_t *pol, int num_rules, int num_src_ip,
                     int num_dst_ip, int num_src_port, int num_dst_port,
                     hal_handle_t sa_hdl)
{
    int i;

    memset(pol, 0, sizeof(ipsec_test_pol_t));
    pol->vrf_id = TEST_VRF_ID; pol->pol_id = TEST_POL_ID;
    pol->hal_hdl = HAL_HANDLE_INVALID;

    pol->num_rules = num_rules;
    for (i = 0; i < num_rules; i++) {
        pol->rules[i].rule_id = i+1;
        rule_match_test_fill(&pol->rules[i].match, num_src_ip, num_dst_ip,
                             num_src_port, num_dst_port, i*100);
        ipsec_test_rule_action_fill(&pol->rules[i].action, sa_hdl);
    }
}

static inline void
ipsec_test_rule_dump (ipsec_test_rule_t *rule, int indent)
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

    printf("%*sSA Encrypt Action: %d, Handle: %lu\n",
           indent, "", rule->action.sa_action,
           rule->action.sa_handle);
}

static inline void
ipsec_test_pol_dump (ipsec_test_pol_t *pol, const char *header, int indent)
{
    printf("%s\n", header);
    printf("%*sKey: vrf_id %d pol_id %d handle %lu\n", indent, "",
           pol->vrf_id, pol->pol_id, pol->hal_hdl);
    printf("%*sData: %d rules\n", indent, "", pol->num_rules);
    for (int i = 0; i < pol->num_rules; i++)
        ipsec_test_rule_dump(&pol->rules[i], indent+2);
}

static inline bool
ipsec_test_pol_cmp (ipsec_test_pol_t *a, ipsec_test_pol_t *b)
{
    if ((a->num_rules != b->num_rules) ||
        (memcmp(a->rules, b->rules, sizeof(a->rules)) != 0))
        return false;

    return true;
}

static inline void
ipsec_sa_encrypt_test_spec_dump (IpsecSAEncryptGetResponseMsg& rsp)
{
    std::string    ipsec_sa_encrypt_cfg_str;

    if (hal::utils::hal_trace_level() < ::utils::trace_debug)  {
        return;
    }

    google::protobuf::util::MessageToJsonString(rsp, &ipsec_sa_encrypt_cfg_str);
    HAL_TRACE_DEBUG("IPSec SA Encrypt Config:");
    HAL_TRACE_DEBUG("{}", ipsec_sa_encrypt_cfg_str.c_str());
    return;
}

static inline void
ipsec_sa_decrypt_test_spec_dump (IpsecSADecryptGetResponseMsg& rsp)
{
    std::string    ipsec_sa_decrypt_cfg_str;

    if (hal::utils::hal_trace_level() < ::utils::trace_debug)  {
        return;
    }

    google::protobuf::util::MessageToJsonString(rsp, &ipsec_sa_decrypt_cfg_str);
    HAL_TRACE_DEBUG("IPSec SA Decrypt Config:");
    HAL_TRACE_DEBUG("{}", ipsec_sa_decrypt_cfg_str.c_str());
    return;
}

//-----------------------------------------------------------------------------
// ipsec policy CREATE routines
//-----------------------------------------------------------------------------

static inline void
ipsec_test_rule_action_spec_build (ipsec_test_rule_action_t *action,
                                   ipsec::IpsecSAAction* spec)
{
    spec->set_sa_action_type(ipsec::IpsecSAActionType::IPSEC_SA_ACTION_TYPE_ENCRYPT);
    spec->mutable_enc_handle()->set_cb_id(1);
}

static inline void
ipsec_test_l4portrange_spec_build (port_range_t *port_range,
                                   types::L4PortRange *spec)
{
    spec->set_port_high(port_range->port_hi);
    spec->set_port_low(port_range->port_lo);
}

static inline void
ipsec_test_rule_match_dst_port_spec_build (ipsec_test_rule_match_t *match,
                                           types::RuleMatch_L4PortAppInfo *spec)
{
    for (int i = 0; i < match->num_dst_port; i++) {
        auto port_range = spec->add_dst_port_range();
        ipsec_test_l4portrange_spec_build(&match->dst_port[i], port_range);
    }
}

static inline void
ipsec_test_rule_match_src_port_spec_build (ipsec_test_rule_match_t *match,
                                           types::RuleMatch_L4PortAppInfo *spec)
{
    for (int i = 0; i < match->num_src_port; i++) {
        auto port_range = spec->add_src_port_range();
        ipsec_test_l4portrange_spec_build(&match->src_port[i], port_range);
    }
}

static inline void
ipsec_test_rule_match_port_app_spec_build (ipsec_test_rule_match_t *match,
                                           types::RuleMatch *spec)
{
    ipsec_test_rule_match_src_port_spec_build(
        match, spec->mutable_app_match()->mutable_port_info());
    ipsec_test_rule_match_dst_port_spec_build(
        match, spec->mutable_app_match()->mutable_port_info());
}

static inline void
ipsec_test_rule_match_app_spec_build (ipsec_test_rule_match_t *match,
                                      types::RuleMatch *spec)
{
    ipsec_test_rule_match_port_app_spec_build(match, spec);
}

static inline void
ipsec_test_rule_match_proto_spec_build (ipsec_test_rule_match_t *match,
                                        types::RuleMatch *spec)
{
    spec->set_protocol(match->proto);
}

static inline void
ipsec_test_ipvx_range_to_spec (types::AddressRange *spec, ipvx_range_t *range)
{
    auto v4_range = spec->mutable_ipv4_range();
    v4_range->mutable_low_ipaddr()->set_v4_addr(range->ip_lo.v4_addr);
    v4_range->mutable_high_ipaddr()->set_v4_addr(range->ip_hi.v4_addr);
}

static inline void
ipsec_test_addr_list_elem_ipaddrobj_spec_build (ipvx_range_t *addr_elem,
                                                types::IPAddressObj *spec)
{
    spec->set_negate(false);
    ipsec_test_ipvx_range_to_spec(spec->mutable_address()->mutable_range(),
                                  addr_elem);
}

static inline void
ipsec_test_rule_match_dst_addr_spec_build (ipsec_test_rule_match_t *match,
                                           types::RuleMatch *spec)
{
    for (int i = 0; i < match->num_dst_ip; i++) {
        auto addr_spec = spec->add_dst_address();
        ipsec_test_addr_list_elem_ipaddrobj_spec_build(&match->dst_ip[i],
                                                       addr_spec);
    }
}

static inline void
ipsec_test_rule_match_src_addr_spec_build (ipsec_test_rule_match_t *match,
                                           types::RuleMatch *spec)
{
    for (int i = 0; i < match->num_src_ip; i++) {
        auto addr_spec = spec->add_src_address();
        ipsec_test_addr_list_elem_ipaddrobj_spec_build(&match->src_ip[i],
                                                       addr_spec);
    }
}

static inline void
ipsec_test_rule_match_addr_spec_build (ipsec_test_rule_match_t *match,
                                       types::RuleMatch *spec)
{
    ipsec_test_rule_match_src_addr_spec_build(match, spec);
    ipsec_test_rule_match_dst_addr_spec_build(match, spec);
}

static inline void
ipsec_test_rule_match_spec_build (ipsec_test_rule_match_t *match,
                                  types::RuleMatch *spec)
{
    ipsec_test_rule_match_addr_spec_build(match, spec);
    ipsec_test_rule_match_proto_spec_build(match, spec);
    ipsec_test_rule_match_app_spec_build(match, spec);
}

static inline void
ipsec_test_rule_data_spec_build (ipsec_test_rule_t *rule, ipsec::IpsecRuleMatchSpec *spec)
{
    ipsec_test_rule_match_spec_build(&rule->match, spec->mutable_match());
    ipsec_test_rule_action_spec_build(&rule->action, spec->mutable_sa_action());
}

static inline void
ipsec_test_rule_key_spec_build (ipsec_test_rule_t *rule, ipsec::IpsecRuleMatchSpec *spec)
{
    spec->set_rule_id(rule->rule_id);
}

static inline void
ipsec_test_rule_spec_build (ipsec_test_rule_t *rule, ipsec::IpsecRuleMatchSpec *spec)
{
    ipsec_test_rule_key_spec_build(rule, spec);
    ipsec_test_rule_data_spec_build(rule, spec);
}

static inline void
ipsec_test_pol_rule_spec_build (ipsec_test_pol_t *pol, ipsec::IpsecRuleSpec *spec)
{
    for (int i = 0; i < pol->num_rules; i++) {
        auto rule_spec = spec->add_rules();
        ipsec_test_rule_spec_build(&pol->rules[i], rule_spec);
    }
}

static inline void
ipsec_test_pol_data_spec_build (ipsec_test_pol_t *pol, ipsec::IpsecRuleSpec *spec)
{
    ipsec_test_pol_rule_spec_build(pol, spec);
}

static inline void
ipsec_test_pol_key_spec_build (ipsec_test_pol_t *pol, kh::IpsecRuleKeyHandle *spec)
{
    if (pol->hal_hdl != HAL_HANDLE_INVALID) {
        spec->set_rule_handle(pol->hal_hdl);
    } else {  
        if (pol->vrf_id != -1) {
            spec->mutable_rule_key()->mutable_vrf_key_or_handle()->
                set_vrf_id(pol->vrf_id);
        }
        if (pol->pol_id != -1) {
            spec->mutable_rule_key()->set_ipsec_rule_id(pol->pol_id);
        }
    }
}

static inline void
ipsec_test_pol_spec_build (ipsec_test_pol_t *pol, ipsec::IpsecRuleSpec *spec)
{
    ipsec_test_pol_key_spec_build(pol, spec->mutable_key_or_handle());
    ipsec_test_pol_data_spec_build(pol, spec);
}

static inline hal_ret_t
ipsec_test_pol_create (ipsec::IpsecRuleSpec& spec, ipsec::IpsecRuleResponse *rsp)
{
    hal_ret_t ret;

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::ipsec_rule_create(spec, rsp);
    hal::hal_cfg_db_close();
    return ret;
}

//-----------------------------------------------------------------------------
// ipsec policy GET routines
//-----------------------------------------------------------------------------

static inline void
ipsec_test_rule_action_spec_extract (const ipsec::IpsecSAAction& spec,
                                     ipsec_test_rule_action_t *action)
{

    action->sa_action = spec.sa_action_type();
    action->sa_handle = spec.enc_handle().cb_id();
}

static inline void
ipsec_test_l4portrange_spec_extract (const types::L4PortRange& port,
                                   port_range_t *port_range)
{
    port_range->port_hi = port.port_high();
    port_range->port_lo = port.port_low();
}

static inline void
ipsec_test_rule_match_app_match_spec_extract (
    const types::RuleMatch_AppMatch spec, ipsec_test_rule_match_t *rule_match)
{
    int i;
    const types::RuleMatch_L4PortAppInfo port_info = spec.port_info();

    rule_match->num_src_port = port_info.src_port_range_size();
    for (i = 0; i < rule_match->num_src_port; i++)
        ipsec_test_l4portrange_spec_extract(port_info.src_port_range(i),
                                          &rule_match->src_port[i]);

    rule_match->num_dst_port = port_info.dst_port_range_size();
    for (i = 0; i < rule_match->num_dst_port; i++)
        ipsec_test_l4portrange_spec_extract(port_info.dst_port_range(i),
                                          &rule_match->dst_port[i]);
}

static inline void
ipsec_test_spec_to_ipvx_range (const types::AddressRange& spec,
                             ipvx_range_t *range)
{
    range->ip_lo.v4_addr = spec.ipv4_range().low_ipaddr().v4_addr();
    range->ip_hi.v4_addr = spec.ipv4_range().high_ipaddr().v4_addr();
}

static inline void
ipsec_test_ipaddressobj_spec_extract (const types::IPAddressObj& spec,
                                    ipvx_range_t *ip_range)
{
    ASSERT_TRUE(spec.negate() == false);
    ipsec_test_spec_to_ipvx_range(spec.address().range(), ip_range); 
}

static inline void
ipsec_test_rule_match_spec_extract (const types::RuleMatch& spec,
                                  ipsec_test_rule_match_t *rule_match)
{
    int i;

    rule_match->proto = spec.protocol();

    rule_match->num_src_ip = spec.src_address_size();
    for (i = 0; i < rule_match->num_src_ip; i++)
        ipsec_test_ipaddressobj_spec_extract(spec.src_address(i),
                                           &rule_match->src_ip[i]);

    rule_match->num_dst_ip = spec.dst_address_size();
    for (i = 0; i < rule_match->num_dst_ip; i++)
        ipsec_test_ipaddressobj_spec_extract(spec.dst_address(i),
                                           &rule_match->dst_ip[i]);

    ipsec_test_rule_match_app_match_spec_extract(spec.app_match(),
                                                   rule_match);
}

static inline void
ipsec_test_rule_data_spec_extract (const ipsec::IpsecRuleMatchSpec& spec,
                                 ipsec_test_rule_t *rule)
{
    ipsec_test_rule_match_spec_extract(spec.match(), &rule->match);
    ipsec_test_rule_action_spec_extract(spec.sa_action(), &rule->action);
}

static inline void
ipsec_test_rule_key_spec_extract (const ipsec::IpsecRuleMatchSpec& spec,
                                ipsec_test_rule_t *rule)
{
    rule->rule_id = spec.rule_id();
}

static inline void
ipsec_test_rule_spec_extract (const ipsec::IpsecRuleMatchSpec& spec, ipsec_test_rule_t *rule)
{
    ipsec_test_rule_key_spec_extract(spec, rule);
    ipsec_test_rule_data_spec_extract(spec, rule);
}

static inline void
ipsec_test_pol_rule_spec_extract (const ipsec::IpsecRuleSpec& spec,
                                ipsec_test_pol_t *pol)
{
    pol->num_rules = spec.rules_size();
    for (int i = 0; i < pol->num_rules; i++)
        ipsec_test_rule_spec_extract(spec.rules(i), &pol->rules[i]);
}

static inline void
ipsec_test_pol_data_spec_extract (const ipsec::IpsecRuleSpec& spec,
                                  ipsec_test_pol_t *pol)
{
    ipsec_test_pol_rule_spec_extract(spec, pol);
}

static inline void
ipsec_test_pol_key_spec_extract (const ipsec::IpsecRuleSpec& spec,
                                 ipsec_test_pol_t *pol)
{
    pol->pol_id = spec.key_or_handle().rule_key().ipsec_rule_id();
    pol->vrf_id = spec.key_or_handle().rule_key().
        vrf_key_or_handle().vrf_id();
    pol->hal_hdl = spec.key_or_handle().rule_handle();
}

static inline void
ipsec_test_pol_spec_extract (const ipsec::IpsecRuleSpec& spec, ipsec_test_pol_t *pol)
{
    ipsec_test_pol_key_spec_extract(spec, pol);
    ipsec_test_pol_data_spec_extract(spec, pol);
}

static inline hal_ret_t
ipsec_test_pol_get (ipsec::IpsecRuleGetRequest& req, ipsec_test_pol_t *pol)
{
    hal_ret_t ret;
    ipsec::IpsecRuleGetResponseMsg rsp;

    memset(pol, 0, sizeof(ipsec_test_pol_t));
    rsp.Clear();

    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    ret = hal::ipsec_rule_get(req, &rsp);
    hal::hal_cfg_db_close();

    if (rsp.response_size() > 0)
        ipsec_test_pol_spec_extract(rsp.response(0).spec(), pol);
    return ret;
}

//-----------------------------------------------------------------------------
// ipsec policy DELETE routines
//-----------------------------------------------------------------------------

static inline hal_ret_t
ipsec_test_pol_delete (ipsec::IpsecRuleDeleteRequest &req)
{
    hal_ret_t ret;
    ipsec::IpsecRuleDeleteResponse rsp;

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::ipsec_rule_delete(req, &rsp);
    hal::hal_cfg_db_close();
    return ret;
}

//-----------------------------------------------------------------------------
// workflows - define common workflows to use across different tests
//-----------------------------------------------------------------------------

static void
ipsec_test_wf_create_get_delete_get_ipsec_pol (int num_rules, bool use_hdl)
{
    hal_ret_t ret;
    IpsecRuleSpec spec; IpsecRuleResponse rsp;
    IpsecRuleGetRequest get_req;
    IpsecRuleDeleteRequest del_req;
    ipsec_test_pol_t in_pol, out_pol;
    hal_handle_t vrf_hdl, sa_hdl;

    // pre-requisites
    ipsec_test_vrf_create(TEST_VRF_ID, &vrf_hdl);
    ipsec_test_sa_obj_create(TEST_VRF_ID, &sa_hdl);

    // create ipsec pol
    ipsec_test_pol_fill(&in_pol, num_rules, 2, 2, 2, 2, sa_hdl);
    ipsec_test_pol_dump(&in_pol, "Input Policy", 2);
    spec.Clear();
    ipsec_test_pol_spec_build(&in_pol, &spec);
    HAL_TRACE_DEBUG("IPSec 1:");
    ret = ipsec_test_pol_create(spec, &rsp);
    HAL_TRACE_DEBUG("IPSec 2:");
    ASSERT_TRUE(ret == HAL_RET_OK);
    out_pol.hal_hdl = rsp.status().handle();
    HAL_TRACE_DEBUG("IPSec 3:");
    ASSERT_TRUE(out_pol.hal_hdl != HAL_HANDLE_INVALID);

    // get ipsec pol
    get_req.Clear();
    HAL_TRACE_DEBUG("IPSec 4:");
    ipsec_test_pol_key_spec_build(use_hdl ? &out_pol : &in_pol,
                                  get_req.mutable_key_or_handle());
    HAL_TRACE_DEBUG("IPSec 5:");
    ret = ipsec_test_pol_get(get_req, &out_pol);
    HAL_TRACE_DEBUG("IPSec 6:");
    ipsec_test_pol_dump(&out_pol, "Output Policy", 2);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // compare to see if both create & get policies match
    ASSERT_TRUE(ipsec_test_pol_cmp(&in_pol, &out_pol) == true);

    HAL_TRACE_DEBUG("IPSec 7:");
    // delete ipsec pol
    del_req.Clear();
    ipsec_test_pol_key_spec_build(use_hdl ? &out_pol : &in_pol,
                                  del_req.mutable_key_or_handle());
    HAL_TRACE_DEBUG("IPSec 8:");
    ret = ipsec_test_pol_delete(del_req);
    ASSERT_TRUE(ret == HAL_RET_OK);
    HAL_TRACE_DEBUG("IPSec 9:");

    // get ipsec pol
    get_req.Clear();
    ipsec_test_pol_key_spec_build(use_hdl ? &out_pol : &in_pol,
                                  get_req.mutable_key_or_handle());
    ret = ipsec_test_pol_get(get_req, &out_pol);
    ASSERT_TRUE(ret == HAL_RET_IPSEC_RULE_NOT_FOUND);
    HAL_TRACE_DEBUG("IPSec 10:");
    ipsec_test_sa_obj_delete(sa_hdl);
    HAL_TRACE_DEBUG("IPSec 11:");
    ipsec_test_vrf_delete(vrf_hdl);
    HAL_TRACE_DEBUG("IPSec 12:");
}

// crgd = create/get/delete
enum Oper { CREATE = 0, GET, DEL };
enum UnknownData { HANDLE = 0, VRF, ID  };
static void
ipsec_test_wf_crgd_ipsec_pol_with_unknown_data (Oper oper, UnknownData ud)
{
    hal_ret_t ret;
    ipsec_test_pol_t pol;
    hal_handle_t vrf_hdl, sa_hdl;

    // assign some random values for unknown data
    memset(&pol, 0, sizeof(ipsec_test_pol_t));
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
    ipsec_test_vrf_create(TEST_VRF_ID, &vrf_hdl);
    ipsec_test_sa_obj_create(TEST_VRF_ID, &sa_hdl);

    switch (oper) {
    case CREATE:
    {
        IpsecRuleSpec spec; IpsecRuleResponse rsp;

        ipsec_test_pol_key_spec_build(&pol, spec.mutable_key_or_handle());
        ret = ipsec_test_pol_create(spec, &rsp);
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
        IpsecRuleGetRequest get_req;
        ipsec_test_pol_key_spec_build(&pol, get_req.mutable_key_or_handle());
        ret = ipsec_test_pol_get(get_req, &pol);
        ASSERT_TRUE(ret == HAL_RET_IPSEC_RULE_NOT_FOUND);
        break;
    }
    case DEL:
    {
        IpsecRuleDeleteRequest del_req;
        ret = ipsec_test_pol_delete(del_req);
        ASSERT_TRUE(ret == HAL_RET_IPSEC_RULE_NOT_FOUND);
        break;
    }
    }

    ipsec_test_sa_obj_delete(sa_hdl);
    ipsec_test_vrf_delete(vrf_hdl);
}

//-----------------------------------------------------------------------------
// tests
//-----------------------------------------------------------------------------

TEST_F(ipsec_policy_test, create_get_delete_get_pol_using_keys)
{
    ipsec_test_wf_create_get_delete_get_ipsec_pol(2, false);
}

TEST_F(ipsec_policy_test, create_get_delete_get_pol_using_hal_hdl)
{
    ipsec_test_wf_create_get_delete_get_ipsec_pol(2, true);
}

TEST_F(ipsec_policy_test, create_pol_without_rules)
{
    ipsec_test_wf_create_get_delete_get_ipsec_pol(0, false);
}

TEST_F(ipsec_policy_test, create_pol_without_key_or_handle)
{
    hal_ret_t ret;
    hal_handle_t vrf_hdl, sa_hdl;
    IpsecRuleSpec spec; IpsecRuleResponse rsp;

    // pre-requisites
    ipsec_test_vrf_create(TEST_VRF_ID, &vrf_hdl);
    ipsec_test_sa_obj_create(TEST_VRF_ID, &sa_hdl);

    ret = ipsec_test_pol_create(spec, &rsp);
    ASSERT_TRUE(ret == HAL_RET_INVALID_ARG);
    ASSERT_TRUE(rsp.status().handle() == HAL_HANDLE_INVALID);

    ipsec_test_sa_obj_delete(sa_hdl);
    ipsec_test_vrf_delete(vrf_hdl);
}

TEST_F(ipsec_policy_test, create_pol_with_unknown_vrf)
{
    ipsec_test_wf_crgd_ipsec_pol_with_unknown_data(CREATE, VRF);
}

TEST_F(ipsec_policy_test, create_pol_with_handle)
{
    ipsec_test_wf_crgd_ipsec_pol_with_unknown_data(CREATE, HANDLE);
}

TEST_F(ipsec_policy_test, get_pol_without_key_or_handle)
{
    hal_ret_t ret;
    ipsec_test_pol_t pol;
    IpsecRuleGetRequest get_req;
    hal_handle_t vrf_hdl, sa_hdl;

    // pre-requisites
    ipsec_test_vrf_create(TEST_VRF_ID, &vrf_hdl);
    ipsec_test_sa_obj_create(TEST_VRF_ID, &sa_hdl);

    get_req.Clear();
    ret = ipsec_test_pol_get(get_req, &pol);
    ASSERT_TRUE(ret == HAL_RET_OK);

    ipsec_test_sa_obj_delete(sa_hdl);
    ipsec_test_vrf_delete(vrf_hdl);
}

TEST_F(ipsec_policy_test, get_pol_with_unknown_vrf)
{
    ipsec_test_wf_crgd_ipsec_pol_with_unknown_data(GET, VRF);
}

TEST_F(ipsec_policy_test, get_pol_with_unknown_id)
{
    ipsec_test_wf_crgd_ipsec_pol_with_unknown_data(GET, ID);
}

TEST_F(ipsec_policy_test, get_pol_with_unknown_handle)
{
    ipsec_test_wf_crgd_ipsec_pol_with_unknown_data(GET, HANDLE);
}

TEST_F(ipsec_policy_test, del_pol_without_key_or_handle)
{
    hal_ret_t ret;
    IpsecRuleDeleteRequest del_req;
    hal_handle_t vrf_hdl, sa_hdl;

    // pre-requisites
    ipsec_test_vrf_create(TEST_VRF_ID, &vrf_hdl);
    ipsec_test_sa_obj_create(TEST_VRF_ID, &sa_hdl);

    ret = ipsec_test_pol_delete(del_req);
    ASSERT_TRUE(ret == HAL_RET_IPSEC_RULE_NOT_FOUND);

    ipsec_test_sa_obj_delete(sa_hdl);
    ipsec_test_vrf_delete(vrf_hdl);
}

TEST_F(ipsec_policy_test, del_pol_with_unknown_vrf)
{
    ipsec_test_wf_crgd_ipsec_pol_with_unknown_data(DEL, VRF);
}

TEST_F(ipsec_policy_test, del_pol_with_unknown_id)
{
    ipsec_test_wf_crgd_ipsec_pol_with_unknown_data(DEL, ID);
}

TEST_F(ipsec_policy_test, del_pol_with_unknown_handle)
{
    ipsec_test_wf_crgd_ipsec_pol_with_unknown_data(DEL, HANDLE);
}

int main (int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
