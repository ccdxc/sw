//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all route table test cases
///
//----------------------------------------------------------------------------
#include <getopt.h>
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/test/utils/base.hpp"
#include "nic/apollo/test/utils/utils.hpp"
#include "nic/apollo/test/utils/batch.hpp"
#include "nic/apollo/test/utils/nh.hpp"
#include "nic/apollo/test/utils/route.hpp"
#include "nic/apollo/test/utils/tep.hpp"
#include "nic/apollo/test/utils/utils.hpp"
#include "nic/apollo/test/utils/vpc.hpp"
#include "nic/apollo/test/utils/workflow.hpp"

using std::cerr;
using std::cout;
using std::endl;

namespace api_test {

// Globals
static char *g_cfg_file = NULL;

static constexpr int k_max_v4_route_table = PDS_MAX_ROUTE_TABLE;
static constexpr int k_max_v6_route_table = PDS_MAX_ROUTE_TABLE;
static constexpr int k_max_route_per_tbl = PDS_MAX_ROUTE_PER_TABLE;
static constexpr pds_vpc_id_t k_vpc_id = 1;

static const char * const k_base_nh_ip = "30.30.30.1";
static const char * const k_base_v4_pfx = "100.100.100.1/21";
static const char * const k_base_v6_pfx = "100:100:100:1:1::1/65";

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
        test_case_params_t params;

        params.cfg_file = api_test::g_cfg_file;
        params.enable_fte = false;
        pds_test_base::SetUpTestCase(params);
        batch_start();
        sample_vpc_setup(PDS_VPC_TYPE_TENANT);
        if (apollo()) {
            // create max TEPs which can be used as NHs for routes
            tep_stepper_seed_t tep_seed = {0};

            TEP_SEED_INIT(&tep_seed, k_base_nh_ip);
            TEP_MANY_CREATE(&tep_seed);
        } else {
            // create max NHs which can be used as NHs for routes
            sample_nh_setup();
        }
        batch_commit();
    }
    static void TearDownTestCase() {
        batch_start();
        if (apollo()) {
            tep_stepper_seed_t tep_seed = {0};

            TEP_SEED_INIT(&tep_seed, k_base_nh_ip);
            TEP_MANY_DELETE(&tep_seed);
        } else {
            sample_nh_teardown();
        }
        sample_vpc_teardown(PDS_VPC_TYPE_TENANT);
        batch_commit();
        pds_test_base::TearDownTestCase();
    }
};

//----------------------------------------------------------------------------
// Route table test cases implementation
//----------------------------------------------------------------------------

/// \defgroup Route table
/// @{

/// \brief Route table WF_1
TEST_F(route_test, v4v6_route_table_workflow_1) {
    route_table_stepper_seed_t seed = {};

    ROUTE_TABLE_SEED_INIT(&seed, k_max_v4_route_table, 22222, k_base_v4_pfx,
                          k_base_nh_ip, IP_AF_IPV4, 0);
    ROUTE_TABLE_SEED_INIT(&seed, k_max_v6_route_table, 11111, k_base_v6_pfx,
                          k_base_nh_ip, IP_AF_IPV6, 0);
    // trigger - test max route tables with zero routes
    workflow_1<route_table_util, route_table_stepper_seed_t>(&seed);

    ROUTE_TABLE_SEED_INIT(&seed, k_max_v4_route_table, 22222, k_base_v4_pfx,
                          k_base_nh_ip, IP_AF_IPV4);
    ROUTE_TABLE_SEED_INIT(&seed, k_max_v6_route_table, 11111, k_base_v6_pfx,
                          k_base_nh_ip, IP_AF_IPV6);
    // trigger - test max route tables with max routes
    workflow_1<route_table_util, route_table_stepper_seed_t>(&seed);
}

