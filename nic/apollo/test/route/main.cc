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
#include "nic/apollo/test/utils/batch.hpp"
#include "nic/apollo/test/utils/workflow.hpp"
#include "nic/apollo/test/utils/tep.hpp"
#include "nic/apollo/test/utils/route.hpp"

using std::cerr;
using std::cout;
using std::endl;

namespace api_test {

// Globals
char *g_cfg_file = NULL;

constexpr int k_max_v4_route_table = PDS_MAX_ROUTE_TABLE;
constexpr int k_max_v6_route_table = PDS_MAX_ROUTE_TABLE;
constexpr int k_max_route_per_tbl = PDS_MAX_ROUTE_PER_TABLE;

const char *k_first_nh_ip_str = "30.30.30.1";
const char *k_first_v4_pfx_str = "100.100.100.1/21";
const char *k_first_v6_pfx_str = "100:100:100:1::1/64";

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

        pds_encap_t encap = {PDS_ENCAP_TYPE_MPLSoUDP, 0};
        tep_stepper_seed_t tep_seed = {};

        TEP_SEED_INIT(PDS_MAX_TEP, k_first_nh_ip_str, PDS_TEP_TYPE_WORKLOAD,
                      encap, TRUE, &tep_seed);

        batch_start();
        // create max TEPs which can be used as NHs for routes
        TEP_MANY_CREATE(&tep_seed);
        batch_commit();
    }
    static void TearDownTestCase() {
        pds_encap_t encap = {PDS_ENCAP_TYPE_MPLSoUDP, 0};
        tep_stepper_seed_t tep_seed = {};

        TEP_SEED_INIT(PDS_MAX_TEP, k_first_nh_ip_str, PDS_TEP_TYPE_WORKLOAD,
                      encap, TRUE, &tep_seed);

        batch_start();
        TEP_MANY_DELETE(&tep_seed);
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

    route_table_util::route_table_stepper_seed_init(k_max_v4_route_table,
                                  22222, k_first_v4_pfx_str, k_first_nh_ip_str,
                                  IP_AF_IPV4, 0, &seed);
    route_table_util::route_table_stepper_seed_init(k_max_v6_route_table,
                                  11111, k_first_v6_pfx_str, k_first_nh_ip_str,
                                  IP_AF_IPV6, 0, &seed);
    // trigger - test max route tables with zero routes
    workflow_1<route_table_util, route_table_stepper_seed_t>(&seed);

    route_table_util::route_table_stepper_seed_init(k_max_v4_route_table,
                                  22222, k_first_v4_pfx_str, k_first_nh_ip_str,
                                  IP_AF_IPV4, k_max_route_per_tbl, &seed);
    route_table_util::route_table_stepper_seed_init(k_max_v6_route_table,
                                  11111, k_first_v6_pfx_str, k_first_nh_ip_str,
                                  IP_AF_IPV6, k_max_route_per_tbl, &seed);
    // trigger - test max route tables with max routes
    workflow_1<route_table_util, route_table_stepper_seed_t>(&seed);
}

/// \brief Route table WF_2
TEST_F(route_test, v4v6_route_table_workflow_2) {
    route_table_stepper_seed_t seed = {};

    route_table_util::route_table_stepper_seed_init(k_max_v4_route_table,
                                  22222, k_first_v4_pfx_str, k_first_nh_ip_str,
                                  IP_AF_IPV4, 0, &seed);
    route_table_util::route_table_stepper_seed_init(k_max_v6_route_table,
                                  11111, k_first_v6_pfx_str, k_first_nh_ip_str,
                                  IP_AF_IPV6, 0, &seed);
    // trigger - test max route tables with zero routes
    workflow_2<route_table_util, route_table_stepper_seed_t>(&seed);

    route_table_util::route_table_stepper_seed_init(k_max_v4_route_table,
                                  22222, k_first_v4_pfx_str, k_first_nh_ip_str,
                                  IP_AF_IPV4, k_max_route_per_tbl, &seed);
    route_table_util::route_table_stepper_seed_init(k_max_v6_route_table,
                                  11111, k_first_v6_pfx_str, k_first_nh_ip_str,
                                  IP_AF_IPV6, k_max_route_per_tbl, &seed);
    // trigger - test max route tables with max routes
    workflow_2<route_table_util, route_table_stepper_seed_t>(&seed);
}

