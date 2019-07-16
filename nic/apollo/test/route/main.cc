//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all route table test cases
///
//----------------------------------------------------------------------------

#include "nic/apollo/test/utils/nexthop.hpp"
#include "nic/apollo/test/utils/route.hpp"
#include "nic/apollo/test/utils/tep.hpp"
#include "nic/apollo/test/utils/vpc.hpp"
#include "nic/apollo/test/utils/workflow1.hpp"

namespace api_test {
/// \cond
static constexpr int k_max_v4_route_table = PDS_MAX_ROUTE_TABLE;
static constexpr int k_max_v6_route_table = PDS_MAX_ROUTE_TABLE;
static constexpr int k_max_route_per_tbl = PDS_MAX_ROUTE_PER_TABLE;
static constexpr pds_vpc_id_t k_vpc_id = 1;

static const std::string k_base_nh_ip = "30.30.30.1";
static const std::string k_base_v4_pfx = "100.100.100.1/21";
static const std::string k_base_v6_pfx = "100:100:100:1:1::1/65";

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
        batch_start();
        sample_vpc_setup(PDS_VPC_TYPE_TENANT);
        if (apollo()) {
            // create max TEPs which can be used as NHs for routes
            sample_tep_setup();
        } else {
            // create max NHs which can be used as NHs for routes
            sample_nexthop_setup();
        }
        batch_commit();
    }
    static void TearDownTestCase() {
        batch_start();
        if (apollo()) {
            sample_tep_teardown();
        } else {
            sample_nexthop_teardown();
        }
        sample_vpc_teardown(PDS_VPC_TYPE_TENANT);
        batch_commit();
        if (!agent_mode())
            pds_test_base::TearDownTestCase();
    }
};
/// \endcond
//----------------------------------------------------------------------------
// Route table test cases implementation
//----------------------------------------------------------------------------

/// \defgroup ROUTE_TESTS Route table tests
/// @{

/// \brief Route table WF_1
/// \ref WF_1
TEST_F(route_test, v4v6_route_table_workflow_1) {
    route_table_feeder feeder;

    // test max v4 route tables with zero routes
    feeder.init(k_base_v4_pfx, k_base_nh_ip, IP_AF_IPV4,
                0, k_max_v4_route_table);
    workflow_1<route_table_feeder>(feeder);
    // test max v6 route tables with zero routes
    feeder.init(k_base_v6_pfx, k_base_nh_ip, IP_AF_IPV6,
                0, k_max_v6_route_table);
    workflow_1<route_table_feeder>(feeder);
    // test max v4 route tables with max routes
    feeder.init(k_base_v4_pfx, k_base_nh_ip, IP_AF_IPV4,
                k_max_route_per_tbl, k_max_v4_route_table);
    workflow_1<route_table_feeder>(feeder);
    // test max v6 route tables with max routes
    feeder.init(k_base_v6_pfx, k_base_nh_ip, IP_AF_IPV6,
                k_max_route_per_tbl, k_max_v6_route_table);
    workflow_1<route_table_feeder>(feeder);
}

/// \brief Route table WF_2
/// \ref WF_2
TEST_F(route_test, v4v6_route_table_workflow_2) {
    route_table_feeder feeder;

    // test max v4 route tables with zero routes
    feeder.init(k_base_v4_pfx, k_base_nh_ip, IP_AF_IPV4,
                0, k_max_v4_route_table);
    workflow_2<route_table_feeder>(feeder);
    // test max v6 route tables with zero routes
    feeder.init(k_base_v6_pfx, k_base_nh_ip, IP_AF_IPV6,
                0, k_max_v6_route_table);
    workflow_2<route_table_feeder>(feeder);
    // test max v4 route tables with max routes
    feeder.init(k_base_v4_pfx, k_base_nh_ip, IP_AF_IPV4,
                k_max_route_per_tbl, k_max_v4_route_table);
    workflow_2<route_table_feeder>(feeder);
    // test max v6 route tables with max routes
    feeder.init(k_base_v6_pfx, k_base_nh_ip, IP_AF_IPV6,
                k_max_route_per_tbl, k_max_v6_route_table);
    workflow_2<route_table_feeder>(feeder);
}

