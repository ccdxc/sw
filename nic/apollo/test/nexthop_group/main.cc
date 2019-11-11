//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all nexthop test cases
///
//----------------------------------------------------------------------------

#include "nic/apollo/test/utils/if.hpp"
#include "nic/apollo/test/utils/nexthop.hpp"
#include "nic/apollo/test/utils/nexthop_group.hpp"
#include "nic/apollo/test/utils/tep.hpp"
#include "nic/apollo/test/utils/workflow1.hpp"

namespace api_test {
/// \cond

// Globals
static constexpr uint16_t k_num_groups = 10;

//----------------------------------------------------------------------------
// NH test class
//----------------------------------------------------------------------------

class nh_group_test : public ::pds_test_base {
protected:
    nh_group_test() {}
    virtual ~nh_group_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        if (!agent_mode())
            pds_test_base::SetUpTestCase(g_tc_params);
        pds_batch_ctxt_t bctxt = batch_start();
        sample_if_setup(bctxt);
        sample_tep_setup(bctxt);
        sample_underlay_nexthop_setup(bctxt);
        batch_commit(bctxt);
    }
    static void TearDownTestCase() {
        pds_batch_ctxt_t bctxt = batch_start();
        sample_underlay_nexthop_teardown(bctxt);
        sample_tep_teardown(bctxt);
        sample_if_teardown(bctxt);
        batch_commit(bctxt);
        if (!agent_mode())
            pds_test_base::TearDownTestCase();
    }
};
/// \endcond
//----------------------------------------------------------------------------
// NH test cases implementation
//----------------------------------------------------------------------------

/// \defgroup NH Nexthop Tests
/// @{

/// \brief NH WF_1
/// \ref WF_1
TEST_F(nh_group_test, nh_group_workflow_1) {
    nexthop_group_feeder feeder;

    feeder.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP);
    workflow_1<nexthop_group_feeder>(feeder);

    //feeder.init(PDS_NHGROUP_TYPE_OVERLAY_ECMP);
    //workflow_1<nexthop_group_feeder>(feeder);
}

/// \brief NH WF_2
/// \ref WF_2
TEST_F(nh_group_test, nh_group_workflow_2) {
    nexthop_group_feeder feeder;

    feeder.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP);
    workflow_2<nexthop_group_feeder>(feeder);

    //feeder.init(PDS_NHGROUP_TYPE_OVERLAY_ECMP);
    //workflow_2<nexthop_group_feeder>(feeder);
}

/// \brief NH WF_3
/// \ref WF_3
TEST_F(nh_group_test, nh_group_workflow_3) {
    nexthop_group_feeder feeder1, feeder2, feeder3;

    feeder1.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP, 10, k_num_groups);
    feeder2.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP, 30, k_num_groups);
    feeder3.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP, 50, k_num_groups);
    workflow_3<nexthop_group_feeder>(feeder1, feeder2, feeder3);

#if 0
    feeder1.init(PDS_NHGROUP_TYPE_OVERLAY_ECMP, 10, k_num_groups);
    feeder2.init(PDS_NHGROUP_TYPE_OVERLAY_ECMP, 30, k_num_groups);
    feeder3.init(PDS_NHGROUP_TYPE_OVERLAY_ECMP, 50, k_num_groups);
    workflow_3<nexthop_group_feeder>(feeder1, feeder2, feeder3);
#endif
}

/// \brief NH WF_4
/// \ref WF_4
TEST_F(nh_group_test, nh_group_workflow_4) {
    nexthop_group_feeder feeder;

    feeder.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP);
    workflow_4<nexthop_group_feeder>(feeder);

#if 0
    feeder.init(PDS_NHGROUP_TYPE_OVERLAY_ECMP);
    workflow_4<nexthop_group_feeder>(feeder);
#endif
}

/// \brief NH WF_5
/// \ref WF_5
TEST_F(nh_group_test, nh_group_workflow_5) {
    nexthop_group_feeder feeder1, feeder2, feeder3;

    feeder1.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP, 100, k_num_groups);
    feeder2.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP, 300, k_num_groups);
    feeder3.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP, 500, k_num_groups);
    workflow_5<nexthop_group_feeder>(feeder1, feeder2, feeder3);

#if 0
    feeder1.init(PDS_NHGROUP_TYPE_OVERLAY_ECMP, 100, k_num_groups);
    feeder2.init(PDS_NHGROUP_TYPE_OVERLAY_ECMP, 300, k_num_groups);
    feeder3.init(PDS_NHGROUP_TYPE_OVERLAY_ECMP, 500, k_num_groups);
    workflow_5<nexthop_group_feeder>(feeder1, feeder2, feeder3);
#endif
}

/// \brief NH WF_6
/// \ref WF_6
TEST_F(nh_group_test, nh_group_workflow_6) {
    nexthop_group_feeder feeder1, feeder1A, feeder1B;

    feeder1.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP, 1, k_max_groups);
    feeder1A.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP, 1, k_max_groups);
    feeder1B.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP, 1, k_max_groups);
    workflow_6<nexthop_group_feeder>(feeder1, feeder1A, feeder1B);

