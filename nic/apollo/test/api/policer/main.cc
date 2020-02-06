//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains all policer test cases
///
//----------------------------------------------------------------------------

#include "nic/apollo/test/api/utils/policer.hpp"
#include "nic/apollo/test/api/utils/workflow.hpp"

namespace test {
namespace api {

// globals
static constexpr uint32_t k_max_policer = PDS_MAX_RX_POLICER - 1;

//----------------------------------------------------------------------------
// POLICER test class
//----------------------------------------------------------------------------

class policer : public ::pds_test_base {
protected:
    policer() {}
    virtual ~policer() {}
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
// POLICER test cases implementation
//----------------------------------------------------------------------------

/// \defgroup POLICER_TEST POLICER Tests
/// @{

/// \brief POLICER WF_B1
/// \ref WF_B1
TEST_F(policer, policer_workflow_b1) {
    pds_obj_key_t key = int2pdsobjkey(1);
    policer_feeder feeder;

    feeder.init(key, sdk::POLICER_TYPE_PPS, PDS_POLICER_DIR_INGRESS,
                500000, 1500000, 1);
    workflow_b1<policer_feeder>(feeder);
}

/// \brief POLICER WF_B2
/// \ref WF_B2
TEST_F(policer, policer_workflow_b2) {
    if (!apulu()) return;

    pds_obj_key_t key = int2pdsobjkey(1);
    policer_feeder feeder1, feeder1A;

    feeder1.init(key, sdk::POLICER_TYPE_PPS, PDS_POLICER_DIR_INGRESS,
                500000, 1500000, 1);
    feeder1A.init(key, sdk::POLICER_TYPE_PPS, PDS_POLICER_DIR_INGRESS,
                600000, 1200000, 1);
    workflow_b2<policer_feeder>(feeder1, feeder1A);
}

/// \brief POLICER WF_1
/// \ref WF_1
TEST_F(policer, policer_workflow_1) {
    pds_obj_key_t key = int2pdsobjkey(1);
    policer_feeder feeder;

    feeder.init(key, sdk::POLICER_TYPE_PPS, PDS_POLICER_DIR_INGRESS,
                500000, 1500000, k_max_policer);
    workflow_1<policer_feeder>(feeder);
}

/// \brief POLICER WF_2
/// \ref WF_2
TEST_F(policer, policer_workflow_2) {
    pds_obj_key_t key = int2pdsobjkey(1);
    policer_feeder feeder;

    feeder.init(key, sdk::POLICER_TYPE_PPS, PDS_POLICER_DIR_INGRESS,
                500000, 1500000, k_max_policer);
    workflow_2<policer_feeder>(feeder);
}

/// \brief POLICER WF_3
/// \ref WF_3
TEST_F(policer, policer_workflow_3) {
    pds_obj_key_t key1 = int2pdsobjkey(10);
    pds_obj_key_t key2 = int2pdsobjkey(40);
    pds_obj_key_t key3 = int2pdsobjkey(70);
    policer_feeder feeder1, feeder2, feeder3;

    feeder1.init(key1, sdk::POLICER_TYPE_PPS, PDS_POLICER_DIR_INGRESS,
                500000, 1500000, 20);
    feeder2.init(key2, sdk::POLICER_TYPE_BPS, PDS_POLICER_DIR_INGRESS,
                600000, 1800000, 20);
    feeder3.init(key3, sdk::POLICER_TYPE_PPS, PDS_POLICER_DIR_EGRESS,
                900000, 4000000, 20);
    workflow_3<policer_feeder>(feeder1, feeder2, feeder3);
}

/// \brief POLICER WF_4
/// \ref WF_4
TEST_F(policer, policer_workflow_4) {
    pds_obj_key_t key = int2pdsobjkey(1);
    policer_feeder feeder;

    feeder.init(key, sdk::POLICER_TYPE_BPS, PDS_POLICER_DIR_INGRESS,
                600000, 1800000, k_max_policer);
    workflow_4<policer_feeder>(feeder);
}

/// \brief POLICER WF_5
/// \ref WF_5
TEST_F(policer, policer_workflow_5) {
    pds_obj_key_t key1 = int2pdsobjkey(10);
    pds_obj_key_t key2 = int2pdsobjkey(40);
    pds_obj_key_t key3 = int2pdsobjkey(70);
    policer_feeder feeder1, feeder2, feeder3;

    feeder1.init(key1, sdk::POLICER_TYPE_PPS, PDS_POLICER_DIR_INGRESS,
                500000, 1500000, 20);
    feeder2.init(key2, sdk::POLICER_TYPE_BPS, PDS_POLICER_DIR_INGRESS,
                600000, 1800000, 20);
    feeder3.init(key3, sdk::POLICER_TYPE_PPS, PDS_POLICER_DIR_EGRESS,
                900000, 4000000, 20);
    workflow_5<policer_feeder>(feeder1, feeder2, feeder3);
}

/// \brief POLICER WF_6
/// \ref WF_6
TEST_F(policer, policer_workflow_6) {
    pds_obj_key_t key = int2pdsobjkey(1);
    policer_feeder feeder1, feeder1A, feeder1B;

    feeder1.init(key, sdk::POLICER_TYPE_PPS, PDS_POLICER_DIR_INGRESS,
                500000, 1500000, k_max_policer);
    feeder1A.init(key, sdk::POLICER_TYPE_BPS, PDS_POLICER_DIR_INGRESS,
                600000, 1800000, k_max_policer);
    feeder1B.init(key, sdk::POLICER_TYPE_PPS, PDS_POLICER_DIR_EGRESS,
                900000, 4000000, k_max_policer);
    workflow_6<policer_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief POLICER WF_7
/// \ref WF_7
TEST_F(policer, policer_workflow_7) {
    pds_obj_key_t key = int2pdsobjkey(1);
    policer_feeder feeder1, feeder1A, feeder1B;

    feeder1.init(key, sdk::POLICER_TYPE_PPS, PDS_POLICER_DIR_INGRESS,
                500000, 1500000, k_max_policer);
    feeder1A.init(key, sdk::POLICER_TYPE_BPS, PDS_POLICER_DIR_INGRESS,
                600000, 1800000, k_max_policer);
    feeder1B.init(key, sdk::POLICER_TYPE_PPS, PDS_POLICER_DIR_EGRESS,
                900000, 4000000, k_max_policer);
    workflow_7<policer_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief POLICER WF_8
/// \ref WF_8
TEST_F(policer, policer_workflow_8) {
    if (!apulu()) return;

    pds_obj_key_t key = int2pdsobjkey(1);
    policer_feeder feeder1, feeder1A, feeder1B;

    feeder1.init(key, sdk::POLICER_TYPE_PPS, PDS_POLICER_DIR_INGRESS,
                500000, 1500000, k_max_policer);
    feeder1A.init(key, sdk::POLICER_TYPE_BPS, PDS_POLICER_DIR_INGRESS,
                600000, 1800000, k_max_policer);
    feeder1B.init(key, sdk::POLICER_TYPE_PPS, PDS_POLICER_DIR_INGRESS,
                900000, 4000000, k_max_policer);
    workflow_8<policer_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief POLICER WF_9
/// \ref WF_9
TEST_F(policer, policer_workflow_9) {
    pds_obj_key_t key = int2pdsobjkey(1);
    policer_feeder feeder1, feeder1A;

    feeder1.init(key, sdk::POLICER_TYPE_PPS, PDS_POLICER_DIR_INGRESS,
                500000, 1500000, k_max_policer);
    feeder1A.init(key, sdk::POLICER_TYPE_BPS, PDS_POLICER_DIR_INGRESS,
                600000, 1800000, k_max_policer);
    workflow_9<policer_feeder>(feeder1, feeder1A);
}

/// \brief POLICER WF_10
/// \ref WF_10
TEST_F(policer, policer_workflow_10) {
    pds_obj_key_t key1 = int2pdsobjkey(10), key2 = int2pdsobjkey(40);
    pds_obj_key_t key3 = int2pdsobjkey(70), key4 = int2pdsobjkey(100);
    policer_feeder feeder1, feeder2, feeder3, feeder4, feeder2A, feeder3A;

    feeder1.init(key1, sdk::POLICER_TYPE_PPS, PDS_POLICER_DIR_INGRESS,
                500000, 1500000, 20);
    feeder2.init(key2, sdk::POLICER_TYPE_BPS, PDS_POLICER_DIR_INGRESS,
                600000, 1800000, 20);
    feeder2A.init(key2, sdk::POLICER_TYPE_BPS, PDS_POLICER_DIR_INGRESS,
                800000, 2800000, 20);
    feeder3.init(key3, sdk::POLICER_TYPE_PPS, PDS_POLICER_DIR_INGRESS,
                900000, 4000000, 20);
    feeder3A.init(key3, sdk::POLICER_TYPE_PPS, PDS_POLICER_DIR_INGRESS,
                700000, 3500000, 20);
    feeder4.init(key4, sdk::POLICER_TYPE_PPS, PDS_POLICER_DIR_INGRESS,
                400000, 1200000, 20);
    workflow_10<policer_feeder>(
        feeder1, feeder2, feeder2A, feeder3, feeder3A, feeder4);
}

/// \brief POLICER WF_N_1
/// \ref WF_N_1
TEST_F(policer, policer_workflow_neg_1) {
    pds_obj_key_t key = int2pdsobjkey(1);
    policer_feeder feeder;

    feeder.init(key, sdk::POLICER_TYPE_PPS, PDS_POLICER_DIR_INGRESS,
                500000, 1500000, k_max_policer);
    workflow_neg_1<policer_feeder>(feeder);
}

/// \brief POLICER WF_N_2
/// \ref WF_N_2
TEST_F(policer, policer_workflow_neg_2) {
    pds_obj_key_t key = int2pdsobjkey(1);
    policer_feeder feeder;

    feeder.init(key, sdk::POLICER_TYPE_PPS, PDS_POLICER_DIR_INGRESS,
                500000, 1500000, k_max_policer + 1);
    workflow_neg_2<policer_feeder>(feeder);
}

/// \brief POLICER WF_N_3
/// \ref WF_N_3
TEST_F(policer, policer_workflow_neg_3) {
    pds_obj_key_t key = int2pdsobjkey(1);
    policer_feeder feeder;

    feeder.init(key, sdk::POLICER_TYPE_PPS, PDS_POLICER_DIR_INGRESS,
                500000, 1500000, k_max_policer);
    workflow_neg_3<policer_feeder>(feeder);
}

/// \brief POLICER WF_N_4
/// \ref WF_N_4
TEST_F(policer, policer_workflow_neg_4) {
    pds_obj_key_t key1 = int2pdsobjkey(10), key2 = int2pdsobjkey(40);
    policer_feeder feeder1, feeder2;

    feeder1.init(key1, sdk::POLICER_TYPE_PPS, PDS_POLICER_DIR_INGRESS,
                500000, 1500000, 20);
    feeder2.init(key2, sdk::POLICER_TYPE_BPS, PDS_POLICER_DIR_INGRESS,
                600000, 1800000, 20);
    workflow_neg_4<policer_feeder>(feeder1, feeder2);
}

/// \brief POLICER WF_N_5
/// \ref WF_N_5
TEST_F(policer, policer_workflow_neg_5) {
    pds_obj_key_t key = int2pdsobjkey(1);
    policer_feeder feeder1, feeder1A;

    feeder1.init(key, sdk::POLICER_TYPE_PPS, PDS_POLICER_DIR_INGRESS,
                500000, 1500000, k_max_policer);
    feeder1A.init(key, sdk::POLICER_TYPE_BPS, PDS_POLICER_DIR_INGRESS,
                600000, 1800000, k_max_policer);
    workflow_neg_5<policer_feeder>(feeder1, feeder1A);
}

/// \brief POLICER WF_N_6
/// \ref WF_N_6
TEST_F(policer, policer_workflow_neg_6) {
    pds_obj_key_t key = int2pdsobjkey(1);
    policer_feeder feeder1, feeder1A;

    feeder1.init(key, sdk::POLICER_TYPE_PPS, PDS_POLICER_DIR_INGRESS,
                500000, 1500000, k_max_policer);
    feeder1A.init(key, sdk::POLICER_TYPE_BPS, PDS_POLICER_DIR_INGRESS,
                600000, 1800000, k_max_policer + 1);
    workflow_neg_6<policer_feeder>(feeder1, feeder1A);
}

/// \brief POLICER WF_N_7
/// \ref WF_N_7
TEST_F(policer, policer_workflow_neg_7) {
    pds_obj_key_t key1 = int2pdsobjkey(10), key2 = int2pdsobjkey(40);
    policer_feeder feeder1, feeder1A, feeder2;

    feeder1.init(key1, sdk::POLICER_TYPE_PPS, PDS_POLICER_DIR_INGRESS,
                500000, 1500000, 20);
    feeder1A.init(key1, sdk::POLICER_TYPE_BPS, PDS_POLICER_DIR_INGRESS,
                700000, 2100000, 20);
    feeder2.init(key2, sdk::POLICER_TYPE_BPS, PDS_POLICER_DIR_INGRESS,
                600000, 1800000, 20);
    workflow_neg_7<policer_feeder>(feeder1, feeder1A, feeder2);
}

/// \brief POLICER WF_N_8
/// \ref WF_N_8
TEST_F(policer, policer_workflow_neg_8) {
    pds_obj_key_t key1 = int2pdsobjkey(10), key2 = int2pdsobjkey(40);
    policer_feeder feeder1, feeder2;

    feeder1.init(key1, sdk::POLICER_TYPE_PPS, PDS_POLICER_DIR_INGRESS,
                500000, 1500000, 20);
    feeder2.init(key2, sdk::POLICER_TYPE_BPS, PDS_POLICER_DIR_INGRESS,
                600000, 1800000, 20);
    workflow_neg_8<policer_feeder>(feeder1, feeder2);
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
#ifdef AGENT_MODE
    if (agent_mode()) {
        test_app_init();
    }
#endif
    return api_test_program_run(argc, argv);
}
