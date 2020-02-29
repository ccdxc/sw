#include "nic/hal/plugins/cfg/nw/vrf.hpp"
#include "nic/hal/plugins/cfg/nw/nw.hpp"
#include "nic/hal/plugins/cfg/nw/interface.hpp"
#include "nic/hal/plugins/cfg/nw/l2segment.hpp"
#include "gen/proto/interface.pb.h"
#include "gen/proto/l2segment.pb.h"
#include "gen/proto/vrf.pb.h"
#include "gen/proto/nwsec.pb.h"
#include "nic/hal/hal.hpp"
#include "nic/hal/plugins/sfw/cfg/nwsec.hpp"
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include "nic/hal/test/utils/hal_test_utils.hpp"
#include "nic/hal/test/utils/hal_base_test.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"

using intf::InterfaceSpec;
using intf::InterfaceResponse;
using kh::InterfaceKeyHandle;
using intf::InterfaceL2SegmentSpec;
using intf::InterfaceL2SegmentResponse;
using intf::LifSpec;
using intf::LifResponse;
using kh::LifKeyHandle;
using l2segment::L2SegmentSpec;
using l2segment::L2SegmentResponse;
using l2segment::L2SegmentDeleteRequest;
using vrf::VrfSpec;
using vrf::VrfResponse;
using vrf::VrfDeleteRequest;
using vrf::VrfDeleteResponseMsg;
using nwsec::SecurityProfileSpec;
using nwsec::SecurityProfileResponse;
using nw::NetworkSpec;
using nw::NetworkResponse;

class l2seg_test : public hal_base_test {
protected:
  l2seg_test() {
  }

