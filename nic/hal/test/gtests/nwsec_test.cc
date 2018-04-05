#include "nic/hal/src/nw/interface.hpp"
#include "nic/hal/src/nw/nw.hpp"
#include "nic/hal/src/firewall/nwsec.hpp"
#include "nic/hal/hal.hpp"
#include "sdk/list.hpp"
#include "nic/gen/proto/hal/interface.pb.h"
#include "nic/gen/proto/hal/l2segment.pb.h"
#include "nic/gen/proto/hal/vrf.pb.h"
#include "nic/gen/proto/hal/nwsec.pb.h"
#include "nic/hal/hal.hpp"
#include "nic/gen/proto/hal/types.pb.h"
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include "nic/hal/test/utils/hal_test_utils.hpp"
#include "nic/hal/test/utils/hal_base_test.hpp"
#include "nic/fte/fte_ctx.hpp"
#include "nic/hal/src/firewall/nwsec_group.hpp"
#include "nic/hal/plugins/sfw/core.hpp"
#include "nic/hal/plugins/app_redir/app_redir_ctx.hpp"

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
using nw::NetworkSpec;
using nw::NetworkResponse;
using types::IPProtocol;
using namespace hal;
using namespace hal::app_redir;
using namespace fte;
using namespace nwsec;


class nwsec_test : public hal_base_test {
protected:
  nwsec_test() {
  }

  virtual ~nwsec_test() {
  }

  // will be called immediately after the constructor before each test
  virtual void SetUp() {
  }

  // will be called immediately after each test before the destructor
  virtual void TearDown() {
  }

  // Will be called at the beginning of all test cases in this class
  static void SetUpTestCase() {
    hal_base_test::SetUpTestCase();
    hal_test_utils_slab_disable_delete();
  }

};

// ----------------------------------------------------------------------------
// Create nwsec profile
// Update
// Delete
// ----------------------------------------------------------------------------
TEST_F(nwsec_test, test1)
{
    hal_ret_t                               ret;
    SecurityProfileSpec                     sp_spec, sp_spec1;
    SecurityProfileResponse                 sp_rsp, sp_rsp1;
    SecurityProfileDeleteRequest            del_req;
    SecurityProfileDeleteResponse           del_rsp;
    slab_stats_t                            *pre = NULL, *post = NULL;
    bool                                    is_leak = false;

    pre = hal_test_utils_collect_slab_stats();

    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(1);
    sp_spec.set_ipsg_en(true);
    sp_spec.set_ip_normalization_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();

    // Update nwsec without id or handle
    // sp_spec.mutable_key_or_handle()->set_profile_id(1);
    sp_spec1.set_ipsg_en(true);
    sp_spec1.set_ip_normalization_en(false);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_update(sp_spec1, &sp_rsp1);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_INVALID_ARG);

    // Update nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(1);
    sp_spec.set_ipsg_en(true);
    sp_spec.set_ip_normalization_en(false);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_update(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Update nwsec
    sp_spec.mutable_key_or_handle()->set_profile_handle(nwsec_hdl);
    sp_spec.set_ipsg_en(false);
    sp_spec.set_ip_normalization_en(false);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_update(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);


    // Delete nwsec
    del_req.mutable_key_or_handle()->set_profile_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // There is a leak of HAL_SLAB_HANDLE_ID_LIST_ENTRY for adding
    post = hal_test_utils_collect_slab_stats();
    hal_test_utils_check_slab_leak(pre, post, &is_leak);
    ASSERT_TRUE(is_leak == false);
}


// ----------------------------------------------------------------------------
// Create a SecurityGroupPolicySpec
// ----------------------------------------------------------------------------
TEST_F(nwsec_test, test2)
{
    hal_ret_t                                   ret;
    SecurityGroupSpec                           sg_spec;
    SecurityGroupResponse                       sg_rsp;
    SecurityGroupPolicySpec                     sp_spec;
    SecurityGroupPolicyResponse                 sp_rsp;
    SecurityGroupPolicyDeleteRequest            del_req;
    SecurityGroupPolicyDeleteResponseMsg        del_rsp;
    //slab_stats_t                               *pre = NULL, *post = NULL;
    //bool                                        is_leak = false;

    //pre = hal_test_utils_collect_slab_stats();

    // Create SecurityGroupPolicySpec
    sp_spec.mutable_key_or_handle()->mutable_security_group_policy_id()->set_security_group_id(1);
    sp_spec.mutable_key_or_handle()->mutable_security_group_policy_id()->set_peer_security_group_id(2);

    nwsec::FirewallRuleSpec *fw_rule = sp_spec.mutable_policy_rules()->add_in_fw_rules();
    Service *svc =  fw_rule->add_svc();
    svc->set_ip_protocol(IPProtocol::IPPROTO_IPV4);
    svc->set_dst_port(1000);
    svc->set_alg(ALGName::APP_SVC_TFTP);

    /*fw_rule = sp_spec.mutable_egress_policy()->add_fw_rules();
    svc = fw_rule->add_svc();
    svc->set_ip_protocol(IPProtocol::IPPROTO_IPV4);
    svc->set_dst_port(2000);
    svc->set_alg(ALGName::APP_SVC_TFTP);*/

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securitygrouppolicy_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Update nwsec
    sp_spec.mutable_key_or_handle()->mutable_security_group_policy_id()->set_security_group_id(1);
    sp_spec.mutable_key_or_handle()->mutable_security_group_policy_id()->set_peer_security_group_id(2);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securitygrouppolicy_update(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);


    // There is a leak of HAL_SLAB_HANDLE_ID_LIST_ENTRY for adding
    //post = hal_test_utils_collect_slab_stats();
    //hal_test_utils_check_slab_leak(pre, post, &is_leak);
    //ASSERT_TRUE(is_leak == false);
}

