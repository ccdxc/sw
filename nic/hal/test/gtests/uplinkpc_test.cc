#include "nic/hal/src/interface.hpp"
#include "nic/hal/src/network.hpp"
#include "nic/proto/hal/interface.pb.h"
#include "nic/proto/hal/l2segment.pb.h"
#include "nic/proto/hal/tenant.pb.h"
#include "nic/proto/hal/nwsec.pb.h"
#include "nic/hal/src/nwsec.hpp"
#include "nic/hal/hal.hpp"
#include "nic/hal/src/l2segment.hpp"
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

class uplinkpc_test : public ::testing::Test {
protected:
  uplinkpc_test() {
  }

  virtual ~uplinkpc_test() {
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
// Creating a uplinkpc
// ----------------------------------------------------------------------------
TEST_F(uplinkpc_test, test1) 
{
    hal_ret_t            ret;
    InterfaceSpec       spec;
    InterfaceResponse   rsp;
    InterfaceSpec       pc_spec;
    InterfaceResponse   pc_rsp;

    // Uplink create
    spec.set_type(intf::IF_TYPE_UPLINK);
    spec.mutable_key_or_handle()->set_interface_id(1);
    spec.mutable_if_uplink_info()->set_port_num(1);
    // spec.mutable_if_uplink_info()->set_native_l2segment_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    ::google::protobuf::uint64 up_hdl = rsp.mutable_status()->if_handle();

    // Uplink PC Create
    pc_spec.set_type(intf::IF_TYPE_UPLINK_PC);
    pc_spec.mutable_key_or_handle()->set_interface_id(2);
    pc_spec.mutable_if_uplink_pc_info()->add_member_if_handle(up_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(pc_spec, &pc_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Uplink PC Update
    pc_spec.mutable_if_uplink_pc_info()->set_native_l2segment_id(2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_update(pc_spec, &pc_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_L2SEG_NOT_FOUND);
    // Release if_uplink_info
    // free spec.release_if_uplink_info();
}

// ----------------------------------------------------------------------------
// Creating muliple uplinkpcs
// ----------------------------------------------------------------------------
TEST_F(uplinkpc_test, test2) 
{
    hal_ret_t            ret;
    InterfaceSpec       spec;
    InterfaceResponse   rsp;
    InterfaceSpec       pc_spec;
    InterfaceResponse   pc_rsp;

    for (int i = 1; i <= 8; i++) {
        spec.set_type(intf::IF_TYPE_UPLINK);

        spec.mutable_key_or_handle()->set_interface_id(100 + i);
        spec.mutable_if_uplink_info()->set_port_num(i);
        // spec.mutable_if_uplink_info()->set_native_l2segment_id(i);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::interface_create(spec, &rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
        ::google::protobuf::uint64 up_hdl = rsp.mutable_status()->if_handle();

        pc_spec.set_type(intf::IF_TYPE_UPLINK_PC);
        pc_spec.mutable_key_or_handle()->set_interface_id(i+150);
        pc_spec.mutable_if_uplink_pc_info()->add_member_if_handle(up_hdl);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::interface_create(pc_spec, &pc_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

}

// ----------------------------------------------------------------------------
// Creating a uplinkpc and segements
// ----------------------------------------------------------------------------
TEST_F(uplinkpc_test, test3) 
{
    hal_ret_t                       ret;
    InterfaceSpec                   if_spec;
    InterfaceResponse               if_rsp;
    L2SegmentSpec                   l2seg_spec;
    L2SegmentResponse               l2seg_rsp;
    TenantSpec                      ten_spec;
    TenantResponse                  ten_rsp;
    InterfaceL2SegmentSpec          if_l2seg_spec;
    InterfaceL2SegmentResponse      if_l2seg_rsp;
    InterfaceSpec                   pc_spec;
    InterfaceResponse               pc_rsp;
    SecurityProfileSpec             sp_spec;
    SecurityProfileResponse         sp_rsp;
    NetworkSpec                     nw_spec;
    NetworkResponse                 nw_rsp;

    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(1);
    sp_spec.set_ipsg_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::security_profile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();

    // Create tenant
    ten_spec.mutable_key_or_handle()->set_tenant_id(1);
    ten_spec.set_security_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::tenant_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create network
    nw_spec.mutable_meta()->set_tenant_id(1);
    nw_spec.set_rmac(0x0000DEADBEEF);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->set_prefix_len(32);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0xa0000000);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl = nw_rsp.mutable_status()->nw_handle();

    // Create Uplink If
    if_spec.set_type(intf::IF_TYPE_UPLINK);
    if_spec.mutable_key_or_handle()->set_interface_id(31);
    if_spec.mutable_if_uplink_info()->set_port_num(1);
    // if_spec.mutable_if_uplink_info()->set_native_l2segment_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    ::google::protobuf::uint64 up_hdl = if_rsp.mutable_status()->if_handle();
    
    // Create l2segment
    l2seg_spec.mutable_meta()->set_tenant_id(1);
    l2seg_spec.add_network_handle(nw_hdl);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(31);
    l2seg_spec.mutable_fabric_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_fabric_encap()->set_encap_value(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    printf("ret: %d\n", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create Uplink PC
    pc_spec.set_type(intf::IF_TYPE_UPLINK_PC);
    pc_spec.mutable_key_or_handle()->set_interface_id(32);
    pc_spec.mutable_if_uplink_pc_info()->add_member_if_handle(up_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(pc_spec, &pc_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Adding L2segment on Uplink
    if_l2seg_spec.mutable_l2segment_key_or_handle()->set_segment_id(31);
    if_l2seg_spec.mutable_if_key_handle()->set_interface_id(32);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::add_l2seg_on_uplink(if_l2seg_spec, &if_l2seg_rsp);
    hal::hal_cfg_db_close();
    printf("ret: %d api_status: %d\n", ret, if_l2seg_rsp.api_status());
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Release if_uplink_info
    // free spec.release_if_uplink_info();
}

// ----------------------------------------------------------------------------
// Creating and deleting a uplinkpc
// ----------------------------------------------------------------------------
TEST_F(uplinkpc_test, test4) 
{
    hal_ret_t            ret;
    // InterfaceSpec       spec;
    // InterfaceResponse   rsp;
    InterfaceSpec       pc_spec;
    InterfaceResponse   pc_rsp;
    InterfaceDeleteRequest          del_req;
    InterfaceDeleteResponseMsg      del_rsp;
    slab_stats_t                    *pre = NULL, *post = NULL;
    bool                            is_leak = false;

    pre = hal_test_utils_collect_slab_stats();
#if 0
    // create uplink if
    spec.set_type(intf::IF_TYPE_UPLINK);
    spec.mutable_key_or_handle()->set_interface_id(1);
    spec.mutable_if_uplink_info()->set_port_num(1);
    // spec.mutable_if_uplink_info()->set_native_l2segment_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    ::google::protobuf::uint64 up_hdl = rsp.mutable_status()->if_handle();
#endif

    pc_spec.set_type(intf::IF_TYPE_UPLINK_PC);
    pc_spec.mutable_key_or_handle()->set_interface_id(42);
    // pc_spec.mutable_if_uplink_pc_info()->add_member_if_handle(up_hdl);
    pc_spec.mutable_if_uplink_pc_info()->set_native_l2segment_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(pc_spec, &pc_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // delete uplink if
    del_req.mutable_key_or_handle()->set_interface_id(42);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    post = hal_test_utils_collect_slab_stats();
    hal_test_utils_check_slab_leak(pre, post, &is_leak);
    ASSERT_TRUE(is_leak == false);
}

// ----------------------------------------------------------------------------
// Test 5:
//      - Create PC
//      - Create 8 Uplink Ifs
//      - Add 4 uplinkifs to PC
//      - Remove 4 ifs
//      - Add 4
//      - Remove 2 and add 4
// ----------------------------------------------------------------------------
TEST_F(uplinkpc_test, test5) 
{
    hal_ret_t                        ret;
    InterfaceSpec                   spec;
    InterfaceResponse               rsp;
    InterfaceDeleteRequest          del_req;
    InterfaceDeleteResponseMsg      del_rsp;
    InterfaceSpec                   pc_spec;
    InterfaceResponse               pc_rsp;
    slab_stats_t                    *pre = NULL, *post = NULL;
    bool                            is_leak = false;
    int                             num_uplinkifs = 8;
    hal_handle_t                    upif_handles[8];

    pre = hal_test_utils_collect_slab_stats();

    for (int i = 1; i <= num_uplinkifs; i++) {
        // create uplink if
        spec.set_type(intf::IF_TYPE_UPLINK);
        spec.mutable_key_or_handle()->set_interface_id(500 + i);
        spec.mutable_if_uplink_info()->set_port_num(i);
        spec.mutable_if_uplink_info()->set_native_l2segment_id(1);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::interface_create(spec, &rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
        upif_handles[i-1] = rsp.mutable_status()->if_handle();
    }

    // Uplink PC Create
    pc_spec.set_type(intf::IF_TYPE_UPLINK_PC);
    pc_spec.mutable_key_or_handle()->set_interface_id(500 + num_uplinkifs + 1);
    // pc_spec.mutable_if_uplink_pc_info()->add_member_if_handle(up_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(pc_spec, &pc_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Add 8 uplinks to PC
    pc_spec.set_type(intf::IF_TYPE_UPLINK_PC);
    pc_spec.mutable_key_or_handle()->set_interface_id(500 + num_uplinkifs + 1);
    for (int i = 1; i <= num_uplinkifs; i++) {
        pc_spec.mutable_if_uplink_pc_info()->add_member_if_handle(upif_handles[i-1]);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::interface_update(pc_spec, &pc_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

    // Remove 8 uplinks to PC
    pc_spec.set_type(intf::IF_TYPE_UPLINK_PC);
    pc_spec.mutable_key_or_handle()->set_interface_id(500 + num_uplinkifs + 1);
    for (int i = 1; i <= num_uplinkifs; i++) {
        pc_spec.mutable_if_uplink_pc_info()->clear_member_if_handle();
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::interface_update(pc_spec, &pc_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

    // Add 4 uplinks to PC
    pc_spec.set_type(intf::IF_TYPE_UPLINK_PC);
    pc_spec.mutable_key_or_handle()->set_interface_id(500 + num_uplinkifs + 1);
    for (int i = 1; i <= 4; i++) {
        pc_spec.mutable_if_uplink_pc_info()->add_member_if_handle(upif_handles[i-1]);
    }
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_update(pc_spec, &pc_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Add other 4 uplinks to PC
    pc_spec.mutable_if_uplink_pc_info()->clear_member_if_handle();
    pc_spec.set_type(intf::IF_TYPE_UPLINK_PC);
    pc_spec.mutable_key_or_handle()->set_interface_id(500 + num_uplinkifs + 1);
    for (int i = 5; i <= 8; i++) {
        pc_spec.mutable_if_uplink_pc_info()->add_member_if_handle(upif_handles[i-1]);
    }
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_update(pc_spec, &pc_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Delete all mbrs
    pc_spec.mutable_if_uplink_pc_info()->clear_member_if_handle();
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_update(pc_spec, &pc_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Add other 4 uplinks to PC
    pc_spec.mutable_if_uplink_pc_info()->clear_member_if_handle();
    pc_spec.set_type(intf::IF_TYPE_UPLINK_PC);
    pc_spec.mutable_key_or_handle()->set_interface_id(500 + num_uplinkifs + 1);
    for (int i = 5; i <= 8; i++) {
        pc_spec.mutable_if_uplink_pc_info()->add_member_if_handle(upif_handles[i-1]);
    }
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_update(pc_spec, &pc_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Delete all mbr ifs
    for (int i = 5; i <= num_uplinkifs; i++) {
        // create uplink if
        del_req.mutable_key_or_handle()->set_interface_id(500 + i);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::interface_delete(del_req, &del_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OBJECT_IN_USE);
    }

    // Delete PC
    del_req.mutable_key_or_handle()->set_interface_id(500 + num_uplinkifs + 1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);


    // Delete all mbr ifs
    for (int i = 1; i <= num_uplinkifs; i++) {
        // create uplink if
        del_req.mutable_key_or_handle()->set_interface_id(500 + i);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::interface_delete(del_req, &del_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

    post = hal_test_utils_collect_slab_stats();
    hal_test_utils_check_slab_leak(pre, post, &is_leak);
    ASSERT_TRUE(is_leak == false);
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
