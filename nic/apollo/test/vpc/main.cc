//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains all vpc test cases
///
//----------------------------------------------------------------------------

#include "nic/apollo/test/utils/workflow1.hpp"
#include "nic/apollo/test/utils/vpc.hpp"

namespace api_test {
/// \cond
// Globals
static constexpr uint32_t k_max_vpc = PDS_MAX_VPC + 1;

//----------------------------------------------------------------------------
// VPC test class
//----------------------------------------------------------------------------

class vpc : public ::pds_test_base {
protected:
    vpc() {}
    virtual ~vpc() {}
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
/// \endcond
//----------------------------------------------------------------------------
// VPC test cases implementation
//----------------------------------------------------------------------------

/// \defgroup VPC_TEST VPC Tests
/// @{

/// \brief VPC WF_B1
/// \ref WF_B1
TEST_F(vpc, vpc_workflow_b1) {
    pds_vpc_key_t key = {.id = 1};
    vpc_feeder feeder;

    feeder.init(key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16", "00:02:01:00:00:01");
    workflow_b1<vpc_feeder>(feeder);
}

/// \brief VPC WF_B2
/// \ref WF_B2
TEST_F(vpc, DISABLED_vpc_workflow_b2) {
    pds_vpc_key_t key = {.id = 1};
    vpc_feeder feeder1, feeder1A;

    feeder1.init(key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16", "00:02:01:00:00:01");
    feeder1A.init(key, PDS_VPC_TYPE_TENANT, "11.0.0.0/16", "00:02:0A:00:00:01");
    workflow_b2<vpc_feeder>(feeder1, feeder1A);
}

/// \brief VPC WF_1
/// \ref WF_1
TEST_F(vpc, vpc_workflow_1) {
    pds_vpc_key_t key = {.id = 1};
    vpc_feeder feeder;

    feeder.init(key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16",
                "00:02:01:00:00:01", k_max_vpc);
    workflow_1<vpc_feeder>(feeder);
}

/// \brief VPC WF_2
/// \ref WF_2
TEST_F(vpc, vpc_workflow_2) {
    pds_vpc_key_t key = {.id = 1};
    vpc_feeder feeder;

    feeder.init(key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16",
                "00:02:01:00:00:01", k_max_vpc);
    workflow_2<vpc_feeder>(feeder);
}

/// \brief VPC WF_3
/// \ref WF_3
TEST_F(vpc, vpc_workflow_3) {
    pds_vpc_key_t key1 = {.id = 10}, key2 = {.id = 40}, key3 = {.id = 70};
    vpc_feeder feeder1, feeder2, feeder3;

    feeder1.init(key1, PDS_VPC_TYPE_TENANT, "10.0.0.0/16",
                 "00:02:01:00:00:01", 20);
    feeder2.init(key2, PDS_VPC_TYPE_TENANT, "30.0.0.0/16",
                 "00:02:0A:00:00:01", 20);
    feeder3.init(key3, PDS_VPC_TYPE_TENANT, "60.0.0.0/16",
                 "00:02:0A:0B:00:01", 20);
    workflow_3<vpc_feeder>(feeder1, feeder2, feeder3);
}

/// \brief VPC WF_4
/// \ref WF_4
TEST_F(vpc, vpc_workflow_4) {
    pds_vpc_key_t key = {.id = 1};
    vpc_feeder feeder;

    feeder.init(key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16",
                "00:02:01:00:00:01", k_max_vpc);
    workflow_4<vpc_feeder>(feeder);
}

/// \brief VPC WF_5
/// \ref WF_5
TEST_F(vpc, vpc_workflow_5) {
    pds_vpc_key_t key1 = {.id = 10}, key2 = {.id = 40}, key3 = {.id = 70};
    vpc_feeder feeder1, feeder2, feeder3;

    feeder1.init(key1, PDS_VPC_TYPE_TENANT, "10.0.0.0/16",
                 "00:02:01:00:00:01", 20);
    feeder2.init(key2, PDS_VPC_TYPE_TENANT, "40.0.0.0/16",
                 "00:02:0A:00:00:01", 20);
    feeder3.init(key3, PDS_VPC_TYPE_TENANT, "70.0.0.0/16",
                 "00:02:0A:0B:00:01", 20);
    workflow_5<vpc_feeder>(feeder1, feeder2, feeder3);
}

/// \brief VPC WF_6
/// \ref WF_6
TEST_F(vpc, vpc_workflow_6) {
    pds_vpc_key_t key = {.id = 1};
    vpc_feeder feeder1, feeder1A, feeder1B;

    feeder1.init(key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16",
                 "00:02:01:00:00:01", k_max_vpc);
    feeder1A.init(key, PDS_VPC_TYPE_TENANT, "11.0.0.0/16",
                  "00:02:0A:00:00:01", k_max_vpc);
    feeder1B.init(key, PDS_VPC_TYPE_TENANT, "12.0.0.0/16",
                  "00:02:0A:0B:00:01", k_max_vpc);
    workflow_6<vpc_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief VPC WF_7
/// \ref WF_7
TEST_F(vpc, vpc_workflow_7) {
    pds_vpc_key_t key = {.id = 1};
    vpc_feeder feeder1, feeder1A, feeder1B;

    feeder1.init(key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16",
                 "00:02:01:00:00:01", k_max_vpc);
    feeder1A.init(key, PDS_VPC_TYPE_TENANT, "11.0.0.0/16",
                  "00:02:0A:00:00:01", k_max_vpc);
    feeder1B.init(key, PDS_VPC_TYPE_TENANT, "12.0.0.0/16",
                  "00:02:0A:0B:00:01", k_max_vpc);
    workflow_7<vpc_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief VPC WF_8
/// \ref WF_8
TEST_F(vpc, DISABLED_vpc_workflow_8) {
    pds_vpc_key_t key = {.id = 1};
    vpc_feeder feeder1, feeder1A, feeder1B;

    feeder1.init(key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16",
                 "00:02:01:00:00:01", k_max_vpc);
    feeder1A.init(key, PDS_VPC_TYPE_TENANT, "11.0.0.0/16",
                  "00:02:0A:00:00:01", k_max_vpc);
    feeder1B.init(key, PDS_VPC_TYPE_TENANT, "12.0.0.0/16",
                  "00:02:0A:0B:00:01", k_max_vpc);
    workflow_8<vpc_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief VPC WF_9
/// \ref WF_9
TEST_F(vpc, vpc_workflow_9) {
    pds_vpc_key_t key = {.id = 1};
    vpc_feeder feeder1, feeder1A;

    feeder1.init(key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16",
                 "00:02:01:00:00:01", k_max_vpc);
    feeder1A.init(key, PDS_VPC_TYPE_TENANT, "11.0.0.0/16",
                  "00:02:0A:00:00:01", k_max_vpc);
    workflow_9<vpc_feeder>(feeder1, feeder1A);
}

/// \brief VPC WF_10
/// \ref WF_10
TEST_F(vpc, DISABLED_vpc_workflow_10) {
    pds_vpc_key_t key1 = {.id = 10}, key2 = {.id = 40}, key3 = {.id = 70},
                  key4 = {.id = 100};
    vpc_feeder feeder1, feeder2, feeder3, feeder4, feeder2A, feeder3A;

    feeder1.init(key1, PDS_VPC_TYPE_TENANT, "10.0.0.0/16",
                 "00:02:01:00:00:01", 20);
    feeder2.init(key2, PDS_VPC_TYPE_TENANT, "11.0.0.0/16",
                 "00:02:0A:00:00:01", 20);
    feeder2A.init(key2, PDS_VPC_TYPE_TENANT, "12.0.0.0/16",
                  "00:02:0A:0B:00:01", 20);
    feeder3.init(key3, PDS_VPC_TYPE_TENANT, "13.0.0.0/16",
                 "00:02:01:00:00:01", 20);
    feeder3A.init(key3, PDS_VPC_TYPE_TENANT, "14.0.0.0/16",
                  "00:02:0A:00:00:01", 20);
    feeder4.init(key4, PDS_VPC_TYPE_TENANT, "15.0.0.0/16",
                 "00:02:0A:0B:00:01", 20);
    workflow_10<vpc_feeder>(
        feeder1, feeder2, feeder2A, feeder3, feeder3A, feeder4);
}

/// \brief VPC WF_N_1
/// \ref WF_N_1
TEST_F(vpc, vpc_workflow_neg_1) {
    pds_vpc_key_t key = {.id = 1};
    vpc_feeder feeder;

    feeder.init(key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16",
                "00:02:01:00:00:01", k_max_vpc);
    workflow_neg_1<vpc_feeder>(feeder);
}

/// \brief VPC WF_N_2
/// \ref WF_N_2
TEST_F(vpc, DISABLED_vpc_workflow_neg_2) {
    pds_vpc_key_t key = {.id = 1};
    vpc_feeder feeder;

    feeder.init(key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16",
                "00:02:01:00:00:01", k_max_vpc+1);
    workflow_neg_2<vpc_feeder>(feeder);
}

/// \brief VPC WF_N_3
/// \ref WF_N_3
TEST_F(vpc, vpc_workflow_neg_3) {
    pds_vpc_key_t key = {.id = 1};
    vpc_feeder feeder;

    feeder.init(key, PDS_VPC_TYPE_TENANT, "0.0.0.0/0",
                "00:02:01:00:00:01", k_max_vpc);
    workflow_neg_3<vpc_feeder>(feeder);
}

/// \brief VPC WF_N_4
/// \ref WF_N_4
TEST_F(vpc, vpc_workflow_neg_4) {
    pds_vpc_key_t key1 = {.id = 10}, key2 = {.id = 40};
    vpc_feeder feeder1, feeder2;

    feeder1.init(key1, PDS_VPC_TYPE_TENANT, "10.0.0.0/16",
                 "00:02:01:00:00:01", 20);
    feeder2.init(key2, PDS_VPC_TYPE_TENANT, "40.0.0.0/16",
                 "00:02:0A:00:00:01", 20);
    workflow_neg_4<vpc_feeder>(feeder1, feeder2);
}

/// \brief VPC WF_N_5
/// \ref WF_N_5
TEST_F(vpc, DISABLED_vpc_workflow_neg_5) {
    pds_vpc_key_t key = {.id = 1};
    vpc_feeder feeder1, feeder1A;

    feeder1.init(key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16",
                 "00:02:01:00:00:01", k_max_vpc);
    feeder1A.init(key, PDS_VPC_TYPE_TENANT, "11.0.0.0/16",
                  "00:02:0A:00:00:01", k_max_vpc);
    workflow_neg_5<vpc_feeder>(feeder1, feeder1A);
}

/// \brief VPC WF_N_6
/// \ref WF_N_6
TEST_F(vpc, vpc_workflow_neg_6) {
    pds_vpc_key_t key = {.id = 1};
    vpc_feeder feeder1, feeder1A;

    feeder1.init(key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16",
                 "00:02:01:00:00:01", k_max_vpc);
    feeder1A.init(key, PDS_VPC_TYPE_TENANT, "11.0.0.0/16",
                  "00:02:0A:00:00:01", k_max_vpc + 1);
    workflow_neg_6<vpc_feeder>(feeder1, feeder1A);
}

/// \brief VPC WF_N_7
/// \ref WF_N_7
TEST_F(vpc, vpc_workflow_neg_7) {
    pds_vpc_key_t key1 = {.id = 10}, key2 = {.id = 40};
    vpc_feeder feeder1, feeder1A, feeder2;

    feeder1.init(key1, PDS_VPC_TYPE_TENANT, "10.0.0.0/16",
                 "00:02:01:00:00:01", 20);
    feeder1A.init(key1, PDS_VPC_TYPE_TENANT, "11.0.0.0/16",
                  "00:02:0A:00:00:01", 20);
    feeder2.init(key2, PDS_VPC_TYPE_TENANT, "12.0.0.0/16",
                 "00:02:0A:0B:00:01", 20);
    workflow_neg_7<vpc_feeder>(feeder1, feeder1A, feeder2);
}

/// \brief VPC WF_N_8
/// \ref WF_N_8
TEST_F(vpc, vpc_workflow_neg_8) {
    pds_vpc_key_t key1 = {.id = 10}, key2 = {.id = 40};
    vpc_feeder feeder1, feeder2;

    feeder1.init(key1, PDS_VPC_TYPE_TENANT, "10.0.0.0/16",
                 "00:02:01:00:00:01", 20);
    feeder2.init(key2, PDS_VPC_TYPE_TENANT, "11.0.0.0/16",
                 "00:02:0A:00:00:01", 20);
    workflow_neg_8<vpc_feeder>(feeder1, feeder2);
}

/// @}

}    // namespace api_test

//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------

/// @private
int
main (int argc, char **argv)
{
#ifdef AGENT_MODE
    if (agent_mode()) {
        test_app_init();
    }
#endif
    return api_test_program_run(argc, argv);
}
