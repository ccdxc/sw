//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all tag table test cases
///
//----------------------------------------------------------------------------
#include <getopt.h>
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/test/utils/base.hpp"
#include "nic/apollo/test/utils/batch.hpp"
#include "nic/apollo/test/utils/tag.hpp"
#include "nic/apollo/test/utils/workflow1.hpp"

using std::cerr;
using std::cout;
using std::endl;

namespace api_test {

// Globals
static char *g_cfg_file = NULL;
static std::string g_pipeline("");

static constexpr int k_max_tag_table = 4;
static constexpr int k_max_prefixes_per_tbl = PDS_MAX_PREFIX_PER_TAG;

static const char * const k_base_v4_pfx = "100.100.100.1/21";
static const char * const k_base_v6_pfx = "100:100:100:1:1::1/65";

//----------------------------------------------------------------------------
// Tag table test class
//----------------------------------------------------------------------------

class tag_test : public pds_test_base {
protected:
    tag_test() {}
    virtual ~tag_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        test_case_params_t params;

        params.cfg_file = api_test::g_cfg_file;
        params.enable_fte = false;
        pds_test_base::SetUpTestCase(params);
    }
    static void TearDownTestCase() {
        pds_test_base::TearDownTestCase();
    }
};

//----------------------------------------------------------------------------
// Tag table test cases implementation
//----------------------------------------------------------------------------

/// \defgroup Tag table
/// @{

/// \brief RTag table WF_1
TEST_F(tag_test, v4v6_tag_table_workflow_1) {
    tag_feeder feeder;

    feeder.init(1, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                IP_AF_IPV4, k_max_tag_table, 0);
    // trigger - test max tag tables with zero prefixes
    workflow_1<tag_feeder>(feeder);

    feeder.init(1, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                IP_AF_IPV4, k_max_tag_table, k_max_prefixes_per_tbl);
    // trigger - test max tag tables with max prefixes
    workflow_1<tag_feeder>(feeder);
}

/// \brief Tag table WF_2
TEST_F(tag_test, v4v6_tag_table_workflow_2) {
    tag_feeder feeder;

    feeder.init(1, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                IP_AF_IPV4, k_max_tag_table, 0);
    // trigger - test max tag tables with zero prefixes
    workflow_2<tag_feeder>(feeder);

    feeder.init(1, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                IP_AF_IPV4, k_max_tag_table, k_max_prefixes_per_tbl);
    // trigger - test max tag tables with max prefixes
    workflow_2<tag_feeder>(feeder);
}

/// \brief Tag table WF_3
TEST_F(tag_test, v4v6_tag_table_workflow_3) {
    tag_feeder feeder1, feeder2, feeder3;

    feeder1.init(1, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                 IP_AF_IPV4, 1, 0);
    feeder2.init(2, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                 IP_AF_IPV6, 1, 0);
    feeder3.init(3, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                 IP_AF_IPV4, 1, 0);
    // trigger - test tag tables with zero prefixes
    workflow_3<tag_feeder>(feeder1,feeder2, feeder3);

    feeder1.init(1, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                 IP_AF_IPV4, 1, k_max_prefixes_per_tbl);
    feeder2.init(2, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                 IP_AF_IPV6, 1, k_max_prefixes_per_tbl);
    feeder3.init(3, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                 IP_AF_IPV4, 1, k_max_prefixes_per_tbl);
    // trigger - test tag tables with max prefixes
    workflow_3<tag_feeder>(feeder1, feeder2, feeder3);
}

/// \brief Tag table WF_4
TEST_F(tag_test, v4v6_tag_table_workflow_4) {
    tag_feeder feeder;

    feeder.init(1, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                IP_AF_IPV4, k_max_tag_table, 0);
    // trigger - test max tag tables with zero prefixes
    workflow_4<tag_feeder>(feeder);

    feeder.init(1, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                IP_AF_IPV4, k_max_tag_table, k_max_prefixes_per_tbl);
    // trigger - test max tag tables with max prefixes
    workflow_4<tag_feeder>(feeder);
}