TEST_F(nwsec_test, test3)
{
    hal_ret_t                               ret;
    SecurityGroupSpec                       sp_spec;
    SecurityGroupResponse                   sp_rsp;

    dllist_ctxt_t                           *curr, *next, *nw_list, *ep_list;
    hal_handle_id_list_entry_t              *nw_ent = NULL, *ep_ent = NULL;
    //slab_stats_t                            *pre = NULL, *post = NULL;
    //bool                                    is_leak = false;

    //pre = hal_test_utils_collect_slab_stats();

    // Create SecurityGroupSpec
    sp_spec.mutable_key_or_handle()->set_security_group_id(1);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securitygroup_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

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
    ret = del_nw_from_security_group(1, 0x1002);
    ASSERT_TRUE(ret == HAL_RET_OK);

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
    ret = del_ep_from_security_group(1, 0x2002);
    ASSERT_TRUE(ret == HAL_RET_OK);

    ep_list = get_ep_list_for_security_group(1);
    if (ep_list != NULL) {
        dllist_for_each_safe(curr, next, ep_list) {
            ep_ent = dllist_entry(curr, hal_handle_id_list_entry_t, dllist_ctxt);
            HAL_TRACE_DEBUG("ep handle {}", ep_ent->handle_id);
        }
    }

    // There is a leak of HAL_SLAB_HANDLE_ID_LIST_ENTRY for adding
    //post = hal_test_utils_collect_slab_stats();
    //hal_test_utils_check_slab_leak(pre, post, &is_leak);
    //ASSERT_TRUE(is_leak == false);
}

TEST_F(nwsec_test, test4)
{
    hal_ret_t                               ret;
    SecurityProfileSpec                     sp_spec;
    SecurityProfileResponse                 sp_rsp;
    SecurityProfileDeleteRequest            del_req;
    SecurityProfileDeleteResponse           del_rsp;
    slab_stats_t                            *pre = NULL, *post = NULL;
    bool                                    is_leak = false;

    pre = hal_test_utils_collect_slab_stats();

    // Create nwsec with no profile id
    // sp_spec.mutable_key_or_handle()->set_profile_id(1);
    sp_spec.set_ipsg_en(true);
    sp_spec.set_ip_normalization_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_INVALID_ARG);

    // Create nwsec with no profile id but with handle
    sp_spec.mutable_key_or_handle()->set_profile_handle(1);
    sp_spec.set_ipsg_en(true);
    sp_spec.set_ip_normalization_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_NWSEC_ID_INVALID);


    // Create 256 nwsecs
    for (int i = 0; i < 256; i++) {
        sp_spec.mutable_key_or_handle()->set_profile_id(i);
        sp_spec.set_ipsg_en(true);
        sp_spec.set_ip_normalization_en(true);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::securityprofile_create(sp_spec, &sp_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK || ret == HAL_RET_NO_RESOURCE || HAL_RET_ENTRY_EXISTS);
    }