#if 0
    feeder1.init(PDS_NHGROUP_TYPE_OVERLAY_ECMP, 1, k_max_groups);
    feeder1A.init(PDS_NHGROUP_TYPE_OVERLAY_ECMP, 1, k_max_groups, 5);
    feeder1B.init(PDS_NHGROUP_TYPE_OVERLAY_ECMP, 1, k_num_groups, 3);
    workflow_6<nexthop_group_feeder>(feeder1, feeder1A, feeder1B);
#endif
}

/// \brief NH WF_7
/// \ref WF_7
TEST_F(nh_group_test, nh_group_workflow_7) {
    nexthop_group_feeder feeder1, feeder1A, feeder1B;

    feeder1.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP, 1, k_max_groups);
    feeder1A.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP, 1, k_max_groups);
    feeder1B.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP, 1, k_max_groups);
    workflow_7<nexthop_group_feeder>(feeder1, feeder1A, feeder1B);

#if 0
    feeder1.init(PDS_NHGROUP_TYPE_OVERLAY_ECMP, 1, k_max_groups);
    feeder1A.init(PDS_NHGROUP_TYPE_OVERLAY_ECMP, 1, k_max_groups, 5);
    feeder1B.init(PDS_NHGROUP_TYPE_OVERLAY_ECMP, 1, k_num_groups, 3);
    workflow_7<nexthop_group_feeder>(feeder1, feeder1A, feeder1B);
#endif
}

/// \brief NH WF_8
/// \ref WF_8
TEST_F(nh_group_test, DISABLED_nh_group_workflow_8) {
    nexthop_group_feeder feeder1, feeder1A, feeder1B;

    feeder1.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP, 1, k_max_groups);
    feeder1A.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP, 1, k_max_groups);
    feeder1B.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP, 1, k_max_groups);
    workflow_8<nexthop_group_feeder>(feeder1, feeder1A, feeder1B);

#if 0
    feeder1.init(PDS_NHGROUP_TYPE_OVERLAY_ECMP, 1, k_max_groups);
    feeder1A.init(PDS_NHGROUP_TYPE_OVERLAY_ECMP, 1, k_max_groups, 5);
    feeder1B.init(PDS_NHGROUP_TYPE_OVERLAY_ECMP, 1, k_num_groups, 3);
    workflow_8<nexthop_group_feeder>(feeder1, feeder1A, feeder1B);
#endif
}

/// \brief NH WF_9
/// \ref WF_9
TEST_F(nh_group_test, DISABLED_nh_group_workflow_9) {
    nexthop_group_feeder feeder1, feeder1A;

    feeder1.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP, 1, k_max_groups);
    feeder1A.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP, 1, k_max_groups);
    workflow_9<nexthop_group_feeder>(feeder1, feeder1A);

#if 0
    feeder1.init(PDS_NHGROUP_TYPE_OVERLAY_ECMP, 1, k_max_groups);
    feeder1A.init(PDS_NHGROUP_TYPE_OVERLAY_ECMP, 1, k_max_groups, 5);
    workflow_9<nexthop_group_feeder>(feeder1, feeder1A);
#endif
}

/// \brief NH WF_10
/// \ref WF_10
TEST_F(nh_group_test, DISABLED_nh_group_workflow_10) {
    nexthop_group_feeder feeder1, feeder2, feeder2A, feeder3, feeder3A, feeder4;


    feeder1.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP, 100, k_num_groups);
    feeder2.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP, 300, k_num_groups);
    feeder2A.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP, 300, k_num_groups);
    feeder3.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP, 500, k_num_groups);
    feeder3A.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP, 500, k_num_groups);
    feeder4.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP, 700, k_num_groups);
    workflow_10<nexthop_group_feeder>(feeder1, feeder2, feeder2A,
                                      feeder3, feeder3A, feeder4);

#if 0
    feeder1.init(PDS_NHGROUP_TYPE_OVERLAY_ECMP, 100, k_num_groups);
    feeder2.init(PDS_NHGROUP_TYPE_OVERLAY_ECMP, 300, k_num_groups);
    feeder2A.init(PDS_NHGROUP_TYPE_OVERLAY_ECMP, 300, k_num_groups);
    feeder3.init(PDS_NHGROUP_TYPE_OVERLAY_ECMP, 500, k_num_groups);
    feeder3A.init(PDS_NHGROUP_TYPE_OVERLAY_ECMP, 500, k_num_groups);
    feeder4.init(PDS_NHGROUP_TYPE_OVERLAY_ECMP, 700, k_num_groups);
    workflow_10<nexthop_group_feeder>(feeder1, feeder2, feeder2A,
                                      feeder3, feeder3A, feeder4);
#endif
}

/// \brief NH WF_N_1
/// \ref WF_N_1
TEST_F(nh_group_test, nh_group_workflow_neg_1) {
    nexthop_group_feeder feeder;

    feeder.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP);
    workflow_neg_1<nexthop_group_feeder>(feeder);

