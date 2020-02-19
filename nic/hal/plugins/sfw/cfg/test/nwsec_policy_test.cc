#include "nic/hal/plugins/cfg/nw/interface.hpp"
#include "nic/hal/plugins/sfw/cfg/nwsec.hpp"
#include "nic/hal/plugins/sfw/cfg/nwsec_group.hpp"
#include "nic/hal/hal.hpp"
#include "lib/list/list.hpp"
#include "gen/proto/interface.pb.h"
#include "gen/proto/l2segment.pb.h"
#include "gen/proto/vrf.pb.h"
#include "gen/proto/nwsec.pb.h"
#include "nic/hal/hal.hpp"
#include "gen/proto/types.pb.h"
#include "gen/hal/svc/session_svc_gen.hpp"
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include "nic/hal/test/utils/hal_test_utils.hpp"
#include "nic/hal/test/utils/hal_base_test.hpp"
#include "nic/fte/fte_ctx.hpp"
#include "nic/hal/plugins/sfw/core.hpp"
#include "nic/hal/plugins/app_redir/app_redir_ctx.hpp"
#include "nic/fte/acl/acl_ctx.hpp"
#include "nic/fte/acl/list.hpp"
#include "nic/fte/test/fte_base_test.hpp"
#include "nic/fte/fte.hpp"


#include "nic/hal/svc/debug_svc.hpp"
#include "nic/hal/svc/table_svc.hpp"
#include "nic/hal/svc/event_svc.hpp"
#include "nic/hal/svc/system_svc.hpp"
#include "nic/hal/svc/interface_svc.hpp"
#include "nic/hal/svc/proxy_svc.hpp"

#include "gen/hal/svc/telemetry_svc_gen.hpp"
#include "gen/hal/svc/nw_svc_gen.hpp"
#include "gen/hal/svc/rdma_svc_gen.hpp"
#include "gen/hal/svc/vrf_svc_gen.hpp"
#include "gen/hal/svc/l2segment_svc_gen.hpp"
#include "gen/hal/svc/internal_svc_gen.hpp"
#include "gen/hal/svc/endpoint_svc_gen.hpp"
#include "gen/hal/svc/l4lb_svc_gen.hpp"
#include "gen/hal/svc/nwsec_svc_gen.hpp"
//#include "gen/hal/svc/dos_svc_gen.hpp"
#include "gen/hal/svc/qos_svc_gen.hpp"
#include "gen/hal/svc/acl_svc_gen.hpp"
#include "gen/hal/svc/cpucb_svc_gen.hpp"
#include "gen/hal/svc/multicast_svc_gen.hpp"

using intf::InterfaceSpec;
using intf::InterfaceResponse;
using kh::InterfaceKeyHandle;
using l2segment::L2SegmentSpec;
using l2segment::L2SegmentResponse;
using vrf::VrfSpec;
using vrf::VrfResponse;
using intf::InterfaceL2SegmentSpec;
using intf::InterfaceL2SegmentResponse;
using intf::LifSpec;
using intf::LifResponse;
using kh::LifKeyHandle;
using nwsec::SecurityProfileSpec;
using nwsec::SecurityProfileResponse;
using nwsec::Service;
using nwsec::SecurityGroupPolicyDeleteResponseMsg;
using nwsec::SecurityGroupPolicyDeleteRequest;
using types::IPProtocol;
using namespace hal;
using namespace hal::app_redir;
using namespace fte;
using namespace nwsec;

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

class nwsec_policy_test : public fte_base_test {
protected:
    nwsec_policy_test() {
    }

    virtual ~nwsec_policy_test() {
    }

    // will be called immediately after the constructor before each test
    virtual void SetUp() {
    }

    // will be called immediately after each test before the destructor
    virtual void TearDown() {
        //Route acl is holding one
        EXPECT_EQ(acl_ctx_t::num_ctx_in_use(), 1);
        EXPECT_EQ(list_t::num_lists_in_use(), 1);
        EXPECT_EQ(list_t::num_items_in_use(), 0);
        EXPECT_EQ(g_hal_state->nwsec_policy_ht()->num_entries(), 0);
        EXPECT_EQ(g_hal_state->nwsec_rule_slab()->num_in_use(), 0);
        EXPECT_EQ(g_hal_state->ipv4_rule_slab()->num_in_use(), 0);
        EXPECT_EQ(g_hal_state->nwsec_group_ht()->num_entries(), 0);
        EXPECT_EQ(g_hal_state->rule_ctr_slab()->num_in_use(), 0);
        EXPECT_EQ(g_hal_state->rule_data_slab()->num_in_use(), 0);
        EXPECT_EQ(g_hal_state->rule_cfg_slab()->num_in_use(), 0);

    }


    // Will be called at the beginning of all test cases in this class
    static void SetUpTestCase() {
        //setenv("HAL_CONFIG_PATH", "/nic/conf", 1);
        //fte_base_test::SetUpTestCase("hal_hw.json");
        fte_base_test::SetUpTestCase();

        // Create a topo
        vrfh = add_vrf();
        hal_handle_t nwh = add_network(vrfh, 0x0A000000, 8, 0xAABB0A000000);
        hal_handle_t l2segh = add_l2segment(nwh, 100);
        hal_handle_t intfh1 = add_uplink(PORT_NUM_1);
        hal_handle_t intfh2 = add_uplink(PORT_NUM_2);
        client_eph = add_endpoint(l2segh, intfh1, 0x0A000001, 0xAABB0A000001, 0);
        server_eph = add_endpoint(l2segh, intfh2, 0x0A000002, 0xAABB0A000002, 0);
            
        hal_test_utils_slab_disable_delete();
    }

    static hal_handle_t client_eph, server_eph, vrfh;

};

hal_handle_t nwsec_policy_test::client_eph, nwsec_policy_test::server_eph, nwsec_policy_test::vrfh;