/// \brief Route table WF_3
TEST_F(route_test, v4v6_route_table_workflow_3) {
    route_table_stepper_seed_t seed1 = {}, seed2 = {}, seed3 = {};

    route_table_util::route_table_stepper_seed_init(50, 101,
                                  k_first_v4_pfx_str, k_first_nh_ip_str,
                                  IP_AF_IPV4, 0, &seed1);
    route_table_util::route_table_stepper_seed_init(50, 151,
                                  k_first_v6_pfx_str, k_first_nh_ip_str,
                                  IP_AF_IPV6, 0, &seed1);
    route_table_util::route_table_stepper_seed_init(50, 201,
                                  "125.100.100.1/21", k_first_nh_ip_str,
                                  IP_AF_IPV4, 0, &seed2);
    route_table_util::route_table_stepper_seed_init(50, 251,
                                  "125:100:100:1::0/64", k_first_nh_ip_str,
                                  IP_AF_IPV6, 0, &seed2);
    route_table_util::route_table_stepper_seed_init(50, 301,
                                  "150.100.100.1/21", k_first_nh_ip_str,
                                  IP_AF_IPV4, 0, &seed3);
    route_table_util::route_table_stepper_seed_init(50, 351,
                                  "150:100:100:1::0/64", k_first_nh_ip_str,
                                  IP_AF_IPV6, 0, &seed3);
    // trigger - test route tables with zero routes
    workflow_3<route_table_util, route_table_stepper_seed_t>(&seed1, &seed2, &seed3);


    route_table_util::route_table_stepper_seed_init(50, 101,
                                  k_first_v4_pfx_str, k_first_nh_ip_str,
                                  IP_AF_IPV4, k_max_route_per_tbl, &seed1);
    route_table_util::route_table_stepper_seed_init(50, 151,
                                  k_first_v6_pfx_str, k_first_nh_ip_str,
                                  IP_AF_IPV6, k_max_route_per_tbl, &seed1);
    route_table_util::route_table_stepper_seed_init(50, 201,
                                  "125.100.100.1/21", k_first_nh_ip_str,
                                  IP_AF_IPV4, k_max_route_per_tbl, &seed2);
    route_table_util::route_table_stepper_seed_init(50, 251,
                                  "125:100:100:1::0/64", k_first_nh_ip_str,
                                  IP_AF_IPV6, k_max_route_per_tbl, &seed2);
    route_table_util::route_table_stepper_seed_init(50, 301,
                                  "150.100.100.1/21", k_first_nh_ip_str,
                                  IP_AF_IPV4, k_max_route_per_tbl, &seed3);
    route_table_util::route_table_stepper_seed_init(50, 351,
                                  "150:100:100:1::0/64", k_first_nh_ip_str,
                                  IP_AF_IPV6, k_max_route_per_tbl, &seed3);
    // trigger - test route tables with max routes
    workflow_3<route_table_util, route_table_stepper_seed_t>(&seed1, &seed2, &seed3);
}

/// \brief Route table WF_4
TEST_F(route_test, v4v6_route_table_workflow_4) {
    route_table_stepper_seed_t seed = {};

    route_table_util::route_table_stepper_seed_init(k_max_v4_route_table,
                                  22222, k_first_v4_pfx_str, k_first_nh_ip_str,
                                  IP_AF_IPV4, 0, &seed);
    route_table_util::route_table_stepper_seed_init(k_max_v6_route_table,
                                  11111, k_first_v6_pfx_str, k_first_nh_ip_str,
                                  IP_AF_IPV6, 0, &seed);
    // trigger - test max route tables with zero routes
    workflow_4<route_table_util, route_table_stepper_seed_t>(&seed);

    route_table_util::route_table_stepper_seed_init(k_max_v4_route_table,
                                  22222, k_first_v4_pfx_str, k_first_nh_ip_str,
                                  IP_AF_IPV4, k_max_route_per_tbl, &seed);
    route_table_util::route_table_stepper_seed_init(k_max_v6_route_table,
                                  11111, k_first_v6_pfx_str, k_first_nh_ip_str,
                                  IP_AF_IPV6, k_max_route_per_tbl, &seed);
    // trigger - test max route tables with max routes
    workflow_4<route_table_util, route_table_stepper_seed_t>(&seed);
}