/// \brief Route table WF_3
/// \ref WF_3
TEST_F(route_test, v4v6_route_table_workflow_3) {
    route_table_feeder feeder1, feeder2, feeder3;

    feeder1.init(k_base_v4_pfx, k_base_nh_ip, IP_AF_IPV4, 0, 50, 101);
    feeder2.init("125.100.100.1/21", k_base_nh_ip, IP_AF_IPV4, 0, 50, 201);
    feeder3.init("150.100.100.1/21", k_base_nh_ip, IP_AF_IPV4, 0, 50, 301);
    // test v4 route tables with zero routes
    workflow_3<route_table_feeder>(feeder1, feeder2, feeder3);

    feeder1.init(k_base_v6_pfx, k_base_nh_ip, IP_AF_IPV6, 0, 50, 151);
    feeder2.init("125:100:100:1:1::0/65", k_base_nh_ip,
                 IP_AF_IPV6, 0, 50, 251);
    feeder3.init("150:100:100:1:1::0/65", k_base_nh_ip,
                 IP_AF_IPV6, 0, 50, 351);
    // test v6 route tables with zero routes
    workflow_3<route_table_feeder>(feeder1, feeder2, feeder3);

    feeder1.init(k_base_v4_pfx, k_base_nh_ip, IP_AF_IPV4,
                 k_max_route_per_tbl, 50, 101);
    feeder2.init("125.100.100.1/21", k_base_nh_ip, IP_AF_IPV4,
                 k_max_route_per_tbl, 50, 201);
    feeder3.init("150.100.100.1/21", k_base_nh_ip, IP_AF_IPV4,
                 k_max_route_per_tbl, 50, 301);
    // test v4 route tables with max routes
    workflow_3<route_table_feeder>(feeder1, feeder2, feeder3);

    feeder1.init(k_base_v6_pfx, k_base_nh_ip, IP_AF_IPV6,
                 k_max_route_per_tbl, 50, 151);
    feeder2.init("125:100:100:1:1::0/65", k_base_nh_ip, IP_AF_IPV6,
                 k_max_route_per_tbl, 50, 251);
    feeder3.init("150:100:100:1:1::0/65", k_base_nh_ip, IP_AF_IPV6,
                 k_max_route_per_tbl, 50, 351);
    // test v6 route tables with max routes
    workflow_3<route_table_feeder>(feeder1, feeder2, feeder3);
}

/// \brief Route table WF_4
/// \ref WF_4
TEST_F(route_test, v4v6_route_table_workflow_4) {
    route_table_feeder feeder;

    // test max v4 route tables with zero routes
    feeder.init(k_base_v4_pfx, k_base_nh_ip, IP_AF_IPV4,
                0, k_max_v4_route_table);
    workflow_1<route_table_feeder>(feeder);
    // test max v6 route tables with zero routes
    feeder.init(k_base_v6_pfx, k_base_nh_ip, IP_AF_IPV6,
                0, k_max_v6_route_table);
    workflow_1<route_table_feeder>(feeder);
    // test max v4 route tables with max routes
    feeder.init(k_base_v4_pfx, k_base_nh_ip, IP_AF_IPV4,
                k_max_route_per_tbl, k_max_v4_route_table);
    workflow_1<route_table_feeder>(feeder);
    // test max v6 route tables with max routes
    feeder.init(k_base_v6_pfx, k_base_nh_ip, IP_AF_IPV6,
                k_max_route_per_tbl, k_max_v6_route_table);
    workflow_4<route_table_feeder>(feeder);
}

