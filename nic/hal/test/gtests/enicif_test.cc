#include "nic/hal/src/l2segment.hpp"
#include "nic/hal/src/interface.hpp"
#include "nic/hal/src/network.hpp"
#include "nic/hal/src/nwsec.hpp"
#include "nic/proto/hal/interface.pb.h"
#include "nic/proto/hal/l2segment.pb.h"
#include "nic/proto/hal/tenant.pb.h"
#include "nic/proto/hal/nwsec.pb.h"
#include "nic/hal/hal.hpp"
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include "nic/hal/test/utils/hal_test_utils.hpp"

using intf::InterfaceSpec;
using intf::InterfaceResponse;
using intf::InterfaceKeyHandle;
using l2segment::L2SegmentSpec;
using l2segment::L2SegmentResponse;
using tenant::TenantSpec;
using tenant::TenantResponse;
using intf::InterfaceL2SegmentSpec;
using intf::InterfaceL2SegmentResponse;
using intf::LifSpec;
using intf::LifResponse;
using intf::LifKeyHandle;
using nwsec::SecurityProfileSpec;
using nwsec::SecurityProfileResponse;
using nw::NetworkSpec;
using nw::NetworkResponse;

void
hal_initialize()
{
    char 			cfg_file[] = "hal.json";
	char 			*cfg_path;
    std::string     full_path;
    hal::hal_cfg_t  hal_cfg = { 0 };

    cfg_path = std::getenv("HAL_CONFIG_PATH");
    if (cfg_path) {
        full_path =  std::string(cfg_path) + "/" + std::string(cfg_file);
        std::cerr << "full path " << full_path << std::endl;
    } else {
        full_path = std::string(cfg_file);
    }

    // make sure cfg file exists
    if (access(full_path.c_str(), R_OK) < 0) {
        fprintf(stderr, "config file %s has no read permissions\n",
                full_path.c_str());
        exit(1);
    }

    printf("Json file: %s\n", full_path.c_str());

    if (hal::hal_parse_cfg(full_path.c_str(), &hal_cfg) != HAL_RET_OK) {
        fprintf(stderr, "HAL config file parsing failed, quitting ...\n");
        ASSERT_TRUE(0);
    }
    printf("Parsed cfg json file \n");

    // initialize HAL
    if (hal::hal_init(&hal_cfg) != HAL_RET_OK) {
        fprintf(stderr, "HAL initialization failed, quitting ...\n");
        exit(1);
    }
    printf("HAL Initialized \n");
}


class enicif_test : public ::testing::Test {
protected:
  enicif_test() {
  }

  virtual ~enicif_test() {
  }

  // will be called immediately after the constructor before each test
  virtual void SetUp() {
  }

  // will be called immediately after each test before the destructor
  virtual void TearDown() {
  }

  // Will be called at the beginning of all test cases in this class
  static void SetUpTestCase() {
    hal_initialize();
    hal_test_utils_slab_disable_delete();
  }
  // Will be called at the end of all test cases in this class
  static void TearDownTestCase() {
  }
};

// ----------------------------------------------------------------------------
// Creating a useg enicif
// ----------------------------------------------------------------------------
TEST_F(enicif_test, test1) 
{
    hal_ret_t                   ret;
    TenantSpec                  ten_spec;
    TenantResponse              ten_rsp;
    LifSpec                     lif_spec;
    LifResponse                 lif_rsp;
    L2SegmentSpec               l2seg_spec;
    L2SegmentResponse           l2seg_rsp;
    InterfaceSpec               enicif_spec;
    InterfaceResponse           enicif_rsp;
    SecurityProfileSpec         sp_spec;
    SecurityProfileResponse     sp_rsp;
    NetworkSpec                 nw_spec;
    NetworkResponse             nw_rsp;
    InterfaceDeleteRequest      del_req;
    InterfaceDeleteResponseMsg  del_rsp;
    slab_stats_t                *pre = NULL, *post = NULL;
    bool                        is_leak = false;

    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(1);
    sp_spec.set_ipsg_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::security_profile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();

    // Create tenant
    ten_spec.mutable_key_or_handle()->set_tenant_id(1);
    ten_spec.set_security_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::tenant_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create network
    nw_spec.mutable_meta()->set_tenant_id(1);
    nw_spec.set_rmac(0x0000DEADBEEF);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->set_prefix_len(32);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0xa0000000);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl = nw_rsp.mutable_status()->nw_handle();

    // Create a lif
    lif_spec.set_port_num(10);
    lif_spec.mutable_key_or_handle()->set_lif_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create L2 Segment
    l2seg_spec.mutable_meta()->set_tenant_id(1);
    l2seg_spec.add_network_handle(nw_hdl);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(1);
    l2seg_spec.mutable_fabric_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_fabric_encap()->set_encap_value(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);


    pre = hal_test_utils_collect_slab_stats();
    // Create enicif
    enicif_spec.mutable_meta()->set_tenant_id(1);
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(1);
    enicif_spec.mutable_key_or_handle()->set_interface_id(1);
    enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec.mutable_if_enic_info()->set_l2segment_id(1);
    enicif_spec.mutable_if_enic_info()->set_encap_vlan_id(20);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // update lif
    lif_spec.set_vlan_strip_en(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_update(lif_spec, &lif_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);


    // delete enicif
    del_req.mutable_key_or_handle()->set_interface_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close(false);
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // There is a leak of HAL_SLAB_HANDLE_ID_LIST_ENTRY for adding 
    post = hal_test_utils_collect_slab_stats();
    hal_test_utils_check_slab_leak(pre, post, &is_leak);
    ASSERT_TRUE(is_leak == false);
}

#if 0
// ----------------------------------------------------------------------------
// Creating muliple enicifs
// ----------------------------------------------------------------------------
TEST_F(enicif_test, test2) 
{
    hal_ret_t            ret;
    enicifSpec spec;
    enicifResponse rsp;

    for (int i = 0; i < 10; i++) {
        spec.set_port_num(i);
        spec.mutable_key_or_handle()->set_enicif_id(i);

        ret = hal::enicif_create(spec, &rsp);
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

}
#endif
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