/// \brief Route table WF_2
TEST_F(route_test, v4v6_route_table_workflow_2) {
    route_table_stepper_seed_t seed = {};

    ROUTE_TABLE_SEED_INIT(&seed, k_max_v4_route_table, 22222, k_base_v4_pfx,
                          k_base_nh_ip, IP_AF_IPV4, 0);
    ROUTE_TABLE_SEED_INIT(&seed, k_max_v6_route_table, 11111, k_base_v6_pfx,
                          k_base_nh_ip, IP_AF_IPV6, 0);
    // trigger - test max route tables with zero routes
    workflow_2<route_table_util, route_table_stepper_seed_t>(&seed);

    ROUTE_TABLE_SEED_INIT(&seed, k_max_v4_route_table, 22222, k_base_v4_pfx,
                          k_base_nh_ip, IP_AF_IPV4);
    ROUTE_TABLE_SEED_INIT(&seed, k_max_v6_route_table, 11111, k_base_v6_pfx,
                          k_base_nh_ip, IP_AF_IPV6);
    // trigger - test max route tables with max routes
    workflow_2<route_table_util, route_table_stepper_seed_t>(&seed);
}

/// \brief Route table WF_3
TEST_F(route_test, v4v6_route_table_workflow_3) {
    route_table_stepper_seed_t seed1 = {}, seed2 = {}, seed3 = {};

    ROUTE_TABLE_SEED_INIT(&seed1, 50, 101, k_base_v4_pfx, k_base_nh_ip,
                          IP_AF_IPV4, 0);
    ROUTE_TABLE_SEED_INIT(&seed1, 50, 151, k_base_v6_pfx, k_base_nh_ip,
                          IP_AF_IPV6, 0);
    ROUTE_TABLE_SEED_INIT(&seed2, 50, 201, "125.100.100.1/21", k_base_nh_ip,
                          IP_AF_IPV4, 0);
    ROUTE_TABLE_SEED_INIT(&seed2, 50, 251, "125:100:100:1:1::0/65",
                          k_base_nh_ip, IP_AF_IPV6, 0);
    ROUTE_TABLE_SEED_INIT(&seed3, 50, 301, "150.100.100.1/21", k_base_nh_ip,
                          IP_AF_IPV4, 0);
    ROUTE_TABLE_SEED_INIT(&seed3, 50, 351, "150:100:100:1:1::0/65",
                          k_base_nh_ip, IP_AF_IPV6, 0);
    // trigger - test route tables with zero routes
    workflow_3<route_table_util, route_table_stepper_seed_t>(&seed1, &seed2,
                                                             &seed3);

    ROUTE_TABLE_SEED_INIT(&seed1, 50, 101, k_base_v4_pfx, k_base_nh_ip,
                          IP_AF_IPV4);
    ROUTE_TABLE_SEED_INIT(&seed1, 50, 151, k_base_v6_pfx, k_base_nh_ip,
                          IP_AF_IPV6);
    ROUTE_TABLE_SEED_INIT(&seed2, 50, 201, "125.100.100.1/21", k_base_nh_ip,
                          IP_AF_IPV4);
    ROUTE_TABLE_SEED_INIT(&seed2, 50, 251, "125:100:100:1:1::0/65",
                          k_base_nh_ip, IP_AF_IPV6);
    ROUTE_TABLE_SEED_INIT(&seed3, 50, 301, "150.100.100.1/21", k_base_nh_ip,
                          IP_AF_IPV4);
    ROUTE_TABLE_SEED_INIT(&seed3, 50, 351, "150:100:100:1:1::0/65",
                          k_base_nh_ip, IP_AF_IPV6);
    // trigger - test route tables with max routes
    workflow_3<route_table_util, route_table_stepper_seed_t>(&seed1, &seed2,
                                                             &seed3);
}

/// \brief Route table WF_4
TEST_F(route_test, v4v6_route_table_workflow_4) {
    route_table_stepper_seed_t seed = {};

    ROUTE_TABLE_SEED_INIT(&seed, k_max_v4_route_table, 22222, k_base_v4_pfx,
                          k_base_nh_ip, IP_AF_IPV4, 0);
    ROUTE_TABLE_SEED_INIT(&seed, k_max_v6_route_table, 11111, k_base_v6_pfx,
                          k_base_nh_ip, IP_AF_IPV6, 0);
    // trigger - test max route tables with zero routes
    workflow_4<route_table_util, route_table_stepper_seed_t>(&seed);

    ROUTE_TABLE_SEED_INIT(&seed, k_max_v4_route_table, 22222, k_base_v4_pfx,
                          k_base_nh_ip, IP_AF_IPV4);
    ROUTE_TABLE_SEED_INIT(&seed, k_max_v6_route_table, 11111, k_base_v6_pfx,
                          k_base_nh_ip, IP_AF_IPV6);
    // trigger - test max route tables with max routes
    workflow_4<route_table_util, route_table_stepper_seed_t>(&seed);
}

