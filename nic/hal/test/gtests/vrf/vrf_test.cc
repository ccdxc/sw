#include "nic/hal/plugins/cfg/nw/vrf.hpp"
#include "nic/hal/plugins/cfg/nw/nw.hpp"
#include "nic/hal/plugins/cfg/nw/nic.hpp"
#include "nic/hal/plugins/cfg/nw/interface.hpp"
#include "nic/hal/plugins/cfg/nw/l2segment.hpp"
#include "gen/proto/interface.pb.h"
#include "gen/proto/l2segment.pb.h"
#include "gen/proto/vrf.pb.h"
#include "gen/proto/nwsec.pb.h"
#include "gen/proto/nic.pb.h"
#include "nic/hal/hal.hpp"
#include "nic/hal/plugins/sfw/cfg/nwsec.hpp"
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include "nic/hal/test/utils/hal_test_utils.hpp"
#include "nic/hal/test/utils/hal_base_test.hpp"
#include "nic/hal/pd/pd_api.hpp"
#include "gen/hal/svc/session_svc_gen.hpp"

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
using intf::InterfaceL2SegmentSpec;
using intf::InterfaceL2SegmentResponse;
using intf::LifSpec;
using intf::LifResponse;
using kh::LifKeyHandle;
using l2segment::L2SegmentSpec;
using l2segment::L2SegmentResponse;
using vrf::VrfSpec;
using vrf::VrfResponse;
using vrf::VrfDeleteRequest;
using vrf::VrfDeleteResponse;
using vrf::VrfDeleteResponse;
using nwsec::SecurityProfileSpec;
using nwsec::SecurityProfileResponse;
using nw::NetworkSpec;
using nw::NetworkResponse;

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

uint16_t g_lif_id = 100; // Use LIFs in the host-lif range

class vrf_test : public hal_base_test {
protected:
  vrf_test() {
  }

  virtual ~vrf_test() {
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
// Vrf delete test
// ----------------------------------------------------------------------------
TEST_F(vrf_test, test1)
{
    hal_ret_t               ret;
    VrfSpec                 ten_spec;
    VrfResponse             ten_rsp;
    SecurityProfileSpec     sp_spec;
    SecurityProfileResponse sp_rsp;
    VrfDeleteRequest        del_req;
    VrfDeleteResponse       del_rsp;
    slab_stats_t            *pre      = NULL   , *post = NULL;
    bool                    is_leak   = false;
    hal::hal_obj_id_t       obj_id;
    void                    *obj;
    hal::vrf_t              *vrf = NULL;
    VrfGetResponseMsg       get_rsp_msg;
    VrfGetRequest           get_req;
    hal::pd::pd_get_object_from_flow_lkupid_args_t args;
    hal::pd::pd_func_args_t pd_func_args = {0};

    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(2);
    sp_spec.set_ipsg_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();

    pre = hal_test_utils_collect_slab_stats();

    // Create vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(1);
    ten_spec.mutable_security_key_handle()->set_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    args.flow_lkupid = 4096;
    args.obj_id = &obj_id;
    args.pi_obj = &obj;
    pd_func_args.pd_get_object_from_flow_lkupid = &args;
    // ret = hal::pd::pd_get_object_from_flow_lkupid(4096, &obj_id, &obj);
    ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_GET_OBJ_FROM_FLOW_LKPID, &pd_func_args);
    ASSERT_TRUE(ret == HAL_RET_OK);

    vrf = (hal::vrf_t *)obj;
    ASSERT_TRUE(vrf->vrf_id == 1);

    // Get vrf
    get_req.mutable_key_or_handle()->set_vrf_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_get(get_req, &get_rsp_msg);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);
    ASSERT_TRUE(get_rsp_msg.response(0).stats().num_l2_segments() == 0);
    ASSERT_TRUE(get_rsp_msg.response(0).stats().num_security_groups() == 0);
    ASSERT_TRUE(get_rsp_msg.response(0).stats().num_l4lb_services() == 0);
    ASSERT_TRUE(get_rsp_msg.response(0).stats().num_endpoints() == 0);

    // Delete vrf
    del_req.mutable_key_or_handle()->set_vrf_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    post = hal_test_utils_collect_slab_stats();
    hal_test_utils_check_slab_leak(pre, post, &is_leak);
    ASSERT_TRUE(is_leak == false);

    // Create  2 vrfs
    ten_spec.mutable_key_or_handle()->set_vrf_id(1);
    ten_spec.mutable_security_key_handle()->set_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    ten_spec.mutable_key_or_handle()->set_vrf_id(2);
    ten_spec.mutable_security_key_handle()->set_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Get vrf without setting key handle.
    get_req.clear_key_or_handle();
    get_rsp_msg.clear_response();
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_get(get_req, &get_rsp_msg);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    for (uint32_t i = 0; i < uint32_t(get_rsp_msg.response_size()); i++) {
        ASSERT_TRUE(get_rsp_msg.response(i).spec().key_or_handle().vrf_id() == (i+1));
        ASSERT_TRUE(get_rsp_msg.response(i).stats().num_l2_segments() == 0);
        ASSERT_TRUE(get_rsp_msg.response(i).stats().num_security_groups() == 0);
        ASSERT_TRUE(get_rsp_msg.response(i).stats().num_l4lb_services() == 0);
        ASSERT_TRUE(get_rsp_msg.response(i).stats().num_endpoints() == 0);
    }