/// \brief Route table WF_5
TEST_F(route_test, v4v6_route_table_workflow_5) {
    route_table_stepper_seed_t seed1 = {}, seed2 = {}, seed3 = {};

    route_table_util::route_table_stepper_seed_init(50, 101,
                                  k_first_v4_pfx_str, k_first_nh_ip_str,
                                  IP_AF_IPV4, 0, &seed1);
    route_table_util::route_table_stepper_seed_init(50, 151,
                                  k_first_v6_pfx_str, k_first_nh_ip_str,
                                  IP_AF_IPV6, 0, &seed1);
    route_table_util::route_table_stepper_seed_init(50, 201,
                                  "125.100.100.1/21", k_first_nh_ip_str,
                                  IP_AF_IPV4, 0, &seed2);
    route_table_util::route_table_stepper_seed_init(50, 251,
                                  "125:100:100:1::0/64", k_first_nh_ip_str,
                                  IP_AF_IPV6, 0, &seed2);
    route_table_util::route_table_stepper_seed_init(50, 301,
                                  "150.100.100.1/21", k_first_nh_ip_str,
                                  IP_AF_IPV4, 0, &seed3);
    route_table_util::route_table_stepper_seed_init(50, 351,
                                  "150:100:100:1::0/64", k_first_nh_ip_str,
                                  IP_AF_IPV6, 0, &seed3);
    // trigger - test route tables with zero routes
    workflow_5<route_table_util, route_table_stepper_seed_t>(&seed1, &seed2, &seed3);


    route_table_util::route_table_stepper_seed_init(50, 101,
                                  k_first_v4_pfx_str, k_first_nh_ip_str,
                                  IP_AF_IPV4, k_max_route_per_tbl, &seed1);
    route_table_util::route_table_stepper_seed_init(50, 151,
                                  k_first_v6_pfx_str, k_first_nh_ip_str,
                                  IP_AF_IPV6, k_max_route_per_tbl, &seed1);
    route_table_util::route_table_stepper_seed_init(50, 201,
                                  "125.100.100.1/21", k_first_nh_ip_str,
                                  IP_AF_IPV4, k_max_route_per_tbl, &seed2);
    route_table_util::route_table_stepper_seed_init(50, 251,
                                  "125:100:100:1::0/64", k_first_nh_ip_str,
                                  IP_AF_IPV6, k_max_route_per_tbl, &seed2);
    route_table_util::route_table_stepper_seed_init(50, 301,
                                  "150.100.100.1/21", k_first_nh_ip_str,
                                  IP_AF_IPV4, k_max_route_per_tbl, &seed3);
    route_table_util::route_table_stepper_seed_init(50, 351,
                                  "150:100:100:1::0/64", k_first_nh_ip_str,
                                  IP_AF_IPV6, k_max_route_per_tbl, &seed3);
    // trigger - test route tables with max routes
    workflow_5<route_table_util, route_table_stepper_seed_t>(&seed1, &seed2, &seed3);
}

/// \brief Route table WF_N_1
TEST_F(route_test, v4v6_route_table_workflow_neg_1) {
    route_table_stepper_seed_t seed = {};

    route_table_util::route_table_stepper_seed_init(k_max_v4_route_table,
                                  22222, k_first_v4_pfx_str, k_first_nh_ip_str,
                                  IP_AF_IPV4, 0, &seed);
    route_table_util::route_table_stepper_seed_init(k_max_v6_route_table,
                                  11111, k_first_v6_pfx_str, k_first_nh_ip_str,
                                  IP_AF_IPV6, 0, &seed);
    // trigger - test max route tables with zero routes
    workflow_neg_1<route_table_util, route_table_stepper_seed_t>(&seed);

    route_table_util::route_table_stepper_seed_init(k_max_v4_route_table,
                                  22222, k_first_v4_pfx_str, k_first_nh_ip_str,
                                  IP_AF_IPV4, k_max_route_per_tbl, &seed);
    route_table_util::route_table_stepper_seed_init(k_max_v6_route_table,
                                  11111, k_first_v6_pfx_str, k_first_nh_ip_str,
                                  IP_AF_IPV6, k_max_route_per_tbl, &seed);
    // trigger - test max route tables with max routes
    workflow_neg_1<route_table_util, route_table_stepper_seed_t>(&seed);
}

