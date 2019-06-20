//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all device test cases
///
//----------------------------------------------------------------------------

#include "nic/apollo/test/utils/base.hpp"
#include "nic/apollo/test/utils/batch.hpp"
#include "nic/apollo/test/utils/device.hpp"
#include "nic/apollo/test/utils/api_base.hpp"
#include "nic/apollo/test/utils/workflow1.hpp"
#include "nic/apollo/test/utils/utils.hpp"

namespace api_test {

//----------------------------------------------------------------------------
// Device test class
//----------------------------------------------------------------------------

class device_test : public pds_test_base {
protected:
    device_test() {}
    virtual ~device_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        pds_test_base::SetUpTestCase(g_tc_params);
    }
    static void TearDownTestCase() {
        pds_test_base::TearDownTestCase();
    }
};

//----------------------------------------------------------------------------
// Device test cases implementation
//----------------------------------------------------------------------------

/// \defgroup DEVICE_TEST
/// @{

/// \brief Device WF_1
TEST_F(device_test, device_workflow_1) {
    device_feeder feeder;
    feeder.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2");
    workflow_1<device_feeder>(feeder);
}

/// \brief Device WF_2
TEST_F(device_test, device_workflow_2) {
    device_feeder feeder;
    feeder.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2");
    workflow_2<device_feeder>(feeder);
}

/// \brief Device WF_4
TEST_F(device_test, device_workflow_4) {
    device_feeder feeder;
    feeder.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2");
    workflow_4<device_feeder>(feeder);
}

/// \brief Device WF_6
TEST_F(device_test, device_workflow_6) {
    device_feeder feeder1, feeder1A, feeder1B;
    feeder1.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2");
    feeder1A.init("1.0.0.1", "00:02:01:00:0A:0B", "111.0.0.111");
    feeder1B.init("1.0.0.1", "00:02:01:0A:0B:0C", "99.99.99.99");
    workflow_6<device_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief Device WF_7
TEST_F(device_test, device_workflow_7) {
    device_feeder feeder1, feeder1A, feeder1B;
    feeder1.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2");
    feeder1A.init("1.0.0.1", "00:02:01:00:0A:0B", "111.0.0.111");
    feeder1B.init("1.0.0.1", "00:02:01:0A:0B:0C", "99.99.99.99");
    workflow_7<device_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief Device WF_8
TEST_F(device_test, DISABLED_device_workflow_8) {
    device_feeder feeder1, feeder1A, feeder1B;
    feeder1.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2");
    feeder1A.init("1.0.0.1", "00:02:01:00:0A:0B", "111.0.0.111");
    feeder1B.init("1.0.0.1", "00:02:01:0A:0B:0C", "99.99.99.99");
    workflow_8<device_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief Device WF_9
TEST_F(device_test, DISABLED_device_workflow_9) {
    device_feeder feeder1, feeder1A;
    feeder1.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2");
    feeder1A.init("1.0.0.1", "00:02:01:00:0A:0B", "111.0.0.111");
    workflow_9<device_feeder>(feeder1, feeder1A);
}

/// \brief Device WF_N_1
TEST_F(device_test, device_workflow_neg_1) {
    device_feeder feeder;
    feeder.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2");
    workflow_neg_1<device_feeder>(feeder);
}

/// \brief Device WF_N_3
TEST_F(device_test, device_workflow_neg_3) {
    device_feeder feeder;
    feeder.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2");
    workflow_neg_3<device_feeder>(feeder);
}

/// \brief Device WF_N_5
TEST_F(device_test, DISABLED_device_workflow_neg_5) {
    device_feeder feeder1, feeder1A;
    feeder1.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2");
    feeder1A.init("1.0.0.1", "00:02:01:00:0A:0B", "111.0.0.111");
    workflow_neg_5<device_feeder>(feeder1, feeder1A);
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
