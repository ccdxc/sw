#include "nic/hal/plugins/cfg/aclqos/qos.hpp"
#include "gen/proto/qos.pb.h"
#include "nic/hal/hal.hpp"
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nic/hal/test/utils/hal_base_test.hpp"
#include "nic/hal/test/utils/hal_test_utils.hpp"

using qos::QosClassSpec;
using qos::QosClassResponse;
using kh::QosClassKeyHandle;
using qos::QosClassDeleteRequest;
using qos::QosClassDeleteResponse;

class qos_class_test : public hal_base_test {
protected:
  qos_class_test() {
  }

  virtual ~qos_class_test() {
  }

  // will be called immediately after the constructor before each test
  virtual void SetUp() {
  }

  // will be called immediately after each test before the destructor
  virtual void TearDown() {
  }
  
  // Will be called at the beginning of all test cases
  static void SetUpTestCase() {
      hal_base_test::SetUpTestCase();
      hal_test_utils_slab_disable_delete();
  }

};

static bool
verify_mtu (const QosClassSpec *spec, uint32_t mtu)
{
    if (spec->mtu() == mtu) {
        return true;
    }
    return false;
}

static bool
verify_dwrr(const QosClassSpec *spec, uint32_t dwrr)
{
    if (spec->sched().dwrr().bw_percentage() == dwrr) {
        return true;
    }
    return false;
}

bool
verify_bps (const QosClassSpec *spec, uint32_t bps)
{
    if (spec->sched().strict().bps() == bps) {
        return true;
    }
    return false;
}

static bool
verify_pcp (const QosClassSpec *spec, uint32_t pcp)
{
    if (spec->class_map().dot1q_pcp() == pcp) {
        return true;
    }
    return false;
}

static bool
verify_ip_dscp (const QosClassSpec *spec, uint32_t dscp)
{
    for (uint32_t i = 0; i < spec->class_map().ip_dscp_size(); ++i) {
        if (spec->class_map().ip_dscp(i) == dscp) {
            return true;
        }
    }
    return false;
}

static bool
verify_pause (const QosClassSpec *spec, qos::QosPauseType pause_type)
{
    //if (spec->has_pause()) {
    if (spec->no_drop()) {
        if (spec->pause().type() == pause_type) {
            return true;
        }
    } else {
        if (pause_type == qos::QOS_PAUSE_TYPE_NONE) {
            return true;
        }
    }
    return false;
}

static bool
verify_pause_pfc_cos (const QosClassSpec *spec, uint32_t pfc_cos)
{
    //if (spec->has_pause()) {
    if (spec->no_drop()) {
        if (spec->pause().pfc_cos() == pfc_cos) {
            return true;
        }
    }
    return false;
}