    // Delete vrf 1
    del_req.mutable_key_or_handle()->set_vrf_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Delete vrf 2
    del_req.mutable_key_or_handle()->set_vrf_id(2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Uncomment these to have gtest work for CLI
    // svc_reg(std::string("0.0.0.0:") + std::string("50054"), hal::HAL_FEATURE_SET_IRIS);
    // hal::hal_wait();
}

// Update vrf test with enicifs
TEST_F(vrf_test, test2)
{
    hal_ret_t                       ret;
    VrfSpec                      ten_spec, ten_spec1;
    VrfResponse                  ten_rsp, ten_rsp1;
    SecurityProfileSpec             sp_spec, sp_spec1;
    SecurityProfileResponse         sp_rsp, sp_rsp1;
    // VrfDeleteRequest             del_req;
    // VrfDeleteResponse         del_rsp;
    LifSpec                         lif_spec;
    LifResponse                     lif_rsp;
    L2SegmentSpec                   l2seg_spec;
    L2SegmentResponse               l2seg_rsp;
    InterfaceSpec                   enicif_spec;
    InterfaceResponse               enicif_rsp;
    NetworkSpec                     nw_spec;
    NetworkResponse                 nw_rsp;
    NetworkKeyHandle                *nkh = NULL;
    hal::hal_obj_id_t       obj_id;
    void                    *obj;
    hal::l2seg_t                 *l2seg;
    hal::pd::pd_get_object_from_flow_lkupid_args_t args;
    hal::pd::pd_func_args_t pd_func_args = {0};
    DeviceRequest               nic_req;
    DeviceResponseMsg           nic_rsp;

    // Set device mode as Smart switch
    nic_req.mutable_device()->set_device_mode(device::DEVICE_MODE_MANAGED_SWITCH);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::device_create(&nic_req, &nic_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(3);
    sp_spec.set_ipsg_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();

    // Create nwsec
    sp_spec1.mutable_key_or_handle()->set_profile_id(21);
    sp_spec1.set_ipsg_en(false);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_create(sp_spec1, &sp_rsp1);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl1 = sp_rsp1.mutable_profile_status()->profile_handle();

    // Create vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(2);
    ten_spec.mutable_security_key_handle()->set_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

#if 0
    // Delete vrf
    del_req.mutable_vrf_key_handle()->set_vrf_id(1);
    del_req.mutable_key_or_handle()->set_vrf_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
#endif

    // Create network
    nw_spec.set_rmac(0x0000DEADBEEF);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->set_prefix_len(32);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0xa0000000);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_vrf_key_handle()->set_vrf_id(2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl = nw_rsp.mutable_status()->mutable_key_or_handle()->nw_handle();

    // Create a lif
    lif_spec.mutable_key_or_handle()->set_lif_id(g_lif_id);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create L2 Segment
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(2);
    nkh = l2seg_spec.add_network_key_handle();
    nkh->set_nw_handle(nw_hdl);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(21);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    args.flow_lkupid = 4097;
    args.obj_id = &obj_id;
    args.pi_obj = &obj;
    // ret = hal::pd::pd_get_object_from_flow_lkupid(4097, &obj_id, &obj);
    pd_func_args.pd_get_object_from_flow_lkupid = &args;
    ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_GET_OBJ_FROM_FLOW_LKPID, &pd_func_args);
    ASSERT_TRUE(ret == HAL_RET_OK);

    l2seg = (hal::l2seg_t *)obj;
    ASSERT_TRUE(l2seg->seg_id == 21);

    // Create enicif
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(g_lif_id++);
    enicif_spec.mutable_key_or_handle()->set_interface_id(21);
    enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(21);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(20);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Update vrf
    ten_spec1.mutable_key_or_handle()->set_vrf_id(2);
    ten_spec1.mutable_security_key_handle()->set_profile_handle(nwsec_hdl1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_update(ten_spec1, &ten_rsp1);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Set device mode as Smart switch
    nic_req.mutable_device()->set_device_mode(device::DEVICE_MODE_MANAGED_HOST_PIN);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::device_create(&nic_req, &nic_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

}

// ----------------------------------------------------------------------------
// Update vrf test with uplink ifs
// ----------------------------------------------------------------------------
TEST_F(vrf_test, test3)
{
    hal_ret_t                       ret;
    VrfSpec                      ten_spec, ten_spec1;
    VrfResponse                  ten_rsp, ten_rsp1;
    SecurityProfileSpec             sp_spec, sp_spec1;
    SecurityProfileResponse         sp_rsp, sp_rsp1;
    // VrfDeleteRequest             del_req;
    // VrfDeleteResponse         del_rsp;
    L2SegmentSpec                   l2seg_spec;
    L2SegmentResponse               l2seg_rsp;
    InterfaceSpec                   if_spec;
    InterfaceResponse               if_rsp;
    NetworkSpec                     nw_spec;
    NetworkResponse                 nw_rsp;
    InterfaceL2SegmentSpec          if_l2seg_spec;
    InterfaceL2SegmentResponse      if_l2seg_rsp;
    NetworkKeyHandle                *nkh = NULL;

    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(31);
    sp_spec.set_ipsg_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();

    // Create vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(3);
    ten_spec.mutable_security_key_handle()->set_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

#if 0
    // Delete vrf
    del_req.mutable_vrf_key_handle()->set_vrf_id(1);
    del_req.mutable_key_or_handle()->set_vrf_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
#endif

    // Create network
    nw_spec.set_rmac(0x0000DEADBEEF);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->set_prefix_len(32);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0xa0000000);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_vrf_key_handle()->set_vrf_id(3);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl = nw_rsp.mutable_status()->mutable_key_or_handle()->nw_handle();

    // Create Uplink If
    if_spec.set_type(intf::IF_TYPE_UPLINK);
    if_spec.mutable_key_or_handle()->set_interface_id(31);
    if_spec.mutable_if_uplink_info()->set_port_num(1);
    // if_spec.mutable_if_uplink_info()->set_native_l2segment_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create L2 Segment
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(3);
    nkh = l2seg_spec.add_network_key_handle();
    nkh->set_nw_handle(nw_hdl);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(31);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Adding L2segment on Uplink
    if_l2seg_spec.mutable_l2segment_key_or_handle()->set_segment_id(31);
    if_l2seg_spec.mutable_if_key_handle()->set_interface_id(31);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::add_l2seg_on_uplink(if_l2seg_spec, &if_l2seg_rsp);
    printf("ret: %d api_status: %d\n", ret, if_l2seg_rsp.api_status());
    ASSERT_TRUE(ret == HAL_RET_OK);
    hal::hal_cfg_db_close();
}

// ----------------------------------------------------------------------------
// Updating a vrf with no nwsec profile
// ----------------------------------------------------------------------------
TEST_F(vrf_test, test4)
{
    hal_ret_t                       ret;
    VrfSpec                      ten_spec, ten_spec1;
    VrfResponse                  ten_rsp, ten_rsp1;
    SecurityProfileSpec             sp_spec, sp_spec1;
    SecurityProfileResponse         sp_rsp, sp_rsp1;
    // VrfDeleteRequest             del_req;
    // VrfDeleteResponse         del_rsp;
    LifSpec                         lif_spec;
    LifResponse                     lif_rsp;
    L2SegmentSpec                   l2seg_spec;
    L2SegmentResponse               l2seg_rsp;
    InterfaceSpec                   enicif_spec;
    InterfaceResponse               enicif_rsp;
    NetworkSpec                     nw_spec;
    NetworkResponse                 nw_rsp;
    NetworkKeyHandle                *nkh = NULL;
    DeviceRequest               nic_req;
    DeviceResponseMsg           nic_rsp;

    // Set device mode as Smart switch
    nic_req.mutable_device()->set_device_mode(device::DEVICE_MODE_MANAGED_SWITCH);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::device_create(&nic_req, &nic_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(41);
    sp_spec.set_ipsg_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();

    // Create nwsec
    sp_spec1.mutable_key_or_handle()->set_profile_id(42);
    sp_spec1.set_ipsg_en(false);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_create(sp_spec1, &sp_rsp1);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    // uint64_t nwsec_hdl1 = sp_rsp1.mutable_profile_status()->profile_handle();

    // Create vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(4);
    ten_spec.mutable_security_key_handle()->set_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

#if 0
    // Delete vrf
    del_req.mutable_vrf_key_handle()->set_vrf_id(1);
    del_req.mutable_key_or_handle()->set_vrf_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
#endif

    // Create network
    nw_spec.set_rmac(0x0000DEADBEEF);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->set_prefix_len(32);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0xa0000000);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_vrf_key_handle()->set_vrf_id(4);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl = nw_rsp.mutable_status()->mutable_key_or_handle()->nw_handle();

    // Create a lif
    lif_spec.mutable_key_or_handle()->set_lif_id(g_lif_id);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create L2 Segment
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(4);
    nkh = l2seg_spec.add_network_key_handle();
    nkh->set_nw_handle(nw_hdl);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(41);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(40);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create enicif
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(g_lif_id++);
    enicif_spec.mutable_key_or_handle()->set_interface_id(41);
    enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(41);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(41);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Update vrf
    ten_spec1.mutable_key_or_handle()->set_vrf_id(4);
    // ten_spec1.mutable_security_key_handle()->set_profile_handle(nwsec_hdl1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_update(ten_spec1, &ten_rsp1);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Set device mode as Smart switch
    nic_req.mutable_device()->set_device_mode(device::DEVICE_MODE_MANAGED_HOST_PIN);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::device_create(&nic_req, &nic_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

}

// ----------------------------------------------------------------------------
// Updating a vrf without any change
// ----------------------------------------------------------------------------
TEST_F(vrf_test, test5)
{
    hal_ret_t                       ret;
    VrfSpec                      ten_spec, ten_spec1;
    VrfResponse                  ten_rsp, ten_rsp1;
    SecurityProfileSpec             sp_spec, sp_spec1;
    SecurityProfileResponse         sp_rsp, sp_rsp1;
    // VrfDeleteRequest             del_req;
    // VrfDeleteResponse         del_rsp;
    LifSpec                         lif_spec;
    LifResponse                     lif_rsp;
    L2SegmentSpec                   l2seg_spec;
    L2SegmentResponse               l2seg_rsp;
    InterfaceSpec                   enicif_spec;
    InterfaceResponse               enicif_rsp;
    NetworkSpec                     nw_spec;
    NetworkResponse                 nw_rsp;
    NetworkKeyHandle                *nkh = NULL;
    DeviceRequest               nic_req;
    DeviceResponseMsg           nic_rsp;

    // Set device mode as Smart switch
    nic_req.mutable_device()->set_device_mode(device::DEVICE_MODE_MANAGED_SWITCH);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::device_create(&nic_req, &nic_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(51);
    sp_spec.set_ipsg_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();

    // Create vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(5);
    ten_spec.mutable_security_key_handle()->set_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

#if 0
    // Delete vrf
    del_req.mutable_vrf_key_handle()->set_vrf_id(1);
    del_req.mutable_key_or_handle()->set_vrf_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
#endif

    // Create network
    nw_spec.set_rmac(0x0000DEADBEEF);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->set_prefix_len(32);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0xa0000000);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_vrf_key_handle()->set_vrf_id(5);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl = nw_rsp.mutable_status()->mutable_key_or_handle()->nw_handle();

    // Create a lif
    lif_spec.mutable_key_or_handle()->set_lif_id(g_lif_id);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create L2 Segment
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(5);
    nkh = l2seg_spec.add_network_key_handle();
    nkh->set_nw_handle(nw_hdl);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(51);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(50);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create enicif
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(g_lif_id++);
    enicif_spec.mutable_key_or_handle()->set_interface_id(51);
    enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(51);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(51);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Update vrf
    ten_spec1.mutable_key_or_handle()->set_vrf_id(5);
    ten_spec1.mutable_security_key_handle()->set_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_update(ten_spec1, &ten_rsp1);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Set device mode as Smart switch
    nic_req.mutable_device()->set_device_mode(device::DEVICE_MODE_MANAGED_HOST_PIN);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::device_create(&nic_req, &nic_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
}

// ----------------------------------------------------------------------------
// Vrf delete test with another create
// ----------------------------------------------------------------------------
TEST_F(vrf_test, test6)
{
    hal_ret_t                    ret;
    VrfSpec                      ten_spec, ten_spec1;
    VrfResponse                  ten_rsp, ten_rsp1;
    SecurityProfileSpec          sp_spec;
    SecurityProfileResponse      sp_rsp;
    VrfDeleteRequest             del_req;
    VrfDeleteResponse            del_rsp;

    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(61);
    sp_spec.set_ipsg_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();

    // Create vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(6);
    ten_spec.mutable_security_key_handle()->set_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Delete vrf
    del_req.mutable_key_or_handle()->set_vrf_id(6);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create vrf
    ten_spec1.mutable_key_or_handle()->set_vrf_id(6);
    ten_spec1.mutable_security_key_handle()->set_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec1, &ten_rsp1);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

}

// ----------------------------------------------------------------------------
// Vrf update test without l2segments
// ----------------------------------------------------------------------------
TEST_F(vrf_test, test7)
{
    hal_ret_t                       ret;
    VrfSpec                      ten_spec, ten_spec1;
    VrfResponse                  ten_rsp, ten_rsp1;
    SecurityProfileSpec             sp_spec, sp_spec1;
    SecurityProfileResponse         sp_rsp, sp_rsp1;
    VrfDeleteRequest             del_req;
    VrfDeleteResponse            del_rsp;
    // slab_stats_t                    *pre = NULL, *post = NULL;
    // bool                            is_leak = false;


    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(71);
    sp_spec.set_ipsg_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();

    // Create nwsec
    sp_spec1.mutable_key_or_handle()->set_profile_id(72);
    sp_spec1.set_ipsg_en(false);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_create(sp_spec1, &sp_rsp1);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl1 = sp_rsp1.mutable_profile_status()->profile_handle();

    // Create vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(7);
    ten_spec.mutable_security_key_handle()->set_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Update vrf
    ten_spec1.mutable_key_or_handle()->set_vrf_id(7);
    ten_spec1.mutable_security_key_handle()->set_profile_handle(nwsec_hdl1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_update(ten_spec1, &ten_rsp1);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
}

// Update vrf test with scale of segs and enicifs
TEST_F(vrf_test, test8)
{
    hal_ret_t                       ret;
    VrfSpec                      ten_spec, ten_spec1;
    VrfResponse                  ten_rsp, ten_rsp1;
    SecurityProfileSpec             sp_spec, sp_spec1;
    SecurityProfileResponse         sp_rsp, sp_rsp1;
    // VrfDeleteRequest             del_req;
    // VrfDeleteResponse         del_rsp;
    LifSpec                         lif_spec;
    LifResponse                     lif_rsp;
    L2SegmentSpec                   l2seg_spec;
    L2SegmentResponse               l2seg_rsp;
    InterfaceSpec                   enicif_spec;
    InterfaceResponse               enicif_rsp;
    NetworkSpec                     nw_spec;
    NetworkResponse                 nw_rsp;
    NetworkKeyHandle                *nkh = NULL;
    DeviceRequest               nic_req;
    DeviceResponseMsg           nic_rsp;

    // Set device mode as Smart switch
    nic_req.mutable_device()->set_device_mode(device::DEVICE_MODE_MANAGED_SWITCH);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::device_create(&nic_req, &nic_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(81);
    sp_spec.set_ipsg_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();

    // Create vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(8);
    ten_spec.mutable_security_key_handle()->set_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

#if 0
    // Delete vrf
    del_req.mutable_vrf_key_handle()->set_vrf_id(1);
    del_req.mutable_key_or_handle()->set_vrf_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
#endif

    // Create network
    nw_spec.set_rmac(0x0000DEADBEEF);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->set_prefix_len(32);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0xa0000000);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_vrf_key_handle()->set_vrf_id(8);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl = nw_rsp.mutable_status()->mutable_key_or_handle()->nw_handle();

    // Create a lif
    lif_spec.mutable_key_or_handle()->set_lif_id(g_lif_id);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create L2 Segment
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(8);
    nkh = l2seg_spec.add_network_key_handle();
    nkh->set_nw_handle(nw_hdl);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);

    for (int i = 0; i < 10; i++) {
        // Creating 10 l2segs
        l2seg_spec.mutable_key_or_handle()->set_segment_id(80 + i);
        l2seg_spec.mutable_wire_encap()->set_encap_value(80 + i);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

    // Create enicif
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(g_lif_id++);


    uint64_t mac = 0x0000DEADBEEF;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 5; j++) {
            enicif_spec.mutable_key_or_handle()->set_interface_id(80 + i*5 + j);
            enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
            enicif_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(80 + i);
            enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(800 + i*j + j);
            enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_mac_address(mac++);

            hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
            ret = hal::interface_create(enicif_spec, &enicif_rsp);
            hal::hal_cfg_db_close();
            ASSERT_TRUE(ret == HAL_RET_OK);

        }
    }

    // Set device mode as Smart switch
    nic_req.mutable_device()->set_device_mode(device::DEVICE_MODE_MANAGED_HOST_PIN);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::device_create(&nic_req, &nic_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
}

// ----------------------------------------------------------------------------
// Update vrf test with scale of uplink ifs
// ----------------------------------------------------------------------------
TEST_F(vrf_test, test9)
{
    hal_ret_t                       ret;
    VrfSpec                      ten_spec, ten_spec1;
    VrfResponse                  ten_rsp, ten_rsp1;
    SecurityProfileSpec             sp_spec, sp_spec1;
    SecurityProfileResponse         sp_rsp, sp_rsp1;
    SecurityProfileDeleteRequest    sp_del_req;
    SecurityProfileDeleteResponse   sp_del_rsp;
    VrfDeleteRequest             ten_del_req;
    VrfDeleteResponse            ten_del_rsp;
    LifSpec                         lif_spec;
    LifResponse                     lif_rsp;
    LifDeleteRequest                lif_del_req;
    LifDeleteResponse               lif_del_rsp;
    L2SegmentSpec                   l2seg_spec;
    L2SegmentResponse               l2seg_rsp;
    L2SegmentDeleteRequest          l2seg_del_req;
    L2SegmentDeleteResponse      l2seg_del_rsp;
    InterfaceSpec                   if_spec;
    InterfaceResponse               if_rsp;
    InterfaceDeleteRequest          enicif_del_req, up_del_req;
    InterfaceDeleteResponse         enicif_del_rsp, up_del_rsp;
    NetworkSpec                     nw_spec;
    NetworkResponse                 nw_rsp;
    NetworkDeleteRequest            nw_del_req;
    NetworkDeleteResponse           nw_del_rsp;
    InterfaceL2SegmentSpec          if_l2seg_spec;
    InterfaceL2SegmentResponse      if_l2seg_rsp;
    InterfaceSpec                   enicif_spec;
    InterfaceResponse               enicif_rsp;
    slab_stats_t                    *pre = NULL, *post = NULL;
    bool                            is_leak = false;
    NetworkKeyHandle                *nkh = NULL;
    DeviceRequest               nic_req;
    DeviceResponseMsg           nic_rsp;

    // Set device mode as Smart switch
    nic_req.mutable_device()->set_device_mode(device::DEVICE_MODE_MANAGED_SWITCH);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::device_create(&nic_req, &nic_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    pre = hal_test_utils_collect_slab_stats();

    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(91);
    sp_spec.set_ipsg_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();

    // Create another nwsec
    sp_spec1.mutable_key_or_handle()->set_profile_id(92);
    sp_spec1.set_ipsg_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_create(sp_spec1, &sp_rsp1);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl1 = sp_rsp1.mutable_profile_status()->profile_handle();

    // Create vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(9);
    ten_spec.mutable_security_key_handle()->set_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

#if 0
    // Delete vrf
    del_req.mutable_vrf_key_handle()->set_vrf_id(1);
    del_req.mutable_key_or_handle()->set_vrf_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
#endif

    // Create network
    nw_spec.set_rmac(0x0000DEADBEEF);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->set_prefix_len(32);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0xa0000000);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_vrf_key_handle()->set_vrf_id(9);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl = nw_rsp.mutable_status()->mutable_key_or_handle()->nw_handle();

    // Create a lif
    lif_spec.mutable_key_or_handle()->set_lif_id(g_lif_id);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create Uplink If
    if_spec.set_type(intf::IF_TYPE_UPLINK);
    for (int i = 0; i < 4; i++) {
        if_spec.mutable_key_or_handle()->set_interface_id(900 + i);
        if_spec.mutable_if_uplink_info()->set_port_num(1);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::interface_create(if_spec, &if_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

    // Create L2 Segment
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(9);
    nkh = l2seg_spec.add_network_key_handle();
    nkh->set_nw_handle(nw_hdl);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    for (int i = 0; i < 10; i++) {
        // Creating 10 l2segs
        l2seg_spec.mutable_key_or_handle()->set_segment_id(90 + i);
        l2seg_spec.mutable_wire_encap()->set_encap_value(90 + i);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

    // Create enicif
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(g_lif_id);
    enicif_spec.mutable_key_or_handle()->set_interface_id(921);
    enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(90);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(91);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Adding L2segment on Uplink
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 4; j++) {
            if_l2seg_spec.mutable_l2segment_key_or_handle()->set_segment_id(90 + i);
            if_l2seg_spec.mutable_if_key_handle()->set_interface_id(900 + j);
            hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
            ret = hal::add_l2seg_on_uplink(if_l2seg_spec, &if_l2seg_rsp);
            printf("ret: %d api_status: %d\n", ret, if_l2seg_rsp.api_status());
            ASSERT_TRUE(ret == HAL_RET_OK);
            hal::hal_cfg_db_close();

        }
    }

    // Update nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(91);
    sp_spec.set_ipsg_en(false);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_update(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Update vrf
    ten_spec1.mutable_key_or_handle()->set_vrf_id(9);
    ten_spec1.mutable_security_key_handle()->set_profile_handle(nwsec_hdl1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_update(ten_spec1, &ten_rsp1);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

#if 0
    // Remove L2 segments - Errors out
    l2seg_del_req.mutable_vrf_key_handle()->set_vrf_id(9);
    for (int i = 0; i < 10; i++) {
        // Delete 10 l2segs
        l2seg_del_req.mutable_key_or_handle()->set_segment_id(90 + i);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::l2segment_delete(l2seg_del_req, &l2seg_del_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OBJECT_IN_USE);
    }
#endif

    // Delete l2segment on uplink
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 4; j++) {
            if_l2seg_spec.mutable_l2segment_key_or_handle()->set_segment_id(90 + i);
            if_l2seg_spec.mutable_if_key_handle()->set_interface_id(900 + j);
            hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
            ret = hal::del_l2seg_on_uplink(if_l2seg_spec, &if_l2seg_rsp);
            printf("ret: %d api_status: %d\n", ret, if_l2seg_rsp.api_status());
            ASSERT_TRUE(ret == HAL_RET_OK);
            hal::hal_cfg_db_close();

        }
    }
    // Remove lif, errors out
    lif_del_req.mutable_key_or_handle()->set_lif_id(g_lif_id);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_delete(lif_del_req, &lif_del_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OBJECT_IN_USE);

    // Remove enicif
    enicif_del_req.mutable_key_or_handle()->set_interface_id(921);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_delete(enicif_del_req, &enicif_del_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // TODO:Remove network. errors out

    // Remove vrf, errors out
    ten_del_req.mutable_key_or_handle()->set_vrf_id(9);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_delete(ten_del_req, &ten_del_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OBJECT_IN_USE);

    // Remove L2 segments
    l2seg_del_req.mutable_vrf_key_handle()->set_vrf_id(9);
    for (int i = 0; i < 10; i++) {
        // Delete 10 l2segs
        l2seg_del_req.mutable_key_or_handle()->set_segment_id(90 + i);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::l2segment_delete(l2seg_del_req, &l2seg_del_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
    }
    // Remove uplink
    // up_del_req.set_type(intf::IF_TYPE_UPLINK);
    for (int i = 0; i < 4; i++) {
        up_del_req.mutable_key_or_handle()->set_interface_id(900 + i);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::interface_delete(up_del_req, &up_del_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

    // Remove lif
    lif_del_req.mutable_key_or_handle()->set_lif_id(g_lif_id++);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_delete(lif_del_req, &lif_del_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Remove network
    nw_del_req.mutable_vrf_key_handle()->set_vrf_id(9);
    nw_del_req.mutable_key_or_handle()->set_nw_handle(nw_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_delete(nw_del_req, &nw_del_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Remove nwsec, errors out
    sp_del_req.mutable_key_or_handle()->set_profile_id(92);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_delete(sp_del_req, &sp_del_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OBJECT_IN_USE);

    // Remove vrf
    ten_del_req.mutable_key_or_handle()->set_vrf_id(9);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_delete(ten_del_req, &ten_del_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Remove both nwsecs
    sp_del_req.mutable_key_or_handle()->set_profile_id(91);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_delete(sp_del_req, &sp_del_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    sp_del_req.mutable_key_or_handle()->set_profile_id(92);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_delete(sp_del_req, &sp_del_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Memleak check
    post = hal_test_utils_collect_slab_stats();
    hal_test_utils_check_slab_leak(pre, post, &is_leak);
    ASSERT_TRUE(is_leak == false);

    // Set device mode as Smart switch
    nic_req.mutable_device()->set_device_mode(device::DEVICE_MODE_MANAGED_SWITCH);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::device_create(&nic_req, &nic_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
}

// ----------------------------------------------------------------------------
// Create Vrf
// Update nwsec
// ----------------------------------------------------------------------------
TEST_F(vrf_test, test10)
{
    hal_ret_t                           ret;
    VrfSpec                          ten_spec;
    VrfResponse                      ten_rsp;
    SecurityProfileSpec                 sp_spec;
    SecurityProfileResponse             sp_rsp;
    SecurityProfileDeleteRequest        sp_del_req;
    SecurityProfileDeleteResponse       sp_del_rsp;
    VrfDeleteRequest                 del_req;
    VrfDeleteResponse                del_rsp;
    slab_stats_t                        *pre = NULL, *post = NULL;
    bool                                is_leak = false;

    pre = hal_test_utils_collect_slab_stats();

    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(10);
    sp_spec.set_ipsg_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();


    // Create vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(10);
    ten_spec.mutable_security_key_handle()->set_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);


    // Try to delete nwsec
    sp_del_req.mutable_key_or_handle()->set_profile_id(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_delete(sp_del_req, &sp_del_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OBJECT_IN_USE);

    // Delete vrf
    del_req.mutable_key_or_handle()->set_vrf_id(10);
    del_req.mutable_key_or_handle()->set_vrf_id(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Delete nwsec
    sp_del_req.mutable_key_or_handle()->set_profile_id(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_delete(sp_del_req, &sp_del_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);


    post = hal_test_utils_collect_slab_stats();
    hal_test_utils_check_slab_leak(pre, post, &is_leak);
    ASSERT_TRUE(is_leak == false);

}

// Test Gipo prefix update
TEST_F(vrf_test, test12)
{
    hal_ret_t               ret;
    VrfSpec                 ten_spec, ten_spec1;
    VrfResponse             ten_rsp, ten_rsp1;
    SecurityProfileSpec     sp_spec;
    SecurityProfileResponse sp_rsp;
    VrfDeleteRequest        del_req;
    VrfDeleteResponse       del_rsp;
    slab_stats_t            *pre      = NULL   , *post = NULL;
    bool                    is_leak   = false;


    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(12);
    sp_spec.set_ipsg_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();

    pre = hal_test_utils_collect_slab_stats();

    // Create vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(12);
    ten_spec.mutable_security_key_handle()->set_profile_handle(nwsec_hdl);
    ten_spec.mutable_gipo_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);;
    ten_spec.mutable_gipo_prefix()->mutable_address()->set_v4_addr(0xa0000000);
    ten_spec.mutable_gipo_prefix()->set_prefix_len(24);
    ten_spec.set_vrf_type(types::VRF_TYPE_INFRA);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Update gipo: pfx len is 0, deprogram earlier gipo and dont program anything
    ten_spec1.mutable_key_or_handle()->set_vrf_id(12);
    ten_spec1.mutable_security_key_handle()->set_profile_handle(nwsec_hdl);
    ten_spec1.mutable_gipo_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);;
    ten_spec1.mutable_gipo_prefix()->mutable_address()->set_v4_addr(0xa0000001);
    ten_spec1.set_vrf_type(types::VRF_TYPE_INFRA);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_update(ten_spec1, &ten_rsp1);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Update gipo: pfx len is 0, deprogram earlier gipo and dont program anything
    ten_spec1.mutable_key_or_handle()->set_vrf_id(12);
    ten_spec1.mutable_security_key_handle()->set_profile_handle(nwsec_hdl);
    ten_spec1.mutable_gipo_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);;
    ten_spec1.mutable_gipo_prefix()->mutable_address()->set_v4_addr(0xa0000001);
    ten_spec1.mutable_gipo_prefix()->set_prefix_len(24);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_update(ten_spec1, &ten_rsp1);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Delete vrf
    del_req.mutable_key_or_handle()->set_vrf_id(12);
    del_req.mutable_key_or_handle()->set_vrf_id(12);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    post = hal_test_utils_collect_slab_stats();
    hal_test_utils_check_slab_leak(pre, post, &is_leak);
    ASSERT_TRUE(is_leak == false);
}

// ----------------------------------------------------------------------------
// Vrf -ve test cases
// ----------------------------------------------------------------------------
TEST_F(vrf_test, test11)
{
    hal_ret_t                       ret;
    VrfSpec                      ten_spec, ten_spec1, infra_spec;
    VrfResponse                  ten_rsp, ten_rsp1, infra_rsp;
    SecurityProfileSpec             sp_spec;
    SecurityProfileResponse         sp_rsp;
    VrfDeleteRequest             del_req;
    VrfDeleteResponse            del_rsp;
    // slab_stats_t                    *pre = NULL, *post = NULL;
    // bool                            is_leak = false;

    // pre = hal_test_utils_collect_slab_stats();

    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(11);
    sp_spec.set_ipsg_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();


    // Create vrf with wrong nwsec profile handle
    ten_spec.mutable_key_or_handle()->set_vrf_id(11);
    ten_spec.mutable_security_key_handle()->set_profile_handle(nwsec_hdl + 1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_SECURITY_PROFILE_NOT_FOUND);


    // Create vrf with no key
    ten_spec1.mutable_security_key_handle()->set_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec1, &ten_rsp1);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_INVALID_ARG);

    // Create vrf with handle and not key
    ten_spec.mutable_key_or_handle()->set_vrf_handle(1000);
    ten_spec.mutable_security_key_handle()->set_profile_handle(nwsec_hdl );
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_INVALID_ARG);

    // Create vrf with vrf id as invalid HAL_VRF_ID_INVALID
    ten_spec.mutable_key_or_handle()->set_vrf_id(HAL_VRF_ID_INVALID);
    ten_spec.mutable_security_key_handle()->set_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_INVALID_ARG);

    // Create vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(11);
    ten_spec.mutable_security_key_handle()->set_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create vrf which already exists
    ten_spec.mutable_key_or_handle()->set_vrf_id(11);
    ten_spec.mutable_security_key_handle()->set_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_ENTRY_EXISTS);

    // Create Infra vrf
    infra_spec.mutable_key_or_handle()->set_vrf_id(85);
    infra_spec.set_vrf_type(types::VRF_TYPE_INFRA);
    infra_spec.mutable_security_key_handle()->set_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(infra_spec, &infra_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create Infra Vrf again
    infra_spec.mutable_key_or_handle()->set_vrf_id(86);
    infra_spec.set_vrf_type(types::VRF_TYPE_INFRA);
    infra_spec.mutable_security_key_handle()->set_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(infra_spec, &infra_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_INVALID_ARG);

    // Create more than 256 vrfs. Will result in PD failure and create
    for (int i = 0; i < 254; i++) {
        ten_spec.mutable_key_or_handle()->set_vrf_id(1100 + i);
        ten_spec.mutable_security_key_handle()->set_profile_handle(nwsec_hdl);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::vrf_create(ten_spec, &ten_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK || ret == HAL_RET_NO_RESOURCE);
    }

    // Create vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(1100 + 255);
    ten_spec.mutable_security_key_handle()->set_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_NO_RESOURCE);

    // abort will be called

    // Update vrf with no key or handle
    // ten_spec.mutable_key_or_handle()->set_vrf_id(2);
    ten_spec1.mutable_security_key_handle()->set_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_update(ten_spec1, &ten_rsp1);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_INVALID_ARG);

    // Update vrf with hdle which doesnt exist
    ten_spec.mutable_key_or_handle()->set_vrf_handle(1000);
    ten_spec.mutable_security_key_handle()->set_profile_handle(nwsec_hdl );
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_INVALID_ARG);

    // Update vrf with nwsec handle which doesnt exist
    ten_spec.mutable_key_or_handle()->set_vrf_id(11);
    ten_spec.mutable_security_key_handle()->set_profile_handle(nwsec_hdl + 1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_update(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_SECURITY_PROFILE_NOT_FOUND);

    // Delete vrf with no key or handle
    del_req.mutable_key_or_handle()->set_vrf_id(111);
    // del_req.mutable_key_or_handle()->set_vrf_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_VRF_NOT_FOUND);

    // Delete vrf with hdle which doesnt exist
    del_req.mutable_key_or_handle()->set_vrf_id(11);
    del_req.mutable_key_or_handle()->set_vrf_handle(2000);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_VRF_NOT_FOUND);

#if 0
    post = hal_test_utils_collect_slab_stats();
    hal_test_utils_check_slab_leak(pre, post, &is_leak);
    ASSERT_TRUE(is_leak == false);
#endif

}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
