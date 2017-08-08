#include <interface.hpp>
#include <interface.pb.h>
#include <l2segment.pb.h>
#include <tenant.pb.h>
#include <nwsec.pb.h>
#include <nwsec.hpp>
#include <hal.hpp>
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>

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

    spec.set_type(intf::IF_TYPE_UPLINK);
    
    spec.mutable_key_or_handle()->set_interface_id(1);
    spec.mutable_if_uplink_info()->set_port_num(1);
    // spec.mutable_if_uplink_info()->set_native_l2segment_id(1);

    ret = hal::interface_create(spec, &rsp);
    ASSERT_TRUE(ret == HAL_RET_OK);
    ::google::protobuf::uint64 up_hdl = rsp.mutable_status()->if_handle();

    pc_spec.set_type(intf::IF_TYPE_UPLINK_PC);
    pc_spec.mutable_key_or_handle()->set_interface_id(2);
    pc_spec.mutable_if_uplink_pc_info()->add_member_if_handle(up_hdl);
    ret = hal::interface_create(pc_spec, &pc_rsp);
    ASSERT_TRUE(ret == HAL_RET_OK);

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

    for (int i = 0; i < 8; i++) {
        spec.set_type(intf::IF_TYPE_UPLINK);

        spec.mutable_key_or_handle()->set_interface_id(i);
        spec.mutable_if_uplink_info()->set_port_num(i);
        // spec.mutable_if_uplink_info()->set_native_l2segment_id(i);
        ret = hal::interface_create(spec, &rsp);
        ASSERT_TRUE(ret == HAL_RET_OK);
        ::google::protobuf::uint64 up_hdl = rsp.mutable_status()->if_handle();

        pc_spec.set_type(intf::IF_TYPE_UPLINK_PC);
        pc_spec.mutable_key_or_handle()->set_interface_id(i+10);
        pc_spec.mutable_if_uplink_pc_info()->add_member_if_handle(up_hdl);
        ret = hal::interface_create(pc_spec, &pc_rsp);
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

    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(1);
    ret = hal::security_profile_create(sp_spec, &sp_rsp);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();

    // Create tenant
    ten_spec.mutable_key_or_handle()->set_tenant_id(1);
    ten_spec.set_security_profile_handle(nwsec_hdl);
    ret = hal::tenant_create(ten_spec, &ten_rsp);
    ASSERT_TRUE(ret == HAL_RET_OK);


    // Create Uplink If
    if_spec.set_type(intf::IF_TYPE_UPLINK);
    if_spec.mutable_key_or_handle()->set_interface_id(1);
    if_spec.mutable_if_uplink_info()->set_port_num(1);
    // if_spec.mutable_if_uplink_info()->set_native_l2segment_id(1);
    ret = hal::interface_create(if_spec, &if_rsp);
    ASSERT_TRUE(ret == HAL_RET_OK);
    ::google::protobuf::uint64 up_hdl = if_rsp.mutable_status()->if_handle();
    
    // Create l2segment
    l2seg_spec.mutable_meta()->set_tenant_id(1);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(1);
    l2seg_spec.mutable_fabric_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_fabric_encap()->set_encap_value(10);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    printf("ret: %d\n", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create Uplink PC
    pc_spec.set_type(intf::IF_TYPE_UPLINK_PC);
    pc_spec.mutable_key_or_handle()->set_interface_id(2);
    pc_spec.mutable_if_uplink_pc_info()->add_member_if_handle(up_hdl);
    ret = hal::interface_create(pc_spec, &pc_rsp);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Adding L2segment on Uplink
    if_l2seg_spec.mutable_l2segment_key_or_handle()->set_segment_id(1);
    if_l2seg_spec.mutable_if_key_handle()->set_interface_id(2);
    ret = hal::add_l2seg_on_uplink(if_l2seg_spec, &if_l2seg_rsp);
    printf("ret: %d api_status: %d\n", ret, if_l2seg_rsp.api_status());
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Release if_uplink_info
    // free spec.release_if_uplink_info();
}
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