// ----------------------------------------------------------------------------
// Create nwsec profile
// Update
// Delete
// ----------------------------------------------------------------------------
TEST_F(nwsec_policy_test, test1)
{
    hal_ret_t                               ret;
    SecurityPolicySpec                      pol_spec;
    SecurityPolicyResponse                  res;
    SecurityRule                           *rule_spec, rule_spec2;
    SecurityPolicyDeleteRequest             pol_del_req;
    SecurityPolicyDeleteResponse            pol_del_rsp;
    //slab_stats_t                            *pre = NULL, *post = NULL;
    //bool                                    is_leak = false;

    //pre = hal_test_utils_collect_slab_stats();
    //current_policy_profile = glbl_rule_profile[0];

    pol_spec.mutable_key_or_handle()->mutable_security_policy_key()->set_security_policy_id(10);
    pol_spec.mutable_key_or_handle()->mutable_security_policy_key()->mutable_vrf_id_or_handle()->set_vrf_id(0);
    rule_spec = pol_spec.add_rule();

    // Create nwsec
    rule_spec->set_rule_id(1);
    rule_spec->mutable_action()->set_sec_action(nwsec::SecurityAction::SECURITY_RULE_ACTION_ALLOW);
    types::RuleMatch *match = rule_spec->mutable_match();
    match->set_protocol(types::IPPROTO_TCP);

    types::IPAddressObj *dst_addr = match->add_dst_address();
    dst_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
    dst_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_v4_addr(0xAABBCC00);
    types::IPAddressObj *src_addr = match->add_src_address();
    src_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
    src_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_v4_addr(0x11223300);

    types::RuleMatch_AppMatch *app = match->mutable_app_match();
    types::L4PortRange *port_range = app->mutable_port_info()->add_dst_port_range();
    port_range->set_port_low(1000);
    port_range->set_port_high(2000);
    types::L4PortRange *src_port_range = app->mutable_port_info()->add_src_port_range();
    src_port_range->set_port_low(100);
    src_port_range->set_port_high(200);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securitypolicy_create(pol_spec, &res);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t policy_handle = res.policy_status().key_or_handle().security_policy_handle();

    ipv4_tuple v4_tuple = {};
    v4_tuple.ip_dst = 0xAABBCC00; // server
    v4_tuple.ip_src = 0x11223300;
    v4_tuple.port_dst = 1000;
    v4_tuple.port_src = 100;
    v4_tuple.proto = types::IPPROTO_TCP;
    ipv4_rule_t *rule = NULL;
    nwsec_policy_t *res_policy;
    res_policy = find_nwsec_policy_by_key(10, 0);
    const char *ctx_name = nwsec_acl_ctx_name(res_policy->key.vrf_id);
    const acl_ctx_t *acl_ctx = acl::acl_get(ctx_name);
    acl_classify(acl_ctx, (const uint8_t *)&v4_tuple, (const acl_rule_t **)&rule, 0x01);
    EXPECT_NE(rule, nullptr);

    HAL_TRACE_DEBUG(" create/classify packet pass");

    v4_tuple.ip_dst = 0xAABB0000;
    v4_tuple.port_dst = 1000;
    v4_tuple.port_src = 300;
    v4_tuple.ip_src = 0x11224400;
    v4_tuple.proto = types::IPPROTO_TCP;
    rule = NULL;
    acl_classify(acl_ctx, (const uint8_t *)&v4_tuple, (const acl_rule_t **)&rule, 0x01);
    EXPECT_EQ(rule, nullptr);
    acl::acl_deref(acl_ctx);
    // Policy Update
    rule_spec = pol_spec.add_rule();
    rule_spec->set_rule_id(2);
    rule_spec->mutable_action()->set_sec_action(nwsec::SecurityAction::SECURITY_RULE_ACTION_ALLOW);
    match = rule_spec->mutable_match();
    match->set_protocol(types::IPPROTO_TCP);
    dst_addr = match->add_dst_address();
    dst_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
    dst_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_v4_addr(0xAABB0000);
    src_addr = match->add_src_address();
    src_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
    src_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_v4_addr(0x11224400);

    app = match->mutable_app_match();
    port_range = app->mutable_port_info()->add_dst_port_range();
    port_range->set_port_low(1000);
    port_range->set_port_high(2000);
    src_port_range = app->mutable_port_info()->add_src_port_range();
    src_port_range->set_port_low(300);
    src_port_range->set_port_high(400);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securitypolicy_update(pol_spec, &res);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    res_policy = find_nwsec_policy_by_key(10, 0);

    acl_ctx = acl::acl_get(ctx_name);
    acl_classify(acl_ctx, (const uint8_t *)&v4_tuple, (const acl_rule_t **)&rule, 0x01);
    EXPECT_NE(rule, nullptr);
    acl::acl_deref(acl_ctx);

    // Delete policy
    pol_del_req.mutable_key_or_handle()->set_security_policy_handle(policy_handle);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securitypolicy_delete(pol_del_req, &pol_del_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

#if 0
    svc_reg(std::string("0.0.0.0:") + std::string("50054"), hal::HAL_FEATURE_SET_IRIS);
    hal::hal_wait();
#endif

    // There is a leak of HAL_SLAB_HANDLE_ID_LIST_ENTRY for adding
#if 0
    post = hal_test_utils_collect_slab_stats();
    hal_test_utils_check_slab_leak(pre, post, &is_leak);
    ASSERT_TRUE(is_leak == false);
#endif
}

// ----------------------------------------------------------------------------
// Create a SecurityGroupPolicySpec
// ----------------------------------------------------------------------------
TEST_F(nwsec_policy_test, test2)
{
    hal_ret_t                               ret;
    SecurityPolicySpec                      pol_spec;
    SecurityPolicyResponse                  res;
    SecurityRule                           *rule_spec, rule_spec2;
    SecurityPolicyDeleteRequest             pol_del_req;
    SecurityPolicyDeleteResponse            pol_del_rsp;

    hal::vrf_t *vrf = hal::vrf_lookup_by_handle(nwsec_policy_test::vrfh);
    pol_spec.mutable_key_or_handle()->mutable_security_policy_key()->set_security_policy_id(11);
    pol_spec.mutable_key_or_handle()->mutable_security_policy_key()->mutable_vrf_id_or_handle()->set_vrf_id(vrf->vrf_id);
    rule_spec = pol_spec.add_rule();

    // Create nwsec
    rule_spec->set_rule_id(1);
    rule_spec->mutable_action()->set_sec_action(nwsec::SecurityAction::SECURITY_RULE_ACTION_ALLOW);
    nwsec::AppData app_data  = rule_spec->mutable_action()->app_data();
    app_data.set_alg(nwsec::APP_SVC_FTP);
    app_data.mutable_ftp_option_info()->set_allow_mismatch_ip_address(1);
    types::RuleMatch *match = rule_spec->mutable_match();
    match->set_protocol(types::IPPROTO_TCP);

    types::IPAddressObj *dst_addr = match->add_dst_address();
    dst_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
    dst_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_v4_addr(0x0A000002);
    types::IPAddressObj *src_addr = match->add_src_address();
    src_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
    src_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_v4_addr(0x0A000001);

    rule_spec = pol_spec.add_rule();
    rule_spec->set_rule_id(1);
    rule_spec->mutable_action()->set_sec_action(nwsec::SecurityAction::SECURITY_RULE_ACTION_ALLOW);
    app_data  = rule_spec->mutable_action()->app_data();
    app_data.set_alg(nwsec::APP_SVC_FTP);
    app_data.mutable_ftp_option_info()->set_allow_mismatch_ip_address(1);
    match = rule_spec->mutable_match();
    match->set_protocol(types::IPPROTO_TCP);

    dst_addr = match->add_dst_address();
    dst_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
    dst_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_v4_addr(0x0A000003);
    src_addr = match->add_src_address();
    src_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
    src_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_v4_addr(0x0A000001);


    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securitypolicy_create(pol_spec, &res);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t policy_handle = res.policy_status().key_or_handle().security_policy_handle();

    Tins::TCP tcp = Tins::TCP(100,101);
    ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, nwsec_policy_test::server_eph, nwsec_policy_test::client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);


    pol_spec.clear_rule();
    rule_spec = pol_spec.add_rule();
    // Update nwsec
    rule_spec->set_rule_id(10);
    rule_spec->mutable_action()->set_sec_action(nwsec::SecurityAction::SECURITY_RULE_ACTION_DENY);
    match = rule_spec->mutable_match();

    match->set_protocol(types::IPPROTO_TCP);
    dst_addr = match->add_dst_address();
    dst_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
    dst_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_v4_addr(0x0A000002);
    src_addr = match->add_src_address();
    src_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
    src_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_v4_addr(0x0A000001);

    rule_spec = pol_spec.add_rule();
    rule_spec->set_rule_id(10);
    rule_spec->mutable_action()->set_sec_action(nwsec::SecurityAction::SECURITY_RULE_ACTION_ALLOW);
    match = rule_spec->mutable_match();
    match->set_protocol(types::IPPROTO_TCP);
    dst_addr = match->add_dst_address();
    dst_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
    dst_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_v4_addr(0xAABB0000);
    src_addr = match->add_src_address();
    src_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
    src_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_v4_addr(0x11224400);

    types::RuleMatch_AppMatch *app = match->mutable_app_match();
    types::L4PortRange *port_range = app->mutable_port_info()->add_dst_port_range();
    port_range->set_port_low(3000);
    port_range->set_port_high(4000);
    types::L4PortRange *src_port_range = app->mutable_port_info()->add_src_port_range();
    src_port_range->set_port_low(200);
    src_port_range->set_port_high(300);


    rule_spec = pol_spec.add_rule();
    rule_spec->set_rule_id(2);
    rule_spec->mutable_action()->set_sec_action(nwsec::SecurityAction::SECURITY_RULE_ACTION_ALLOW);
    match = rule_spec->mutable_match();
    match->set_protocol(types::IPPROTO_TCP);
    dst_addr = match->add_dst_address();
    dst_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
    dst_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_v4_addr(0xAABB0000);
    src_addr = match->add_src_address();
    src_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
    src_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_v4_addr(0x11224400);

    app = match->mutable_app_match();
    port_range = app->mutable_port_info()->add_dst_port_range();
    port_range->set_port_low(1000);
    port_range->set_port_high(2000);
    src_port_range = app->mutable_port_info()->add_src_port_range();
    src_port_range->set_port_low(300);
    src_port_range->set_port_high(400);


    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securitypolicy_update(pol_spec, &res);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    tcp = Tins::TCP(100,101);
    ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, nwsec_policy_test::server_eph, nwsec_policy_test::client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);


    tcp = Tins::TCP(103,104);
    ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, nwsec_policy_test::server_eph, nwsec_policy_test::client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);

    HAL_TRACE_DEBUG("RET_OK");

    // Delete policy
    pol_del_req.mutable_key_or_handle()->set_security_policy_handle(policy_handle);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securitypolicy_delete(pol_del_req, &pol_del_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

#if 0
    svc_reg(std::string("0.0.0.0:") + std::string("50054"), hal::HAL_FEATURE_SET_IRIS);
    hal::hal_wait();
#endif


    // There is a leak of HAL_SLAB_HANDLE_ID_LIST_ENTRY for adding
    //post = hal_test_utils_collect_slab_stats();
    //hal_test_utils_check_slab_leak(pre, post, &is_leak);
    //ASSERT_TRUE(is_leak == false);
}

