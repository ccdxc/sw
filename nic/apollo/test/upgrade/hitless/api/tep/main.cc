//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all upg tep test cases
///
//----------------------------------------------------------------------------

#include "nic/apollo/test/api/utils/if.hpp"
#include "nic/apollo/test/api/utils/nexthop.hpp"
#include "nic/apollo/test/api/utils/nexthop_group.hpp"
#include "nic/apollo/test/api/utils/tep.hpp"
#include "nic/apollo/test/api/utils/workflow.hpp"

namespace test {
namespace api {

//----------------------------------------------------------------------------
// TEP upg test class
//----------------------------------------------------------------------------

class tep_upg_test : public ::pds_test_base {
protected:
    tep_upg_test() {}
    virtual ~tep_upg_test() {}
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
tep_upg_setup (void)
{
    pds_batch_ctxt_t bctxt = batch_start();
    sample_if_setup(bctxt);
    sample_nexthop_setup(bctxt);
    sample_nexthop_group_setup(bctxt);
    batch_commit(bctxt);
}

static inline void
tep_upg_teardown (void)
{
    pds_batch_ctxt_t bctxt = batch_start();
    sample_nexthop_teardown(bctxt);
    sample_if_teardown(bctxt);
    sample_nexthop_group_teardown(bctxt);
    batch_commit(bctxt);
}

//----------------------------------------------------------------------------
// TEP upg test cases implementation
//----------------------------------------------------------------------------

/// \defgroup TEP TEP upg Tests
/// @{

/// \brief Nexthop group WF_U_1
/// \ref WF_U_1
TEST_F(tep_upg_test, tep_upg_workflow_u1) {
    tep_feeder feeder;

    pds_batch_ctxt_t bctxt = batch_start();
    // setup precursor
    tep_upg_setup();
    // setup tep
    feeder.init(2, 0x0E0D0A0B0200, "50.50.1.1");
    feeder.set_stash(true);
    // backup
    workflow_u1_s1<tep_feeder>(feeder);

    // tearup precursor
    tep_upg_teardown();
    // restore
    workflow_u1_s2<tep_feeder>(feeder);
    // setup precursor again
    tep_upg_setup();
    // config replay
    workflow_u1_s3<tep_feeder>(feeder);
    // tearup precursor
    tep_upg_teardown();
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
