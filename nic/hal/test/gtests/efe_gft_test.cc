//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include "nic/hal/src/gft.hpp"
#include "nic/gen/proto/hal/gft.pb.h"
#include "nic/hal/hal.hpp"
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include "nic/hal/test/utils/hal_test_utils.hpp"
#include "nic/hal/test/utils/hal_base_test.hpp"

using gft::GftExactMatchProfileSpec;
using gft::GftExactMatchProfileResponse;
using gft::GftHeaderGroupExactMatchProfile;
using gft::GftHeaders;
using gft::GftHeaderFields;
using gft::GftExactMatchFlowEntrySpec;
using gft::GftExactMatchFlowEntryResponse;
using gft::GftHeaderGroupTransposition;
using gft::GftHeaderGroupExactMatch;



class efe_gft_test : public hal_base_test {
public:
  efe_gft_test() {
  }

  virtual ~efe_gft_test() {
  }

  // will be called immediately after the constructor before each test
  virtual void SetUp() {
  }

  // will be called immediately after each test before the destructor
  virtual void TearDown() {
  }

  // Will be called at the beginning of all test cases in this class
  static void SetUpTestCase() {
    hal_base_test::SetUpTestCase("hal_gft.json");
    hal_test_utils_slab_disable_delete();
  }

};

