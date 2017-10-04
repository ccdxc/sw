#include "nic/hal/src/tenant.hpp"
#include "nic/hal/src/network.hpp"
#include "nic/hal/src/interface.hpp"
#include "nic/hal/src/l2segment.hpp"
#include "nic/proto/hal/interface.pb.h"
#include "nic/proto/hal/l2segment.pb.h"
#include "nic/proto/hal/tenant.pb.h"
#include "nic/proto/hal/nwsec.pb.h"
#include "nic/hal/hal.hpp"
#include "nic/hal/src/nwsec.hpp"
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include "nic/hal/test/utils/hal_test_utils.hpp"

using intf::InterfaceSpec;
using intf::InterfaceResponse;
using intf::InterfaceKeyHandle;
using intf::InterfaceL2SegmentSpec;
using intf::InterfaceL2SegmentResponse;
using intf::LifSpec;
using intf::LifResponse;
using intf::LifKeyHandle;
using l2segment::L2SegmentSpec;
using l2segment::L2SegmentResponse;
using tenant::TenantSpec;
using tenant::TenantResponse;
using tenant::TenantDeleteRequest;
using tenant::TenantDeleteResponseMsg;
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

class tenant_test : public ::testing::Test {
protected:
  tenant_test() {
  }

  virtual ~tenant_test() {
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
// Tenant delete test
// ----------------------------------------------------------------------------
TEST_F(tenant_test, test1) 
{
    hal_ret_t                       ret;
    TenantSpec                      ten_spec;
    TenantResponse                  ten_rsp;
    SecurityProfileSpec             sp_spec;
    SecurityProfileResponse         sp_rsp;
    TenantDeleteRequest             del_req;
    TenantDeleteResponseMsg         del_rsp;
    slab_stats_t                    *pre = NULL, *post = NULL;
    bool                            is_leak = false;


    hal_test_utils_slab_disable_delete();

    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(1);
    sp_spec.set_ipsg_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::security_profile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();

    pre = hal_test_utils_collect_slab_stats();

    // Create tenant
    ten_spec.mutable_key_or_handle()->set_tenant_id(1);
    ten_spec.set_security_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::tenant_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close(false);
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Delete tenant
    del_req.mutable_meta()->set_tenant_id(1);
    del_req.mutable_key_or_handle()->set_tenant_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::tenant_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close(false);
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    post = hal_test_utils_collect_slab_stats();

    hal_test_utils_check_slab_leak(pre, post, &is_leak);
    ASSERT_TRUE(is_leak == false);

}

// Update tenant test with enicifs
TEST_F(tenant_test, test2) 
{
    hal_ret_t                       ret;
    TenantSpec                      ten_spec, ten_spec1;
    TenantResponse                  ten_rsp, ten_rsp1;
    SecurityProfileSpec             sp_spec, sp_spec1;
    SecurityProfileResponse         sp_rsp, sp_rsp1;
    // TenantDeleteRequest             del_req;
    // TenantDeleteResponseMsg         del_rsp;
    LifSpec                         lif_spec;
    LifResponse                     lif_rsp;
    L2SegmentSpec                   l2seg_spec;
    L2SegmentResponse               l2seg_rsp;
    InterfaceSpec                   enicif_spec;
    InterfaceResponse               enicif_rsp;
    NetworkSpec                     nw_spec;
    NetworkResponse                 nw_rsp;

    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(2);
    sp_spec.set_ipsg_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::security_profile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();

    // Create nwsec
    sp_spec1.mutable_key_or_handle()->set_profile_id(21);
    sp_spec1.set_ipsg_en(false);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::security_profile_create(sp_spec1, &sp_rsp1);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl1 = sp_rsp1.mutable_profile_status()->profile_handle();

    // Create tenant
    ten_spec.mutable_key_or_handle()->set_tenant_id(2);
    ten_spec.set_security_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::tenant_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

#if 0
    // Delete tenant
    del_req.mutable_meta()->set_tenant_id(1);
    del_req.mutable_key_or_handle()->set_tenant_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::tenant_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
#endif

    // Create network
    nw_spec.mutable_meta()->set_tenant_id(2);
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
    lif_spec.mutable_key_or_handle()->set_lif_id(21);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create L2 Segment
    l2seg_spec.mutable_meta()->set_tenant_id(2);
    l2seg_spec.add_network_handle(nw_hdl);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(21);
    l2seg_spec.mutable_fabric_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_fabric_encap()->set_encap_value(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create enicif
    enicif_spec.mutable_meta()->set_tenant_id(2);
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(21);
    enicif_spec.mutable_key_or_handle()->set_interface_id(21);
    enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec.mutable_if_enic_info()->set_l2segment_id(21);
    enicif_spec.mutable_if_enic_info()->set_encap_vlan_id(20);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Update tenant
    ten_spec1.mutable_key_or_handle()->set_tenant_id(2);
    ten_spec1.set_security_profile_handle(nwsec_hdl1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::tenant_update(ten_spec1, &ten_rsp1);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
}

// ----------------------------------------------------------------------------
// Update tenant test with uplink ifs
// ----------------------------------------------------------------------------
TEST_F(tenant_test, test3) 
{
    hal_ret_t                       ret;
    TenantSpec                      ten_spec, ten_spec1;
    TenantResponse                  ten_rsp, ten_rsp1;
    SecurityProfileSpec             sp_spec, sp_spec1;
    SecurityProfileResponse         sp_rsp, sp_rsp1;
    // TenantDeleteRequest             del_req;
    // TenantDeleteResponseMsg         del_rsp;
    L2SegmentSpec                   l2seg_spec;
    L2SegmentResponse               l2seg_rsp;
    InterfaceSpec                   if_spec;
    InterfaceResponse               if_rsp;
    NetworkSpec                     nw_spec;
    NetworkResponse                 nw_rsp;
    InterfaceL2SegmentSpec          if_l2seg_spec;
    InterfaceL2SegmentResponse      if_l2seg_rsp;

    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(31);
    sp_spec.set_ipsg_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::security_profile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();

    // Create nwsec
    sp_spec1.mutable_key_or_handle()->set_profile_id(32);
    sp_spec1.set_ipsg_en(false);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::security_profile_create(sp_spec1, &sp_rsp1);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl1 = sp_rsp1.mutable_profile_status()->profile_handle();

    // Create tenant
    ten_spec.mutable_key_or_handle()->set_tenant_id(3);
    ten_spec.set_security_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::tenant_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

#if 0
    // Delete tenant
    del_req.mutable_meta()->set_tenant_id(1);
    del_req.mutable_key_or_handle()->set_tenant_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::tenant_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
#endif

    // Create network
    nw_spec.mutable_meta()->set_tenant_id(3);
    nw_spec.set_rmac(0x0000DEADBEEF);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->set_prefix_len(32);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0xa0000000);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl = nw_rsp.mutable_status()->nw_handle();

    // Create Uplink If
    if_spec.set_type(intf::IF_TYPE_UPLINK);
    if_spec.mutable_key_or_handle()->set_interface_id(31);
    if_spec.mutable_if_uplink_info()->set_port_num(1);
    // if_spec.mutable_if_uplink_info()->set_native_l2segment_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(if_spec, &if_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create L2 Segment
    l2seg_spec.mutable_meta()->set_tenant_id(3);
    l2seg_spec.add_network_handle(nw_hdl);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(31);
    l2seg_spec.mutable_fabric_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_fabric_encap()->set_encap_value(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Adding L2segment on Uplink
    if_l2seg_spec.mutable_l2segment_key_or_handle()->set_segment_id(31);
    if_l2seg_spec.mutable_if_key_handle()->set_interface_id(31);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::add_l2seg_on_uplink(if_l2seg_spec, &if_l2seg_rsp);
    printf("ret: %d api_status: %d\n", ret, if_l2seg_rsp.api_status());
    ASSERT_TRUE(ret == HAL_RET_OK);
    hal::hal_cfg_db_close(false);

    // Update tenant
    ten_spec1.mutable_key_or_handle()->set_tenant_id(3);
    ten_spec1.set_security_profile_handle(nwsec_hdl1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::tenant_update(ten_spec1, &ten_rsp1);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
}

// ----------------------------------------------------------------------------
// Updating a tenant with no nwsec profile
// ----------------------------------------------------------------------------
TEST_F(tenant_test, test4) 
{
    hal_ret_t                       ret;
    TenantSpec                      ten_spec, ten_spec1;
    TenantResponse                  ten_rsp, ten_rsp1;
    SecurityProfileSpec             sp_spec, sp_spec1;
    SecurityProfileResponse         sp_rsp, sp_rsp1;
    // TenantDeleteRequest             del_req;
    // TenantDeleteResponseMsg         del_rsp;
    LifSpec                         lif_spec;
    LifResponse                     lif_rsp;
    L2SegmentSpec                   l2seg_spec;
    L2SegmentResponse               l2seg_rsp;
    InterfaceSpec                   enicif_spec;
    InterfaceResponse               enicif_rsp;
    NetworkSpec                     nw_spec;
    NetworkResponse                 nw_rsp;

    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(41);
    sp_spec.set_ipsg_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::security_profile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();

    // Create nwsec
    sp_spec1.mutable_key_or_handle()->set_profile_id(42);
    sp_spec1.set_ipsg_en(false);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::security_profile_create(sp_spec1, &sp_rsp1);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    // uint64_t nwsec_hdl1 = sp_rsp1.mutable_profile_status()->profile_handle();

    // Create tenant
    ten_spec.mutable_key_or_handle()->set_tenant_id(4);
    ten_spec.set_security_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::tenant_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

#if 0
    // Delete tenant
    del_req.mutable_meta()->set_tenant_id(1);
    del_req.mutable_key_or_handle()->set_tenant_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::tenant_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
#endif

    // Create network
    nw_spec.mutable_meta()->set_tenant_id(4);
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
    lif_spec.mutable_key_or_handle()->set_lif_id(41);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create L2 Segment
    l2seg_spec.mutable_meta()->set_tenant_id(4);
    l2seg_spec.add_network_handle(nw_hdl);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(41);
    l2seg_spec.mutable_fabric_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_fabric_encap()->set_encap_value(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create enicif
    enicif_spec.mutable_meta()->set_tenant_id(4);
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(41);
    enicif_spec.mutable_key_or_handle()->set_interface_id(41);
    enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec.mutable_if_enic_info()->set_l2segment_id(41);
    enicif_spec.mutable_if_enic_info()->set_encap_vlan_id(20);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Update tenant
    ten_spec1.mutable_key_or_handle()->set_tenant_id(4);
    // ten_spec1.set_security_profile_handle(nwsec_hdl1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::tenant_update(ten_spec1, &ten_rsp1);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
}

// ----------------------------------------------------------------------------
// Updating a tenant without any change
// ----------------------------------------------------------------------------
TEST_F(tenant_test, test5) 
{
    hal_ret_t                       ret;
    TenantSpec                      ten_spec, ten_spec1;
    TenantResponse                  ten_rsp, ten_rsp1;
    SecurityProfileSpec             sp_spec, sp_spec1;
    SecurityProfileResponse         sp_rsp, sp_rsp1;
    // TenantDeleteRequest             del_req;
    // TenantDeleteResponseMsg         del_rsp;
    LifSpec                         lif_spec;
    LifResponse                     lif_rsp;
    L2SegmentSpec                   l2seg_spec;
    L2SegmentResponse               l2seg_rsp;
    InterfaceSpec                   enicif_spec;
    InterfaceResponse               enicif_rsp;
    NetworkSpec                     nw_spec;
    NetworkResponse                 nw_rsp;

    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(51);
    sp_spec.set_ipsg_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::security_profile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();

    // Create nwsec
    sp_spec1.mutable_key_or_handle()->set_profile_id(52);
    sp_spec1.set_ipsg_en(false);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::security_profile_create(sp_spec1, &sp_rsp1);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    // uint64_t nwsec_hdl1 = sp_rsp1.mutable_profile_status()->profile_handle();

    // Create tenant
    ten_spec.mutable_key_or_handle()->set_tenant_id(5);
    ten_spec.set_security_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::tenant_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

#if 0
    // Delete tenant
    del_req.mutable_meta()->set_tenant_id(1);
    del_req.mutable_key_or_handle()->set_tenant_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::tenant_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
#endif

    // Create network
    nw_spec.mutable_meta()->set_tenant_id(5);
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
    lif_spec.mutable_key_or_handle()->set_lif_id(51);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create L2 Segment
    l2seg_spec.mutable_meta()->set_tenant_id(5);
    l2seg_spec.add_network_handle(nw_hdl);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(51);
    l2seg_spec.mutable_fabric_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_fabric_encap()->set_encap_value(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create enicif
    enicif_spec.mutable_meta()->set_tenant_id(5);
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(51);
    enicif_spec.mutable_key_or_handle()->set_interface_id(51);
    enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec.mutable_if_enic_info()->set_l2segment_id(51);
    enicif_spec.mutable_if_enic_info()->set_encap_vlan_id(20);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Update tenant
    ten_spec1.mutable_key_or_handle()->set_tenant_id(5);
    ten_spec1.set_security_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::tenant_update(ten_spec1, &ten_rsp1);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
}

// ----------------------------------------------------------------------------
// Tenant delete test with another create
// ----------------------------------------------------------------------------
TEST_F(tenant_test, test6) 
{
    hal_ret_t                       ret;
    TenantSpec                      ten_spec, ten_spec1;
    TenantResponse                  ten_rsp, ten_rsp1;
    SecurityProfileSpec             sp_spec;
    SecurityProfileResponse         sp_rsp;
    TenantDeleteRequest             del_req;
    TenantDeleteResponseMsg         del_rsp;


    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(61);
    sp_spec.set_ipsg_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::security_profile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();

    // Create tenant
    ten_spec.mutable_key_or_handle()->set_tenant_id(6);
    ten_spec.set_security_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::tenant_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close(false);
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Delete tenant
    del_req.mutable_meta()->set_tenant_id(6);
    del_req.mutable_key_or_handle()->set_tenant_id(6);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::tenant_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close(false);
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create tenant
    ten_spec1.mutable_key_or_handle()->set_tenant_id(6);
    ten_spec1.set_security_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::tenant_create(ten_spec1, &ten_rsp1);
    hal::hal_cfg_db_close(false);
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);
}

// ----------------------------------------------------------------------------
// Tenant update test without l2segments
// ----------------------------------------------------------------------------
TEST_F(tenant_test, test7) 
{
    hal_ret_t                       ret;
    TenantSpec                      ten_spec, ten_spec1;
    TenantResponse                  ten_rsp, ten_rsp1;
    SecurityProfileSpec             sp_spec, sp_spec1;
    SecurityProfileResponse         sp_rsp, sp_rsp1;
    TenantDeleteRequest             del_req;
    TenantDeleteResponseMsg         del_rsp;
    // slab_stats_t                    *pre = NULL, *post = NULL;
    // bool                            is_leak = false;


    hal_test_utils_slab_disable_delete();

    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(71);
    sp_spec.set_ipsg_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::security_profile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();

    // Create nwsec
    sp_spec1.mutable_key_or_handle()->set_profile_id(72);
    sp_spec1.set_ipsg_en(false);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::security_profile_create(sp_spec1, &sp_rsp1);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl1 = sp_rsp1.mutable_profile_status()->profile_handle();

    // Create tenant
    ten_spec.mutable_key_or_handle()->set_tenant_id(7);
    ten_spec.set_security_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::tenant_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close(false);
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Update tenant
    ten_spec1.mutable_key_or_handle()->set_tenant_id(7);
    ten_spec1.set_security_profile_handle(nwsec_hdl1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::tenant_update(ten_spec1, &ten_rsp1);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
}

// Update tenant test with scale of segs and enicifs
TEST_F(tenant_test, test8) 
{
    hal_ret_t                       ret;
    TenantSpec                      ten_spec, ten_spec1;
    TenantResponse                  ten_rsp, ten_rsp1;
    SecurityProfileSpec             sp_spec, sp_spec1;
    SecurityProfileResponse         sp_rsp, sp_rsp1;
    // TenantDeleteRequest             del_req;
    // TenantDeleteResponseMsg         del_rsp;
    LifSpec                         lif_spec;
    LifResponse                     lif_rsp;
    L2SegmentSpec                   l2seg_spec;
    L2SegmentResponse               l2seg_rsp;
    InterfaceSpec                   enicif_spec;
    InterfaceResponse               enicif_rsp;
    NetworkSpec                     nw_spec;
    NetworkResponse                 nw_rsp;

    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(81);
    sp_spec.set_ipsg_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::security_profile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();

    // Create nwsec
    sp_spec1.mutable_key_or_handle()->set_profile_id(82);
    sp_spec1.set_ipsg_en(false);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::security_profile_create(sp_spec1, &sp_rsp1);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl1 = sp_rsp1.mutable_profile_status()->profile_handle();

    // Create tenant
    ten_spec.mutable_key_or_handle()->set_tenant_id(8);
    ten_spec.set_security_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::tenant_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

#if 0
    // Delete tenant
    del_req.mutable_meta()->set_tenant_id(1);
    del_req.mutable_key_or_handle()->set_tenant_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::tenant_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
#endif

    // Create network
    nw_spec.mutable_meta()->set_tenant_id(8);
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
    lif_spec.mutable_key_or_handle()->set_lif_id(81);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create L2 Segment
    l2seg_spec.mutable_meta()->set_tenant_id(8);
    l2seg_spec.add_network_handle(nw_hdl);
    l2seg_spec.mutable_fabric_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);

    for (int i = 0; i < 10; i++) {
        // Creating 10 l2segs
        l2seg_spec.mutable_key_or_handle()->set_segment_id(80 + i);
        l2seg_spec.mutable_fabric_encap()->set_encap_value(10 + i);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
        hal::hal_cfg_db_close(false);
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

    // Create enicif
    enicif_spec.mutable_meta()->set_tenant_id(8);
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(81);


    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 5; j++) {
            enicif_spec.mutable_key_or_handle()->set_interface_id(80 + i*5 + j);
            enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
            enicif_spec.mutable_if_enic_info()->set_l2segment_id(80 + i);
            enicif_spec.mutable_if_enic_info()->set_encap_vlan_id(20 + i*j + j);

            hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
            ret = hal::interface_create(enicif_spec, &enicif_rsp);
            hal::hal_cfg_db_close(false);
            ASSERT_TRUE(ret == HAL_RET_OK);

        }
    }

    // Update tenant
    ten_spec1.mutable_key_or_handle()->set_tenant_id(8);
    ten_spec1.set_security_profile_handle(nwsec_hdl1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::tenant_update(ten_spec1, &ten_rsp1);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
}

// ----------------------------------------------------------------------------
// Update tenant test with scale of uplink ifs
// ----------------------------------------------------------------------------
TEST_F(tenant_test, test9) 
{
    hal_ret_t                       ret;
    TenantSpec                      ten_spec, ten_spec1;
    TenantResponse                  ten_rsp, ten_rsp1;
    SecurityProfileSpec             sp_spec, sp_spec1;
    SecurityProfileResponse         sp_rsp, sp_rsp1;
    // TenantDeleteRequest             del_req;
    // TenantDeleteResponseMsg         del_rsp;
    L2SegmentSpec                   l2seg_spec;
    L2SegmentResponse               l2seg_rsp;
    InterfaceSpec                   if_spec;
    InterfaceResponse               if_rsp;
    NetworkSpec                     nw_spec;
    NetworkResponse                 nw_rsp;
    InterfaceL2SegmentSpec          if_l2seg_spec;
    InterfaceL2SegmentResponse      if_l2seg_rsp;

    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(91);
    sp_spec.set_ipsg_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::security_profile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();

    // Create nwsec
    sp_spec1.mutable_key_or_handle()->set_profile_id(92);
    sp_spec1.set_ipsg_en(false);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::security_profile_create(sp_spec1, &sp_rsp1);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl1 = sp_rsp1.mutable_profile_status()->profile_handle();

    // Create tenant
    ten_spec.mutable_key_or_handle()->set_tenant_id(9);
    ten_spec.set_security_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::tenant_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

#if 0
    // Delete tenant
    del_req.mutable_meta()->set_tenant_id(1);
    del_req.mutable_key_or_handle()->set_tenant_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::tenant_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
#endif

    // Create network
    nw_spec.mutable_meta()->set_tenant_id(9);
    nw_spec.set_rmac(0x0000DEADBEEF);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->set_prefix_len(32);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0xa0000000);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl = nw_rsp.mutable_status()->nw_handle();

    // Create Uplink If
    if_spec.set_type(intf::IF_TYPE_UPLINK);

    for (int i = 0; i < 4; i++) {
        if_spec.mutable_key_or_handle()->set_interface_id(900 + i);
        if_spec.mutable_if_uplink_info()->set_port_num(1);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::interface_create(if_spec, &if_rsp);
        hal::hal_cfg_db_close(false);
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

    // Create L2 Segment
    l2seg_spec.mutable_meta()->set_tenant_id(9);
    l2seg_spec.add_network_handle(nw_hdl);
    l2seg_spec.mutable_fabric_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);

    for (int i = 0; i < 10; i++) {
        // Creating 10 l2segs
        l2seg_spec.mutable_key_or_handle()->set_segment_id(90 + i);
        l2seg_spec.mutable_fabric_encap()->set_encap_value(10 + i);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
        hal::hal_cfg_db_close(false);
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

    // Adding L2segment on Uplink
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 4; j++) {
            if_l2seg_spec.mutable_l2segment_key_or_handle()->set_segment_id(90 + i);
            if_l2seg_spec.mutable_if_key_handle()->set_interface_id(900 + j);
            hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
            ret = hal::add_l2seg_on_uplink(if_l2seg_spec, &if_l2seg_rsp);
            printf("ret: %d api_status: %d\n", ret, if_l2seg_rsp.api_status());
            ASSERT_TRUE(ret == HAL_RET_OK);
            hal::hal_cfg_db_close(false);

        }
    }

    // Update tenant
    ten_spec1.mutable_key_or_handle()->set_tenant_id(9);
    ten_spec1.set_security_profile_handle(nwsec_hdl1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::tenant_update(ten_spec1, &ten_rsp1);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
