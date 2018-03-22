//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include "nic/hal/src/gft/gft.hpp"
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



class emp_gft_test : public hal_base_test {
public:
  emp_gft_test() {
  }

  virtual ~emp_gft_test() {
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
// Creating a emp
// ----------------------------------------------------------------------------
TEST_F(emp_gft_test, test1) 
{
    hal_ret_t                           ret;
    GftExactMatchProfileSpec            spec;
    GftExactMatchProfileResponse        rsp;
    GftHeaderGroupExactMatchProfile     *emp;
    GftHeaders                          *headers;
    GftHeaderFields                     *hdr_fields;

    spec.set_table_type(gft::GFT_TABLE_TYPE_EXACT_MATCH_INGRESS);
    emp = spec.add_exact_match_profiles();
    headers = emp->mutable_headers();
    headers->set_ethernet_header(true);
    headers->set_ipv4_header(true);
    headers->set_tcp_header(true);

    hdr_fields = emp->mutable_match_fields();
    hdr_fields->set_tenant_id(true);
    hdr_fields->set_src_ip_addr(true);
    hdr_fields->set_dst_ip_addr(true);
    hdr_fields->set_src_port(true);
    hdr_fields->set_dst_port(true);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::gft_exact_match_profile_create(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
}

// ----------------------------------------------------------------------------
// Creating a emp for tX
// ----------------------------------------------------------------------------
TEST_F(emp_gft_test, test2) 
{
    hal_ret_t                           ret;
    GftExactMatchProfileSpec            spec;
    GftExactMatchProfileResponse        rsp;
    GftHeaderGroupExactMatchProfile     *emp;
    GftHeaders                          *headers;
    GftHeaderFields                     *hdr_fields;

    spec.set_table_type(gft::GFT_TABLE_TYPE_EXACT_MATCH_EGRESS);
    spec.mutable_key_or_handle()->set_profile_id(1);
    emp = spec.add_exact_match_profiles();
    headers = emp->mutable_headers();
    headers->set_ethernet_header(true);
    headers->set_ipv4_header(true);
    headers->set_tcp_header(true);

    hdr_fields = emp->mutable_match_fields();
    hdr_fields->set_tenant_id(true);
    hdr_fields->set_src_ip_addr(true);
    hdr_fields->set_dst_ip_addr(true);
    hdr_fields->set_src_port(true);
    hdr_fields->set_dst_port(true);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::gft_exact_match_profile_create(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