/// \brief Route table WF_5
TEST_F(route_test, v4v6_route_table_workflow_5) {
    route_table_stepper_seed_t seed1 = {}, seed2 = {}, seed3 = {};

    ROUTE_TABLE_SEED_INIT(&seed1, 50, 101, k_base_v4_pfx, k_base_nh_ip,
                          IP_AF_IPV4, 0);
    ROUTE_TABLE_SEED_INIT(&seed1, 50, 151, k_base_v6_pfx, k_base_nh_ip,
                          IP_AF_IPV6, 0);
    ROUTE_TABLE_SEED_INIT(&seed2, 50, 201, "125.100.100.1/21", k_base_nh_ip,
                          IP_AF_IPV4, 0);
    ROUTE_TABLE_SEED_INIT(&seed2, 50, 251, "125:100:100:1:1::0/65",
                          k_base_nh_ip, IP_AF_IPV6, 0);
    ROUTE_TABLE_SEED_INIT(&seed3, 50, 301, "150.100.100.1/21", k_base_nh_ip,
                          IP_AF_IPV4, 0);
    ROUTE_TABLE_SEED_INIT(&seed3, 50, 351, "150:100:100:1:1::0/65",
                          k_base_nh_ip, IP_AF_IPV6, 0);
    // trigger - test route tables with zero routes
    workflow_5<route_table_util, route_table_stepper_seed_t>(&seed1, &seed2,
                                                             &seed3);

    ROUTE_TABLE_SEED_INIT(&seed1, 50, 101, k_base_v4_pfx, k_base_nh_ip,
                          IP_AF_IPV4);
    ROUTE_TABLE_SEED_INIT(&seed1, 50, 151, k_base_v6_pfx, k_base_nh_ip,
                          IP_AF_IPV6);
    ROUTE_TABLE_SEED_INIT(&seed2, 50, 201, "125.100.100.1/21", k_base_nh_ip,
                          IP_AF_IPV4);
    ROUTE_TABLE_SEED_INIT(&seed2, 50, 251, "125:100:100:1:1::0/65",
                          k_base_nh_ip, IP_AF_IPV6);
    ROUTE_TABLE_SEED_INIT(&seed3, 50, 301, "150.100.100.1/21", k_base_nh_ip,
                          IP_AF_IPV4);
    ROUTE_TABLE_SEED_INIT(&seed3, 50, 351, "150:100:100:1:1::0/65",
                          k_base_nh_ip, IP_AF_IPV6);
    // trigger - test route tables with max routes
    workflow_5<route_table_util, route_table_stepper_seed_t>(&seed1, &seed2,
                                                             &seed3);
}

/// \brief Route table WF_6
TEST_F(route_test, v4v6_route_table_workflow_6) {
    route_table_stepper_seed_t seed1 = {}, seed1A = {}, seed1B = {};

    ROUTE_TABLE_SEED_INIT(&seed1, k_max_v4_route_table, 22222, k_base_v4_pfx,
                          k_base_nh_ip, IP_AF_IPV4);
    ROUTE_TABLE_SEED_INIT(&seed1, k_max_v6_route_table, 11111, k_base_v6_pfx,
                          k_base_nh_ip, IP_AF_IPV6);
    // seed1A =  seed1 with vpc peering routes
    ROUTE_TABLE_SEED_INIT(&seed1A, k_max_v4_route_table, 22222, k_base_v4_pfx,
                          k_base_nh_ip, IP_AF_IPV4, k_max_route_per_tbl,
                          PDS_NH_TYPE_PEER_VPC, k_vpc_id);
    ROUTE_TABLE_SEED_INIT(&seed1A, k_max_v6_route_table, 11111, k_base_v6_pfx,
                          k_base_nh_ip, IP_AF_IPV6, k_max_route_per_tbl,
                          PDS_NH_TYPE_PEER_VPC, k_vpc_id);
    // seed1B =  seed1A with TEP routes but less scale
    ROUTE_TABLE_SEED_INIT(&seed1B, k_max_v4_route_table, 22222, k_base_v4_pfx,
                          k_base_nh_ip, IP_AF_IPV4, 100);
    ROUTE_TABLE_SEED_INIT(&seed1B, k_max_v6_route_table, 11111, k_base_v6_pfx,
                          k_base_nh_ip, IP_AF_IPV6, 100);
    workflow_6<route_table_util, route_table_stepper_seed_t>(&seed1, &seed1A,
                                                             &seed1B);
}

