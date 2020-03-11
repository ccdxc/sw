//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all dhcp_relay test cases
///
//----------------------------------------------------------------------------

#include "nic/apollo/test/api/utils/dhcp_relay.hpp"
#include "nic/apollo/test/api/utils/workflow.hpp"

namespace test {
namespace api {

// globals
static const char* const agent_ip1 = "20.1.1.1";
static const char* const agent_ip2 = "20.1.2.1";
static const char* const agent_ip3 = "20.1.3.1";
static const char* const server_ip1 = "200.1.1.1";
static const char* const server_ip2 = "200.1.2.1";
static const char* const server_ip3 = "200.1.3.1";

//----------------------------------------------------------------------------
// DHCP Relay test class
//----------------------------------------------------------------------------

class dhcp_relay_test : public pds_test_base {
protected:
    dhcp_relay_test() {}
    virtual ~dhcp_relay_test() {}
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
// DHCP Relay test cases implementation
//----------------------------------------------------------------------------

/// \defgroup DHCP_RELAY_TEST DHCP Relay Tests
/// @{

/// \brief DHCP Relay WF_B1
/// \ref WF_B1
TEST_F(dhcp_relay_test, dhcp_relay_workflow_b1) {
    if (!apulu()) return;

    dhcp_relay_feeder feeder;
    feeder.init(int2pdsobjkey(1), agent_ip1, server_ip1, int2pdsobjkey(1));
    workflow_b1<dhcp_relay_feeder>(feeder);
}

/// \brief Device WF_B2
/// \ref WF_B2
TEST_F(dhcp_relay_test, dhcp_relay_workflow_b2) {
    if (!apulu()) return;

    dhcp_relay_feeder feeder1, feeder1A;
    feeder1.init(int2pdsobjkey(1), agent_ip1, server_ip1, int2pdsobjkey(1));
    feeder1A.init(int2pdsobjkey(1), agent_ip2, server_ip2, int2pdsobjkey(1));
    workflow_b2<dhcp_relay_feeder>(feeder1, feeder1A);
}

/// \brief Device WF_1
/// \ref WF_1
TEST_F(dhcp_relay_test, dhcp_relay_workflow_1) {
    dhcp_relay_feeder feeder;
    feeder.init(int2pdsobjkey(1), agent_ip1, server_ip1, int2pdsobjkey(1));
    workflow_1<dhcp_relay_feeder>(feeder);
}

/// \brief Device WF_2
/// \ref WF_2
TEST_F(dhcp_relay_test, dhcp_relay_workflow_2) {
    dhcp_relay_feeder feeder;
    feeder.init(int2pdsobjkey(1), agent_ip1, server_ip1, int2pdsobjkey(1));
    workflow_2<dhcp_relay_feeder>(feeder);
}

/// \brief Device WF_4
/// \ref WF_4
TEST_F(dhcp_relay_test, dhcp_relay_workflow_4) {
    dhcp_relay_feeder feeder;
    feeder.init(int2pdsobjkey(1), agent_ip1, server_ip1, int2pdsobjkey(1));
    workflow_4<dhcp_relay_feeder>(feeder);
}

/// \brief Device WF_6
/// \ref WF_6
TEST_F(dhcp_relay_test, dhcp_relay_workflow_6) {
    dhcp_relay_feeder feeder1, feeder1A, feeder1B;
    feeder1.init(int2pdsobjkey(1), agent_ip1, server_ip1, int2pdsobjkey(1));
    feeder1A.init(int2pdsobjkey(1), agent_ip1, server_ip2, int2pdsobjkey(1));
    feeder1B.init(int2pdsobjkey(1), agent_ip1, server_ip3, int2pdsobjkey(1));
    workflow_6<dhcp_relay_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief Device WF_7
/// \ref WF_7
TEST_F(dhcp_relay_test, dhcp_relay_workflow_7) {
    dhcp_relay_feeder feeder1, feeder1A, feeder1B;
    feeder1.init(int2pdsobjkey(1), agent_ip1, server_ip1, int2pdsobjkey(1));
    feeder1A.init(int2pdsobjkey(1), agent_ip1, server_ip2, int2pdsobjkey(1));
    feeder1B.init(int2pdsobjkey(1), agent_ip1, server_ip3, int2pdsobjkey(1));
    workflow_7<dhcp_relay_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief Device WF_8
/// \ref WF_8
TEST_F(dhcp_relay_test, dhcp_relay_workflow_8) {
    if (!apulu()) return;

    dhcp_relay_feeder feeder1, feeder1A, feeder1B;
    feeder1.init(int2pdsobjkey(1), agent_ip1, server_ip1, int2pdsobjkey(1));
    feeder1A.init(int2pdsobjkey(1), agent_ip1, server_ip2, int2pdsobjkey(1));
    feeder1B.init(int2pdsobjkey(1), agent_ip1, server_ip3, int2pdsobjkey(1));
    workflow_8<dhcp_relay_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief Device WF_9
/// \ref WF_9
TEST_F(dhcp_relay_test, dhcp_relay_workflow_9) {
    if (!apulu()) return;

    dhcp_relay_feeder feeder1, feeder1A;
    feeder1.init(int2pdsobjkey(1), agent_ip1, server_ip1, int2pdsobjkey(1));
    feeder1A.init(int2pdsobjkey(1), agent_ip1, server_ip2, int2pdsobjkey(1));
    workflow_9<dhcp_relay_feeder>(feeder1, feeder1A);
}

/// \brief Device WF_N_1
/// \ref WF_N_1
TEST_F(dhcp_relay_test, dhcp_relay_workflow_neg_1) {
    if (!apulu()) return;

    dhcp_relay_feeder feeder;
    feeder.init(int2pdsobjkey(1), agent_ip1, server_ip1, int2pdsobjkey(1));
    workflow_neg_1<dhcp_relay_feeder>(feeder);
}

/// \brief Device WF_N_3
/// \ref WF_N_3
TEST_F(dhcp_relay_test, dhcp_relay_workflow_neg_3) {
    dhcp_relay_feeder feeder;
    feeder.init(int2pdsobjkey(1), agent_ip1, server_ip1, int2pdsobjkey(1));
    workflow_neg_3<dhcp_relay_feeder>(feeder);
}

/// \brief Device WF_N_5
/// \ref WF_N_5
TEST_F(dhcp_relay_test, dhcp_relay_workflow_neg_5) {
    dhcp_relay_feeder feeder1, feeder1A;
    feeder1.init(int2pdsobjkey(1), agent_ip1, server_ip1, int2pdsobjkey(1));
    feeder1A.init(int2pdsobjkey(1), agent_ip1, server_ip2, int2pdsobjkey(1));
    workflow_neg_5<dhcp_relay_feeder>(feeder1, feeder1A);
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