TEST_F(nwsec_policy_test, test3)
{
    hal_ret_t                               ret;
    SecurityPolicySpec                      pol_spec;
    SecurityPolicyResponse                  res;
    SecurityRule                           *rule_spec, rule_spec2;
    SecurityPolicyDeleteRequest             pol_del_req;
    SecurityPolicyDeleteResponse            pol_del_rsp;

    hal::vrf_t *vrf = hal::vrf_lookup_by_handle(nwsec_policy_test::vrfh);
    pol_spec.mutable_key_or_handle()->mutable_security_policy_key()->set_security_policy_id(11);
    pol_spec.mutable_key_or_handle()->mutable_security_policy_key()->mutable_vrf_id_or_handle()->set_vrf_id(vrf->vrf_id);
    rule_spec = pol_spec.add_rule();

    // Create nwsec
    rule_spec->set_rule_id(1);
    rule_spec->mutable_action()->set_sec_action(nwsec::SecurityAction::SECURITY_RULE_ACTION_ALLOW);
    types::RuleMatch *match = rule_spec->mutable_match();

    match->set_protocol(types::IPPROTO_ICMP);

    types::IPAddressObj *dst_addr = match->add_dst_address();
    dst_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
    dst_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_v4_addr(0x0A000002);
    types::IPAddressObj *src_addr = match->add_src_address();
    src_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
    src_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_v4_addr(0x0A000001);
    rule_spec->mutable_action()->set_sec_action(nwsec::SecurityAction::SECURITY_RULE_ACTION_ALLOW);

    types::RuleMatch_AppMatch *app = match->mutable_app_match();
    app->mutable_icmp_info()->set_icmp_type(8);
    app->mutable_icmp_info()->set_icmp_code(0);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securitypolicy_create(pol_spec, &res);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t policy_handle = res.policy_status().key_or_handle().security_policy_handle();

    Tins::ICMP icmp = Tins::ICMP(Tins::ICMP::ECHO_REQUEST);
    ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, nwsec_policy_test::server_eph, nwsec_policy_test::client_eph, icmp);
    EXPECT_EQ(ret, HAL_RET_OK);

    // Delete policy
    pol_del_req.mutable_key_or_handle()->set_security_policy_handle(policy_handle);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securitypolicy_delete(pol_del_req, &pol_del_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
#if 0    

    svc_reg(std::string("0.0.0.0:") + std::string("50054"), hal::HAL_FEATURE_SET_IRIS);
    hal::hal_wait();
#endif


    // There is a leak of HAL_SLAB_HANDLE_ID_LIST_ENTRY for adding
    //post = hal_test_utils_collect_slab_stats();
    //hal_test_utils_check_slab_leak(pre, post, &is_leak);
    //ASSERT_TRUE(is_leak == false);
}