/// \brief Route table WF_7
TEST_F(route_test, v4v6_route_table_workflow_7) {
    route_table_stepper_seed_t seed1 = {}, seed1A = {}, seed1B = {};

    ROUTE_TABLE_SEED_INIT(&seed1, k_max_v4_route_table, 22222, k_base_v4_pfx,
                          k_base_nh_ip, IP_AF_IPV4);
    ROUTE_TABLE_SEED_INIT(&seed1, k_max_v6_route_table, 11111, k_base_v6_pfx,
                          k_base_nh_ip, IP_AF_IPV6);
    // seed1A =  seed1 with vpc peering routes
    ROUTE_TABLE_SEED_INIT(&seed1A, k_max_v4_route_table, 22222, k_base_v4_pfx,
                          k_base_nh_ip, IP_AF_IPV4, k_max_route_per_tbl,
                          PDS_NH_TYPE_PEER_VPC, k_vpc_id);
    ROUTE_TABLE_SEED_INIT(&seed1A, k_max_v6_route_table, 11111, k_base_v6_pfx,
                          k_base_nh_ip, IP_AF_IPV6, k_max_route_per_tbl,
                          PDS_NH_TYPE_PEER_VPC, k_vpc_id);
    // seed1B =  seed1 but less scale
    ROUTE_TABLE_SEED_INIT(&seed1B, k_max_v4_route_table, 22222, k_base_v4_pfx,
                          k_base_nh_ip, IP_AF_IPV4, 100);
    ROUTE_TABLE_SEED_INIT(&seed1B, k_max_v6_route_table, 11111, k_base_v6_pfx,
                          k_base_nh_ip, IP_AF_IPV6, 100);
    workflow_7<route_table_util, route_table_stepper_seed_t>(&seed1, &seed1A,
                                                             &seed1B);
}

/// \brief Route table WF_8
TEST_F(route_test, DISABLED_v4v6_route_table_workflow_8) {
    route_table_stepper_seed_t seed1 = {}, seed1A = {}, seed1B = {};

    ROUTE_TABLE_SEED_INIT(&seed1, k_max_v4_route_table, 22222, k_base_v4_pfx,
                          k_base_nh_ip, IP_AF_IPV4);
    ROUTE_TABLE_SEED_INIT(&seed1, k_max_v6_route_table, 11111, k_base_v6_pfx,
                          k_base_nh_ip, IP_AF_IPV6);
    // seed1A =  seed1 with vpc peering routes
    ROUTE_TABLE_SEED_INIT(&seed1A, k_max_v4_route_table, 22222, k_base_v4_pfx,
                          k_base_nh_ip, IP_AF_IPV4, k_max_route_per_tbl,
                          PDS_NH_TYPE_PEER_VPC, k_vpc_id);
    ROUTE_TABLE_SEED_INIT(&seed1A, k_max_v6_route_table, 11111, k_base_v6_pfx,
                          k_base_nh_ip, IP_AF_IPV6, k_max_route_per_tbl,
                          PDS_NH_TYPE_PEER_VPC, k_vpc_id);
    // seed1B =  seed1 but less scale
    ROUTE_TABLE_SEED_INIT(&seed1B, k_max_v4_route_table, 22222, k_base_v4_pfx,
                          k_base_nh_ip, IP_AF_IPV4, 100);
    ROUTE_TABLE_SEED_INIT(&seed1B, k_max_v6_route_table, 11111, k_base_v6_pfx,
                          k_base_nh_ip, IP_AF_IPV6, 100);
    workflow_8<route_table_util, route_table_stepper_seed_t>(&seed1, &seed1A,
                                                             &seed1B);
}