// Creating user-defined qos_classs
// create user-defined-1 with pcp=3
// update user-defined-1 MTU
// create user-defined-1 with pcp=3 (qos class already exists)
// create user-defined-2 with pcp=10 (invalid pcp)
// create user-defined-2 with pcp=3 (pcp already exists)
// create user-defined-2 with dscp=64 (invalid dscp)
// create user-defined-2 with dscp=63
// update user-defined-2 with dscp=62,63 (add one more dscp)
// update user-defined-2 with dscp=60 (change both dscp)
// create user-defined-3 with PFC (invalid since global pause type
//                                 is link-level)
// change global pause type to PFC
// update default class with no-pause
// create user-defined-3 with PFC
// delete user-defined-1
// delete user-defined-1 (not found)
// delete default class (default class delete not allowed)
TEST_F(qos_class_test, test1)
{
    hal_ret_t        ret;
    QosClassSpec     spec;
    const QosClassSpec *get_spec;
    QosClassResponse rsp;
    QosClassGetRequest get_req;
    QosClassGetResponseMsg get_rsp;
    qos::QosClassSetGlobalPauseTypeRequest global_pause_req;
    qos::QosClassSetGlobalPauseTypeResponseMsg global_pause_rsp;
    QosClassDeleteRequest del_req;
    QosClassDeleteResponse del_resp;

    // create user-defined-1
    //     MTU: 9216
    //     Pause: link-level
    //     Sched: dwrr: 20
    //     ClassMap: PCP: 3
    //     Marking: pcp_rewrite_en: true
    spec.Clear();
    rsp.Clear();
    spec.mutable_key_or_handle()->set_qos_group(kh::USER_DEFINED_1);
    spec.set_mtu(9216);
    spec.mutable_pause()->set_type(qos::QOS_PAUSE_TYPE_LINK_LEVEL);
    spec.mutable_sched()->mutable_dwrr()->set_bw_percentage(20);
    spec.mutable_class_map()->set_type(qos::QOS_CLASS_MAP_TYPE_PCP);
    spec.mutable_class_map()->set_dot1q_pcp(3);
    spec.mutable_marking()->set_dot1q_pcp_rewrite_en(true);
    spec.set_no_drop(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::qosclass_create(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_EQ(ret, HAL_RET_OK);

    // get user-defined-1
    get_req.Clear();
    get_rsp.Clear();
    get_req.mutable_key_or_handle()->set_qos_group(kh::USER_DEFINED_1);
    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    ret = hal::qosclass_get(get_req, &get_rsp);
    hal::hal_cfg_db_close();
    ASSERT_EQ(ret, HAL_RET_OK);

    // verify user-defined-1
    get_spec = &get_rsp.response(0).spec();
    ASSERT_EQ(verify_mtu(get_spec, 9216), true);
    ASSERT_EQ(verify_pause(get_spec, qos::QOS_PAUSE_TYPE_LINK_LEVEL), true);
    ASSERT_EQ(verify_dwrr(get_spec, 20), true);
    ASSERT_EQ(verify_pcp(get_spec, 3), true);

    // update user-defined-1
    //     MTU: 1500
    //     Pause: link-level
    //     Sched: dwrr: 20
    //     ClassMap: PCP: 3
    //     Marking: pcp_rewrite_en: true
    spec.Clear();
    rsp.Clear();
    spec.mutable_key_or_handle()->set_qos_group(kh::USER_DEFINED_1);
    spec.set_mtu(1500);
    spec.mutable_pause()->set_type(qos::QOS_PAUSE_TYPE_LINK_LEVEL);
    spec.mutable_sched()->mutable_dwrr()->set_bw_percentage(20);
    spec.mutable_class_map()->set_type(qos::QOS_CLASS_MAP_TYPE_PCP);
    spec.mutable_class_map()->set_dot1q_pcp(3);
    spec.mutable_marking()->set_dot1q_pcp_rewrite_en(true);
    spec.set_no_drop(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::qosclass_update(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_EQ(ret, HAL_RET_OK);

    // get user-defined-1
    get_req.Clear();
    get_rsp.Clear();
    get_req.mutable_key_or_handle()->set_qos_group(kh::USER_DEFINED_1);
    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    ret = hal::qosclass_get(get_req, &get_rsp);
    hal::hal_cfg_db_close();
    ASSERT_EQ(ret, HAL_RET_OK);

    // verify user-defined-1
    get_spec = &get_rsp.response(0).spec();
    ASSERT_EQ(verify_mtu(get_spec, 1500), true);
    ASSERT_EQ(verify_pause(get_spec, qos::QOS_PAUSE_TYPE_LINK_LEVEL), true);
    ASSERT_EQ(verify_dwrr(get_spec, 20), true);
    ASSERT_EQ(verify_pcp(get_spec, 3), true);

    // create user-defined-1 (Already exists)
    //     MTU: 9216
    //     Pause: link-level
    //     Sched: dwrr: 20
    //     ClassMap: PCP: 3
    //     Marking: pcp_rewrite_en: true
    spec.Clear();
    rsp.Clear();
    spec.mutable_key_or_handle()->set_qos_group(kh::USER_DEFINED_1);
    spec.set_mtu(9216);
    spec.mutable_pause()->set_type(qos::QOS_PAUSE_TYPE_LINK_LEVEL);
    spec.mutable_sched()->mutable_dwrr()->set_bw_percentage(20);
    spec.mutable_class_map()->set_type(qos::QOS_CLASS_MAP_TYPE_PCP);
    spec.mutable_class_map()->set_dot1q_pcp(3);
    spec.mutable_marking()->set_dot1q_pcp_rewrite_en(true);
    spec.set_no_drop(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::qosclass_create(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_NE(ret, HAL_RET_OK);
    ASSERT_EQ(rsp.api_status(), types::ApiStatus::API_STATUS_EXISTS_ALREADY);

    // create user-defined-2
    //     MTU: 9216
    //     Pause: link-level
    //     Sched: dwrr: 20
    //     ClassMap: PCP: 10 (Invalid)
    //     Marking: pcp_rewrite_en: true
    spec.Clear();
    rsp.Clear();
    spec.mutable_key_or_handle()->set_qos_group(kh::USER_DEFINED_2);
    spec.set_mtu(9216);
    spec.mutable_pause()->set_type(qos::QOS_PAUSE_TYPE_LINK_LEVEL);
    spec.mutable_sched()->mutable_dwrr()->set_bw_percentage(20);
    spec.mutable_class_map()->set_type(qos::QOS_CLASS_MAP_TYPE_PCP);
    spec.mutable_class_map()->set_dot1q_pcp(10);
    spec.mutable_marking()->set_dot1q_pcp_rewrite_en(true);
    spec.set_no_drop(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::qosclass_create(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_NE(ret, HAL_RET_OK);
    ASSERT_EQ(rsp.api_status(),
              types::ApiStatus::API_STATUS_QOS_CLASS_DOT1Q_PCP_INVALID);

    // create user-defined-2
    //     MTU: 9216
    //     Pause: link-level
    //     Sched: dwrr: 20
    //     ClassMap: PCP: 3 (Already in use)
    //     Marking: pcp_rewrite_en: true
    spec.Clear();
    rsp.Clear();
    spec.mutable_key_or_handle()->set_qos_group(kh::USER_DEFINED_2);
    spec.set_mtu(9216);
    spec.mutable_pause()->set_type(qos::QOS_PAUSE_TYPE_LINK_LEVEL);
    spec.mutable_sched()->mutable_dwrr()->set_bw_percentage(20);
    spec.mutable_class_map()->set_type(qos::QOS_CLASS_MAP_TYPE_PCP);
    spec.mutable_class_map()->set_dot1q_pcp(3);
    spec.mutable_marking()->set_dot1q_pcp_rewrite_en(true);
    spec.set_no_drop(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::qosclass_create(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_NE(ret, HAL_RET_OK);
    ASSERT_EQ(rsp.api_status(),
              types::ApiStatus::API_STATUS_QOS_CLASS_DOT1Q_PCP_ALREADY_IN_USE);

    // create user-defined-2
    //     MTU: 9216
    //     Pause: link-level
    //     Sched: dwrr: 20
    //     ClassMap: DSCP: 64 (Invalid DSCP)
    //     Marking: pcp_rewrite_en: true
    spec.Clear();
    rsp.Clear();
    spec.mutable_key_or_handle()->set_qos_group(kh::USER_DEFINED_2);
    spec.set_mtu(9216);
    spec.mutable_pause()->set_type(qos::QOS_PAUSE_TYPE_LINK_LEVEL);
    spec.mutable_sched()->mutable_dwrr()->set_bw_percentage(20);
    spec.mutable_class_map()->set_type(qos::QOS_CLASS_MAP_TYPE_DSCP);
    spec.mutable_class_map()->add_ip_dscp(64);
    spec.mutable_marking()->set_dot1q_pcp_rewrite_en(true);
    spec.set_no_drop(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::qosclass_create(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_NE(ret, HAL_RET_OK);
    ASSERT_EQ(rsp.api_status(),
              types::ApiStatus::API_STATUS_QOS_CLASS_IP_DSCP_INVALID);

    // create user-defined-2
    //     MTU: 9216
    //     Pause: link-level
    //     Sched: dwrr: 20
    //     ClassMap: DSCP: 63
    //     Marking: pcp_rewrite_en: true
    spec.Clear();
    rsp.Clear();
    spec.mutable_key_or_handle()->set_qos_group(kh::USER_DEFINED_2);
    spec.set_mtu(9216);
    spec.mutable_pause()->set_type(qos::QOS_PAUSE_TYPE_LINK_LEVEL);
    spec.mutable_sched()->mutable_dwrr()->set_bw_percentage(20);
    spec.mutable_class_map()->set_type(qos::QOS_CLASS_MAP_TYPE_DSCP);
    spec.mutable_class_map()->add_ip_dscp(63);
    spec.mutable_marking()->set_dot1q_pcp_rewrite_en(true);
    spec.set_no_drop(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::qosclass_create(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_EQ(ret, HAL_RET_OK);

    // get user-defined-2
    get_req.Clear();
    get_rsp.Clear();
    get_req.mutable_key_or_handle()->set_qos_group(kh::USER_DEFINED_2);
    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    ret = hal::qosclass_get(get_req, &get_rsp);
    hal::hal_cfg_db_close();
    ASSERT_EQ(ret, HAL_RET_OK);

    // verify user-defined-2
    get_spec = &get_rsp.response(0).spec();
    ASSERT_EQ(verify_mtu(get_spec, 9216), true);
    ASSERT_EQ(verify_pause(get_spec, qos::QOS_PAUSE_TYPE_LINK_LEVEL), true);
    ASSERT_EQ(verify_dwrr(get_spec, 20), true);
    ASSERT_EQ(verify_ip_dscp(get_spec, 63), true);

    // update user-defined-2 (invalid dscp)
    //     MTU: 9216
    //     Pause: link-level
    //     Sched: dwrr: 20
    //     ClassMap: DSCP: 65 (Invalid)
    //     Marking: pcp_rewrite_en: true
    spec.Clear();
    rsp.Clear();
    spec.mutable_key_or_handle()->set_qos_group(kh::USER_DEFINED_2);
    spec.set_mtu(9216);
    spec.mutable_pause()->set_type(qos::QOS_PAUSE_TYPE_LINK_LEVEL);
    spec.mutable_sched()->mutable_dwrr()->set_bw_percentage(20);
    spec.mutable_class_map()->set_type(qos::QOS_CLASS_MAP_TYPE_DSCP);
    spec.mutable_class_map()->add_ip_dscp(65);
    spec.mutable_marking()->set_dot1q_pcp_rewrite_en(true);
    spec.set_no_drop(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::qosclass_update(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_NE(ret, HAL_RET_OK);
    ASSERT_EQ(rsp.api_status(),
              types::ApiStatus::API_STATUS_QOS_CLASS_IP_DSCP_INVALID);

    // update user-defined-2
    //     MTU: 9216
    //     Pause: link-level
    //     Sched: dwrr: 20
    //     ClassMap: DSCP: 62,63 (add one more DSCP value)
    //     Marking: pcp_rewrite_en: true
    spec.Clear();
    rsp.Clear();
    spec.mutable_key_or_handle()->set_qos_group(kh::USER_DEFINED_2);
    spec.set_mtu(9216);
    spec.mutable_pause()->set_type(qos::QOS_PAUSE_TYPE_LINK_LEVEL);
    spec.mutable_sched()->mutable_dwrr()->set_bw_percentage(20);
    spec.mutable_class_map()->set_type(qos::QOS_CLASS_MAP_TYPE_DSCP);
    spec.mutable_class_map()->add_ip_dscp(62);
    spec.mutable_class_map()->add_ip_dscp(63);
    spec.mutable_marking()->set_dot1q_pcp_rewrite_en(true);
    spec.set_no_drop(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::qosclass_update(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_EQ(ret, HAL_RET_OK);

    // get user-defined-2
    get_req.Clear();
    get_rsp.Clear();
    get_req.mutable_key_or_handle()->set_qos_group(kh::USER_DEFINED_2);
    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    ret = hal::qosclass_get(get_req, &get_rsp);
    hal::hal_cfg_db_close();
    ASSERT_EQ(ret, HAL_RET_OK);

    // verify user-defined-2
    get_spec = &get_rsp.response(0).spec();
    ASSERT_EQ(verify_mtu(get_spec, 9216), true);
    ASSERT_EQ(verify_pause(get_spec, qos::QOS_PAUSE_TYPE_LINK_LEVEL), true);
    ASSERT_EQ(verify_dwrr(get_spec, 20), true);
    ASSERT_EQ(verify_ip_dscp(get_spec, 62), true);
    ASSERT_EQ(verify_ip_dscp(get_spec, 63), true);

    // update user-defined-2
    //     MTU: 9216
    //     Pause: link-level
    //     Sched: dwrr: 20
    //     ClassMap: DSCP: 60 (change both DSCP values)
    //     Marking: pcp_rewrite_en: true
    spec.Clear();
    rsp.Clear();
    spec.mutable_key_or_handle()->set_qos_group(kh::USER_DEFINED_2);
    spec.set_mtu(9216);
    spec.mutable_pause()->set_type(qos::QOS_PAUSE_TYPE_LINK_LEVEL);
    spec.mutable_sched()->mutable_dwrr()->set_bw_percentage(20);
    spec.mutable_class_map()->set_type(qos::QOS_CLASS_MAP_TYPE_DSCP);
    spec.mutable_class_map()->add_ip_dscp(60);
    spec.mutable_marking()->set_dot1q_pcp_rewrite_en(true);
    spec.set_no_drop(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::qosclass_update(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_EQ(ret, HAL_RET_OK);

    // get user-defined-2
    get_req.Clear();
    get_rsp.Clear();
    get_req.mutable_key_or_handle()->set_qos_group(kh::USER_DEFINED_2);
    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    ret = hal::qosclass_get(get_req, &get_rsp);
    hal::hal_cfg_db_close();
    ASSERT_EQ(ret, HAL_RET_OK);

    // verify user-defined-2
    get_spec = &get_rsp.response(0).spec();
    ASSERT_EQ(verify_mtu(get_spec, 9216), true);
    ASSERT_EQ(verify_pause(get_spec, qos::QOS_PAUSE_TYPE_LINK_LEVEL), true);
    ASSERT_EQ(verify_dwrr(get_spec, 20), true);
    ASSERT_EQ(verify_ip_dscp(get_spec, 60), true);
    // verify 62 and 63 not present
    ASSERT_EQ(verify_ip_dscp(get_spec, 62), false);
    ASSERT_EQ(verify_ip_dscp(get_spec, 63), false);

    // create user-defined-3
    //     MTU: 9216
    //     Pause: PFC, pfc_cos=4 (Invalid since global pause type is link-level)
    //     Sched: dwrr: 20
    //     ClassMap: PCP: 4
    //     Marking: pcp_rewrite_en: true
    spec.Clear();
    rsp.Clear();
    spec.mutable_key_or_handle()->set_qos_group(kh::USER_DEFINED_3);
    spec.set_mtu(9216);
    spec.mutable_pause()->set_type(qos::QOS_PAUSE_TYPE_PFC);
    spec.mutable_sched()->mutable_dwrr()->set_bw_percentage(20);
    spec.mutable_class_map()->set_type(qos::QOS_CLASS_MAP_TYPE_PCP);
    spec.mutable_class_map()->set_dot1q_pcp(4);
    spec.mutable_marking()->set_dot1q_pcp_rewrite_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::qosclass_create(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_NE(ret, HAL_RET_OK);

    // change pause type to PFC
    global_pause_req.Clear();
    global_pause_rsp.Clear();
    global_pause_req.set_pause_type(qos::QOS_PAUSE_TYPE_PFC);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::qos_class_set_global_pause_type(
                        global_pause_req, &global_pause_rsp);
    hal::hal_cfg_db_close();
    ASSERT_EQ(ret, HAL_RET_OK);

    // update default class with no-pause
    //     MTU: 9216 (default param)
    //     Pause: None
    //     Sched: dwrr: 50 (default param)
    //     ClassMap: None
    spec.Clear();
    rsp.Clear();
    spec.mutable_key_or_handle()->set_qos_group(kh::DEFAULT);
    spec.set_mtu(9216);
    spec.mutable_sched()->mutable_dwrr()->set_bw_percentage(50);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::qosclass_update(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_EQ(ret, HAL_RET_OK);

    // get default
    get_req.Clear();
    get_rsp.Clear();
    get_req.mutable_key_or_handle()->set_qos_group(kh::DEFAULT);
    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    ret = hal::qosclass_get(get_req, &get_rsp);
    hal::hal_cfg_db_close();
    ASSERT_EQ(ret, HAL_RET_OK);

    // verify default
    get_spec = &get_rsp.response(0).spec();
    ASSERT_EQ(verify_mtu(get_spec, 9216), true);
    // pause is not disabled for default class - hence it wont be NONE
    ASSERT_EQ(verify_pause(get_spec, qos::QOS_PAUSE_TYPE_NONE), false);
    ASSERT_EQ(verify_dwrr(get_spec, 50), true);
    ASSERT_EQ(verify_pcp(get_spec, 0), true);

    // create user-defined-3
    //     MTU: 9216
    //     Pause: PFC, pfc_cos=4
    //     Sched: dwrr: 20
    //     ClassMap: PCP: 4
    //     Marking: pcp_rewrite_en: true
    spec.Clear();
    rsp.Clear();
    spec.mutable_key_or_handle()->set_qos_group(kh::USER_DEFINED_3);
    spec.set_mtu(9216);
    spec.mutable_pause()->set_type(qos::QOS_PAUSE_TYPE_PFC);
    spec.mutable_pause()->set_pfc_cos(4);
    spec.mutable_sched()->mutable_dwrr()->set_bw_percentage(20);
    spec.mutable_class_map()->set_type(qos::QOS_CLASS_MAP_TYPE_PCP);
    spec.mutable_class_map()->set_dot1q_pcp(4);
    spec.mutable_marking()->set_dot1q_pcp_rewrite_en(true);
    spec.set_no_drop(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::qosclass_create(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_EQ(ret, HAL_RET_OK);

    // get user-defined-3
    get_req.Clear();
    get_rsp.Clear();
    get_req.mutable_key_or_handle()->set_qos_group(kh::USER_DEFINED_3);
    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    ret = hal::qosclass_get(get_req, &get_rsp);
    hal::hal_cfg_db_close();
    ASSERT_EQ(ret, HAL_RET_OK);

    // verify user-defined-3
    get_spec = &get_rsp.response(0).spec();
    ASSERT_EQ(verify_mtu(get_spec, 9216), true);
    ASSERT_EQ(verify_pause(get_spec, qos::QOS_PAUSE_TYPE_PFC), true);
    ASSERT_EQ(verify_pause_pfc_cos(get_spec, 4), true);
    ASSERT_EQ(verify_dwrr(get_spec, 20), true);
    ASSERT_EQ(verify_pcp(get_spec, 4), true);

    // delete user-defined-1
    del_req.Clear();
    del_resp.Clear();
    del_req.mutable_key_or_handle()->set_qos_group(kh::USER_DEFINED_1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::qosclass_delete(del_req, &del_resp);
    hal::hal_cfg_db_close();
    ASSERT_EQ(ret, HAL_RET_OK);

    // delete user-defined-1 (not found error)
    del_req.Clear();
    del_resp.Clear();
    del_req.mutable_key_or_handle()->set_qos_group(kh::USER_DEFINED_1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::qosclass_delete(del_req, &del_resp);
    hal::hal_cfg_db_close();
    ASSERT_NE(ret, HAL_RET_OK);
    ASSERT_EQ(del_resp.api_status(), types::ApiStatus::API_STATUS_NOT_FOUND);

    // delete default (default class delete not allowed)
    del_req.Clear();
    del_resp.Clear();
    del_req.mutable_key_or_handle()->set_qos_group(kh::DEFAULT);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::qosclass_delete(del_req, &del_resp);
    hal::hal_cfg_db_close();
    ASSERT_NE(ret, HAL_RET_OK);
}

// Creating internal qos_classs
TEST_F(qos_class_test, test2)
{
    hal_ret_t        ret;
    QosClassSpec     spec;
    QosClassResponse rsp;

    spec.Clear();

    spec.mutable_key_or_handle()->set_qos_group(kh::INTERNAL_RX_PROXY_NO_DROP);
    spec.set_mtu(2000);
    spec.mutable_sched()->mutable_dwrr()->set_bw_percentage(100);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::qosclass_create(spec, &rsp);
    hal::hal_cfg_db_close();
    // Internal classes are created during init. So this should fail
    ASSERT_NE(ret, HAL_RET_OK);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
