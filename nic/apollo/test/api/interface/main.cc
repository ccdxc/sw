//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all interface test cases
///
//----------------------------------------------------------------------------

#include "nic/apollo/test/api/utils/if.hpp"
#include "nic/apollo/test/api/utils/workflow.hpp"

namespace test {
namespace api {

//----------------------------------------------------------------------------
// Interface test class
//----------------------------------------------------------------------------

class if_test : public pds_test_base {
protected:
    if_test() {}
    virtual ~if_test() {}
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
// Interface test cases implementation
//----------------------------------------------------------------------------

/// \defgroup INTERFACE_TEST Interface Tests
/// @{

/// \brief Inteface WF_1
/// \ref WF_1
TEST_F(if_test, if_workflow_1) {
    if_feeder feeder;
    feeder.init(1);
    workflow_1<if_feeder>(feeder);
}

/// \brief Interface WF_2
/// \ref WF_2
TEST_F(if_test, if_workflow_2) {
    if_feeder feeder;
    feeder.init(1);
    workflow_2<if_feeder>(feeder);
}

/// \brief Interface WF_4
/// \ref WF_4
TEST_F(if_test, if_workflow_4) {
    if_feeder feeder;
    feeder.init(1);
    workflow_4<if_feeder>(feeder);
}

/// \brief Interface WF_6
/// \ref WF_6
TEST_F(if_test, if_workflow_6) {
    if_feeder feeder1, feeder1A, feeder1B;
    feeder1.init(1, "80.1.1.1");
    feeder1A.init(1, "80.1.2.1");
    feeder1B.init(1, "80.1.3.1");
    workflow_6<if_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief Interface WF_7
/// \ref WF_7
TEST_F(if_test, if_workflow_7) {
    if_feeder feeder1, feeder1A, feeder1B;
    feeder1.init(1, "80.1.1.1");
    feeder1A.init(1, "80.1.2.1");
    feeder1B.init(1, "80.1.3.1");
    workflow_7<if_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief Interface WF_8
/// \ref WF_8
TEST_F(if_test, DISABLED_if_workflow_8) {
    if_feeder feeder1, feeder1A, feeder1B;
    feeder1.init(1, "80.1.1.1");
    feeder1A.init(1, "80.1.2.1");
    feeder1B.init(1, "80.1.3.1");
    workflow_8<if_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief Interface WF_9
/// \ref WF_9
TEST_F(if_test, if_workflow_9) {
    if_feeder feeder1, feeder1A;
    feeder1.init(1, "80.1.1.1");
    feeder1A.init(1, "80.1.2.1");
    workflow_9<if_feeder>(feeder1, feeder1A);
}
/// \brief Interface WF_N_1
/// \ref WF_N_1
TEST_F(if_test, if_workflow_neg_1) {
    if_feeder feeder;
    feeder.init(1);
    workflow_neg_1<if_feeder>(feeder);
}

/// \brief Interface WF_N_3
/// \ref WF_N_3
TEST_F(if_test, if_workflow_neg_3) {
    if_feeder feeder;
    feeder.init(1);
    workflow_neg_3<if_feeder>(feeder);
}

/// \brief Interface WF_N_5
/// \ref WF_N_5
TEST_F(if_test, DISABLED_if_workflow_neg_5) {
    if_feeder feeder1, feeder1A;
    feeder1.init(1);
    feeder1A.init(10);
    workflow_neg_5<if_feeder>(feeder1, feeder1A);
}

/// \brief Interface WF_N_7
/// \ref WF_N_7
TEST_F(if_test, if_workflow_neg_7) {
    if_feeder feeder1, feeder1A, feeder2;
    feeder1.init(1, "80.1.1.1");
    feeder1A.init(1, "80.1.2.1");
    feeder2.init(10, "80.1.1.1");
    workflow_neg_7<if_feeder>(feeder1, feeder1A, feeder2);
}

/// \brief Interface WF_N_8
/// \ref WF_N_8
TEST_F(if_test, if_workflow_neg_8) {
    if_feeder feeder1, feeder2;
    feeder1.init(1);
    feeder2.init(10);
    workflow_neg_8<if_feeder>(feeder1, feeder2);
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