#if 0
    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(1);
    sp_spec.set_ipsg_en(true);
    sp_spec.set_ip_normalization_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();

    // Update nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(1);
    sp_spec.set_ipsg_en(true);
    sp_spec.set_ip_normalization_en(false);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_update(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Update nwsec
    sp_spec.mutable_key_or_handle()->set_profile_handle(nwsec_hdl);
    sp_spec.set_ipsg_en(false);
    sp_spec.set_ip_normalization_en(false);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_update(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
#endif


    for ( int i = 0; i < 256; i++) {
        // Delete nwsec
        del_req.mutable_key_or_handle()->set_profile_id(i);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::securityprofile_delete(del_req, &del_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK || ret == HAL_RET_SECURITY_PROFILE_NOT_FOUND);
    }

    // There is a leak of HAL_SLAB_HANDLE_ID_LIST_ENTRY for adding
    post = hal_test_utils_collect_slab_stats();
    hal_test_utils_check_slab_leak(pre, post, &is_leak);
    ASSERT_TRUE(is_leak == false);
}

namespace hal {
namespace plugins {
namespace sfw {
extern hal_ret_t
net_sfw_match_rules(fte::ctx_t& ctx,
                    hal::nwsec_policy_rules_t *rules,
                    hal::plugins::sfw::net_sfw_match_result_t *rslt);
extern hal_ret_t
net_sfw_check_policy_pair(fte::ctx_t                    &ctx,
                          uint32_t                      src_sg,
                          uint32_t                      dst_sg,
                          net_sfw_match_result_t   *match_rslt);
}
}
}

// test ctx with access to protected members
class test_ctx_t :  public ctx_t {
public:
    using ctx_t::init;

};
// Test to validate the appid logic in firewall.cc
TEST_F(nwsec_test, test5)
{
    hal_ret_t ret;
    test_ctx_t ctx1 = {};
    hal::nwsec_policy_rules_t rules;
    hal::plugins::sfw::net_sfw_match_result_t rslt;

    nwsec_policy_appid_t* nwsec_plcy_appid = NULL;
    nwsec_plcy_appid = nwsec_policy_appid_alloc_and_init();
    if (nwsec_plcy_appid == NULL) ASSERT_TRUE(0);

    nwsec_plcy_appid->appid = 747;

    fte::feature_info_t info = {};

    info.state_size = sizeof(app_redir_ctx_t);
    auto fn1 = [](fte::ctx_t& ctx) {
        return fte::PIPELINE_CONTINUE;
    };
    fte::add_feature(FTE_FEATURE_APP_REDIR_APPID);
    fte::register_feature(FTE_FEATURE_APP_REDIR_APPID, fn1, info);
    uint16_t num_features = 1;
    size_t sz = fte::feature_state_size(&num_features);
    fte::feature_state_t *st = (fte::feature_state_t *)HAL_CALLOC(hal::HAL_MEM_ALLOC_FTE, sz);
    ctx1.init({2,1,1}, st, num_features);
    nwsec_policy_svc_t* nwsec_plcy_svc = nwsec_policy_svc_alloc_and_init();
    if (nwsec_plcy_svc == NULL) ASSERT_TRUE(0);
    nwsec_plcy_svc->ipproto = types::IPPROTO_NONE;
    nwsec_plcy_svc->dst_port = 0;

    dllist_add_tail(&rules.fw_svc_list_head,
                    &nwsec_plcy_svc->lentry);

    //To Do: Check to Get lock on nwsec_plcy_rules ??
    dllist_add_tail(&rules.appid_list_head,
                    &nwsec_plcy_appid->lentry);

    ret = hal::plugins::sfw::net_sfw_match_rules(ctx1, &rules, &rslt);
    ASSERT_TRUE(ret == HAL_RET_OK);
    ASSERT_TRUE(app_redir_ctx(ctx1, false)->appid_needed());
}

TEST_F(nwsec_test, test6)
{
    hal_ret_t ret;
    SecurityGroupPolicySpec sp_spec;
    SecurityGroupPolicyResponse sp_rsp;
    test_ctx_t ctx = {};
    hal::nwsec_policy_rules_t rules;
    hal::plugins::sfw::net_sfw_match_result_t rslt;

    // Create SecurityGroupPolicySpec
    sp_spec.mutable_key_or_handle()->mutable_security_group_policy_id()->set_security_group_id(100);
    sp_spec.mutable_key_or_handle()->mutable_security_group_policy_id()->set_peer_security_group_id(102);

    nwsec::FirewallRuleSpec *fw_rule = sp_spec.mutable_policy_rules()->add_in_fw_rules();
    Service *svc =  fw_rule->add_svc();
    svc->set_ip_protocol(types::IPPROTO_NONE);
    svc->set_dst_port(0);
    fw_rule->add_apps("MYSQL");

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securitygrouppolicy_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    ctx = {};
    fte::feature_info_t info = {};

    info.state_size = sizeof(app_redir_ctx_t);
    auto fn1 = [](fte::ctx_t& ctx) {
        return fte::PIPELINE_CONTINUE;
    };
    fte::add_feature(FTE_FEATURE_APP_REDIR_APPID);
    fte::register_feature(FTE_FEATURE_APP_REDIR_APPID, fn1, info);
    uint16_t num_features = 1;
    size_t sz = fte::feature_state_size(&num_features);
    fte::feature_state_t *st = (fte::feature_state_t *)HAL_CALLOC(hal::HAL_MEM_ALLOC_FTE, sz);
    ctx.init({2,1,1}, st, num_features);

    ret = net_sfw_check_policy_pair(ctx, 100, 102, &rslt);
    ASSERT_TRUE(ret == HAL_RET_OK);
    ASSERT_TRUE(app_redir_ctx(ctx, false)->appid_needed());
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