#if 0
    feeder.init(PDS_NHGROUP_TYPE_OVERLAY_ECMP);
    workflow_neg_1<nexthop_group_feeder>(feeder);
#endif
}

/// \brief NH WF_N_2
/// \ref WF_N_2
TEST_F(nh_group_test, DISABLED_nh_group_workflow_neg_2) {
    nexthop_group_feeder feeder;

    feeder.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP, 1, k_max_groups+1);
    workflow_neg_2<nexthop_group_feeder>(feeder);

#if 0
    feeder.init(PDS_NHGROUP_TYPE_OVERLAY_ECMP);
    workflow_neg_2<nexthop_group_feeder>(feeder);
#endif
}

/// \brief NH WF_N_3
/// \ref WF_N_3
TEST_F(nh_group_test, nh_group_workflow_neg_3) {
    nexthop_group_feeder feeder;

    feeder.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP);
    workflow_neg_3<nexthop_group_feeder>(feeder);

#if 0
    feeder.init(PDS_NHGROUP_TYPE_OVERLAY_ECMP);
    workflow_neg_3<nexthop_group_feeder>(feeder);
#endif
}

/// \brief NH WF_N_4
/// \ref WF_N_4
TEST_F(nh_group_test, nh_group_workflow_neg_4) {
    nexthop_group_feeder feeder1, feeder2;

    feeder1.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP, 100, k_num_groups);
    feeder2.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP, 300, k_num_groups);
    workflow_neg_4<nexthop_group_feeder>(feeder1, feeder2);

#if 0
    feeder1.init(PDS_NHGROUP_TYPE_OVERLAY_ECMP, 100, k_num_groups);
    feeder2.init(PDS_NHGROUP_TYPE_OVERLAY_ECMP, 300, k_num_groups);
    workflow_neg_4<nexthop_group_feeder>(feeder1, feeder2);
#endif
}

/// \brief NH WF_N_5
/// \ref WF_N_5
TEST_F(nh_group_test, DISABLED_nh_group_workflow_neg_5) {
    nexthop_group_feeder feeder1, feeder1A;

    feeder1.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP, 1, k_max_groups);
    feeder1A.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP, 1, k_max_groups);
    workflow_neg_5<nexthop_group_feeder>(feeder1, feeder1A);

#if 0
    feeder1.init(PDS_NHGROUP_TYPE_OVERLAY_ECMP, 1, k_max_groups);
    feeder1A.init(PDS_NHGROUP_TYPE_OVERLAY_ECMP, 1, k_max_groups, 5);
    workflow_neg_5<nexthop_group_feeder>(feeder1, feeder1A);
#endif
}

/// \brief NH WF_N_6
/// \ref WF_N_6
TEST_F(nh_group_test, nh_group_workflow_neg_6) {
    nexthop_group_feeder feeder1, feeder1A;

    feeder1.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP, 1, k_max_groups);
    feeder1A.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP, 1, k_max_groups+1);
    workflow_neg_6<nexthop_group_feeder>(feeder1, feeder1A);

#if 0
    feeder1.init(PDS_NHGROUP_TYPE_OVERLAY_ECMP, 1, k_max_groups);
    feeder1A.init(PDS_NHGROUP_TYPE_OVERLAY_ECMP, 1, k_max_groups+1, 5);
    workflow_neg_6<nexthop_group_feeder>(feeder1, feeder1A);
#endif
}

/// \brief NH WF_N_7
/// \ref WF_N_7
TEST_F(nh_group_test, DISABLED_nh_group_workflow_neg_7) {
    nexthop_group_feeder feeder1, feeder1A, feeder2;

    feeder1.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP, 100, k_num_groups);
    feeder1A.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP, 100, k_num_groups);
    feeder2.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP, 300, k_num_groups);
    workflow_neg_7<nexthop_group_feeder>(feeder1, feeder1A, feeder2);

#if 0
    feeder1.init(PDS_NHGROUP_TYPE_OVERLAY_ECMP, 100, k_num_groups);
    feeder1A.init(PDS_NHGROUP_TYPE_OVERLAY_ECMP, 100, k_num_groups);
    feeder2.init(PDS_NHGROUP_TYPE_OVERLAY_ECMP, 300, k_num_groups);
    workflow_neg_7<nexthop_group_feeder>(feeder1, feeder1A, feeder2);
#endif
}

/// \brief NH WF_N_8
/// \ref WF_N_8
TEST_F(nh_group_test, nh_group_workflow_neg_8) {
    nexthop_group_feeder feeder1, feeder2;

    feeder1.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP, 100, k_num_groups);
    feeder2.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP, 300, k_num_groups);
    workflow_neg_8<nexthop_group_feeder>(feeder1, feeder2);

#if 0
    feeder1.init(PDS_NHGROUP_TYPE_OVERLAY_ECMP, 100, k_num_groups);
    feeder2.init(PDS_NHGROUP_TYPE_OVERLAY_ECMP, 300, k_num_groups);
    workflow_neg_8<nexthop_group_feeder>(feeder1, feeder2);
#endif
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
    api_test_program_run(argc, argv);
}