/// \brief Tag table WF_5
TEST_F(tag_test, v4v6_tag_table_workflow_5) {
    tag_feeder feeder1, feeder2, feeder3;

    feeder1.init(1, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                 IP_AF_IPV4, 1, 0);
    feeder2.init(2, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                 IP_AF_IPV6, 1, 0);
    feeder3.init(3, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                 IP_AF_IPV4, 1, 0);
    // trigger - test tag tables with zero prefixes
    workflow_5<tag_feeder>(feeder1, feeder2, feeder3);

    feeder1.init(1, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                 IP_AF_IPV4, 1, k_max_prefixes_per_tbl);
    feeder2.init(2, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                 IP_AF_IPV6, 1, k_max_prefixes_per_tbl);
    feeder3.init(3, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                 IP_AF_IPV4, 1, k_max_prefixes_per_tbl);
    // trigger - test tag tables with max prefixes
    workflow_5<tag_feeder>(feeder1, feeder2, feeder3);
}

/// \brief Tag table WF_6
TEST_F(tag_test, DISABLED_v4v6_tag_table_workflow_6) {
    tag_feeder feeder1, feeder1A, feeder1B;

    feeder1.init(1, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                 IP_AF_IPV4, k_max_tag_table, k_max_prefixes_per_tbl);
    //seed1A = with change in priority and tag
    feeder1A.init(1, k_base_v4_pfx, k_base_v6_pfx, 1, 1,
                 IP_AF_IPV4, k_max_tag_table, k_max_prefixes_per_tbl);
    //seed1B = with change in priority and tag
    feeder1B.init(1, k_base_v4_pfx, k_base_v6_pfx, 2, 2,
                 IP_AF_IPV4, k_max_tag_table, k_max_prefixes_per_tbl);
    // trigger - test tag tables with max prefixes
    workflow_6<tag_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief Tag table WF_7
TEST_F(tag_test, DISABLED_v4v6_tag_table_workflow_7) {
    tag_feeder feeder1, feeder1A, feeder1B;

    feeder1.init(1, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                 IP_AF_IPV4, k_max_tag_table, k_max_prefixes_per_tbl);
    //seed1A = with change in priority and tag
    feeder1A.init(1, k_base_v4_pfx, k_base_v6_pfx, 1, 1,
                        IP_AF_IPV4, k_max_tag_table, k_max_prefixes_per_tbl);
    //seed1B = with change in priority and tag
    feeder1B.init(1, k_base_v4_pfx, k_base_v6_pfx, 2, 2,
                  IP_AF_IPV4, k_max_tag_table, k_max_prefixes_per_tbl);
    // trigger - test tag tables with max prefixes
    workflow_7<tag_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief Tag table WF_8
TEST_F(tag_test, DISABLED_v4v6_tag_table_workflow_8) {
    tag_feeder feeder1, feeder1A, feeder1B;

    feeder1.init(1, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                 IP_AF_IPV4, k_max_tag_table, k_max_prefixes_per_tbl);
    //seed1A = with change in priority and tag
    feeder1A.init(1, k_base_v4_pfx, k_base_v6_pfx, 1, 1,
                  IP_AF_IPV4, k_max_tag_table, k_max_prefixes_per_tbl);
    //seed1B = with change in priority and tag
    feeder1B.init(1, k_base_v4_pfx, k_base_v6_pfx, 2, 2,
                  IP_AF_IPV4, k_max_tag_table, k_max_prefixes_per_tbl);
    // trigger - test tag tables with max prefixes
    workflow_8<tag_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief tag table WF_9
TEST_F(tag_test, DISABLED_v4v6_tag_table_workflow_9) {
    tag_feeder feeder1, feeder1A;

    feeder1.init(1, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                 IP_AF_IPV4, k_max_tag_table, k_max_prefixes_per_tbl);
    //seed1A = with change in priority and tag
    feeder1A.init(1, k_base_v4_pfx, k_base_v6_pfx, 1, 1,
                  IP_AF_IPV4, k_max_tag_table, k_max_prefixes_per_tbl);
    // trigger - test tag tables with max prefixes
    workflow_9<tag_feeder>(feeder1, feeder1A);
}

/// \brief Tag table WF_10
TEST_F(tag_test, DISABLED_v4v6_tag_table_workflow_10) {
    tag_feeder feeder1, feeder2, feeder3, feeder4, feeder2A, feeder3A;

    feeder1.init(1, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                 IP_AF_IPV4, 1, k_max_prefixes_per_tbl);
    feeder2.init(2, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                 IP_AF_IPV6, 1, k_max_prefixes_per_tbl);
    feeder3.init(3, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                 IP_AF_IPV4, 1, k_max_prefixes_per_tbl);
    feeder4.init(4, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                 IP_AF_IPV6, 1, k_max_prefixes_per_tbl);
    feeder2A.init(2, k_base_v4_pfx, k_base_v6_pfx, 1, 1,
                  IP_AF_IPV6, 1, k_max_prefixes_per_tbl);
    feeder3A.init(3, k_base_v4_pfx, k_base_v6_pfx, 2, 2,
                  IP_AF_IPV4, 1, k_max_prefixes_per_tbl);
    // trigger - test tag tables
    workflow_10<tag_feeder>(feeder1, feeder2, feeder2A,
                            feeder3, feeder3A, feeder4);
}

/// \brief Tag table WF_N_1
TEST_F(tag_test, v4v6_tag_table_workflow_neg_1) {
    tag_feeder feeder;

    feeder.init(1, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                IP_AF_IPV4, k_max_tag_table, 0);
    // trigger - test max tag tables with zero prefixes
    workflow_neg_1<tag_feeder>(feeder);

    feeder.init(1, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                IP_AF_IPV4, k_max_tag_table, k_max_prefixes_per_tbl);
    // trigger - test max tag tables with max prefixes
    workflow_neg_1<tag_feeder>(feeder);
}

/// \brief Tag table WF_N_2
TEST_F(tag_test, v4v6_tag_table_workflow_neg_2) {
    tag_feeder feeder;

    feeder.init(1, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                IP_AF_IPV4, k_max_tag_table+1, 0);
    // trigger - test max tag tables with zero prefixes
    workflow_neg_2<tag_feeder>(feeder);

    feeder.init(1, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                IP_AF_IPV4, k_max_tag_table+1, k_max_prefixes_per_tbl+1);
    // trigger - test max tag tables with max prefixes
    workflow_neg_2<tag_feeder>(feeder);
}

/// \brief Tag table WF_N_3
TEST_F(tag_test, v4v6_tag_table_workflow_neg_3) {
    tag_feeder feeder;

    feeder.init(1, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                IP_AF_IPV4, k_max_tag_table, 0);
    // trigger - test max tag tables with zero prefixes
    workflow_neg_3<tag_feeder>(feeder);

    feeder.init(1, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                IP_AF_IPV4, k_max_tag_table, k_max_prefixes_per_tbl);
    // trigger - test max tag tables with max prefixes
    workflow_neg_3<tag_feeder>(feeder);
}

/// \brief Tag table WF_N_4
TEST_F(tag_test, v4v6_tag_table_workflow_neg_4) {
    tag_feeder feeder1, feeder2;

    feeder1.init(1, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                        IP_AF_IPV4, 1, 0);
    feeder2.init(2, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                        IP_AF_IPV6, 1, 0);
    // trigger - test tag tables with zero prefixes
    workflow_neg_4<tag_feeder>(feeder1, feeder2);

    feeder1.init(1, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                 IP_AF_IPV4, 1, k_max_prefixes_per_tbl);
    feeder2.init(2, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                 IP_AF_IPV6, 1, k_max_prefixes_per_tbl);
    // trigger - test tag tables with max prefixes
    workflow_neg_4<tag_feeder>(feeder1, feeder2);
}

/// \brief Tag table WF_N_5
TEST_F(tag_test, DISABLED_v4v6_tag_table_workflow_neg_5) {
    tag_feeder feeder1, feeder1A;

    feeder1.init(1, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                 IP_AF_IPV4, k_max_tag_table, k_max_prefixes_per_tbl);
    //seed1A = change in priority and tag
    feeder1A.init(1, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                  IP_AF_IPV4, k_max_tag_table, k_max_prefixes_per_tbl);
    // trigger - test max tag tables with max prefixes
    workflow_neg_5<tag_feeder>(feeder1, feeder1A);
}

/// \brief Tag table WF_N_6
TEST_F(tag_test, DISABLED_v4v6_tag_table_workflow_neg_6) {
    tag_feeder feeder1, feeder1A;

    feeder1.init(1, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                 IP_AF_IPV4, k_max_tag_table, k_max_prefixes_per_tbl);
    //seed1A = change in priority and tag
    feeder1A.init(1, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                  IP_AF_IPV4, k_max_tag_table+1, k_max_prefixes_per_tbl);
    // trigger - test max tag tables with max prefixes
    workflow_neg_6<tag_feeder>(feeder1, feeder1A);
}

/// \brief Tag table WF_N_7
TEST_F(tag_test, DISABLED_v4v6_tag_table_workflow_neg_7) {
    tag_feeder feeder1, feeder2, feeder1A;

    feeder1.init(1, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                 IP_AF_IPV4, 1, k_max_prefixes_per_tbl);
    feeder1A.init(1, k_base_v4_pfx, k_base_v6_pfx, 1, 1,
                  IP_AF_IPV4, 1, k_max_prefixes_per_tbl);
    feeder2.init(2, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                 IP_AF_IPV6, 1, k_max_prefixes_per_tbl);
    // trigger - test tag tables with max prefixes
    workflow_neg_7<tag_feeder>(feeder1, feeder1A, feeder2);
}

/// \brief Tag table WF_N_8
TEST_F(tag_test, DISABLED_v4v6_tag_table_workflow_neg_8) {
    tag_feeder feeder1 = {}, feeder2 = {};

    feeder1.init(1, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                 IP_AF_IPV4, 1, k_max_prefixes_per_tbl);
    feeder2.init(2, k_base_v4_pfx, k_base_v6_pfx, 0, 0,
                 IP_AF_IPV6, 1, k_max_prefixes_per_tbl);
    // trigger - test tag tables with max prefixes
    workflow_neg_8<tag_feeder>(feeder1, feeder2);
}
/// @}

}    // namespace api_test

//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------

static inline void
tag_test_usage_print (char **argv)
{
    cout << "Usage : " << argv[0] << " -c <hal.json>" << endl;
    return;
}

static inline sdk_ret_t
tag_test_options_validate (void)
{
    if (!api_test::g_cfg_file) {
        cerr << "HAL config file is not specified" << endl;
        return sdk::SDK_RET_ERR;
    }
    return sdk::SDK_RET_OK;
}

static void
tag_test_options_parse (int argc, char **argv)
{
    int oc = -1;
    struct option longopts[] = {{"config", required_argument, NULL, 'c'},
                                {"help", no_argument, NULL, 'h'},
                                {0, 0, 0, 0}};

    while ((oc = getopt_long(argc, argv, ":hc:f:", longopts, NULL)) != -1) {
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
    tag_test_options_parse(argc, argv);
    if (tag_test_options_validate() != sdk::SDK_RET_OK) {
        tag_test_usage_print(argv);
        exit(1);
    }

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
