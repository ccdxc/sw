//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all device test cases
///
//----------------------------------------------------------------------------

#include "nic/apollo/test/utils/device.hpp"
#include "nic/apollo/test/utils/workflow1.hpp"

namespace api_test {

//----------------------------------------------------------------------------
// Device test class
//----------------------------------------------------------------------------
/// \cond
class device_test : public pds_test_base {
protected:
    device_test() {}
    virtual ~device_test() {}
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
// Device test cases implementation
//----------------------------------------------------------------------------

/// \defgroup DEVICE_TEST Device Tests
/// @{

/// \brief Device WF_1
/// \ref WF_1
TEST_F(device_test, device_workflow_1) {
    device_feeder feeder;
    feeder.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2");
    workflow_1<device_feeder>(feeder);

    if (apulu()) {
        feeder.init("2001:1::1", "00:02:01:00:00:01", "2001:1::2");
        workflow_1<device_feeder>(feeder);
    }
}

/// \brief Device WF_2
/// \ref WF_2
TEST_F(device_test, device_workflow_2) {
    device_feeder feeder;
    feeder.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2");
    workflow_2<device_feeder>(feeder);

    if (apulu()) {
        feeder.init("2001:1::1", "00:02:01:00:00:01", "2001:1::2");
        workflow_2<device_feeder>(feeder);
    }
}

/// \brief Device WF_4
/// \ref WF_4
TEST_F(device_test, device_workflow_4) {
    device_feeder feeder;
    feeder.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2");
    workflow_4<device_feeder>(feeder);

    if (apulu()) {
        feeder.init("2001:1::1", "00:02:01:00:00:01", "2001:1::2");
        workflow_4<device_feeder>(feeder);
    }
}

/// \brief Device WF_6
/// \ref WF_6
TEST_F(device_test, device_workflow_6) {
    device_feeder feeder1, feeder1A, feeder1B;
    feeder1.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2");
    feeder1A.init("1.0.0.1", "00:02:01:00:0A:0B", "111.0.0.111");
    feeder1B.init("1.0.0.1", "00:02:01:0A:0B:0C", "99.99.99.99");
    workflow_6<device_feeder>(feeder1, feeder1A, feeder1B);

    if (apulu()) {
        feeder1.init("2001:1::1", "00:02:01:00:00:01", "2001:1::2");
        feeder1A.init("2001:1::1", "00:02:01:00:0A:0B", "2002:1::2");
        feeder1B.init("2001:1::1", "00:02:01:0A:0B:0C", "2003:1::2");
        workflow_6<device_feeder>(feeder1, feeder1A, feeder1B);
    }
}

/// \brief Device WF_7
/// \ref WF_7
TEST_F(device_test, device_workflow_7) {
    device_feeder feeder1, feeder1A, feeder1B;
    feeder1.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2");
    feeder1A.init("1.0.0.1", "00:02:01:00:0A:0B", "111.0.0.111");
    feeder1B.init("1.0.0.1", "00:02:01:0A:0B:0C", "99.99.99.99");
    workflow_7<device_feeder>(feeder1, feeder1A, feeder1B);

    if (apulu()) {
        feeder1.init("2001:1::1", "00:02:01:00:00:01", "2001:1::2");
        feeder1A.init("2001:1::1", "00:02:01:00:0A:0B", "2002:1::2");
        feeder1B.init("2001:1::1", "00:02:01:0A:0B:0C", "2003:1::2");
        workflow_7<device_feeder>(feeder1, feeder1A, feeder1B);
    }
}

/// \brief Device WF_8
/// \ref WF_8
TEST_F(device_test, device_workflow_8) {
    if (!apulu()) return;

    device_feeder feeder1, feeder1A, feeder1B;
    feeder1.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2");
    feeder1A.init("1.0.0.1", "00:02:01:00:0A:0B", "111.0.0.111");
    feeder1B.init("1.0.0.1", "00:02:01:0A:0B:0C", "99.99.99.99");
    workflow_8<device_feeder>(feeder1, feeder1A, feeder1B);

    if (apulu()) {
        feeder1.init("2001:1::1", "00:02:01:00:00:01", "2001:1::2");
        feeder1A.init("2001:1::1", "00:02:01:00:0A:0B", "2002:1::2");
        feeder1B.init("2001:1::1", "00:02:01:0A:0B:0C", "2003:1::2");
        workflow_8<device_feeder>(feeder1, feeder1A, feeder1B);
    }
}

/// \brief Device WF_9
/// \ref WF_9
TEST_F(device_test, device_workflow_9) {
    if (!apulu()) return;

    device_feeder feeder1, feeder1A;
    feeder1.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2");
    feeder1A.init("1.0.0.1", "00:02:01:00:0A:0B", "111.0.0.111");
    workflow_9<device_feeder>(feeder1, feeder1A);

    if (apulu()) {
        feeder1.init("2001:1::1", "00:02:01:00:00:01", "2001:1::2");
        feeder1A.init("2001:1::1", "00:02:01:00:0A:0B", "2002:1::2");
        workflow_9<device_feeder>(feeder1, feeder1A);
    }
}

/// \brief Device WF_N_1
/// \ref WF_N_1
TEST_F(device_test, device_workflow_neg_1) {
    if (!apulu()) return;

    device_feeder feeder;
    feeder.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2");
    workflow_neg_1<device_feeder>(feeder);

    if (apulu()) {
        feeder.init("2001:1::1", "00:02:01:00:00:01", "2001:1::2");
        workflow_neg_1<device_feeder>(feeder);
    }
}

/// \brief Device WF_N_3
/// \ref WF_N_3
TEST_F(device_test, device_workflow_neg_3) {
    device_feeder feeder;
    feeder.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2");
    workflow_neg_3<device_feeder>(feeder);

    if (apulu()) {
        feeder.init("2001:1::1", "00:02:01:00:00:01", "2001:1::2");
        workflow_neg_3<device_feeder>(feeder);
    }
}

/// \brief Device WF_N_5
/// \ref WF_N_5
TEST_F(device_test, DISABLED_device_workflow_neg_5) {
    device_feeder feeder1, feeder1A;
    feeder1.init("1.0.0.1", "00:02:01:00:00:01", "1.0.0.2");
    feeder1A.init("1.0.0.1", "00:02:01:00:0A:0B", "111.0.0.111");
    workflow_neg_5<device_feeder>(feeder1, feeder1A);

    if (apulu()) {
        feeder1.init("2001:1::1", "00:02:01:00:00:01", "2001:1::2");
        feeder1A.init("2001:1::1", "00:02:01:00:0A:0B", "2002:1::2");
        workflow_neg_5<device_feeder>(feeder1, feeder1A);
    }
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
    return api_test_program_run(argc, argv);
}
