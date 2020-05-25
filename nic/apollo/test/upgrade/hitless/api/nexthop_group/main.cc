//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all nexthop group hitless upgrade test cases
///
//----------------------------------------------------------------------------

#include "nic/apollo/test/api/utils/if.hpp"
#include "nic/apollo/test/api/utils/nexthop.hpp"
#include "nic/apollo/test/api/utils/nexthop_group.hpp"
#include "nic/apollo/test/api/utils/workflow.hpp"

namespace test {
namespace api {

//----------------------------------------------------------------------------
// NH Group upg test class
//----------------------------------------------------------------------------

class nh_group_upg_test : public ::pds_test_base {
protected:
    nh_group_upg_test() {}
    virtual ~nh_group_upg_test() {}
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
nh_group_upg_setup (void)
{
    pds_batch_ctxt_t bctxt = batch_start();
    sample_if_setup(bctxt);
    sample_underlay_nexthop_setup(bctxt);
    batch_commit(bctxt);
}

static inline void
nh_group_upg_teardown (void)
{
    pds_batch_ctxt_t bctxt = batch_start();
    sample_underlay_nexthop_teardown(bctxt);
    sample_if_teardown(bctxt);
    batch_commit(bctxt);
}

//----------------------------------------------------------------------------
// NH group upg test cases implementation
//----------------------------------------------------------------------------

/// \defgroup NH_GRP Nexthop group upg tests
/// @{

/// \brief Nexthop group WF_U_1
/// \ref WF_U_1
TEST_F(nh_group_upg_test, nh_group_upg_workflow_u1) {
    nexthop_group_feeder feeder1;

    pds_batch_ctxt_t bctxt = batch_start();
    // setup precursor
    nh_group_upg_setup();
    // setup nh group
    feeder1.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP, PDS_MAX_ECMP_NEXTHOP,
                 int2pdsobjkey(1), 10, 1);
    // backup
    workflow_u1_s1<nexthop_group_feeder>(feeder1);

    // tearup precursor
    nh_group_upg_teardown();
    // restore
    workflow_u1_s2<nexthop_group_feeder>(feeder1);

    // setup precursor again
    nh_group_upg_setup();
    // config replay
    workflow_u1_s3<nexthop_group_feeder>(feeder1);
    // tearup precursor
    nh_group_upg_teardown();
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
    api_test_program_run(argc, argv);
}