TEST_F(nwsec_policy_test, test4)
{
    hal_ret_t                               ret;
    SecurityGroupSpec                       sp_spec;
    SecurityGroupResponse                   sp_rsp;
    SecurityGroupDeleteRequest              del_req;
    SecurityGroupDeleteResponseMsg          del_res;

    dllist_ctxt_t                           *curr, *next, *nw_list, *ep_list;
    hal_handle_id_list_entry_t              *nw_ent = NULL, *ep_ent = NULL;

    // Create SecurityGroupSpec
    sp_spec.mutable_key_or_handle()->set_security_group_id(1);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securitygroup_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    uint64_t sg_handle = sp_rsp.status().key_or_handle().security_group_handle();


    for (int i = 0; i < 4; i++) {
        ret = add_nw_to_security_group(1, 0x1000 + i);
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

    nw_list = get_nw_list_for_security_group(1);
    if (nw_list != NULL) {
        dllist_for_each_safe(curr, next, nw_list) {
            nw_ent = dllist_entry(curr, hal_handle_id_list_entry_t, dllist_ctxt);
            HAL_TRACE_DEBUG("nw handle {}", nw_ent->handle_id);
        }
    }


    for (int i = 0; i < 4; i++) {
        ret = del_nw_from_security_group(1, 0x1000 + i);
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

    nw_list = get_nw_list_for_security_group(1);
    if (nw_list != NULL) {
        dllist_for_each_safe(curr, next, nw_list) {
            nw_ent = dllist_entry(curr, hal_handle_id_list_entry_t, dllist_ctxt);
            HAL_TRACE_DEBUG("nw handle {}", nw_ent->handle_id);
        }
    }

    curr = NULL;
    next = NULL;
    for (int i = 0; i < 4; i++) {
        ret = add_ep_to_security_group(1, 0x2000 + i);
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

    ep_list = get_ep_list_for_security_group(1);
    if (ep_list != NULL) {
        dllist_for_each_safe(curr, next, ep_list) {
            ep_ent = dllist_entry(curr, hal_handle_id_list_entry_t, dllist_ctxt);
            HAL_TRACE_DEBUG("ep handle {}", ep_ent->handle_id);
        }
    }


    for (int i = 0; i < 4; i++) {
        ret = del_ep_from_security_group(1, 0x2000 + i);
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

    ep_list = get_ep_list_for_security_group(1);
    if (ep_list != NULL) {
        dllist_for_each_safe(curr, next, ep_list) {
            ep_ent = dllist_entry(curr, hal_handle_id_list_entry_t, dllist_ctxt);
            HAL_TRACE_DEBUG("ep handle {}", ep_ent->handle_id);
        }
    }


    del_req.mutable_key_or_handle()->set_security_group_handle(sg_handle);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securitygroup_delete(del_req, &del_res);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

}

/** Scale rules **/
TEST_F(nwsec_policy_test, test5)
{
    hal_ret_t                               ret;
    SecurityPolicySpec                      pol_spec;
    SecurityPolicyResponse                  res;
    SecurityRule                           *rule_spec, rule_spec2;
    SecurityPolicyDeleteRequest             pol_del_req;
    SecurityPolicyDeleteResponse            pol_del_rsp;
    uint32_t                                num_rules = 100000;
    uint32_t                                num_tenants = 1;
    vector <fte_base_test::v4_rule_t *>        rules;
    
    //acl_test::gen_rules(num_rules, num_tenants, rules, keys);    

    hal::vrf_t *vrf = hal::vrf_lookup_by_handle(nwsec_policy_test::vrfh);
    pol_spec.mutable_key_or_handle()->mutable_security_policy_key()->set_security_policy_id(11);
    pol_spec.mutable_key_or_handle()->mutable_security_policy_key()->mutable_vrf_id_or_handle()->set_vrf_id(vrf->vrf_id);

    fte_base_test::gen_rules(num_rules, num_tenants, rules); 
    uint64_t policy_handle;

    fte_base_test::timeit("insert", num_rules, [&]() {

        for (uint32_t i = 0; i < num_rules; i++) {
            // Create nwsec

            rule_spec = pol_spec.add_rule();
            rule_spec->set_rule_id(i);
            rule_spec->mutable_action()->set_sec_action(nwsec::SecurityAction::SECURITY_RULE_ACTION_ALLOW);
            types::RuleMatch *match = rule_spec->mutable_match();

            if (rules[i]->app.proto == 6 ) {
                match->set_protocol(types::IPPROTO_TCP);
            } else if (rules[i]->app.proto == 17) {
                match->set_protocol(types::IPPROTO_UDP);
            }

            types::IPAddressObj *dst_addr = match->add_dst_address();
            dst_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
            dst_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_v4_addr(rules[i]->to.addr);
            dst_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->set_prefix_len(rules[i]->to.plen);
            types::IPAddressObj *src_addr = match->add_src_address();
            src_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
            src_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_v4_addr(rules[i]->from.addr);
            src_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->set_prefix_len(rules[i]->from.plen);
            

            types::RuleMatch_AppMatch *app = match->mutable_app_match();
            types::L4PortRange *dst_port_range = app->mutable_port_info()->add_dst_port_range();
            dst_port_range->set_port_low(rules[i]->app.dport_low); 
            dst_port_range->set_port_high(rules[i]->app.dport_high);
        }

        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::securitypolicy_create(pol_spec, &res);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
        policy_handle = res.policy_status().key_or_handle().security_policy_handle();
    });

    ::testing::internal::TimeInMillis elapsed(
            ::testing::UnitTest::GetInstance()->elapsed_time());

    cout << "elapsed time" << elapsed;

    pol_spec.clear_rule();


    fte_base_test::gen_rules(num_rules, num_tenants, rules); 

    fte_base_test::timeit("update", num_rules, [&]() {

        for (uint32_t i = 0; i < num_rules; i++) {
            // Create nwsec

            rule_spec = pol_spec.add_rule();
            rule_spec->set_rule_id(i);
            rule_spec->mutable_action()->set_sec_action(nwsec::SecurityAction::SECURITY_RULE_ACTION_ALLOW);
            types::RuleMatch *match = rule_spec->mutable_match();

            if (rules[i]->app.proto == 6 ) {
                match->set_protocol(types::IPPROTO_TCP);
            } else if (rules[i]->app.proto == 17) {
                match->set_protocol(types::IPPROTO_UDP);
            }

            types::IPAddressObj *dst_addr = match->add_dst_address();
            dst_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
            dst_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_v4_addr(rules[i]->to.addr);
            dst_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->set_prefix_len(rules[i]->to.plen);
            types::IPAddressObj *src_addr = match->add_src_address();
            src_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
            src_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_v4_addr(rules[i]->from.addr);
            src_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->set_prefix_len(rules[i]->from.plen);
            

            types::RuleMatch_AppMatch *app = match->mutable_app_match();
            types::L4PortRange *dst_port_range = app->mutable_port_info()->add_dst_port_range();
            dst_port_range->set_port_low(rules[i]->app.dport_low); 
            dst_port_range->set_port_high(rules[i]->app.dport_high);
        }

        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::securitypolicy_update(pol_spec, &res);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
        policy_handle = res.policy_status().key_or_handle().security_policy_handle();
    });

    fte_base_test::timeit("delete", num_rules, [&]() {

        // Delete policy
        pol_del_req.mutable_key_or_handle()->set_security_policy_handle(policy_handle);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::securitypolicy_delete(pol_del_req, &pol_del_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
    });
#if 0    

    svc_reg(std::string("0.0.0.0:") + std::string("50054"), hal::HAL_FEATURE_SET_IRIS);
    hal::hal_wait();
#endif


    // There is a leak of HAL_SLAB_HANDLE_ID_LIST_ENTRY for adding
    //post = hal_test_utils_collect_slab_stats();
    //hal_test_utils_check_slab_leak(pre, post, &is_leak);
    //ASSERT_TRUE(is_leak == false);
}

//Verify policy reeval
TEST_F(nwsec_policy_test, test6) {
    hal_ret_t                               ret;
    SecurityPolicySpec                      pol_spec;
    SecurityPolicyResponse                  res;
    SecurityRule                           *rule_spec, rule_spec2;
    SecurityPolicyDeleteRequest             pol_del_req;
    SecurityPolicyDeleteResponse            pol_del_rsp;
    hal_handle_t                            sess_hdl = 0, skip_sess_hdl = 0;
    hal::session_t                          *session = NULL;

    hal::vrf_t *vrf = hal::vrf_lookup_by_handle(nwsec_policy_test::vrfh);
    pol_spec.mutable_key_or_handle()->mutable_security_policy_key()->set_security_policy_id(11);
    pol_spec.mutable_key_or_handle()->mutable_security_policy_key()->mutable_vrf_id_or_handle()->set_vrf_id(vrf->vrf_id);
    rule_spec = pol_spec.add_rule();

    // Create nwsec
    rule_spec->set_rule_id(1);
    rule_spec->mutable_action()->set_sec_action(nwsec::SecurityAction::SECURITY_RULE_ACTION_ALLOW);
    types::RuleMatch *match = rule_spec->mutable_match();
    match->set_protocol(types::IPPROTO_TCP);

    types::IPAddressObj *dst_addr = match->add_dst_address();
    dst_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
    dst_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_v4_addr(0x0A000002);
    types::IPAddressObj *src_addr = match->add_src_address();
    src_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
    src_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_v4_addr(0x0A000001);


    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securitypolicy_create(pol_spec, &res);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t policy_handle = res.policy_status().key_or_handle().security_policy_handle();

    Tins::TCP tcp = Tins::TCP(100,101);
    ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, nwsec_policy_test::server_eph, nwsec_policy_test::client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    session = ctx_.session();
    HAL_TRACE_DEBUG("skip reval: {}", session->skip_sfw_reval);
    if (session) {
        sess_hdl = ctx_.session()->hal_handle;
        HAL_TRACE_DEBUG("hal ses hanlde : {}", sess_hdl);
    }

    tcp = Tins::TCP(200,201);
    ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, nwsec_policy_test::server_eph, nwsec_policy_test::client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    session = ctx_.session();
    if (session) {
        ctx_.session()->skip_sfw_reval = 1;
        skip_sess_hdl = ctx_.session()->hal_handle;
    }
    

    pol_spec.clear_rule();
    rule_spec = pol_spec.add_rule();
    // Update nwsec
    rule_spec->set_rule_id(10);
    rule_spec->mutable_action()->set_sec_action(nwsec::SecurityAction::SECURITY_RULE_ACTION_DENY);
    match = rule_spec->mutable_match();

    match->set_protocol(types::IPPROTO_TCP);
    dst_addr = match->add_dst_address();
    dst_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
    dst_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_v4_addr(0x0A000002);
    src_addr = match->add_src_address();
    src_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
    src_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_v4_addr(0x0A000001);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securitypolicy_update(pol_spec, &res);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    
    sleep(2);
    HAL_TRACE_DEBUG("hal sess handle: {}", sess_hdl);
    session = hal::find_session_by_handle(sess_hdl); 
    EXPECT_TRUE(session == NULL);
    session = hal::find_session_by_handle(skip_sess_hdl); 
    EXPECT_TRUE(session != NULL);

    tcp = Tins::TCP(100,101);
    ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, nwsec_policy_test::server_eph, nwsec_policy_test::client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    if (ctx_.session()) {
        sess_hdl = ctx_.session()->hal_handle;
    }
    
    // Delete policy
    pol_del_req.mutable_key_or_handle()->set_security_policy_handle(policy_handle);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securitypolicy_delete(pol_del_req, &pol_del_rsp);
    hal::hal_cfg_db_close();
    EXPECT_TRUE(ret == HAL_RET_OK);
    sleep(2);
    HAL_TRACE_DEBUG("hal sess handle: {}", sess_hdl);
    session = hal::find_session_by_handle(sess_hdl); 
    EXPECT_TRUE(session != NULL);
    session = hal::find_session_by_handle(skip_sess_hdl); 
    EXPECT_TRUE(session != NULL);
#if 0
    svc_reg(std::string("0.0.0.0:") + std::string("50054"), hal::HAL_FEATURE_SET_IRIS);
    hal::hal_wait();
#endif


    // There is a leak of HAL_SLAB_HANDLE_ID_LIST_ENTRY for adding
    //post = hal_test_utils_collect_slab_stats();
    //hal_test_utils_check_slab_leak(pre, post, &is_leak);
    //ASSERT_TRUE(is_leak == false);
}

//Verify policy reeval
TEST_F(nwsec_policy_test, test7) {
    hal_ret_t                               ret;
    SecurityPolicySpec                      pol_spec;
    SecurityPolicyResponse                  res;
    SecurityRule                           *rule_spec, rule_spec2;
    SecurityPolicyDeleteRequest             pol_del_req;
    SecurityPolicyDeleteResponse            pol_del_rsp;

    hal::vrf_t *vrf = hal::vrf_lookup_by_handle(nwsec_policy_test::vrfh);
    pol_spec.mutable_key_or_handle()->mutable_security_policy_key()->set_security_policy_id(11);
    pol_spec.mutable_key_or_handle()->mutable_security_policy_key()->mutable_vrf_id_or_handle()->set_vrf_id(vrf->vrf_id);
    rule_spec = pol_spec.add_rule();

    // Create nwsec
    rule_spec->set_rule_id(1);
    rule_spec->mutable_action()->set_sec_action(nwsec::SecurityAction::SECURITY_RULE_ACTION_ALLOW);
    types::RuleMatch *match = rule_spec->mutable_match();
    match->set_protocol(types::IPPROTO_TCP);

    types::IPAddressObj *dst_addr = match->add_dst_address();
    dst_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
    dst_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_v4_addr(0x0A000002);
    types::IPAddressObj *src_addr = match->add_src_address();
    src_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
    src_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_v4_addr(0x0A000001);


    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securitypolicy_create(pol_spec, &res);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t policy_handle = res.policy_status().key_or_handle().security_policy_handle();


    pol_spec.mutable_key_or_handle()->mutable_security_policy_key()->set_security_policy_id(10);
    pol_spec.mutable_key_or_handle()->mutable_security_policy_key()->mutable_vrf_id_or_handle()->set_vrf_id(vrf->vrf_id);
    rule_spec = pol_spec.add_rule();

    // Create nwsec
    rule_spec->set_rule_id(1);
    rule_spec->mutable_action()->set_sec_action(nwsec::SecurityAction::SECURITY_RULE_ACTION_ALLOW);
    match = rule_spec->mutable_match();
    match->set_protocol(types::IPPROTO_TCP);

    dst_addr = match->add_dst_address();
    dst_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
    dst_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_v4_addr(0x0A000002);
    src_addr = match->add_src_address();
    src_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
    src_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_v4_addr(0x0A000001);


    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securitypolicy_create(pol_spec, &res);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret != HAL_RET_OK);
    
    // Delete policy
    pol_del_req.mutable_key_or_handle()->set_security_policy_handle(policy_handle);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securitypolicy_delete(pol_del_req, &pol_del_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

#if 0
    svc_reg(std::string("0.0.0.0:") + std::string("50054"), hal::HAL_FEATURE_SET_IRIS);
    hal::hal_wait();
#endif


    // There is a leak of HAL_SLAB_HANDLE_ID_LIST_ENTRY for adding
    //post = hal_test_utils_collect_slab_stats();
    //hal_test_utils_check_slab_leak(pre, post, &is_leak);
    //ASSERT_TRUE(is_leak == false);
}

//Valid rules (0/32, 0/32,     NONE     allow) => 
// validate (0.0.0.0, 0.0.0.0, any allow)
// validate (0.0.0.0, <dst_ip>, any, deny)
// validate (<src_ip>, 0.0.0.0, any, deny)
// validate (<src_ip>, dst_ip,  any, deny)
TEST_F(nwsec_policy_test, test8) {
    hal_ret_t                               ret;
    SecurityPolicySpec                      pol_spec;
    SecurityPolicyResponse                  res;
    SecurityRule                           *rule_spec, rule_spec2;
    SecurityPolicyDeleteRequest             pol_del_req;
    SecurityPolicyDeleteResponse            pol_del_rsp;

    hal::vrf_t *vrf = hal::vrf_lookup_by_handle(nwsec_policy_test::vrfh);
    pol_spec.mutable_key_or_handle()->mutable_security_policy_key()->set_security_policy_id(11);
    pol_spec.mutable_key_or_handle()->mutable_security_policy_key()->mutable_vrf_id_or_handle()->set_vrf_id(vrf->vrf_id);
    rule_spec = pol_spec.add_rule();

    // Create nwsec
    rule_spec->set_rule_id(1);
    rule_spec->mutable_action()->set_sec_action(nwsec::SecurityAction::SECURITY_RULE_ACTION_ALLOW);
    types::RuleMatch *match = rule_spec->mutable_match();
    match->set_protocol(types::IPPROTO_NONE);

    //Rule:(0/32, 0/32, NONE)
    types::IPAddressObj *dst_addr = match->add_dst_address();
    dst_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
    dst_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_v4_addr(0x0);
    dst_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->set_prefix_len(32);

    types::IPAddressObj *src_addr = match->add_src_address();
    src_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
    src_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_v4_addr(0x0);
    src_addr->mutable_address()->mutable_prefix()->mutable_ipv4_subnet()->set_prefix_len(32);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securitypolicy_create(pol_spec, &res);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t policy_handle = res.policy_status().key_or_handle().security_policy_handle();

    /* (0,0, TCP) pkt rule: (0/32, 0/32, NONE) : ALLOW */ 
    ipv4_tuple v4_tuple = {};
    v4_tuple.ip_dst = 0x0; // server
    v4_tuple.ip_src = 0x0;
    v4_tuple.proto = types::IPPROTO_TCP;
    ipv4_rule_t *rule = NULL;
    nwsec_policy_t *res_policy;
    res_policy = find_nwsec_policy_by_key(11, vrf->vrf_id);
    const char *ctx_name = nwsec_acl_ctx_name(res_policy->key.vrf_id);
    const acl_ctx_t *acl_ctx = acl::acl_get(ctx_name);
    acl_classify(acl_ctx, (const uint8_t *)&v4_tuple, (const acl_rule_t **)&rule, 0x01);
    EXPECT_NE(rule, nullptr);

    v4_tuple.ip_dst = 0xAABBCCDD;
    v4_tuple.ip_src = 0x0;
    v4_tuple.proto = types::IPPROTO_TCP;
    acl_classify(acl_ctx, (const uint8_t *)&v4_tuple, (const acl_rule_t **)&rule, 0x01);
    EXPECT_EQ(rule, nullptr);


    v4_tuple.ip_dst = 0x0;
    v4_tuple.ip_src = 0x01020304;
    v4_tuple.proto = types::IPPROTO_TCP;
    acl_classify(acl_ctx, (const uint8_t *)&v4_tuple, (const acl_rule_t **)&rule, 0x01);
    EXPECT_EQ(rule, nullptr);


    v4_tuple.ip_dst = 0xAABBCCDD;
    v4_tuple.ip_src = 0x01020304;
    v4_tuple.proto = types::IPPROTO_TCP;
    acl_classify(acl_ctx, (const uint8_t *)&v4_tuple, (const acl_rule_t **)&rule, 0x01);
    EXPECT_EQ(rule, nullptr);
    acl::acl_deref(acl_ctx);


    // Delete policy
    pol_del_req.mutable_key_or_handle()->set_security_policy_handle(policy_handle);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securitypolicy_delete(pol_del_req, &pol_del_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

#if 0
    svc_reg(std::string("0.0.0.0:") + std::string("50054"), hal::HAL_FEATURE_SET_IRIS);
    hal::hal_wait();
#endif


    // There is a leak of HAL_SLAB_HANDLE_ID_LIST_ENTRY for adding
    //post = hal_test_utils_collect_slab_stats();
    //hal_test_utils_check_slab_leak(pre, post, &is_leak);
    //ASSERT_TRUE(is_leak == false);
}


//Valid rules ( any, any,      TCP     allow) => 
// validate (0.0.0.0, 0.0.0.0, TCP allow)
// validate (0.0.0.0, <dst_ip>, TCP, allow)
// validate (<src_ip>, 0.0.0.0, TCP, allow)
// validate (0.0.0.0, 0.0.0.0, ICMP  deny)
// validate (0.0.0.0, <dst_ip>, ICMP, deny)
// validate (<src_ip>, 0.0.0.0, ICMP, deny)
TEST_F(nwsec_policy_test, test9) {
    hal_ret_t                               ret;
    SecurityPolicySpec                      pol_spec;
    SecurityPolicyResponse                  res;
    SecurityRule                           *rule_spec, rule_spec2;
    SecurityPolicyDeleteRequest             pol_del_req;
    SecurityPolicyDeleteResponse            pol_del_rsp;

    hal::vrf_t *vrf = hal::vrf_lookup_by_handle(nwsec_policy_test::vrfh);
    pol_spec.mutable_key_or_handle()->mutable_security_policy_key()->set_security_policy_id(11);
    pol_spec.mutable_key_or_handle()->mutable_security_policy_key()->mutable_vrf_id_or_handle()->set_vrf_id(vrf->vrf_id);
    rule_spec = pol_spec.add_rule();

    // Create nwsec
    rule_spec->set_rule_id(1);
    rule_spec->mutable_action()->set_sec_action(nwsec::SecurityAction::SECURITY_RULE_ACTION_ALLOW);
    types::RuleMatch *match = rule_spec->mutable_match();
    match->set_protocol(types::IPPROTO_TCP);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securitypolicy_create(pol_spec, &res);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t policy_handle = res.policy_status().key_or_handle().security_policy_handle();

    /* (0,0, TCP) pkt ALLOW */ 
    ipv4_tuple v4_tuple = {};
    v4_tuple.ip_dst = 0x0; // server
    v4_tuple.ip_src = 0x0;
    v4_tuple.proto = types::IPPROTO_TCP;
    ipv4_rule_t *rule = NULL;
    nwsec_policy_t *res_policy;
    res_policy = find_nwsec_policy_by_key(11, vrf->vrf_id);
    const char *ctx_name = nwsec_acl_ctx_name(res_policy->key.vrf_id);
    const acl_ctx_t *acl_ctx = acl::acl_get(ctx_name);
    acl_classify(acl_ctx, (const uint8_t *)&v4_tuple, (const acl_rule_t **)&rule, 0x01);
    EXPECT_NE(rule, nullptr);

    /*  (0,dip, TCP) pkt ALLOW */
    v4_tuple.ip_dst = 0xAABBCCDD;
    v4_tuple.ip_src = 0x0;
    v4_tuple.proto = types::IPPROTO_TCP;
    acl_classify(acl_ctx, (const uint8_t *)&v4_tuple, (const acl_rule_t **)&rule, 0x01);
    EXPECT_NE(rule, nullptr);

    /*  (sip,0, TCP) pkt ALLOW */ 
    v4_tuple.ip_dst = 0x0;
    v4_tuple.ip_src = 0x01020304;
    v4_tuple.proto = types::IPPROTO_TCP;
    acl_classify(acl_ctx, (const uint8_t *)&v4_tuple, (const acl_rule_t **)&rule, 0x01);
    EXPECT_NE(rule, nullptr);

    /*  (sip,dip, TCP) pkt ALLOW */ 
    v4_tuple.ip_dst = 0xAABBCCDD;
    v4_tuple.ip_src = 0x01020304;
    v4_tuple.proto = types::IPPROTO_TCP;
    acl_classify(acl_ctx, (const uint8_t *)&v4_tuple, (const acl_rule_t **)&rule, 0x01);
    EXPECT_NE(rule, nullptr);


    /*  (sip,dip, TCP) pkt ALLOW */ 
    v4_tuple.ip_dst = 0xAABBCCDD;
    v4_tuple.ip_src = 0x01020304;
    v4_tuple.proto = types::IPPROTO_ICMP;
    acl_classify(acl_ctx, (const uint8_t *)&v4_tuple, (const acl_rule_t **)&rule, 0x01);
    EXPECT_EQ(rule, nullptr);

    /*  (0,dip, ICMP) pkt DENY */
    v4_tuple.ip_dst = 0xAABBCCDD;
    v4_tuple.ip_src = 0x0;
    v4_tuple.proto = types::IPPROTO_ICMP;
    acl_classify(acl_ctx, (const uint8_t *)&v4_tuple, (const acl_rule_t **)&rule, 0x01);
    EXPECT_EQ(rule, nullptr);

    /*  (sip,0, ICMP) pkt DENY */ 
    v4_tuple.ip_dst = 0x0;
    v4_tuple.ip_src = 0x01020304;
    v4_tuple.proto = types::IPPROTO_ICMP;
    acl_classify(acl_ctx, (const uint8_t *)&v4_tuple, (const acl_rule_t **)&rule, 0x01);
    EXPECT_EQ(rule, nullptr);

    /*  (sip,dip, ICMP) pkt DENY*/ 
    v4_tuple.ip_dst = 0xAABBCCDD;
    v4_tuple.ip_src = 0x01020304;
    v4_tuple.proto = types::IPPROTO_ICMP;
    acl_classify(acl_ctx, (const uint8_t *)&v4_tuple, (const acl_rule_t **)&rule, 0x01);
    EXPECT_EQ(rule, nullptr);
    acl::acl_deref(acl_ctx);


    // Delete policy
    pol_del_req.mutable_key_or_handle()->set_security_policy_handle(policy_handle);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securitypolicy_delete(pol_del_req, &pol_del_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

#if 0
    svc_reg(std::string("0.0.0.0:") + std::string("50054"), hal::HAL_FEATURE_SET_IRIS);
    hal::hal_wait();
#endif


    // There is a leak of HAL_SLAB_HANDLE_ID_LIST_ENTRY for adding
    //post = hal_test_utils_collect_slab_stats();
    //hal_test_utils_check_slab_leak(pre, post, &is_leak);
    //ASSERT_TRUE(is_leak == false);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