// ----------------------------------------------------------------------------
// Creating a efe
// ----------------------------------------------------------------------------
TEST_F(efe_gft_test, test1) 
{
    hal_ret_t                           ret;
    GftExactMatchFlowEntrySpec          spec;
    GftExactMatchFlowEntryResponse      rsp;
    GftHeaderGroupExactMatch            *gem;
    GftHeaderGroupTransposition         *gt;
    GftHeaders                          *headers, *xpos_hdrs;
    GftHeaderFields                     *hdr_fields, *xpos_fields;
    GftExactMatchProfileSpec            mp_spec;
    GftExactMatchProfileResponse        mp_rsp;
    GftHeaderGroupExactMatchProfile     *efe;
    GftHeaders                          *mp_headers;
    GftHeaderFields                     *mp_hdr_fields;


    // Match Profile
    mp_spec.set_table_type(gft::GFT_TABLE_TYPE_WILDCARD_INGRESS);
    mp_spec.mutable_key_or_handle()->set_profile_id(1);
    efe = mp_spec.add_exact_match_profiles();
    mp_headers = efe->mutable_headers();
    mp_headers->set_ethernet_header(true);
    mp_headers->set_ipv4_header(true);
    mp_headers->set_tcp_header(true);

    mp_hdr_fields = efe->mutable_match_fields();
    // mp_hdr_fields->set_tenant_id(true);
    mp_hdr_fields->set_src_ip_addr(true);
    mp_hdr_fields->set_dst_ip_addr(true);
    mp_hdr_fields->set_src_port(true);
    mp_hdr_fields->set_dst_port(true);
    mp_hdr_fields->set_dst_mac_addr(true);
    mp_hdr_fields->set_src_mac_addr(true);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::gft_exact_match_profile_create(mp_spec, &mp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);



    // Flow entry
    spec.mutable_exact_match_profile()->set_profile_id(1);
    gem = spec.add_exact_matches();
    gt = spec.add_transpositions();

    spec.mutable_key_or_handle()->set_flow_entry_id(1);

    // Exact Match Headers
    headers = gem->mutable_headers();
    hdr_fields = gem->mutable_match_fields();

    headers->set_ethernet_header(true);
    hdr_fields->set_dst_mac_addr(true);
    hdr_fields->set_src_mac_addr(true);
    gem->mutable_eth_fields()->set_dst_mac_addr(0xDEAD);
    gem->mutable_eth_fields()->set_src_mac_addr(0xBEEF);

    headers->set_ipv4_header(true);
    hdr_fields->set_src_ip_addr(true);
    hdr_fields->set_dst_ip_addr(true);
    gem->mutable_src_ip_addr()->set_ip_af(types::IP_AF_INET);
    gem->mutable_src_ip_addr()->set_v4_addr(0x0a000001);
    gem->mutable_dst_ip_addr()->set_ip_af(types::IP_AF_INET);
    gem->mutable_dst_ip_addr()->set_v4_addr(0x0a000002);

    headers->set_tcp_header(true);
    hdr_fields->set_src_port(true);
    hdr_fields->set_dst_port(true);
    gem->mutable_encap_or_transport()->mutable_tcp_fields()->set_sport(80);
    gem->mutable_encap_or_transport()->mutable_tcp_fields()->set_dport(8080);

    // Transpostions
    xpos_hdrs = gt->mutable_headers();
    xpos_fields = gt->mutable_header_fields();

    xpos_hdrs->set_ethernet_header(true);
    xpos_fields->set_dst_mac_addr(true);
    xpos_fields->set_src_mac_addr(true);
    gt->mutable_eth_fields()->set_dst_mac_addr(0xABCD);
    gt->mutable_eth_fields()->set_src_mac_addr(0xABCD);

    xpos_hdrs->set_ipv4_header(true);
    xpos_fields->set_src_ip_addr(true);
    xpos_fields->set_dst_ip_addr(true);
    gt->mutable_src_ip_addr()->set_ip_af(types::IP_AF_INET);
    gt->mutable_src_ip_addr()->set_v4_addr(0x0a000003);
    gt->mutable_dst_ip_addr()->set_ip_af(types::IP_AF_INET);
    gt->mutable_dst_ip_addr()->set_v4_addr(0x0a000004);

    xpos_hdrs->set_tcp_header(true);
    xpos_fields->set_src_port(true);
    xpos_fields->set_dst_port(true);
    gt->mutable_encap_or_transport()->mutable_tcp_fields()->set_sport(90);
    gt->mutable_encap_or_transport()->mutable_tcp_fields()->set_dport(9090);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::gft_exact_match_flow_entry_create(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);



#if 0
    hal_ret_t                           ret;
    GftExactMatchProfileSpec            spec;
    GftExactMatchProfileResponse        rsp;
    GftHeaderGroupExactMatchProfile     *efe;
    GftHeaders                          *headers;
    GftHeaderFields                     *hdr_fields;

    efe = spec.add_exact_match_profiles();
    headers = efe->mutable_headers();
    headers->set_ethernet_header(true);
    headers->set_ipv4_header(true);
    headers->set_tcp_header(true);

    hdr_fields = efe->mutable_match_fields();
    hdr_fields->set_tenant_id(true);
    hdr_fields->set_src_ip_addr(true);
    hdr_fields->set_dst_ip_addr(true);
    hdr_fields->set_src_port(true);
    hdr_fields->set_dst_port(true);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::gft_exact_match_profile_create(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
#endif
}

// ----------------------------------------------------------------------------
// Creating a efe TX
// ----------------------------------------------------------------------------
TEST_F(efe_gft_test, test2) 
{
    hal_ret_t                           ret;
    GftExactMatchFlowEntrySpec          spec;
    GftExactMatchFlowEntryResponse      rsp;
    GftHeaderGroupExactMatch            *gem;
    GftHeaderGroupTransposition         *gt;
    GftHeaders                          *headers, *xpos_hdrs;
    GftHeaderFields                     *hdr_fields, *xpos_fields;
    GftExactMatchProfileSpec            mp_spec;
    GftExactMatchProfileResponse        mp_rsp;
    GftHeaderGroupExactMatchProfile     *efe;
    GftHeaders                          *mp_headers;
    GftHeaderFields                     *mp_hdr_fields;


    // Match Profile
    mp_spec.set_table_type(gft::GFT_TABLE_TYPE_WILDCARD_EGRESS);
    mp_spec.mutable_key_or_handle()->set_profile_id(2);
    efe = mp_spec.add_exact_match_profiles();
    mp_headers = efe->mutable_headers();
    mp_headers->set_ethernet_header(true);
    mp_headers->set_ipv4_header(true);
    mp_headers->set_tcp_header(true);

    mp_hdr_fields = efe->mutable_match_fields();
    // mp_hdr_fields->set_tenant_id(true);
    mp_hdr_fields->set_src_ip_addr(true);
    mp_hdr_fields->set_dst_ip_addr(true);
    mp_hdr_fields->set_src_port(true);
    mp_hdr_fields->set_dst_port(true);
    mp_hdr_fields->set_dst_mac_addr(true);
    mp_hdr_fields->set_src_mac_addr(true);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::gft_exact_match_profile_create(mp_spec, &mp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);



    // Flow entry
    spec.mutable_exact_match_profile()->set_profile_id(2);
    gem = spec.add_exact_matches();
    gt = spec.add_transpositions();

    spec.mutable_key_or_handle()->set_flow_entry_id(2);

    // Exact Match Headers
    headers = gem->mutable_headers();
    hdr_fields = gem->mutable_match_fields();

    headers->set_ethernet_header(true);
    hdr_fields->set_dst_mac_addr(true);
    hdr_fields->set_src_mac_addr(true);
    gem->mutable_eth_fields()->set_dst_mac_addr(0xDEAD);
    gem->mutable_eth_fields()->set_src_mac_addr(0xBEEF);

    headers->set_ipv4_header(true);
    hdr_fields->set_src_ip_addr(true);
    hdr_fields->set_dst_ip_addr(true);
    gem->mutable_src_ip_addr()->set_ip_af(types::IP_AF_INET);
    gem->mutable_src_ip_addr()->set_v4_addr(0x0a000001);
    gem->mutable_dst_ip_addr()->set_ip_af(types::IP_AF_INET);
    gem->mutable_dst_ip_addr()->set_v4_addr(0x0a000002);

    headers->set_tcp_header(true);
    hdr_fields->set_src_port(true);
    hdr_fields->set_dst_port(true);
    gem->mutable_encap_or_transport()->mutable_tcp_fields()->set_sport(80);
    gem->mutable_encap_or_transport()->mutable_tcp_fields()->set_dport(8080);

    // Transpostions
    xpos_hdrs = gt->mutable_headers();
    xpos_fields = gt->mutable_header_fields();

    xpos_hdrs->set_ethernet_header(true);
    xpos_fields->set_dst_mac_addr(true);
    xpos_fields->set_src_mac_addr(true);
    gt->mutable_eth_fields()->set_dst_mac_addr(0xABCD);
    gt->mutable_eth_fields()->set_src_mac_addr(0xABCD);

    xpos_hdrs->set_ipv4_header(true);
    xpos_fields->set_src_ip_addr(true);
    xpos_fields->set_dst_ip_addr(true);
    gt->mutable_src_ip_addr()->set_ip_af(types::IP_AF_INET);
    gt->mutable_src_ip_addr()->set_v4_addr(0x0a000003);
    gt->mutable_dst_ip_addr()->set_ip_af(types::IP_AF_INET);
    gt->mutable_dst_ip_addr()->set_v4_addr(0x0a000004);

    xpos_hdrs->set_tcp_header(true);
    xpos_fields->set_src_port(true);
    xpos_fields->set_dst_port(true);
    gt->mutable_encap_or_transport()->mutable_tcp_fields()->set_sport(90);
    gt->mutable_encap_or_transport()->mutable_tcp_fields()->set_dport(9090);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::gft_exact_match_flow_entry_create(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