/// \brief Route table WF_5
/// \ref WF_5
TEST_F(route_test, v4v6_route_table_workflow_5) {
    route_table_feeder feeder1, feeder2, feeder3;

    feeder1.init(k_base_v4_pfx, k_base_nh_ip, IP_AF_IPV4, 0, 50, 101);
    feeder2.init("125.100.100.1/21", k_base_nh_ip, IP_AF_IPV4, 0, 50, 201);
    feeder3.init("150.100.100.1/21", k_base_nh_ip, IP_AF_IPV4, 0, 50, 301);
    // test v4 route tables with zero routes
    workflow_3<route_table_feeder>(feeder1, feeder2, feeder3);

    feeder1.init(k_base_v6_pfx, k_base_nh_ip, IP_AF_IPV6, 0, 50, 151);
    feeder2.init("125:100:100:1:1::0/65", k_base_nh_ip,
                 IP_AF_IPV6, 0, 50, 251);
    feeder3.init("150:100:100:1:1::0/65", k_base_nh_ip,
                 IP_AF_IPV6, 0, 50, 351);
    // test v6 route tables with zero routes
    workflow_3<route_table_feeder>(feeder1, feeder2, feeder3);

    feeder1.init(k_base_v4_pfx, k_base_nh_ip, IP_AF_IPV4,
                 k_max_route_per_tbl, 50, 101);
    feeder2.init("125.100.100.1/21", k_base_nh_ip, IP_AF_IPV4,
                 k_max_route_per_tbl, 50, 201);
    feeder3.init("150.100.100.1/21", k_base_nh_ip, IP_AF_IPV4,
                 k_max_route_per_tbl, 50, 301);
    // test v4 route tables with max routes
    workflow_3<route_table_feeder>(feeder1, feeder2, feeder3);

    feeder1.init(k_base_v6_pfx, k_base_nh_ip, IP_AF_IPV6,
                 k_max_route_per_tbl, 50, 151);
    feeder2.init("125:100:100:1:1::0/65", k_base_nh_ip, IP_AF_IPV6,
                 k_max_route_per_tbl, 50, 251);
    feeder3.init("150:100:100:1:1::0/65", k_base_nh_ip, IP_AF_IPV6,
                 k_max_route_per_tbl, 50, 351);
    // test v6 route tables with max routes
    workflow_5<route_table_feeder>(feeder1, feeder2, feeder3);
}