  virtual ~l2seg_test() {
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

void
create_uplink(uint32_t if_id, uint32_t port, uint32_t native_l2seg)
{
    hal_ret_t            ret;
    InterfaceSpec       spec;
    InterfaceResponse   rsp;

    spec.set_type(intf::IF_TYPE_UPLINK);

    spec.mutable_key_or_handle()->set_interface_id(if_id);
    spec.mutable_if_uplink_info()->set_port_num(port);
    spec.mutable_if_uplink_info()->set_native_l2segment_id(native_l2seg);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
}

hal_ret_t
create_vrf(uint32_t vrf_id)
{
    hal_ret_t ret;
    VrfSpec spec;
    VrfResponse rsp;

    spec.mutable_key_or_handle()->set_vrf_id(vrf_id);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(spec, &rsp);
    hal::hal_cfg_db_close();
    return ret;
}

hal_ret_t
create_l2seg(uint32_t vrf_id, uint32_t l2seg_id, uint32_t encap,
             uint32_t upif_id)
{
    hal_ret_t ret;
    L2SegmentSpec       spec;
    L2SegmentResponse   rsp;

    spec.mutable_vrf_key_handle()->set_vrf_id(vrf_id);
    spec.mutable_key_or_handle()->set_segment_id(l2seg_id);
    spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    spec.mutable_wire_encap()->set_encap_value(encap);
    spec.add_if_key_handle()->set_interface_id(upif_id);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(spec, &rsp);
    hal::hal_cfg_db_close();
    return ret;
}

void
fill_inp_prop(uint32_t seed)
{
    hal_ret_t ret = HAL_RET_OK;
    sdk_ret_t      sdk_ret;
    sdk_hash *inp_prop_tbl = NULL;
    input_properties_swkey_t key = {0};
    input_properties_actiondata_t data = {0};
    input_properties_otcam_swkey_mask_t key_mask = {0};
    uint32_t idx = 0;

    printf("Using seed: %d\n", seed);
    srand(seed);

    inp_prop_tbl = hal::pd::g_hal_state_pd->hash_tcam_table(P4TBL_ID_INPUT_PROPERTIES);

    while(1) {
        key.capri_intrinsic_lif = rand();
        key.tunnel_metadata_tunnel_vni = rand();

        sdk_ret = inp_prop_tbl->insert(&key, &data, &idx, &key_mask, false);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            printf("Inp. props FULL. %d, #hash_entries: %d, #tcam_entries: %d\n",
                   ret, inp_prop_tbl->num_entries_in_use(),
                   inp_prop_tbl->oflow_num_entries_in_use());
            break;
        }
    }

}

TEST_F(l2seg_test, test1)
{
    hal_ret_t      ret = HAL_RET_OK;
    slab_stats_t   *pre = NULL, *post = NULL;
    bool           is_leak = false;
    // uint32_t       seed = time(NULL);

    hal::g_hal_state->set_forwarding_mode(sdk::lib::FORWARDING_MODE_CLASSIC);

    // fill_inp_prop(seed);
    fill_inp_prop(1549822106);

    // Create uplink
    create_uplink(UPLINK_IF_ID_OFFSET + 1, PORT_NUM_1, 1);
    // Create Vrf
    create_vrf(1);
    // Collect slabs
    pre = hal_test_utils_collect_slab_stats();
    // Create l2seg with uplink
    ret = create_l2seg(1, 1, 10, UPLINK_IF_ID_OFFSET + 1);
    ASSERT_TRUE(ret == HAL_RET_NO_RESOURCE);
    // Check for slab leak
    post = hal_test_utils_collect_slab_stats();
    hal_test_utils_check_slab_leak(pre, post, &is_leak);
    ASSERT_TRUE(is_leak == false);
}


TEST_F(l2seg_test, test2)
{
    hal_ret_t      ret = HAL_RET_OK;
    slab_stats_t   *pre = NULL, *post = NULL;
    bool           is_leak = false;
    // uint32_t       seed = time(NULL);
    uint32_t       test_id = 2;
    sdk_hash *inp_prop_tbl = NULL;
    input_properties_swkey_t key = {0};
    input_properties_actiondata_t data = {0};
    input_properties_otcam_swkey_mask_t key_mask = {0};
    uint32_t idx = 0;

    hal::g_hal_state->set_forwarding_mode(sdk::lib::FORWARDING_MODE_CLASSIC);

    key.capri_intrinsic_lif = 0x21;
    key.vlan_tag_valid = 1;
    key.vlan_tag_vid = 2;
    inp_prop_tbl = hal::pd::g_hal_state_pd->hash_tcam_table(P4TBL_ID_INPUT_PROPERTIES);
    inp_prop_tbl->insert(&key, &data, &idx, &key_mask, false);

    // fill_inp_prop(seed);

    // Create uplink
    create_uplink(UPLINK_IF_ID_OFFSET + test_id, test_id, test_id);
    pre = hal_test_utils_collect_slab_stats();
    // Create Vrf
    ret = create_vrf(test_id);
    printf("ret: %d", ret);
    ASSERT_TRUE(ret == HAL_RET_HW_PROG_ERR || ret == HAL_RET_NO_RESOURCE);
    // Check for slab leak
    post = hal_test_utils_collect_slab_stats();
    hal_test_utils_check_slab_leak(pre, post, &is_leak);
    ASSERT_TRUE(is_leak == false);
}

// Single Wire Management Test Case
TEST_F(l2seg_test, test3)
{
    hal_ret_t           ret = HAL_RET_OK;
    VrfSpec             vrf_spec;
    VrfResponse         vrf_rsp;
    L2SegmentSpec       spec;
    L2SegmentResponse   rsp;
    L2SegmentDeleteRequest  del_req;
    L2SegmentDeleteResponse del_rsp;


    hal::g_hal_state->set_forwarding_mode(sdk::lib::FORWARDING_MODE_CLASSIC);

    // Create uplink
    create_uplink(UPLINK_IF_ID_OFFSET + 1, 1, 0);
    create_uplink(UPLINK_IF_ID_OFFSET + 3, 2, 0);
    // Create Vrf
    vrf_spec.mutable_key_or_handle()->set_vrf_id(3);
    vrf_spec.mutable_designated_uplink()->set_interface_id(UPLINK_IF_ID_OFFSET + 1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(vrf_spec, &vrf_rsp);
    hal::hal_cfg_db_close();
    // Create l2seg with uplink
    // ret = create_l2seg(3, 1, 8192, UPLINK_IF_ID_OFFSET + 1);
    // ASSERT_TRUE(ret == HAL_RET_OK);

    spec.mutable_vrf_key_handle()->set_vrf_id(3);
    spec.mutable_key_or_handle()->set_segment_id(1);
    spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    spec.mutable_wire_encap()->set_encap_value(8192);
    spec.add_if_key_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    spec.Clear();
    spec.mutable_vrf_key_handle()->set_vrf_id(3);
    spec.mutable_key_or_handle()->set_segment_id(1);
    spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    spec.mutable_wire_encap()->set_encap_value(8192);
    spec.add_if_key_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 1);
    spec.add_if_key_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 3);
    spec.set_single_wire_management(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_update(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    spec.Clear();
    spec.mutable_vrf_key_handle()->set_vrf_id(3);
    spec.mutable_key_or_handle()->set_segment_id(1);
    spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    spec.mutable_wire_encap()->set_encap_value(8192);
    spec.add_if_key_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 1);
    spec.set_single_wire_management(false);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_update(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    spec.Clear();
    spec.mutable_vrf_key_handle()->set_vrf_id(3);
    spec.mutable_key_or_handle()->set_segment_id(2);
    spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    spec.mutable_wire_encap()->set_encap_value(30);
    spec.add_if_key_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 1);
    spec.add_if_key_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 3);
    spec.set_single_wire_management(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    del_req.mutable_vrf_key_handle()->set_vrf_id(3);
    del_req.mutable_key_or_handle()->set_segment_id(2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
