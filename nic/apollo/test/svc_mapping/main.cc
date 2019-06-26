//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all svc mapping test cases
///
//----------------------------------------------------------------------------

#include "nic/apollo/test/utils/base.hpp"
#include "nic/apollo/test/utils/svc_mapping.hpp"
#include "nic/apollo/test/utils/utils.hpp"
#include "nic/apollo/test/utils/workflow1.hpp"
#include "nic/apollo/test/utils/vpc.hpp"

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
        pds_test_base::SetUpTestCase(g_tc_params);
        batch_start();
        sample1_vpc_setup(PDS_VPC_TYPE_TENANT);
        batch_commit();
    }
    static void TearDownTestCase() {
        batch_start();
        sample1_vpc_teardown(PDS_VPC_TYPE_TENANT);
        batch_commit();
        pds_test_base::TearDownTestCase();
    }
};

//----------------------------------------------------------------------------
// Svc mapping test cases implementation
//----------------------------------------------------------------------------

/// \defgroup SVC_MAPPING_TEST
/// @{

/// \brief SVC_MAPPING WF_TMP_1
TEST_F(svc_mapping_test, svc_mapping_workflow_tmp_1) {
    svc_mapping_feeder feeder;

    feeder.init(1, "10.1.1.1", 10, 2, "20.1.1.1", 20, "30.1.1.1", 10);
    workflow_tmp_1<svc_mapping_feeder>(feeder);
}

/// \brief SVC_MAPPING WF_1
TEST_F(svc_mapping_test, DISABLED_svc_mapping_workflow_1) {
    svc_mapping_feeder feeder;

    feeder.init(1, "10.1.1.1", 10, 2, "20.1.1.1", 20, "30.1.1.1", 10);
    workflow_1<svc_mapping_feeder>(feeder);
}

/// \brief SVC MAPPING WF_2
TEST_F(svc_mapping_test, DISABLED_svc_mapping_workflow_2) {
    svc_mapping_feeder feeder;

    workflow_2<svc_mapping_feeder>(feeder);
}

/// \brief SVC MAPPING WF_3
TEST_F(svc_mapping_test, DISABLED_svc_mapping_workflow_3) {
    svc_mapping_feeder feeder1, feeder2, feeder3;

    workflow_3<svc_mapping_feeder>(feeder1, feeder2, feeder3);
}

/// \brief SVC MAPPING WF_4
TEST_F(svc_mapping_test, DISABLED_svc_mapping_workflow_4) {
    svc_mapping_feeder feeder;

    workflow_4<svc_mapping_feeder>(feeder);
}

/// \brief SVC MAPPING WF_5
TEST_F(svc_mapping_test, DISABLED_svc_mapping_workflow_5) {
    svc_mapping_feeder feeder1, feeder2, feeder3;

    workflow_5<svc_mapping_feeder>(feeder1, feeder2, feeder3);
}

/// \brief SVC MAPPING WF_6
TEST_F(svc_mapping_test, DISABLED_svc_mapping_workflow_6) {
    svc_mapping_feeder feeder1, feeder1A, feeder1B;

    workflow_6<svc_mapping_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief SVC MAPPING WF_7
TEST_F(svc_mapping_test, DISABLED_svc_mapping_workflow7) {
    svc_mapping_feeder feeder1, feeder1A, feeder1B;

    workflow_7<svc_mapping_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief SVC MAPPING WF_8
TEST_F(svc_mapping_test, DISABLED_DISABLED_svc_mapping_workflow8) {
    svc_mapping_feeder feeder1, feeder1A, feeder1B;

    workflow_8<svc_mapping_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief SVC MAPPING WF_9
TEST_F(svc_mapping_test, DISABLED_svc_mapping_workflow9) {
    svc_mapping_feeder feeder1, feeder1A;

    workflow_9<svc_mapping_feeder>(feeder1, feeder1A);
}

/// \brief SVC MAPPING WF_10
TEST_F(svc_mapping_test, DISABLED_DISABLED_svc_mapping_workflow10) {
    svc_mapping_feeder feeder1, feeder2, feeder2A, feeder3, feeder3A, feeder4;

    workflow_10<svc_mapping_feeder>(feeder1, feeder2, feeder2A,
                            feeder3, feeder3A, feeder4);
}

/// \brief SVC MAPPING WF_N_1
TEST_F(svc_mapping_test, DISABLED_svc_mapping_workflow_neg_1) {
    svc_mapping_feeder feeder;

    workflow_neg_1<svc_mapping_feeder>(feeder);
}

/// \brief SVC MAPPING WF_N_2
TEST_F(svc_mapping_test, DISABLED_svc_mapping_workflow_neg_2) {
    svc_mapping_feeder feeder;

    workflow_neg_2<svc_mapping_feeder>(feeder);
}

/// \brief SVC MAPPING WF_N_3
TEST_F(svc_mapping_test, DISABLED_svc_mapping_workflow_neg_3) {
    svc_mapping_feeder feeder;

    workflow_neg_3<svc_mapping_feeder>(feeder);
}

/// \brief SVC MAPPING WF_N_4
TEST_F(svc_mapping_test, DISABLED_svc_mapping_workflow_neg_4) {
    svc_mapping_feeder feeder1, feeder2;

    workflow_neg_4<svc_mapping_feeder>(feeder1, feeder2);
}

/// \brief SVC MAPPING WF_N_5
TEST_F(svc_mapping_test, DISABLED_DISABLED_svc_mapping_workflow_neg_5) {
    svc_mapping_feeder feeder1, feeder1A;

    workflow_neg_5<svc_mapping_feeder>(feeder1, feeder1A);
}

/// \brief SVC MAPPING WF_N_6
TEST_F(svc_mapping_test, DISABLED_svc_mapping_workflow_neg_6) {
    svc_mapping_feeder feeder1, feeder1A;

    workflow_neg_6<svc_mapping_feeder>(feeder1, feeder1A);
}

/// \brief SVC MAPPING WF_N_7
TEST_F(svc_mapping_test, DISABLED_svc_mapping_workflow_neg_7) {
    svc_mapping_feeder feeder1, feeder1A, feeder2;

    workflow_neg_7<svc_mapping_feeder>(feeder1, feeder1A, feeder2);
}

/// \brief SVC MAPPING WF_N_8
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
    api_test_program_run(argc, argv);
}