/// \brief Route table WF_9
TEST_F(route_test, v4v6_route_table_workflow_9) {
    route_table_stepper_seed_t seed1 = {}, seed1A = {};

    ROUTE_TABLE_SEED_INIT(&seed1, k_max_v4_route_table, 22222, k_base_v4_pfx,
                          k_base_nh_ip, IP_AF_IPV4);
    ROUTE_TABLE_SEED_INIT(&seed1, k_max_v6_route_table, 11111, k_base_v6_pfx,
                          k_base_nh_ip, IP_AF_IPV6);
    // seed1A =  seed1 with vpc peering routes
    ROUTE_TABLE_SEED_INIT(&seed1A, k_max_v4_route_table, 22222, k_base_v4_pfx,
                          k_base_nh_ip, IP_AF_IPV4, k_max_route_per_tbl,
                          PDS_NH_TYPE_PEER_VPC, k_vpc_id);
    ROUTE_TABLE_SEED_INIT(&seed1A, k_max_v6_route_table, 11111, k_base_v6_pfx,
                          k_base_nh_ip, IP_AF_IPV6, k_max_route_per_tbl,
                          PDS_NH_TYPE_PEER_VPC, k_vpc_id);
    workflow_9<route_table_util, route_table_stepper_seed_t>(&seed1, &seed1A);
}

/// \brief Route table WF_10
TEST_F(route_test, DISABLED_v4v6_route_table_workflow_10) {
    route_table_stepper_seed_t seed1 = {}, seed2 = {}, seed3 = {}, seed4 = {};
    route_table_stepper_seed_t seed2A = {}, seed3A = {};

    ROUTE_TABLE_SEED_INIT(&seed1, 50, 101, k_base_v4_pfx, k_base_nh_ip,
                          IP_AF_IPV4);
    ROUTE_TABLE_SEED_INIT(&seed1, 50, 151, k_base_v6_pfx, k_base_nh_ip,
                          IP_AF_IPV6);
    ROUTE_TABLE_SEED_INIT(&seed2, 50, 201, "125.100.100.1/21", k_base_nh_ip,
                          IP_AF_IPV4);
    ROUTE_TABLE_SEED_INIT(&seed2, 50, 251, "125:100:100:1:1::0/65",
                          k_base_nh_ip, IP_AF_IPV6);
    // seed2A =  seed2 with vpc peering routes
    ROUTE_TABLE_SEED_INIT(&seed2A, 50, 201, "125.100.100.1/21", k_base_nh_ip,
                          IP_AF_IPV4, k_max_route_per_tbl,
                          PDS_NH_TYPE_PEER_VPC, k_vpc_id);
    ROUTE_TABLE_SEED_INIT(&seed2A, 50, 251, "125:100:100:1:1::0/65",
                          k_base_nh_ip, IP_AF_IPV6, k_max_route_per_tbl,
                          PDS_NH_TYPE_PEER_VPC, k_vpc_id);
    ROUTE_TABLE_SEED_INIT(&seed3, 50, 301, "150.100.100.1/21", k_base_nh_ip,
                          IP_AF_IPV4);
    ROUTE_TABLE_SEED_INIT(&seed3, 50, 351, "150:100:100:1:1::0/65",
                          k_base_nh_ip, IP_AF_IPV6);
    // seed3A =  seed3 with vpc peering routes
    ROUTE_TABLE_SEED_INIT(&seed3A, 50, 301, "150.100.100.1/21", k_base_nh_ip,
                          IP_AF_IPV4, k_max_route_per_tbl,
                          PDS_NH_TYPE_PEER_VPC, k_vpc_id);
    ROUTE_TABLE_SEED_INIT(&seed3A, 50, 351, "150:100:100:1:1::0/65",
                          k_base_nh_ip, IP_AF_IPV6, k_max_route_per_tbl,
                          PDS_NH_TYPE_PEER_VPC, k_vpc_id);
    ROUTE_TABLE_SEED_INIT(&seed4, 50, 401, "175.100.100.1/21", k_base_nh_ip,
                          IP_AF_IPV4);
    ROUTE_TABLE_SEED_INIT(&seed4, 50, 451, "175:100:100:1:1::0/65",
                          k_base_nh_ip, IP_AF_IPV6);
    workflow_10<route_table_util, route_table_stepper_seed_t>(
                &seed1, &seed2, &seed2A, &seed3, &seed3A, &seed4);
}