/// \brief Route table WF_N_2
TEST_F(route_test, v4v6_route_table_workflow_neg_2) {
    route_table_stepper_seed_t seed1 = {}, seed2 = {};

    route_table_util::route_table_stepper_seed_init(k_max_v4_route_table+2,
                                  22222, k_first_v4_pfx_str, k_first_nh_ip_str,
                                  IP_AF_IPV4, 0, &seed1);
    // trigger - test MAX+1 v4 route tables with zero routes
    // using max + 2 as max+1 is reserved to handle update
    workflow_neg_2<route_table_util, route_table_stepper_seed_t>(&seed1);

    route_table_util::route_table_stepper_seed_init(k_max_v6_route_table+2,
                                  11111, k_first_v6_pfx_str, k_first_nh_ip_str,
                                  IP_AF_IPV6, 0, &seed2);
    // trigger - test MAX+1 v6 route tables with zero routes
    workflow_neg_2<route_table_util, route_table_stepper_seed_t>(&seed2);

    route_table_util::route_table_stepper_seed_init(k_max_v4_route_table+2,
                                  22222, k_first_v4_pfx_str, k_first_nh_ip_str,
                                  IP_AF_IPV4, k_max_route_per_tbl, &seed1);
    // trigger - test MAX+1 v4 route tables with max routes
    workflow_neg_2<route_table_util, route_table_stepper_seed_t>(&seed1);

    route_table_util::route_table_stepper_seed_init(k_max_v6_route_table+2,
                                  11111, k_first_v6_pfx_str, k_first_nh_ip_str,
                                  IP_AF_IPV6, k_max_route_per_tbl, &seed2);
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

    route_table_util::route_table_stepper_seed_init(k_max_v4_route_table,
                                  22222, k_first_v4_pfx_str, k_first_nh_ip_str,
                                  IP_AF_IPV4, 0, &seed);
    route_table_util::route_table_stepper_seed_init(k_max_v6_route_table,
                                  11111, k_first_v6_pfx_str, k_first_nh_ip_str,
                                  IP_AF_IPV6, 0, &seed);
    // trigger - test max route tables with zero routes
    workflow_neg_3<route_table_util, route_table_stepper_seed_t>(&seed);

    route_table_util::route_table_stepper_seed_init(k_max_v4_route_table,
                                  22222, k_first_v4_pfx_str, k_first_nh_ip_str,
                                  IP_AF_IPV4, k_max_route_per_tbl, &seed);
    route_table_util::route_table_stepper_seed_init(k_max_v6_route_table,
                                  11111, k_first_v6_pfx_str, k_first_nh_ip_str,
                                  IP_AF_IPV6, k_max_route_per_tbl, &seed);
    // trigger - test max route tables with max routes
    workflow_neg_3<route_table_util, route_table_stepper_seed_t>(&seed);
}

/// \brief Route table WF_N_4
TEST_F(route_test, v4v6_route_table_workflow_neg_4) {
    route_table_stepper_seed_t seed1 = {}, seed2 = {};

    route_table_util::route_table_stepper_seed_init(40, 100,
                                  k_first_v4_pfx_str, k_first_nh_ip_str,
                                  IP_AF_IPV4, 0, &seed1);
    route_table_util::route_table_stepper_seed_init(40, 150,
                                  k_first_v6_pfx_str, k_first_nh_ip_str,
                                  IP_AF_IPV6, 0, &seed1);
    route_table_util::route_table_stepper_seed_init(40, 300,
                                  k_first_v4_pfx_str, k_first_nh_ip_str,
                                  IP_AF_IPV4, 0, &seed2);
    route_table_util::route_table_stepper_seed_init(40, 350,
                                  k_first_v6_pfx_str, k_first_nh_ip_str,
                                  IP_AF_IPV6, 0, &seed2);
    // trigger - test route tables with zero routes
    workflow_neg_4<route_table_util, route_table_stepper_seed_t>(&seed1,
                                                                 &seed2);

    route_table_util::route_table_stepper_seed_init(40, 100,
                                  k_first_v4_pfx_str, k_first_nh_ip_str,
                                  IP_AF_IPV4, k_max_route_per_tbl, &seed1);
    route_table_util::route_table_stepper_seed_init(40, 150,
                                  k_first_v6_pfx_str, k_first_nh_ip_str,
                                  IP_AF_IPV6, k_max_route_per_tbl, &seed1);
    route_table_util::route_table_stepper_seed_init(40, 300,
                                  k_first_v4_pfx_str, k_first_nh_ip_str,
                                  IP_AF_IPV4, k_max_route_per_tbl, &seed2);
    route_table_util::route_table_stepper_seed_init(40, 350,
                                  k_first_v6_pfx_str, k_first_nh_ip_str,
                                  IP_AF_IPV6, k_max_route_per_tbl, &seed2);
    // trigger - test route tables with max routes
    workflow_neg_4<route_table_util, route_table_stepper_seed_t>(&seed1,
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

int
main (int argc, char **argv)
{
    route_test_options_parse(argc, argv);
    if (route_test_options_validate() != sdk::SDK_RET_OK) {
        route_test_usage_print(argv);
        exit(1);
    }

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
