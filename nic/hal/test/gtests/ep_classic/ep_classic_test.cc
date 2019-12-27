#include "nic/hal/plugins/cfg/nw/interface.hpp"
#include "nic/hal/plugins/cfg/nw/endpoint.hpp"
#include "nic/hal/plugins/cfg/nw/session.hpp"
#include "nic/hal/plugins/cfg/nw/nw.hpp"
#include "nic/hal/plugins/sfw/cfg/nwsec.hpp"
#include "nic/hal/plugins/cfg/l4lb/l4lb.hpp"
#include "gen/proto/interface.pb.h"
#include "gen/proto/l2segment.pb.h"
#include "gen/proto/vrf.pb.h"
#include "gen/proto/nwsec.pb.h"
#include "gen/proto/endpoint.pb.h"
#include "gen/proto/session.pb.h"
#include "gen/proto/l4lb.pb.h"
#include "gen/proto/nw.pb.h"
#include "nic/hal/hal.hpp"
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/hal/test/utils/hal_base_test.hpp"
#include "nic/hal/test/utils/hal_test_utils.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"

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
using endpoint::EndpointSpec;
using endpoint::EndpointResponse;
using endpoint::EndpointUpdateRequest;
using session::SessionSpec;
using session::SessionResponse;
using nw::NetworkSpec;
using nw::NetworkResponse;
using l4lb::L4LbServiceSpec;
using l4lb::L4LbServiceResponse;


class endpoint_test : public hal_base_test {
protected:
  endpoint_test() {
  }

  virtual ~endpoint_test() {
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
create_lif(uint32_t lif_id, uint32_t if_id)
{
    hal_ret_t ret;
    LifSpec spec;
    LifResponse rsp;

    spec.mutable_key_or_handle()->set_lif_id(lif_id);
    spec.set_type(types::LIF_TYPE_HOST);
    spec.set_hw_lif_id(lif_id);
    spec.mutable_pinned_uplink_if_key_handle()->set_interface_id(if_id);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(spec, &rsp, NULL);
    ASSERT_TRUE(ret == HAL_RET_OK);
    hal::hal_cfg_db_close();
}

void
create_enic(uint32_t lif_id, uint32_t if_id, uint32_t l2seg_id, uint32_t encap)
{
    hal_ret_t ret;
    InterfaceSpec               enicif_spec;
    InterfaceResponse           enicif_rsp;

    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(lif_id);
    enicif_spec.mutable_key_or_handle()->set_interface_id(if_id);
    enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(l2seg_id);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(encap);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
}

hal_ret_t
create_ep(uint32_t vrf_id, uint32_t l2seg_id, uint32_t if_id, uint64_t mac)
{
    hal_ret_t ret;
    EndpointSpec             ep_spec;
    EndpointResponse         ep_rsp;

    ep_spec.mutable_vrf_key_handle()->set_vrf_id(vrf_id);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_segment_id(l2seg_id);
    ep_spec.mutable_endpoint_attrs()->mutable_interface_key_handle()->set_interface_id(if_id);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(mac);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec, &ep_rsp);
    hal::hal_cfg_db_close();
    return ret;
}

void
fill_registered_mac(uint32_t seed)
{
    hal_ret_t ret = HAL_RET_OK;
    sdk_ret_t      sdk_ret;
    registered_macs_swkey_t         key;
    registered_macs_actiondata_t    data;
    sdk_hash                        *reg_mac_tbl = NULL;
    uint32_t idx = 0;

    printf("Using seed: %d\n", seed);
    srand(seed);

    reg_mac_tbl = hal::pd::g_hal_state_pd->hash_tcam_table(P4TBL_ID_REGISTERED_MACS);

    while(1) {
        key.flow_lkp_metadata_lkp_reg_mac_vrf = rand();
        for (int i = 0; i < 6; i++) {
            key.flow_lkp_metadata_lkp_dstMacAddr[i] = rand();
        }

        sdk_ret = reg_mac_tbl->insert(&key, &data, &idx);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            printf("Reg Mac FULL. %d, #hash_entries: %d, #tcam_entries: %d\n",
                   ret, reg_mac_tbl->num_entries_in_use(),
                   reg_mac_tbl->oflow_num_entries_in_use());
            break;
        }
    }

}

// ----------------------------------------------------------------------------
// Creating a endpoint
// ----------------------------------------------------------------------------
TEST_F(endpoint_test, test1)
{
    hal_ret_t      ret = HAL_RET_OK;
    slab_stats_t   *pre = NULL, *post = NULL;
    bool           is_leak = false;
    uint32_t       test_id = 1;
    uint32_t       lif_id = 66;
    uint32_t       uplinkif_id = UPLINK_IF_ID_OFFSET + test_id;
    uint32_t       enicif_id = uplinkif_id + 1;
    uint32_t       vrf_id = test_id;
    uint32_t       l2seg_id = test_id;
    uint32_t       encap = 10;
    uint64_t       mac = 0x0001DEADBEEF;
    uint32_t       seed = time(NULL);

    hal::g_hal_state->set_forwarding_mode(hal::HAL_FORWARDING_MODE_CLASSIC);

    seed = 1577395672;
    fill_registered_mac(seed);
    // seed++;
    // fill_registered_mac(seed);
    // seed++;
    // fill_registered_mac(seed);

    registered_macs_swkey_t         key;
    registered_macs_actiondata_t    data;
    sdk_hash                        *reg_mac_tbl = NULL;
    uint32_t idx = 0;
    reg_mac_tbl = hal::pd::g_hal_state_pd->hash_tcam_table(P4TBL_ID_REGISTERED_MACS);

    key.flow_lkp_metadata_lkp_reg_mac_vrf = 0x1001;
    key.flow_lkp_metadata_lkp_dstMacAddr[5] = 0x00;
    key.flow_lkp_metadata_lkp_dstMacAddr[4] = 0x01;
    key.flow_lkp_metadata_lkp_dstMacAddr[3] = 0xde;
    key.flow_lkp_metadata_lkp_dstMacAddr[2] = 0xad;
    key.flow_lkp_metadata_lkp_dstMacAddr[1] = 0xbe;
    key.flow_lkp_metadata_lkp_dstMacAddr[0] = 0xef;
    reg_mac_tbl->insert(&key, &data, &idx);


    // Create uplink
    create_uplink(uplinkif_id, test_id, test_id);
    // Create vrf
    create_vrf(test_id);
    // Create lif
    create_lif(lif_id, uplinkif_id);
    // Create l2seg
    create_l2seg(vrf_id, l2seg_id, encap, uplinkif_id);
    // Create enic
    create_enic(lif_id, enicif_id, l2seg_id, encap);
    pre = hal_test_utils_collect_slab_stats();
    // Create ep
    ret = create_ep(vrf_id, l2seg_id, enicif_id, mac);
    HAL_TRACE_DEBUG("Return value: {}", ret);
    printf("ret: %d\n", ret);
    ASSERT_TRUE(ret == HAL_RET_NO_RESOURCE || ret == HAL_RET_HW_PROG_ERR);
    post = hal_test_utils_collect_slab_stats();
    hal_test_utils_check_slab_leak(pre, post, &is_leak);
    ASSERT_TRUE(is_leak == false);

}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