/// \brief Route table WF_N_1
TEST_F(route_test, v4v6_route_table_workflow_neg_1) {
    route_table_stepper_seed_t seed = {};

    ROUTE_TABLE_SEED_INIT(&seed, k_max_v4_route_table, 22222, k_base_v4_pfx,
                          k_base_nh_ip, IP_AF_IPV4, 0);
    ROUTE_TABLE_SEED_INIT(&seed, k_max_v6_route_table, 11111, k_base_v6_pfx,
                          k_base_nh_ip, IP_AF_IPV6, 0);
    // trigger - test max route tables with zero routes
    workflow_neg_1<route_table_util, route_table_stepper_seed_t>(&seed);

    ROUTE_TABLE_SEED_INIT(&seed, k_max_v4_route_table, 22222, k_base_v4_pfx,
                          k_base_nh_ip, IP_AF_IPV4);
    ROUTE_TABLE_SEED_INIT(&seed, k_max_v6_route_table, 11111, k_base_v6_pfx,
                          k_base_nh_ip, IP_AF_IPV6);
    // trigger - test max route tables with max routes
    workflow_neg_1<route_table_util, route_table_stepper_seed_t>(&seed);
}

/// \brief Route table WF_N_2
TEST_F(route_test, v4v6_route_table_workflow_neg_2) {
    route_table_stepper_seed_t seed1 = {}, seed2 = {};

    ROUTE_TABLE_SEED_INIT(&seed1, k_max_v4_route_table+2, 22222, k_base_v4_pfx,
                          k_base_nh_ip, IP_AF_IPV4, 0);
    // trigger - test MAX+1 v4 route tables with zero routes
    // using max + 2 as max+1 is reserved to handle update
    workflow_neg_2<route_table_util, route_table_stepper_seed_t>(&seed1);

    ROUTE_TABLE_SEED_INIT(&seed2, k_max_v6_route_table+2, 11111, k_base_v6_pfx,
                          k_base_nh_ip, IP_AF_IPV6, 0);
    // trigger - test MAX+1 v6 route tables with zero routes
    workflow_neg_2<route_table_util, route_table_stepper_seed_t>(&seed2);

    ROUTE_TABLE_SEED_INIT(&seed1, k_max_v4_route_table+2, 22222, k_base_v4_pfx,
                          k_base_nh_ip, IP_AF_IPV4);
    // trigger - test MAX+1 v4 route tables with max routes
    workflow_neg_2<route_table_util, route_table_stepper_seed_t>(&seed1);

    ROUTE_TABLE_SEED_INIT(&seed2, k_max_v6_route_table+2, 11111, k_base_v6_pfx,
                          k_base_nh_ip, IP_AF_IPV6);
    // trigger - test MAX+1 v6 route tables with max routes
    workflow_neg_2<route_table_util, route_table_stepper_seed_t>(&seed2);
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
TEST_F(route_test, v4v6_route_table_workflow_neg_3) {
    route_table_stepper_seed_t seed = {};

    ROUTE_TABLE_SEED_INIT(&seed, k_max_v4_route_table, 22222, k_base_v4_pfx,
                          k_base_nh_ip, IP_AF_IPV4, 0);
    ROUTE_TABLE_SEED_INIT(&seed, k_max_v6_route_table, 11111, k_base_v6_pfx,
                          k_base_nh_ip, IP_AF_IPV6, 0);
    // trigger - test max route tables with zero routes
    workflow_neg_3<route_table_util, route_table_stepper_seed_t>(&seed);

    ROUTE_TABLE_SEED_INIT(&seed, k_max_v4_route_table, 22222, k_base_v4_pfx,
                          k_base_nh_ip, IP_AF_IPV4);
    ROUTE_TABLE_SEED_INIT(&seed, k_max_v6_route_table, 11111, k_base_v6_pfx,
                          k_base_nh_ip, IP_AF_IPV6);
    // trigger - test max route tables with max routes
    workflow_neg_3<route_table_util, route_table_stepper_seed_t>(&seed);
}

/// \brief Route table WF_N_4
TEST_F(route_test, v4v6_route_table_workflow_neg_4) {
    route_table_stepper_seed_t seed1 = {}, seed2 = {};

    ROUTE_TABLE_SEED_INIT(&seed1, 40, 100, k_base_v4_pfx, k_base_nh_ip,
                          IP_AF_IPV4, 0);
    ROUTE_TABLE_SEED_INIT(&seed1, 40, 150, k_base_v6_pfx, k_base_nh_ip,
                          IP_AF_IPV6, 0);
    ROUTE_TABLE_SEED_INIT(&seed2, 40, 300, k_base_v4_pfx, k_base_nh_ip,
                          IP_AF_IPV4, 0);
    ROUTE_TABLE_SEED_INIT(&seed2, 40, 350, k_base_v6_pfx, k_base_nh_ip,
                          IP_AF_IPV6, 0);
    // trigger - test route tables with zero routes
    workflow_neg_4<route_table_util, route_table_stepper_seed_t>(&seed1,
                                                                 &seed2);

    ROUTE_TABLE_SEED_INIT(&seed1, 40, 100, k_base_v4_pfx, k_base_nh_ip,
                          IP_AF_IPV4);
    ROUTE_TABLE_SEED_INIT(&seed1, 40, 150, k_base_v6_pfx, k_base_nh_ip,
                          IP_AF_IPV6);
    ROUTE_TABLE_SEED_INIT(&seed2, 40, 300, k_base_v4_pfx, k_base_nh_ip,
                          IP_AF_IPV4);
    ROUTE_TABLE_SEED_INIT(&seed2, 40, 350, k_base_v6_pfx, k_base_nh_ip,
                          IP_AF_IPV6);
    // trigger - test route tables with max routes
    workflow_neg_4<route_table_util, route_table_stepper_seed_t>(&seed1,
                                                                 &seed2);
}

/// \brief Route table WF_N_5
TEST_F(route_test, DISABLED_v4v6_route_table_workflow_neg_5) {
    route_table_stepper_seed_t seed1 = {}, seed1A = {};

    ROUTE_TABLE_SEED_INIT(&seed1, k_max_v4_route_table, 22222, k_base_v4_pfx,
                          k_base_nh_ip, IP_AF_IPV4);
    ROUTE_TABLE_SEED_INIT(&seed1, k_max_v6_route_table, 11111, k_base_v6_pfx,
                          k_base_nh_ip, IP_AF_IPV6);
    // seed1A = seed1 updated to vpc peering routes
    ROUTE_TABLE_SEED_INIT(&seed1A, k_max_v4_route_table, 22222, k_base_v4_pfx,
                          k_base_nh_ip, IP_AF_IPV4, k_max_route_per_tbl,
                          PDS_NH_TYPE_PEER_VPC, k_vpc_id);
    ROUTE_TABLE_SEED_INIT(&seed1A, k_max_v6_route_table, 11111, k_base_v6_pfx,
                          k_base_nh_ip, IP_AF_IPV6, k_max_route_per_tbl,
                          PDS_NH_TYPE_PEER_VPC, k_vpc_id);
    // trigger - test max route tables with addition of max routes
    workflow_neg_5<route_table_util, route_table_stepper_seed_t>(&seed1,
                                                                 &seed1A);
}


/// \brief Route table WF_N_6
TEST_F(route_test, v4v6_route_table_workflow_neg_6) {
    route_table_stepper_seed_t seed1 = {}, seed1A = {};

    ROUTE_TABLE_SEED_INIT(&seed1, k_max_v4_route_table, 22222, k_base_v4_pfx,
                          k_base_nh_ip, IP_AF_IPV4);
    ROUTE_TABLE_SEED_INIT(&seed1, k_max_v6_route_table, 11111, k_base_v6_pfx,
                          k_base_nh_ip, IP_AF_IPV6);
    // seed1A = seed1 updated to vpc peering routes
    ROUTE_TABLE_SEED_INIT(&seed1A, k_max_v4_route_table, 22222, k_base_v4_pfx,
                          k_base_nh_ip, IP_AF_IPV4, k_max_route_per_tbl,
                          PDS_NH_TYPE_PEER_VPC, k_vpc_id);
    ROUTE_TABLE_SEED_INIT(&seed1A, k_max_v6_route_table, 11111, k_base_v6_pfx,
                          k_base_nh_ip, IP_AF_IPV6, k_max_route_per_tbl,
                          PDS_NH_TYPE_PEER_VPC, k_vpc_id);
    workflow_neg_6<route_table_util, route_table_stepper_seed_t>(&seed1,
                                                                 &seed1A);
}

/// \brief Route table WF_N_7
TEST_F(route_test, v4v6_route_table_workflow_neg_7) {
    route_table_stepper_seed_t seed1 = {}, seed1A = {}, seed2 = {};

    ROUTE_TABLE_SEED_INIT(&seed1, 50, 101, k_base_v4_pfx, k_base_nh_ip,
                          IP_AF_IPV4);
    ROUTE_TABLE_SEED_INIT(&seed1, 50, 151, k_base_v6_pfx, k_base_nh_ip,
                          IP_AF_IPV6);
    ROUTE_TABLE_SEED_INIT(&seed1A, 50, 101, k_base_v4_pfx, k_base_nh_ip,
                          IP_AF_IPV4, k_max_route_per_tbl,
                          PDS_NH_TYPE_PEER_VPC, k_vpc_id);
    ROUTE_TABLE_SEED_INIT(&seed1A, 50, 151, k_base_v6_pfx, k_base_nh_ip,
                          IP_AF_IPV6, k_max_route_per_tbl,
                          PDS_NH_TYPE_PEER_VPC, k_vpc_id);
    ROUTE_TABLE_SEED_INIT(&seed2, 50, 201, "125.100.100.1/21", k_base_nh_ip,
                          IP_AF_IPV4);
    ROUTE_TABLE_SEED_INIT(&seed2, 50, 251, "125:100:100:1:1::0/65",
                          k_base_nh_ip, IP_AF_IPV6);
    workflow_neg_7<route_table_util, route_table_stepper_seed_t>(&seed1,
                                                                 &seed1A,
                                                                 &seed2);
}

/// \brief Route table WF_N_8
TEST_F(route_test, v4v6_route_table_workflow_neg_8) {
    route_table_stepper_seed_t seed1 = {}, seed2 = {};

    ROUTE_TABLE_SEED_INIT(&seed1, 50, 101, k_base_v4_pfx, k_base_nh_ip,
                          IP_AF_IPV4);
    ROUTE_TABLE_SEED_INIT(&seed1, 50, 151, k_base_v6_pfx, k_base_nh_ip,
                          IP_AF_IPV6);
    ROUTE_TABLE_SEED_INIT(&seed2, 50, 201, "125.100.100.1/21", k_base_nh_ip,
                          IP_AF_IPV4);
    ROUTE_TABLE_SEED_INIT(&seed2, 50, 251, "125:100:100:1:1::0/65",
                          k_base_nh_ip, IP_AF_IPV6);
    workflow_neg_8<route_table_util, route_table_stepper_seed_t>(&seed1,
                                                                 &seed2);
}

/// @}

}    // namespace api_test

