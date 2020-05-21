//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all nexthop upg test cases
///
//----------------------------------------------------------------------------

#include "nic/apollo/test/api/utils/nexthop.hpp"
#include "nic/apollo/test/api/utils/if.hpp"
#include "nic/apollo/test/api/utils/workflow.hpp"

namespace test {
namespace api {

//----------------------------------------------------------------------------
// NH upg test class
//----------------------------------------------------------------------------

class nh_upg_test : public ::pds_test_base {
protected:
    nh_upg_test() {}
    virtual ~nh_upg_test() {}
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

static inline void
nexthop_upg_setup (void)
{
    pds_batch_ctxt_t bctxt = batch_start();
    sample_if_setup(bctxt);
    batch_commit(bctxt);
}

static inline void
nexthop_upg_teardown (void)
{
    pds_batch_ctxt_t bctxt = batch_start();
    sample_if_teardown(bctxt);
    batch_commit(bctxt);
}

//----------------------------------------------------------------------------
// NH upg test cases implementation
//----------------------------------------------------------------------------

/// \defgroup NH Nexthop upg Tests
/// @{

/// \brief Nexthop group WF_U_1
/// \ref WF_U_1
TEST_F(nh_upg_test, nh_upg_workflow_u1) {
    nexthop_feeder feeder;

    pds_batch_ctxt_t bctxt = batch_start();
    // setup precursor
    nexthop_upg_setup();
    // setup nexthop
    feeder.init("", 0x0E0D0A0B0100, 10, int2pdsobjkey(1),
                PDS_NH_TYPE_UNDERLAY);
    feeder.set_stash(true);
    // backup
    workflow_u1_s1<nexthop_feeder>(feeder);

    // tearup precursor
    nexthop_upg_teardown();
    // restore
    workflow_u1_s2<nexthop_feeder>(feeder);
    // setup precursor again
    nexthop_upg_setup();
    // config replay
    workflow_u1_s3<nexthop_feeder>(feeder);
    // tearup precursor
    nexthop_upg_teardown();
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
