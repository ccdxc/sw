//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all svc mapping test cases
///
//----------------------------------------------------------------------------

#include "nic/apollo/test/utils/svc_mapping.hpp"
#include "nic/apollo/test/utils/vpc.hpp"
#include "nic/apollo/test/utils/workflow1.hpp"

namespace api_test {

//----------------------------------------------------------------------------
// Svc mapping test class
//----------------------------------------------------------------------------

class svc_mapping_test : public ::pds_test_base {
protected:
    svc_mapping_test() {}
    virtual ~svc_mapping_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        if (!agent_mode())
            pds_test_base::SetUpTestCase(g_tc_params);
        pds_batch_ctxt_t bctxt = batch_start();
        sample1_vpc_setup(bctxt, PDS_VPC_TYPE_TENANT);
        batch_commit(bctxt);
    }
    static void TearDownTestCase() {
        pds_batch_ctxt_t bctxt = batch_start();
        sample1_vpc_teardown(bctxt, PDS_VPC_TYPE_TENANT);
        batch_commit(bctxt);
        if (!agent_mode())
            pds_test_base::TearDownTestCase();
    }
};

//----------------------------------------------------------------------------
// Svc mapping test cases implementation
//----------------------------------------------------------------------------

/// \defgroup SVC_MAPPING_TEST Service Mapping Tests
/// @{

/// \brief SVC MAPPING WF_TMP_1
/// \ref WF_TMP_1
TEST_F(svc_mapping_test, svc_mapping_workflow_tmp_1) {
    svc_mapping_feeder feeder;

    feeder.init("10.1.1.1", 10, 2, "20.1.1.1", 20, "30.1.1.1", 10);
    workflow_tmp_1<svc_mapping_feeder>(feeder);
}

/// \brief SVC MAPPING WF_1
/// \ref WF_1
TEST_F(svc_mapping_test, DISABLED_svc_mapping_workflow_1) {
    svc_mapping_feeder feeder;

    feeder.init("10.1.1.1", 10, 2, "20.1.1.1", 20, "30.1.1.1", 10);
    workflow_1<svc_mapping_feeder>(feeder);
}

/// \brief SVC MAPPING WF_2
/// \ref WF_2
TEST_F(svc_mapping_test, DISABLED_svc_mapping_workflow_2) {
    svc_mapping_feeder feeder;

    workflow_2<svc_mapping_feeder>(feeder);
}

/// \brief SVC MAPPING WF_3
/// \ref WF_3
TEST_F(svc_mapping_test, DISABLED_svc_mapping_workflow_3) {
    svc_mapping_feeder feeder1, feeder2, feeder3;

    workflow_3<svc_mapping_feeder>(feeder1, feeder2, feeder3);
}

/// \brief SVC MAPPING WF_4
/// \ref WF_4
TEST_F(svc_mapping_test, DISABLED_svc_mapping_workflow_4) {
    svc_mapping_feeder feeder;

    workflow_4<svc_mapping_feeder>(feeder);
}

/// \brief SVC MAPPING WF_5
/// \ref WF_5
TEST_F(svc_mapping_test, DISABLED_svc_mapping_workflow_5) {
    svc_mapping_feeder feeder1, feeder2, feeder3;

    workflow_5<svc_mapping_feeder>(feeder1, feeder2, feeder3);
}

/// \brief SVC MAPPING WF_6
/// \ref WF_6
TEST_F(svc_mapping_test, DISABLED_svc_mapping_workflow_6) {
    svc_mapping_feeder feeder1, feeder1A, feeder1B;

    workflow_6<svc_mapping_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief SVC MAPPING WF_7
/// \ref WF_7
TEST_F(svc_mapping_test, DISABLED_svc_mapping_workflow7) {
    svc_mapping_feeder feeder1, feeder1A, feeder1B;

    workflow_7<svc_mapping_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief SVC MAPPING WF_8
/// \ref WF_8
TEST_F(svc_mapping_test, DISABLED_DISABLED_svc_mapping_workflow8) {
    svc_mapping_feeder feeder1, feeder1A, feeder1B;

    workflow_8<svc_mapping_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief SVC MAPPING WF_9
/// \ref WF_9
TEST_F(svc_mapping_test, DISABLED_svc_mapping_workflow9) {
    svc_mapping_feeder feeder1, feeder1A;

    workflow_9<svc_mapping_feeder>(feeder1, feeder1A);
}

/// \brief SVC MAPPING WF_10
/// \ref WF_10
TEST_F(svc_mapping_test, DISABLED_DISABLED_svc_mapping_workflow10) {
    svc_mapping_feeder feeder1, feeder2, feeder2A, feeder3, feeder3A, feeder4;

    workflow_10<svc_mapping_feeder>(feeder1, feeder2, feeder2A,
                            feeder3, feeder3A, feeder4);
}

/// \brief SVC MAPPING WF_N_1
/// \ref WF_N_1
TEST_F(svc_mapping_test, DISABLED_svc_mapping_workflow_neg_1) {
    svc_mapping_feeder feeder;

    workflow_neg_1<svc_mapping_feeder>(feeder);
}

/// \brief SVC MAPPING WF_N_2
/// \ref WF_N_2
TEST_F(svc_mapping_test, DISABLED_svc_mapping_workflow_neg_2) {
    svc_mapping_feeder feeder;

    workflow_neg_2<svc_mapping_feeder>(feeder);
}

/// \brief SVC MAPPING WF_N_3
/// \ref WF_N_3
TEST_F(svc_mapping_test, DISABLED_svc_mapping_workflow_neg_3) {
    svc_mapping_feeder feeder;

    workflow_neg_3<svc_mapping_feeder>(feeder);
}

/// \brief SVC MAPPING WF_N_4
/// \ref WF_N_4
TEST_F(svc_mapping_test, DISABLED_svc_mapping_workflow_neg_4) {
    svc_mapping_feeder feeder1, feeder2;

    workflow_neg_4<svc_mapping_feeder>(feeder1, feeder2);
}

/// \brief SVC MAPPING WF_N_5
/// \ref WF_N_5
TEST_F(svc_mapping_test, DISABLED_DISABLED_svc_mapping_workflow_neg_5) {
    svc_mapping_feeder feeder1, feeder1A;

    workflow_neg_5<svc_mapping_feeder>(feeder1, feeder1A);
}

/// \brief SVC MAPPING WF_N_6
/// \ref WF_N_6
TEST_F(svc_mapping_test, DISABLED_svc_mapping_workflow_neg_6) {
    svc_mapping_feeder feeder1, feeder1A;

    workflow_neg_6<svc_mapping_feeder>(feeder1, feeder1A);
}

/// \brief SVC MAPPING WF_N_7
/// \ref WF_N_7
TEST_F(svc_mapping_test, DISABLED_svc_mapping_workflow_neg_7) {
    svc_mapping_feeder feeder1, feeder1A, feeder2;

    workflow_neg_7<svc_mapping_feeder>(feeder1, feeder1A, feeder2);
}

/// \brief SVC MAPPING WF_N_8
/// \ref WF_N_8
TEST_F(svc_mapping_test, DISABLED_svc_mapping_workflow_neg_8) {
    svc_mapping_feeder feeder1, feeder2;

    workflow_neg_8<svc_mapping_feeder>(feeder1, feeder2);
}

/// @}

}    // namespace api_test

//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------

int
main (int argc, char **argv)
{
    return api_test_program_run(argc, argv);
}
