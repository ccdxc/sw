//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all tag table test cases
///
//----------------------------------------------------------------------------

#include "nic/apollo/test/api/utils/tag.hpp"
#include "nic/apollo/test/api/utils/workflow.hpp"

namespace test {
namespace api {

// globals
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
        if (!agent_mode())
            pds_test_base::SetUpTestCase(g_tc_params);
    }
    static void TearDownTestCase() {
        if (!agent_mode())
            pds_test_base::TearDownTestCase();
    }
};

//----------------------------------------------------------------------------
// Tag table test cases implementation
//----------------------------------------------------------------------------

/// \defgroup TAG_TESTS Tag table tests
/// @{

/// \brief Tag table WF_1
/// \ref WF_1
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
/// \ref WF_2
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
/// \ref WF_3
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
/// \ref WF_4
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
/// \ref WF_5
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
/// \ref WF_6
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
/// \ref WF_7
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
/// \ref WF_8
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
/// \ref WF_10
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
/// \ref WF_N_1
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
/// \ref WF_N_2
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
/// \ref WF_N_3
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
/// \ref WF_N_4
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
/// \ref WF_N_5
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
/// \ref WF_N_6
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
/// \ref WF_N_7
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
/// \ref WF_N_8
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