//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------

static inline void
route_test_usage_print (char **argv)
{
    cout << "Usage : " << argv[0] << " -c <hal.json>" << endl;
    return;
}

static inline sdk_ret_t
route_test_options_validate (void)
{
    if (!api_test::g_cfg_file) {
        cerr << "HAL config file is not specified" << endl;
        return sdk::SDK_RET_ERR;
    }
    return sdk::SDK_RET_OK;
}

static void
route_test_options_parse (int argc, char **argv)
{
    int oc = -1;
    struct option longopts[] = {{"config", required_argument, NULL, 'c'},
                                {"help", no_argument, NULL, 'h'},
                                {0, 0, 0, 0}};

    while ((oc = getopt_long(argc, argv, ":hc:", longopts, NULL)) != -1) {
        switch (oc) {
        case 'c':
            api_test::g_cfg_file = optarg;
            break;
        default:    // ignore all other options
            break;
        }
    }
    return;
}

static void
route_test_init_parameters (void)
{
    // init default global parameters
    if (apollo()) {
        api_test::g_rt_def_nh_type = PDS_NH_TYPE_TEP;
    } else if (artemis()) {
        api_test::g_rt_def_nh_type = PDS_NH_TYPE_IP;
    }
}

int
main (int argc, char **argv)
{
    route_test_options_parse(argc, argv);
    if (route_test_options_validate() != sdk::SDK_RET_OK) {
        route_test_usage_print(argv);
        exit(1);
    }

    route_test_init_parameters();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
