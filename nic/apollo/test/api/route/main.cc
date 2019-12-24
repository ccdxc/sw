//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all route table test cases
///
//----------------------------------------------------------------------------

#include "nic/apollo/test/api/utils/nexthop.hpp"
#include "nic/apollo/test/api/utils/route.hpp"
#include "nic/apollo/test/api/utils/device.hpp"
#include "nic/apollo/test/api/utils/policy.hpp"
#include "nic/apollo/test/api/utils/tep.hpp"
#include "nic/apollo/test/api/utils/vpc.hpp"
#include "nic/apollo/test/api/utils/subnet.hpp"
#include "nic/apollo/test/api/utils/vnic.hpp"
#include "nic/apollo/test/api/utils/if.hpp"
#include "nic/apollo/test/api/utils/nexthop.hpp"
#include "nic/apollo/test/api/utils/nexthop_group.hpp"
#include "nic/apollo/test/api/utils/workflow.hpp"

namespace test {
namespace api {

// globals
static constexpr int k_max_v4_route_table = PDS_MAX_ROUTE_TABLE;
static constexpr int k_max_v6_route_table = PDS_MAX_ROUTE_TABLE;
static constexpr int k_max_route_per_tbl = PDS_MAX_ROUTE_PER_TABLE;

static const std::string k_base_v4_pfx  = "100.100.100.1/21";
static const std::string k_base_v4_pfx1 = "101.100.100.1/21";
static const std::string k_base_v6_pfx  = "100:100:100:1:1::1/65";
static const std::string k_base_v6_pfx1 = "101:100:100:1:1::1/65";

//----------------------------------------------------------------------------
// Route table test class
//----------------------------------------------------------------------------

class route_test : public pds_test_base {
protected:
    route_test() {}
    virtual ~route_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        if (!agent_mode())
            pds_test_base::SetUpTestCase(g_tc_params);
        pds_batch_ctxt_t bctxt = batch_start();
        sample1_vpc_setup(bctxt, PDS_VPC_TYPE_TENANT);
        if (apollo()) {
            // create max TEPs which can be used as NHs for routes
            sample_tep_setup(bctxt);
        } else if (apulu()) {
            // device and security policy config required for vnic
            sample_device_setup(bctxt);
            sample_policy_setup(bctxt);
            sample_if_setup(bctxt);
            sample_nexthop_setup(bctxt);
            sample_nexthop_group_setup(bctxt);
            sample_tep_setup(bctxt);
            sample_subnet_setup(bctxt);
            sample_vnic_setup(bctxt);
        } else {
            // create max NHs which can be used as NHs for routes
            sample_nexthop_setup(bctxt);
        }
        batch_commit(bctxt);
    }
    static void TearDownTestCase() {
        pds_batch_ctxt_t bctxt = batch_start();
        if (apollo()) {
            sample_tep_teardown(bctxt);
        } else if (apulu()) {
            sample_vnic_teardown(bctxt);
            sample_subnet_teardown(bctxt);
            sample_tep_teardown(bctxt);
            sample_nexthop_group_teardown(bctxt);
            sample_nexthop_teardown(bctxt);
            sample_if_teardown(bctxt);
            sample_policy_teardown(bctxt);
            sample_device_teardown(bctxt);
        } else {
            sample_nexthop_teardown(bctxt);
        }
        sample1_vpc_teardown(bctxt, PDS_VPC_TYPE_TENANT);
        batch_commit(bctxt);
        if (!agent_mode())
            pds_test_base::TearDownTestCase();
    }
};

//----------------------------------------------------------------------------
// Route table test cases implementation
//----------------------------------------------------------------------------

/// \defgroup ROUTE_TESTS Route table tests
/// @{

/// \brief Route table WF_B1
/// \ref WF_B1
TEST_F(route_test, v4v6_route_table_workflow_b1) {
    route_table_feeder feeder;

    // test one v4 route table with zero routes
    feeder.init(k_base_v4_pfx, IP_AF_IPV4, 0, 1);
    workflow_b1<route_table_feeder>(feeder);

    // test one v6 route table with zero routes
    feeder.init(k_base_v6_pfx, IP_AF_IPV6, 0, 1);
    workflow_b1<route_table_feeder>(feeder);

    // test one v4 route table with max routes
    feeder.init(k_base_v4_pfx, IP_AF_IPV4, k_max_route_per_tbl, 1);
    workflow_b1<route_table_feeder>(feeder);

    // test one v6 route tables with max routes
    feeder.init(k_base_v6_pfx, IP_AF_IPV6, k_max_route_per_tbl, 1);
    workflow_b1<route_table_feeder>(feeder);
}

/// \brief Route table WF_B2
/// \ref WF_B2
TEST_F(route_test, v4v6_route_table_workflow_b2) {
    route_table_feeder feeder1, feeder1A;

    if (!apulu()) return;

    // test one v4 route table with zero routes
    feeder1.init(k_base_v4_pfx, IP_AF_IPV4, 0, 1);
    feeder1A.init(k_base_v4_pfx1, IP_AF_IPV4, 0, 1);
    workflow_b2<route_table_feeder>(feeder1, feeder1A);

    // test one v6 route table with zero routes
    feeder1.init(k_base_v6_pfx, IP_AF_IPV6, 0, 1);
    feeder1A.init(k_base_v6_pfx1, IP_AF_IPV6, 0, 1);
    workflow_b2<route_table_feeder>(feeder1, feeder1A);

    // test one v4 route table with max routes
    feeder1.init(k_base_v4_pfx, IP_AF_IPV4, k_max_route_per_tbl, 1);
    feeder1A.init(k_base_v4_pfx1, IP_AF_IPV4, k_max_route_per_tbl, 1);
    workflow_b2<route_table_feeder>(feeder1, feeder1A);

    // test one v6 route tables with max routes
    feeder1.init(k_base_v6_pfx, IP_AF_IPV6, k_max_route_per_tbl, 1);
    feeder1A.init(k_base_v6_pfx1, IP_AF_IPV6, k_max_route_per_tbl, 1);
    workflow_b2<route_table_feeder>(feeder1, feeder1A);
}

/// \brief Route table WF_1
/// \ref WF_1
TEST_F(route_test, v4v6_route_table_workflow_1) {
    route_table_feeder feeder;

    // test max v4 route tables with zero routes
    feeder.init(k_base_v4_pfx, IP_AF_IPV4, 0, k_max_v4_route_table);
    workflow_1<route_table_feeder>(feeder);

    // test max v6 route tables with zero routes
    feeder.init(k_base_v6_pfx, IP_AF_IPV6, 0, k_max_v6_route_table);
    workflow_1<route_table_feeder>(feeder);

    // test max v4 route tables with max routes
    feeder.init(k_base_v4_pfx, IP_AF_IPV4,
                k_max_route_per_tbl, k_max_v4_route_table);
    workflow_1<route_table_feeder>(feeder);

    // test max v6 route tables with max routes
    feeder.init(k_base_v6_pfx, IP_AF_IPV6,
                k_max_route_per_tbl, k_max_v6_route_table);
    workflow_1<route_table_feeder>(feeder);
}

/// \brief Route table WF_2
/// \ref WF_2
TEST_F(route_test, v4v6_route_table_workflow_2) {
    route_table_feeder feeder;

    // test max v4 route tables with zero routes
    feeder.init(k_base_v4_pfx, IP_AF_IPV4, 0, k_max_v4_route_table);
    workflow_2<route_table_feeder>(feeder);

    // test max v6 route tables with zero routes
    feeder.init(k_base_v6_pfx, IP_AF_IPV6, 0, k_max_v6_route_table);
    workflow_2<route_table_feeder>(feeder);

    // test max v4 route tables with max routes
    feeder.init(k_base_v4_pfx, IP_AF_IPV4,
                k_max_route_per_tbl, k_max_v4_route_table);
    workflow_2<route_table_feeder>(feeder);

    // test max v6 route tables with max routes
    feeder.init(k_base_v6_pfx, IP_AF_IPV6,
                k_max_route_per_tbl, k_max_v6_route_table);
    workflow_2<route_table_feeder>(feeder);
}

/// \brief Route table WF_3
/// \ref WF_3
TEST_F(route_test, v4v6_route_table_workflow_3) {
    route_table_feeder feeder1, feeder2, feeder3;

    feeder1.init(k_base_v4_pfx, IP_AF_IPV4, 0, 50, 101);
    feeder2.init("125.100.100.1/21", IP_AF_IPV4, 0, 50, 201);
    feeder3.init("150.100.100.1/21", IP_AF_IPV4, 0, 50, 301);
    // test v4 route tables with zero routes
    workflow_3<route_table_feeder>(feeder1, feeder2, feeder3);

    feeder1.init(k_base_v6_pfx, IP_AF_IPV6, 0, 50, 151);
    feeder2.init("125:100:100:1:1::0/65",
                 IP_AF_IPV6, 0, 50, 251);
    feeder3.init("150:100:100:1:1::0/65",
                 IP_AF_IPV6, 0, 50, 351);
    // test v6 route tables with zero routes
    workflow_3<route_table_feeder>(feeder1, feeder2, feeder3);

    feeder1.init(k_base_v4_pfx, IP_AF_IPV4,
                 k_max_route_per_tbl, 50, 101);
    feeder2.init("125.100.100.1/21", IP_AF_IPV4,
                 k_max_route_per_tbl, 50, 201);
    feeder3.init("150.100.100.1/21", IP_AF_IPV4,
                 k_max_route_per_tbl, 50, 301);
    // test v4 route tables with max routes
    workflow_3<route_table_feeder>(feeder1, feeder2, feeder3);

    feeder1.init(k_base_v6_pfx, IP_AF_IPV6,
                 k_max_route_per_tbl, 50, 151);
    feeder2.init("125:100:100:1:1::0/65", IP_AF_IPV6,
                 k_max_route_per_tbl, 50, 251);
    feeder3.init("150:100:100:1:1::0/65", IP_AF_IPV6,
                 k_max_route_per_tbl, 50, 351);
    // test v6 route tables with max routes
    workflow_3<route_table_feeder>(feeder1, feeder2, feeder3);
}

/// \brief Route table WF_4
/// \ref WF_4
TEST_F(route_test, v4v6_route_table_workflow_4) {
    route_table_feeder feeder;

    // test max v4 route tables with zero routes
    feeder.init(k_base_v4_pfx, IP_AF_IPV4,
                0, k_max_v4_route_table);
    workflow_1<route_table_feeder>(feeder);
    // test max v6 route tables with zero routes
    feeder.init(k_base_v6_pfx, IP_AF_IPV6,
                0, k_max_v6_route_table);
    workflow_1<route_table_feeder>(feeder);
    // test max v4 route tables with max routes
    feeder.init(k_base_v4_pfx, IP_AF_IPV4,
                k_max_route_per_tbl, k_max_v4_route_table);
    workflow_1<route_table_feeder>(feeder);
    // test max v6 route tables with max routes
    feeder.init(k_base_v6_pfx, IP_AF_IPV6,
                k_max_route_per_tbl, k_max_v6_route_table);
    workflow_4<route_table_feeder>(feeder);
}

/// \brief Route table WF_5
/// \ref WF_5
TEST_F(route_test, v4v6_route_table_workflow_5) {
    route_table_feeder feeder1, feeder2, feeder3;

    feeder1.init(k_base_v4_pfx, IP_AF_IPV4, 0, 50, 101);
    feeder2.init("125.100.100.1/21", IP_AF_IPV4, 0, 50, 201);
    feeder3.init("150.100.100.1/21", IP_AF_IPV4, 0, 50, 301);
    // test v4 route tables with zero routes
    workflow_3<route_table_feeder>(feeder1, feeder2, feeder3);

    feeder1.init(k_base_v6_pfx, IP_AF_IPV6, 0, 50, 151);
    feeder2.init("125:100:100:1:1::0/65",
                 IP_AF_IPV6, 0, 50, 251);
    feeder3.init("150:100:100:1:1::0/65",
                 IP_AF_IPV6, 0, 50, 351);
    // test v6 route tables with zero routes
    workflow_3<route_table_feeder>(feeder1, feeder2, feeder3);

    feeder1.init(k_base_v4_pfx, IP_AF_IPV4,
                 k_max_route_per_tbl, 50, 101);
    feeder2.init("125.100.100.1/21", IP_AF_IPV4,
                 k_max_route_per_tbl, 50, 201);
    feeder3.init("150.100.100.1/21", IP_AF_IPV4,
                 k_max_route_per_tbl, 50, 301);
    // test v4 route tables with max routes
    workflow_3<route_table_feeder>(feeder1, feeder2, feeder3);

    feeder1.init(k_base_v6_pfx, IP_AF_IPV6,
                 k_max_route_per_tbl, 50, 151);
    feeder2.init("125:100:100:1:1::0/65", IP_AF_IPV6,
                 k_max_route_per_tbl, 50, 251);
    feeder3.init("150:100:100:1:1::0/65", IP_AF_IPV6,
                 k_max_route_per_tbl, 50, 351);
    // test v6 route tables with max routes
    workflow_5<route_table_feeder>(feeder1, feeder2, feeder3);
}

/// \brief Route table WF_6
/// \ref WF_6
TEST_F(route_test, v4v6_route_table_workflow_6) {
    route_table_feeder feeder1, feeder1A, feeder1B;

    feeder1.init(k_base_v4_pfx, IP_AF_IPV4,
                 k_max_route_per_tbl, k_max_v4_route_table);
    // feeder1A,1B = feeder1 with less routes per table
    feeder1A.init(k_base_v4_pfx, IP_AF_IPV4, 512,
                  k_max_v4_route_table);
    feeder1B.init(k_base_v4_pfx, IP_AF_IPV4, 100,
                  k_max_v4_route_table);
    workflow_6<route_table_feeder>(feeder1, feeder1A, feeder1B);

    feeder1.init(k_base_v6_pfx, IP_AF_IPV6,
                 k_max_route_per_tbl, k_max_v6_route_table);
    // feeder1A,1B = feeder1 with less routes per table
    feeder1A.init(k_base_v6_pfx, IP_AF_IPV6, 512,
                  k_max_v6_route_table);
    feeder1B.init(k_base_v6_pfx, IP_AF_IPV6, 100,
                  k_max_v6_route_table);
    workflow_6<route_table_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief Route table WF_7
/// \ref WF_7
TEST_F(route_test, v4v6_route_table_workflow_7) {
    route_table_feeder feeder1, feeder1A, feeder1B;

    feeder1.init(k_base_v4_pfx, IP_AF_IPV4,
                 k_max_route_per_tbl, k_max_v4_route_table);
    // feeder1A,1B = feeder1 with less routes per table
    feeder1A.init(k_base_v4_pfx, IP_AF_IPV4, 512,
                  k_max_v4_route_table);
    feeder1B.init(k_base_v4_pfx, IP_AF_IPV4, 100,
                  k_max_v4_route_table);
    workflow_7<route_table_feeder>(feeder1, feeder1A, feeder1B);

    feeder1.init(k_base_v6_pfx, IP_AF_IPV6,
                 k_max_route_per_tbl, k_max_v6_route_table);
    // feeder1A,1B = feeder1 with less routes per table
    feeder1A.init(k_base_v6_pfx, IP_AF_IPV6, 512,
                  k_max_v6_route_table);
    feeder1B.init(k_base_v6_pfx, IP_AF_IPV6, 100,
                  k_max_v6_route_table);
    workflow_7<route_table_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief Route table WF_8
/// \ref WF_8
/// NOTE: this test case is incorrect, we can't update more than 1 route table
///       in batch because of N+1 update scheme
TEST_F(route_test, DISABLED_v4v6_route_table_workflow_8) {
    route_table_feeder feeder1, feeder1A, feeder1B;

    feeder1.init(k_base_v4_pfx, IP_AF_IPV4,
                 k_max_route_per_tbl, k_max_v4_route_table);
    // feeder1A,1B = feeder1 with less routes per table
    feeder1A.init(k_base_v4_pfx, IP_AF_IPV4, 512,
                  k_max_v4_route_table);
    feeder1B.init(k_base_v4_pfx, IP_AF_IPV4, 100,
                  k_max_v4_route_table);
    workflow_8<route_table_feeder>(feeder1, feeder1A, feeder1B);

    feeder1.init(k_base_v6_pfx, IP_AF_IPV6,
                 k_max_route_per_tbl, k_max_v6_route_table);
    // feeder1A,1B = feeder1 with less routes per table
    feeder1A.init(k_base_v6_pfx, IP_AF_IPV6, 512,
                  k_max_v6_route_table);
    feeder1B.init(k_base_v6_pfx, IP_AF_IPV6, 100,
                  k_max_v6_route_table);
    workflow_8<route_table_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief Route table WF_9
/// \ref WF_9
/// NOTE: this test case is incorrect, we can't update more than 1 route table
///       in batch because of N+1 update scheme
TEST_F(route_test, DISABLED_v4v6_route_table_workflow_9) {
    route_table_feeder feeder1, feeder1A;

    feeder1.init(k_base_v4_pfx, IP_AF_IPV4,
                 k_max_route_per_tbl, k_max_v4_route_table);
    feeder1A.init(k_base_v4_pfx, IP_AF_IPV4, 512,
                  k_max_v4_route_table);
    workflow_9<route_table_feeder>(feeder1, feeder1A);

    feeder1.init(k_base_v6_pfx, IP_AF_IPV6,
                 k_max_route_per_tbl, k_max_v6_route_table);
    feeder1A.init(k_base_v6_pfx, IP_AF_IPV6, 512,
                  k_max_v6_route_table);
    workflow_9<route_table_feeder>(feeder1, feeder1A);
}

/// \brief Route table WF_10
/// \ref WF_10
TEST_F(route_test, DISABLED_v4v6_route_table_workflow_10) {
    route_table_feeder feeder1, feeder2, feeder2A, feeder3, feeder3A, feeder4;

    feeder1.init(k_base_v4_pfx, IP_AF_IPV4,
                 k_max_route_per_tbl, 50, 101);
    feeder2.init("125.100.100.1/21", IP_AF_IPV4,
                 512, 50, 201);
    feeder2A.init("125.100.100.1/21", IP_AF_IPV4,
                  k_max_route_per_tbl, 50, 201);
    feeder3.init("150.100.100.1/21", IP_AF_IPV4,
                 k_max_route_per_tbl, 50, 301);
    feeder3A.init("150.100.100.1/21", IP_AF_IPV4,
                  512, 50, 301);
    feeder4.init("175.100.100.1/21", IP_AF_IPV4,
                 k_max_route_per_tbl, 50, 401);
    workflow_10<route_table_feeder>(
                feeder1, feeder2, feeder2A, feeder3, feeder3A, feeder4);

    feeder1.init(k_base_v6_pfx, IP_AF_IPV6, k_max_route_per_tbl, 50, 151);
    feeder2.init("125:100:100:1:1::0/65", IP_AF_IPV6,
                 k_max_route_per_tbl, 50, 251);
    feeder2A.init("125:100:100:1:1::0/65", IP_AF_IPV6,
                  512, 50, 251);
    feeder3.init("150:100:100:1:1::0/65", IP_AF_IPV6,
                 k_max_route_per_tbl, 50, 351);
    feeder3A.init("150:100:100:1:1::0/65", IP_AF_IPV6,
                  512, 50, 351);
    feeder4.init("175:100:100:1:1::0/65", IP_AF_IPV6,
                 k_max_route_per_tbl, 50, 451);
    workflow_10<route_table_feeder>(
                feeder1, feeder2, feeder2A, feeder3, feeder3A, feeder4);
}

/// \brief Route table WF_N_1
/// \ref WF_N_1
TEST_F(route_test, v4v6_route_table_workflow_neg_1) {
    route_table_feeder feeder;

    // test max v4 route tables with zero routes
    feeder.init(k_base_v4_pfx, IP_AF_IPV4,
                0, k_max_v4_route_table);
    workflow_1<route_table_feeder>(feeder);
    // test max v6 route tables with zero routes
    feeder.init(k_base_v6_pfx, IP_AF_IPV6,
                0, k_max_v6_route_table);
    workflow_1<route_table_feeder>(feeder);
    // test max v4 route tables with max routes
    feeder.init(k_base_v4_pfx, IP_AF_IPV4,
                k_max_route_per_tbl, k_max_v4_route_table);
    workflow_1<route_table_feeder>(feeder);
    // test max v6 route tables with max routes
    feeder.init(k_base_v6_pfx, IP_AF_IPV6,
                k_max_route_per_tbl, k_max_v6_route_table);
    workflow_neg_1<route_table_feeder>(feeder);
}

/// \brief Route table WF_N_2
/// \ref WF_N_2
TEST_F(route_test, v4v6_route_table_workflow_neg_2) {
    route_table_feeder feeder;

    feeder.init(k_base_v4_pfx, IP_AF_IPV4,
                0, k_max_v4_route_table+2);
    // test MAX+1 v4 route tables with zero routes
    // using max + 2 as max+1 is reserved to handle update
    workflow_neg_2<route_table_feeder>(feeder);

    feeder.init(k_base_v6_pfx, IP_AF_IPV6,
                0, k_max_v6_route_table+2);
    // test MAX+1 v6 route tables with zero routes
    workflow_neg_2<route_table_feeder>(feeder);

    feeder.init(k_base_v4_pfx, IP_AF_IPV4,
                k_max_route_per_tbl, k_max_v4_route_table+2);
    // test MAX+1 v4 route tables with max routes
    workflow_neg_2<route_table_feeder>(feeder);

    feeder.init(k_base_v6_pfx, IP_AF_IPV6,
                k_max_route_per_tbl, k_max_v6_route_table+2);
    // test MAX+1 v6 route tables with max routes
    workflow_neg_2<route_table_feeder>(feeder);
#if 0
    // TODO: move this input validation testcase to agent test
    // as hal should not catch this and will only assert
    // trigger - test max v4 route tables with MAX+1 routes
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(k_max_v4_route_table,
        first_v4_route_table_id, k_first_v4_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV4, k_max_route_per_tbl+1) == SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_NO_RESOURCE);

    // trigger - test max v6 route tables with MAX+1 routes
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(k_max_v6_route_table,
        first_v6_route_table_id, k_first_v6_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV6, k_max_route_per_tbl+1) == SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_NO_RESOURCE);
#endif
}

/// \brief Route table WF_N_3
/// \ref WF_N_3
TEST_F(route_test, v4v6_route_table_workflow_neg_3) {
    route_table_feeder feeder;

    // test max v4 route tables with zero routes
    feeder.init(k_base_v4_pfx, IP_AF_IPV4,
                0, k_max_v4_route_table);
    workflow_1<route_table_feeder>(feeder);
    // test max v6 route tables with zero routes
    feeder.init(k_base_v6_pfx, IP_AF_IPV6,
                0, k_max_v6_route_table);
    workflow_1<route_table_feeder>(feeder);
    // test max v4 route tables with max routes
    feeder.init(k_base_v4_pfx, IP_AF_IPV4,
                k_max_route_per_tbl, k_max_v4_route_table);
    workflow_1<route_table_feeder>(feeder);
    // test max v6 route tables with max routes
    feeder.init(k_base_v6_pfx, IP_AF_IPV6,
                k_max_route_per_tbl, k_max_v6_route_table);
    workflow_neg_3<route_table_feeder>(feeder);
}

/// \brief Route table WF_N_4
/// \ref WF_N_4
TEST_F(route_test, v4v6_route_table_workflow_neg_4) {
    route_table_feeder feeder1, feeder2;

    feeder1.init(k_base_v4_pfx, IP_AF_IPV4, 0, 40, 100);
    feeder2.init(k_base_v4_pfx, IP_AF_IPV4, 0, 40, 300);
    // test v4 route tables with zero routes
    workflow_neg_4<route_table_feeder>(feeder1, feeder2);

    feeder1.init(k_base_v6_pfx, IP_AF_IPV6, 0, 40, 150);
    feeder2.init(k_base_v6_pfx, IP_AF_IPV6, 0, 40, 350);
    // test v6 route tables with zero routes
    workflow_neg_4<route_table_feeder>(feeder1, feeder2);

    feeder1.init(k_base_v4_pfx, IP_AF_IPV4,
                 k_max_route_per_tbl, 40, 100);
    feeder2.init(k_base_v4_pfx, IP_AF_IPV4,
                 k_max_route_per_tbl, 40, 300);
    // test v4 route tables with max routes
    workflow_neg_4<route_table_feeder>(feeder1, feeder2);

    feeder1.init(k_base_v6_pfx, IP_AF_IPV6,
                 k_max_route_per_tbl, 40, 150);
    feeder2.init(k_base_v6_pfx, IP_AF_IPV6,
                 k_max_route_per_tbl, 40, 350);
    // test v6 route tables with max routes
    workflow_neg_4<route_table_feeder>(feeder1, feeder2);
}

/// \brief Route table WF_N_5
/// \ref WF_N_5
TEST_F(route_test, DISABLED_v4v6_route_table_workflow_neg_5) {
    route_table_feeder feeder1, feeder1A;

    feeder1.init(k_base_v4_pfx, IP_AF_IPV4);
    feeder1A.init(k_base_v4_pfx, IP_AF_IPV4,
                  512, k_max_v4_route_table);
    // test max route tables with addition of max routes
    workflow_neg_5<route_table_feeder>(feeder1, feeder1A);

    feeder1.init(k_base_v6_pfx, IP_AF_IPV6);
    feeder1A.init(k_base_v6_pfx, IP_AF_IPV6,
                  512, k_max_v6_route_table);
    // test max route tables with addition of max routes
    workflow_neg_5<route_table_feeder>(feeder1, feeder1A);
}


/// \brief Route table WF_N_6
/// \ref WF_N_6
TEST_F(route_test, v4v6_route_table_workflow_neg_6) {
    route_table_feeder feeder1, feeder1A;

    feeder1.init(k_base_v4_pfx, IP_AF_IPV4);
    feeder1A.init(k_base_v4_pfx, IP_AF_IPV4,
                  512, k_max_v4_route_table);
    // test max route tables with addition of max routes
    workflow_neg_6<route_table_feeder>(feeder1, feeder1A);

    feeder1.init(k_base_v6_pfx, IP_AF_IPV6);
    feeder1A.init(k_base_v6_pfx, IP_AF_IPV6,
                  512, k_max_v6_route_table);
    // test max route tables with addition of max routes
    workflow_neg_6<route_table_feeder>(feeder1, feeder1A);
}

/// \brief Route table WF_N_7
/// \ref WF_N_7
TEST_F(route_test, v4v6_route_table_workflow_neg_7) {
    route_table_feeder feeder1, feeder1A, feeder2;

    feeder1.init(k_base_v4_pfx, IP_AF_IPV4,
                 k_max_route_per_tbl, 50, 101);
    feeder1A.init(k_base_v4_pfx, IP_AF_IPV4,
                  512, 50, 101);
    feeder2.init("125.100.100.1/21", IP_AF_IPV4,
                 k_max_route_per_tbl, 50, 201);
    workflow_neg_7<route_table_feeder>(feeder1, feeder1A, feeder2);

    feeder1.init(k_base_v6_pfx, IP_AF_IPV6,
                 k_max_route_per_tbl, 50, 151);
    feeder1A.init(k_base_v6_pfx, IP_AF_IPV6,
                  512, 50, 151);
    feeder2.init("125:100:100:1:1::0/65", IP_AF_IPV6,
                 k_max_route_per_tbl, 50, 251);
    workflow_neg_7<route_table_feeder>(feeder1, feeder1A, feeder2);
}

/// \brief Route table WF_N_8
/// \ref WF_N_8
TEST_F(route_test, v4v6_route_table_workflow_neg_8) {
    route_table_feeder feeder1, feeder2;

    feeder1.init(k_base_v4_pfx, IP_AF_IPV4,
                 k_max_route_per_tbl, 50, 101);
    feeder2.init("125.100.100.1/21", IP_AF_IPV4,
                 k_max_route_per_tbl, 50, 201);
    workflow_neg_8<route_table_feeder>(feeder1, feeder2);

    feeder1.init(k_base_v6_pfx, IP_AF_IPV6,
                 k_max_route_per_tbl, 50, 151);
    feeder2.init("125:100:100:1:1::0/65", IP_AF_IPV6,
                 k_max_route_per_tbl, 50, 251);
    workflow_neg_8<route_table_feeder>(feeder1, feeder2);
}

// TEST_F input validation with assert_death

/// @}

}    // namespace api
}    // namespace test

//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------

/// @private
int
main (int argc, char **argv)
{
    return api_test_program_run(argc, argv);
}