/// \brief Route table WF_6
/// \ref WF_6
TEST_F(route_test, v4v6_route_table_workflow_6) {
    route_table_feeder feeder1, feeder1A, feeder1B;

    feeder1.init(k_base_v4_pfx, k_base_nh_ip, IP_AF_IPV4,
                 k_max_route_per_tbl, k_max_v4_route_table);
    // feeder1A =  feeder1 with vpc peering routes
    feeder1A.init(k_base_v4_pfx, k_base_nh_ip, IP_AF_IPV4, k_max_route_per_tbl,
                  k_max_v4_route_table, 1, PDS_NH_TYPE_PEER_VPC, k_vpc_id);
    // feeder1B =  feeder1A with TEP routes but less scale
    feeder1B.init(k_base_v4_pfx, k_base_nh_ip, IP_AF_IPV4, 100,
                  k_max_v4_route_table);
    workflow_6<route_table_feeder>(feeder1, feeder1A, feeder1B);

    feeder1.init(k_base_v6_pfx, k_base_nh_ip, IP_AF_IPV6,
                 k_max_route_per_tbl, k_max_v6_route_table);
    // feeder1A =  feeder1 with vpc peering routes
    feeder1A.init(k_base_v6_pfx, k_base_nh_ip, IP_AF_IPV6, k_max_route_per_tbl,
                  k_max_v6_route_table, 1, PDS_NH_TYPE_PEER_VPC, k_vpc_id);
    // feeder1B =  feeder1A with TEP routes but less scale
    feeder1B.init(k_base_v6_pfx, k_base_nh_ip, IP_AF_IPV6, 100,
                  k_max_v6_route_table);
    workflow_6<route_table_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief Route table WF_7
/// \ref WF_7
TEST_F(route_test, v4v6_route_table_workflow_7) {
    route_table_feeder feeder1, feeder1A, feeder1B;

    feeder1.init(k_base_v4_pfx, k_base_nh_ip, IP_AF_IPV4,
                 k_max_route_per_tbl, k_max_v4_route_table);
    // feeder1A =  feeder1 with vpc peering routes
    feeder1A.init(k_base_v4_pfx, k_base_nh_ip, IP_AF_IPV4, k_max_route_per_tbl,
                  k_max_v4_route_table, 1, PDS_NH_TYPE_PEER_VPC, k_vpc_id);
    // feeder1B =  feeder1A with TEP routes but less scale
    feeder1B.init(k_base_v4_pfx, k_base_nh_ip, IP_AF_IPV4, 100,
                  k_max_v4_route_table);
    workflow_7<route_table_feeder>(feeder1, feeder1A, feeder1B);

    feeder1.init(k_base_v6_pfx, k_base_nh_ip, IP_AF_IPV6,
                 k_max_route_per_tbl, k_max_v6_route_table);
    // feeder1A =  feeder1 with vpc peering routes
    feeder1A.init(k_base_v6_pfx, k_base_nh_ip, IP_AF_IPV6, k_max_route_per_tbl,
                  k_max_v6_route_table, 1, PDS_NH_TYPE_PEER_VPC, k_vpc_id);
    // feeder1B =  feeder1A with TEP routes but less scale
    feeder1B.init(k_base_v6_pfx, k_base_nh_ip, IP_AF_IPV6, 100,
                  k_max_v6_route_table);
    workflow_7<route_table_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief Route table WF_8
/// \ref WF_8
TEST_F(route_test, DISABLED_v4v6_route_table_workflow_8) {
    route_table_feeder feeder1, feeder1A, feeder1B;

    feeder1.init(k_base_v4_pfx, k_base_nh_ip, IP_AF_IPV4,
                 k_max_route_per_tbl, k_max_v4_route_table);
    // feeder1A =  feeder1 with vpc peering routes
    feeder1A.init(k_base_v4_pfx, k_base_nh_ip, IP_AF_IPV4, k_max_route_per_tbl,
                  k_max_v4_route_table, 1, PDS_NH_TYPE_PEER_VPC, k_vpc_id);
    // feeder1B =  feeder1A with TEP routes but less scale
    feeder1B.init(k_base_v4_pfx, k_base_nh_ip, IP_AF_IPV4, 100,
                  k_max_v4_route_table);
    workflow_8<route_table_feeder>(feeder1, feeder1A, feeder1B);

    feeder1.init(k_base_v6_pfx, k_base_nh_ip, IP_AF_IPV6,
                 k_max_route_per_tbl, k_max_v6_route_table);
    // feeder1A =  feeder1 with vpc peering routes
    feeder1A.init(k_base_v6_pfx, k_base_nh_ip, IP_AF_IPV6, k_max_route_per_tbl,
                  k_max_v6_route_table, 1, PDS_NH_TYPE_PEER_VPC, k_vpc_id);
    // feeder1B =  feeder1A with TEP routes but less scale
    feeder1B.init(k_base_v6_pfx, k_base_nh_ip, IP_AF_IPV6, 100,
                  k_max_v6_route_table);
    workflow_8<route_table_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief Route table WF_9
/// \ref WF_9
TEST_F(route_test, v4v6_route_table_workflow_9) {
    route_table_feeder feeder1, feeder1A;

    feeder1.init(k_base_v4_pfx, k_base_nh_ip, IP_AF_IPV4,
                 k_max_route_per_tbl, k_max_v4_route_table);
    // feeder1A =  feeder1 with vpc peering routes
    feeder1A.init(k_base_v4_pfx, k_base_nh_ip, IP_AF_IPV4, k_max_route_per_tbl,
                  k_max_v4_route_table, 1, PDS_NH_TYPE_PEER_VPC, k_vpc_id);
    workflow_9<route_table_feeder>(feeder1, feeder1A);

    feeder1.init(k_base_v6_pfx, k_base_nh_ip, IP_AF_IPV6,
                 k_max_route_per_tbl, k_max_v6_route_table);
    // feeder1A =  feeder1 with vpc peering routes
    feeder1A.init(k_base_v6_pfx, k_base_nh_ip, IP_AF_IPV6, k_max_route_per_tbl,
                  k_max_v6_route_table, 1, PDS_NH_TYPE_PEER_VPC, k_vpc_id);
    workflow_9<route_table_feeder>(feeder1, feeder1A);
}

/// \brief Route table WF_10
/// \ref WF_10
TEST_F(route_test, DISABLED_v4v6_route_table_workflow_10) {
    route_table_feeder feeder1, feeder2, feeder2A, feeder3, feeder3A, feeder4;

    feeder1.init(k_base_v4_pfx, k_base_nh_ip,
                 IP_AF_IPV4, k_max_route_per_tbl, 50, 101);
    feeder2.init("125.100.100.1/21", k_base_nh_ip,
                 IP_AF_IPV4, k_max_route_per_tbl, 50, 201);
    // feeder2A =  feeder2 with vpc peering routes
    feeder2A.init("125.100.100.1/21", k_base_nh_ip, IP_AF_IPV4,
                  k_max_route_per_tbl, 50, 201,
                  PDS_NH_TYPE_PEER_VPC, k_vpc_id);
    feeder3.init("150.100.100.1/21", k_base_nh_ip,
                 IP_AF_IPV4, k_max_route_per_tbl, 50, 301);
    // feeder3A =  feeder3 with vpc peering routes
    feeder3A.init("150.100.100.1/21", k_base_nh_ip, IP_AF_IPV4,
                  k_max_route_per_tbl, 50, 301,
                  PDS_NH_TYPE_PEER_VPC, k_vpc_id);
    feeder4.init("175.100.100.1/21", k_base_nh_ip,
                 IP_AF_IPV4, k_max_route_per_tbl, 50, 401);
    workflow_10<route_table_feeder>(
                feeder1, feeder2, feeder2A, feeder3, feeder3A, feeder4);

    feeder1.init(k_base_v6_pfx, k_base_nh_ip,
                 IP_AF_IPV6, k_max_route_per_tbl, 50, 151);
    feeder2.init("125:100:100:1:1::0/65", k_base_nh_ip,
                 IP_AF_IPV6, k_max_route_per_tbl, 50, 251);
    // feeder2A =  feeder2 with vpc peering routes
    feeder2A.init("125:100:100:1:1::0/65", k_base_nh_ip, IP_AF_IPV6,
                  k_max_route_per_tbl, 50, 251,
                  PDS_NH_TYPE_PEER_VPC, k_vpc_id);
    feeder3.init("150:100:100:1:1::0/65", k_base_nh_ip,
                 IP_AF_IPV6, k_max_route_per_tbl, 50, 351);
    // feeder3A =  feeder3 with vpc peering routes
    feeder3A.init("150:100:100:1:1::0/65", k_base_nh_ip, IP_AF_IPV6,
                  k_max_route_per_tbl, 50, 351,
                  PDS_NH_TYPE_PEER_VPC, k_vpc_id);
    feeder4.init("175:100:100:1:1::0/65", k_base_nh_ip,
                 IP_AF_IPV6, k_max_route_per_tbl, 50, 451);
    workflow_10<route_table_feeder>(
                feeder1, feeder2, feeder2A, feeder3, feeder3A, feeder4);
}

/// \brief Route table WF_N_1
/// \ref WF_N_1
TEST_F(route_test, v4v6_route_table_workflow_neg_1) {
    route_table_feeder feeder;

    // test max v4 route tables with zero routes
    feeder.init(k_base_v4_pfx, k_base_nh_ip, IP_AF_IPV4,
                0, k_max_v4_route_table);
    workflow_1<route_table_feeder>(feeder);
    // test max v6 route tables with zero routes
    feeder.init(k_base_v6_pfx, k_base_nh_ip, IP_AF_IPV6,
                0, k_max_v6_route_table);
    workflow_1<route_table_feeder>(feeder);
    // test max v4 route tables with max routes
    feeder.init(k_base_v4_pfx, k_base_nh_ip, IP_AF_IPV4,
                k_max_route_per_tbl, k_max_v4_route_table);
    workflow_1<route_table_feeder>(feeder);
    // test max v6 route tables with max routes
    feeder.init(k_base_v6_pfx, k_base_nh_ip, IP_AF_IPV6,
                k_max_route_per_tbl, k_max_v6_route_table);
    workflow_neg_1<route_table_feeder>(feeder);
}

/// \brief Route table WF_N_2
/// \ref WF_N_2
TEST_F(route_test, v4v6_route_table_workflow_neg_2) {
    route_table_feeder feeder;

    feeder.init(k_base_v4_pfx, k_base_nh_ip, IP_AF_IPV4,
                0, k_max_v4_route_table+2);
    // test MAX+1 v4 route tables with zero routes
    // using max + 2 as max+1 is reserved to handle update
    workflow_neg_2<route_table_feeder>(feeder);

    feeder.init(k_base_v6_pfx, k_base_nh_ip, IP_AF_IPV6,
                0, k_max_v6_route_table+2);
    // test MAX+1 v6 route tables with zero routes
    workflow_neg_2<route_table_feeder>(feeder);

    feeder.init(k_base_v4_pfx, k_base_nh_ip, IP_AF_IPV4,
                k_max_route_per_tbl, k_max_v4_route_table+2);
    // test MAX+1 v4 route tables with max routes
    workflow_neg_2<route_table_feeder>(feeder);

    feeder.init(k_base_v6_pfx, k_base_nh_ip, IP_AF_IPV6,
                k_max_route_per_tbl, k_max_v6_route_table+2);
    // test MAX+1 v6 route tables with max routes
    workflow_neg_2<route_table_feeder>(feeder);
#if 0
    // TODO: move this input validation testcase to agent test
    // as hal should not catch this and will only assert
    // trigger - test max v4 route tables with MAX+1 routes
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(k_max_v4_route_table,
        first_v4_route_table_id, k_first_v4_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV4, k_max_route_per_tbl+1) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_NO_RESOURCE);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    // trigger - test max v6 route tables with MAX+1 routes
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(route_table_util::many_create(k_max_v6_route_table,
        first_v6_route_table_id, k_first_v6_pfx_str, k_first_nh_ip_str,
        IP_AF_IPV6, k_max_route_per_tbl+1) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_NO_RESOURCE);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);
#endif
}

/// \brief Route table WF_N_3
/// \ref WF_N_3
TEST_F(route_test, v4v6_route_table_workflow_neg_3) {
    route_table_feeder feeder;

    // test max v4 route tables with zero routes
    feeder.init(k_base_v4_pfx, k_base_nh_ip, IP_AF_IPV4,
                0, k_max_v4_route_table);
    workflow_1<route_table_feeder>(feeder);
    // test max v6 route tables with zero routes
    feeder.init(k_base_v6_pfx, k_base_nh_ip, IP_AF_IPV6,
                0, k_max_v6_route_table);
    workflow_1<route_table_feeder>(feeder);
    // test max v4 route tables with max routes
    feeder.init(k_base_v4_pfx, k_base_nh_ip, IP_AF_IPV4,
                k_max_route_per_tbl, k_max_v4_route_table);
    workflow_1<route_table_feeder>(feeder);
    // test max v6 route tables with max routes
    feeder.init(k_base_v6_pfx, k_base_nh_ip, IP_AF_IPV6,
                k_max_route_per_tbl, k_max_v6_route_table);
    workflow_neg_3<route_table_feeder>(feeder);
}

/// \brief Route table WF_N_4
/// \ref WF_N_4
TEST_F(route_test, v4v6_route_table_workflow_neg_4) {
    route_table_feeder feeder1, feeder2;

    feeder1.init(k_base_v4_pfx, k_base_nh_ip, IP_AF_IPV4, 0, 40, 100);
    feeder2.init(k_base_v4_pfx, k_base_nh_ip, IP_AF_IPV4, 0, 40, 300);
    // test v4 route tables with zero routes
    workflow_neg_4<route_table_feeder>(feeder1, feeder2);

    feeder1.init(k_base_v6_pfx, k_base_nh_ip, IP_AF_IPV6, 0, 40, 150);
    feeder2.init(k_base_v6_pfx, k_base_nh_ip, IP_AF_IPV6, 0, 40, 350);
    // test v6 route tables with zero routes
    workflow_neg_4<route_table_feeder>(feeder1, feeder2);

    feeder1.init(k_base_v4_pfx, k_base_nh_ip, IP_AF_IPV4,
                 k_max_route_per_tbl, 40, 100);
    feeder2.init(k_base_v4_pfx, k_base_nh_ip, IP_AF_IPV4,
                 k_max_route_per_tbl, 40, 300);
    // test v4 route tables with max routes
    workflow_neg_4<route_table_feeder>(feeder1, feeder2);

    feeder1.init(k_base_v6_pfx, k_base_nh_ip, IP_AF_IPV6,
                 k_max_route_per_tbl, 40, 150);
    feeder2.init(k_base_v6_pfx, k_base_nh_ip, IP_AF_IPV6,
                 k_max_route_per_tbl, 40, 350);
    // test v6 route tables with max routes
    workflow_neg_4<route_table_feeder>(feeder1, feeder2);
}

/// \brief Route table WF_N_5
/// \ref WF_N_5
TEST_F(route_test, DISABLED_v4v6_route_table_workflow_neg_5) {
    route_table_feeder feeder1, feeder1A;

    feeder1.init(k_base_v4_pfx, k_base_nh_ip, IP_AF_IPV4);
    // feeder1A = feeder1 updated to vpc peering routes
    feeder1A.init(k_base_v4_pfx, k_base_nh_ip, IP_AF_IPV4,
                  k_max_route_per_tbl, k_max_v4_route_table, 1,
                  PDS_NH_TYPE_PEER_VPC, k_vpc_id);
    // test max route tables with addition of max routes
    workflow_neg_5<route_table_feeder>(feeder1, feeder1A);

    feeder1.init(k_base_v6_pfx, k_base_nh_ip, IP_AF_IPV6);
    // feeder1A = feeder1 updated to vpc peering routes
    feeder1A.init(k_base_v6_pfx, k_base_nh_ip, IP_AF_IPV6,
                  k_max_route_per_tbl, k_max_v6_route_table, 1,
                  PDS_NH_TYPE_PEER_VPC, k_vpc_id);
    // test max route tables with addition of max routes
    workflow_neg_5<route_table_feeder>(feeder1, feeder1A);
}


/// \brief Route table WF_N_6
/// \ref WF_N_6
TEST_F(route_test, v4v6_route_table_workflow_neg_6) {
    route_table_feeder feeder1, feeder1A;

    feeder1.init(k_base_v4_pfx, k_base_nh_ip, IP_AF_IPV4);
    // feeder1A = feeder1 updated to vpc peering routes
    feeder1A.init(k_base_v4_pfx, k_base_nh_ip, IP_AF_IPV4,
                  k_max_route_per_tbl, k_max_v4_route_table, 1,
                  PDS_NH_TYPE_PEER_VPC, k_vpc_id);
    // test max route tables with addition of max routes
    workflow_neg_6<route_table_feeder>(feeder1, feeder1A);

    feeder1.init(k_base_v6_pfx, k_base_nh_ip, IP_AF_IPV6);
    // feeder1A = feeder1 updated to vpc peering routes
    feeder1A.init(k_base_v6_pfx, k_base_nh_ip, IP_AF_IPV6,
                  k_max_route_per_tbl, k_max_v6_route_table, 1,
                  PDS_NH_TYPE_PEER_VPC, k_vpc_id);
    // test max route tables with addition of max routes
    workflow_neg_6<route_table_feeder>(feeder1, feeder1A);
}

/// \brief Route table WF_N_7
/// \ref WF_N_7
TEST_F(route_test, v4v6_route_table_workflow_neg_7) {
    route_table_feeder feeder1, feeder1A, feeder2;

    feeder1.init(k_base_v4_pfx, k_base_nh_ip, IP_AF_IPV4,
                 k_max_route_per_tbl, 50, 101);
    feeder1A.init(k_base_v4_pfx, k_base_nh_ip, IP_AF_IPV4,
                  k_max_route_per_tbl, 50, 101,
                  PDS_NH_TYPE_PEER_VPC, k_vpc_id);
    feeder2.init("125.100.100.1/21", k_base_nh_ip, IP_AF_IPV4,
                 k_max_route_per_tbl, 50, 201);
    workflow_neg_7<route_table_feeder>(feeder1, feeder1A, feeder2);

    feeder1.init(k_base_v6_pfx, k_base_nh_ip, IP_AF_IPV6,
                 k_max_route_per_tbl, 50, 151);
    feeder1A.init(k_base_v6_pfx, k_base_nh_ip, IP_AF_IPV6,
                  k_max_route_per_tbl, 50, 151,
                  PDS_NH_TYPE_PEER_VPC, k_vpc_id);
    feeder2.init("125:100:100:1:1::0/65", k_base_nh_ip, IP_AF_IPV6,
                 k_max_route_per_tbl, 50, 251);
    workflow_neg_7<route_table_feeder>(feeder1, feeder1A, feeder2);
}

/// \brief Route table WF_N_8
/// \ref WF_N_8
TEST_F(route_test, v4v6_route_table_workflow_neg_8) {
    route_table_feeder feeder1, feeder2;

    feeder1.init(k_base_v4_pfx, k_base_nh_ip, IP_AF_IPV4,
                 k_max_route_per_tbl, 50, 101);
    feeder2.init("125.100.100.1/21", k_base_nh_ip, IP_AF_IPV4,
                 k_max_route_per_tbl, 50, 201);
    workflow_neg_8<route_table_feeder>(feeder1, feeder2);

    feeder1.init(k_base_v6_pfx, k_base_nh_ip, IP_AF_IPV6,
                 k_max_route_per_tbl, 50, 151);
    feeder2.init("125:100:100:1:1::0/65", k_base_nh_ip, IP_AF_IPV6,
                 k_max_route_per_tbl, 50, 251);
    workflow_neg_8<route_table_feeder>(feeder1, feeder2);
}

// TEST_F input validation with assert_death

/// @}

}    // namespace api_test

//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------

/// @private
int
main (int argc, char **argv)
{
    api_test_program_run(argc, argv);
}
