//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains all vpc peer test cases
///
//----------------------------------------------------------------------------

#include "nic/apollo/test/utils/vpc.hpp"
#include "nic/apollo/test/utils/vpc_peer.hpp"
#include "nic/apollo/test/utils/workflow.hpp"

namespace api_test {
/// \cond
// Globals
static constexpr uint32_t k_max_vpc = PDS_MAX_VPC;
static constexpr uint32_t k_max_vpc_peer = PDS_MAX_VPC/2;

//----------------------------------------------------------------------------
// VPC_PEER test class
//----------------------------------------------------------------------------

class vpc_peer : public ::pds_test_base {
protected:
    vpc_peer() {}
    virtual ~vpc_peer() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        if (!agent_mode())
            pds_test_base::SetUpTestCase(g_tc_params);

        pds_batch_ctxt_t bctxt = batch_start();
        sample1_vpc_setup(bctxt, PDS_VPC_TYPE_TENANT);
        batch_commit(bctxt);
        sample1_vpc_setup_validate(PDS_VPC_TYPE_TENANT);
    }

    static void TearDownTestCase() {
        if (!agent_mode())
            pds_test_base::TearDownTestCase();

        pds_batch_ctxt_t bctxt = batch_start();
        sample1_vpc_teardown(bctxt, PDS_VPC_TYPE_TENANT);
        batch_commit(bctxt);
    }
};
/// \endcond
//----------------------------------------------------------------------------
// VPC peer test cases implementation
//----------------------------------------------------------------------------

/// \defgroup VPC_PEER_TEST VPC Peer Tests
/// @{

/// \brief VPC_PEER WF_1
/// \ref WF_1
TEST_F(vpc_peer, DISABLED_vpc_peer_workflow_1) {
    vpc_peer_feeder feeder;
    pds_vpc_peer_key_t key = {.id = 1};
    pds_vpc_key_t vpc1 = {.id = 1};
    pds_vpc_key_t vpc2 = {.id = 2};

    feeder.init(key, vpc1, vpc2, k_max_vpc_peer);
    workflow_1<vpc_peer_feeder>(feeder);
}

/// \brief VPC_PEER WF_2
/// \ref WF_2
TEST_F(vpc_peer, DISABLED_vpc_peer_workflow_2) {
    vpc_peer_feeder feeder;
    pds_vpc_peer_key_t key = {.id = 1};
    pds_vpc_key_t vpc1 = {.id = 1};
    pds_vpc_key_t vpc2 = {.id = 2};

    feeder.init(key, vpc1, vpc2, k_max_vpc_peer);
    workflow_2<vpc_peer_feeder>(feeder);
}

/// \brief VPC_PEER WF_3
/// \ref WF_3
TEST_F(vpc_peer, DISABLED_vpc_peer_workflow_3) {
    vpc_peer_feeder feeder1, feeder2, feeder3;
    pds_vpc_peer_key_t key1 = {.id = 10}, key2 = {.id = 30}, key3 = {.id = 50};
    pds_vpc_key_t vpc1 = {.id = 1};
    pds_vpc_key_t vpc2 = {.id = 2};
    pds_vpc_key_t vpc3 = {.id = 1};
    pds_vpc_key_t vpc4 = {.id = 20};
    pds_vpc_key_t vpc5 = {.id = 1};
    pds_vpc_key_t vpc6 = {.id = 40};

    feeder1.init(key1, vpc1, vpc2, 10);
    feeder2.init(key2, vpc3, vpc4, 10);
    feeder3.init(key3, vpc5, vpc6, 10);
    workflow_3<vpc_peer_feeder>(feeder1, feeder2, feeder3);
}

/// \brief VPC_PEER WF_4
/// \ref WF_4
TEST_F(vpc_peer, DISABLED_vpc_peer_workflow_4) {
    vpc_peer_feeder feeder;
    pds_vpc_peer_key_t key = {.id = 1};
    pds_vpc_key_t vpc1 = {.id = 1};
    pds_vpc_key_t vpc2 = {.id = 2};

    feeder.init(key, vpc1, vpc2, k_max_vpc_peer);
    workflow_4<vpc_peer_feeder>(feeder);
}

/// \brief VPC_PEER WF_5
/// \ref WF_5
TEST_F(vpc_peer, DISABLED_vpc_peer_workflow_5) {
    vpc_peer_feeder feeder1, feeder2, feeder3;
    pds_vpc_peer_key_t key1 = {.id = 10}, key2 = {.id = 30}, key3 = {.id = 50};
    pds_vpc_key_t vpc1 = {.id = 1};
    pds_vpc_key_t vpc2 = {.id = 2};
    pds_vpc_key_t vpc3 = {.id = 1};
    pds_vpc_key_t vpc4 = {.id = 20};
    pds_vpc_key_t vpc5 = {.id = 1};
    pds_vpc_key_t vpc6 = {.id = 40};

    feeder1.init(key1, vpc1, vpc2, 10);
    feeder2.init(key2, vpc1, vpc2, 10);
    feeder3.init(key3, vpc1, vpc2, 10);
    workflow_5<vpc_peer_feeder>(feeder1, feeder2, feeder3);
}

/// \brief VPC_PEER WF_6
/// \ref WF_6
TEST_F(vpc_peer, DISABLED_vpc_peer_workflow_6) {
    vpc_peer_feeder feeder1, feeder1A, feeder1B;
    pds_vpc_peer_key_t key = {.id = 1};
    pds_vpc_key_t vpc1 = {.id = 1};
    pds_vpc_key_t vpc2 = {.id = 2};
    pds_vpc_key_t vpc3 = {.id = 1};
    pds_vpc_key_t vpc4 = {.id = 20};
    pds_vpc_key_t vpc5 = {.id = 1};
    pds_vpc_key_t vpc6 = {.id = 40};

    feeder1.init(key, vpc1, vpc2, k_max_vpc_peer);
    feeder1A.init(key, vpc3, vpc4, k_max_vpc_peer);
    feeder1B.init(key, vpc5, vpc6, k_max_vpc_peer);
    workflow_6<vpc_peer_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief VPC_PEER WF_7
/// \ref WF_7
TEST_F(vpc_peer, DISABLED_vpc_peer_workflow_7) {
    vpc_peer_feeder feeder1, feeder1A, feeder1B;
    pds_vpc_peer_key_t key = {.id = 1};
    pds_vpc_key_t vpc1 = {.id = 1};
    pds_vpc_key_t vpc2 = {.id = 2};
    pds_vpc_key_t vpc3 = {.id = 1};
    pds_vpc_key_t vpc4 = {.id = 20};
    pds_vpc_key_t vpc5 = {.id = 1};
    pds_vpc_key_t vpc6 = {.id = 40};

    feeder1.init(key, vpc1, vpc2, k_max_vpc_peer);
    feeder1A.init(key, vpc3, vpc4, k_max_vpc_peer);
    feeder1B.init(key, vpc5, vpc6, k_max_vpc_peer);
    workflow_7<vpc_peer_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief VPC_PEER WF_8
/// \ref WF_8
TEST_F(vpc_peer, DISABLED_vpc_peer_workflow_8) {
    vpc_peer_feeder feeder1, feeder1A, feeder1B;
    pds_vpc_peer_key_t key = {.id = 1};
    pds_vpc_key_t vpc1 = {.id = 1};
    pds_vpc_key_t vpc2 = {.id = 2};
    pds_vpc_key_t vpc3 = {.id = 1};
    pds_vpc_key_t vpc4 = {.id = 20};
    pds_vpc_key_t vpc5 = {.id = 1};
    pds_vpc_key_t vpc6 = {.id = 40};

    feeder1.init(key, vpc1, vpc2, k_max_vpc_peer);
    feeder1A.init(key, vpc3, vpc4, k_max_vpc_peer);
    feeder1B.init(key, vpc5, vpc6, k_max_vpc_peer);
    workflow_8<vpc_peer_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief VPC_PEER WF_9
/// \ref WF_9
TEST_F(vpc_peer, DISABLED_vpc_peer_workflow_9) {
    vpc_peer_feeder feeder1, feeder1A;
    pds_vpc_peer_key_t key = {.id = 1};
    pds_vpc_key_t vpc1 = {.id = 1};
    pds_vpc_key_t vpc2 = {.id = 2};
    pds_vpc_key_t vpc3 = {.id = 1};
    pds_vpc_key_t vpc4 = {.id = 20};

    feeder1.init(key, vpc1, vpc2, k_max_vpc_peer);
    feeder1A.init(key, vpc3, vpc4, k_max_vpc_peer);
    workflow_9<vpc_peer_feeder>(feeder1, feeder1A);
}

/// \brief VPC_PEER WF_10
/// \ref WF_10
TEST_F(vpc_peer, DISABLED_vpc_peer_workflow_10) {
    vpc_peer_feeder feeder1, feeder2, feeder3, feeder4, feeder2A, feeder3A;
    pds_vpc_peer_key_t key1 = {.id = 10}, key2 = {.id = 40}, key3 = {.id = 70},
                       key4 = {.id = 100};
    pds_vpc_key_t vpc1 = {.id = 1};
    pds_vpc_key_t vpc2 = {.id = 2};
    pds_vpc_key_t vpc3 = {.id = 1};
    pds_vpc_key_t vpc4 = {.id = 20};
    pds_vpc_key_t vpc5 = {.id = 1};
    pds_vpc_key_t vpc6 = {.id = 21};
    pds_vpc_key_t vpc7 = {.id = 1};
    pds_vpc_key_t vpc8 = {.id = 40};
    pds_vpc_key_t vpc9 = {.id = 1};
    pds_vpc_key_t vpc10 = {.id = 41};
    pds_vpc_key_t vpc11 = {.id = 1};
    pds_vpc_key_t vpc12 = {.id = 60};

    feeder1.init(key1, vpc1, vpc2, 10);
    feeder2.init(key2, vpc3, vpc4, 10);
    feeder2A.init(key2, vpc5, vpc6, 10);
    feeder3.init(key3, vpc7, vpc8, 10);
    feeder3A.init(key3, vpc9, vpc10, 10);
    feeder4.init(key4, vpc11, vpc12, 10);
    workflow_10<vpc_peer_feeder>(
        feeder1, feeder2, feeder2A, feeder3, feeder3A, feeder4);
}

/// \brief VPC_PEER WF_N_1
/// \ref WF_N_1
TEST_F(vpc_peer, DISABLED_vpc_peer_workflow_neg_1) {
    vpc_peer_feeder feeder;
    pds_vpc_peer_key_t key = {.id = 1};
    pds_vpc_key_t vpc1 = {.id = 1};
    pds_vpc_key_t vpc2 = {.id = 2};

    feeder.init(key, vpc1, vpc2, k_max_vpc_peer);
    workflow_neg_1<vpc_peer_feeder>(feeder);
}

/// \brief VPC_PEER WF_N_2
/// \ref WF_N_2
TEST_F(vpc_peer, DISABLED_vpc_peer_workflow_neg_2) {
    vpc_peer_feeder feeder;
    pds_vpc_peer_key_t key = {.id = 1};
    pds_vpc_key_t vpc1 = {.id = 1};
    pds_vpc_key_t vpc2 = {.id = 2};

    feeder.init(key, vpc1, vpc2, k_max_vpc_peer+1);
    workflow_neg_2<vpc_peer_feeder>(feeder);
}

/// \brief VPC_PEER WF_N_3
/// \ref WF_N_3
TEST_F(vpc_peer, DISABLED_vpc_peer_workflow_neg_3) {
    vpc_peer_feeder feeder;
    pds_vpc_peer_key_t key = {.id = 1};
    pds_vpc_key_t vpc1 = {.id = 1};
    pds_vpc_key_t vpc2 = {.id = 2};

    feeder.init(key, vpc1, vpc2, k_max_vpc_peer);
    workflow_neg_3<vpc_peer_feeder>(feeder);
}

/// \brief VPC_PEER WF_N_4
/// \ref WF_N_4
TEST_F(vpc_peer, DISABLED_vpc_peer_workflow_neg_4) {
    vpc_peer_feeder feeder1, feeder2;
    pds_vpc_peer_key_t key1 = {.id = 10}, key2 = {.id = 30};
    pds_vpc_key_t vpc1 = {.id = 1};
    pds_vpc_key_t vpc2 = {.id = 2};
    pds_vpc_key_t vpc3 = {.id = 1};
    pds_vpc_key_t vpc4 = {.id = 20};

    feeder1.init(key1, vpc1, vpc2, 10);
    feeder2.init(key2, vpc3, vpc4, 10);
    workflow_neg_4<vpc_peer_feeder>(feeder1, feeder2);
}

/// \brief VPC_PEER WF_N_5
/// \ref WF_N_5
TEST_F(vpc_peer, DISABLED_vpc_peer_workflow_neg_5) {
    vpc_peer_feeder feeder1, feeder1A;
    pds_vpc_peer_key_t key = {.id = 1};
    pds_vpc_key_t vpc1 = {.id = 1};
    pds_vpc_key_t vpc2 = {.id = 2};
    pds_vpc_key_t vpc3 = {.id = 1};
    pds_vpc_key_t vpc4 = {.id = 20};

    feeder1.init(key, vpc1, vpc2, k_max_vpc_peer);
    feeder1A.init(key, vpc3, vpc4, k_max_vpc_peer);
    workflow_neg_5<vpc_peer_feeder>(feeder1, feeder1A);
}

/// \brief VPC_PEER WF_N_6
/// \ref WF_N_6
TEST_F(vpc_peer, DISABLED_vpc_peer_workflow_neg_6) {
    vpc_peer_feeder feeder1, feeder1A;
    pds_vpc_peer_key_t key = {.id = 1};
    pds_vpc_key_t vpc1 = {.id = 1};
    pds_vpc_key_t vpc2 = {.id = 2};
    pds_vpc_key_t vpc3 = {.id = 1};
    pds_vpc_key_t vpc4 = {.id = 20};

    feeder1.init(key, vpc1, vpc2, k_max_vpc_peer);
    feeder1A.init(key, vpc3, vpc4, k_max_vpc_peer + 1);
    workflow_neg_6<vpc_peer_feeder>(feeder1, feeder1A);
}

/// \brief VPC_PEER WF_N_7
/// \ref WF_N_7
TEST_F(vpc_peer, DISABLED_vpc_peer_workflow_neg_7) {
    vpc_peer_feeder feeder1, feeder1A, feeder2;
    pds_vpc_peer_key_t key1 = {.id = 10}, key2 = {.id = 30};
    pds_vpc_key_t vpc1 = {.id = 1};
    pds_vpc_key_t vpc2 = {.id = 2};
    pds_vpc_key_t vpc3 = {.id = 1};
    pds_vpc_key_t vpc4 = {.id = 3};
    pds_vpc_key_t vpc5 = {.id = 1};
    pds_vpc_key_t vpc6 = {.id = 40};

    feeder1.init(key1, vpc1, vpc2, 10);
    feeder1A.init(key1, vpc3, vpc4, 10);
    feeder2.init(key2, vpc5, vpc6, 10);
    workflow_neg_7<vpc_peer_feeder>(feeder1, feeder1A, feeder2);
}

/// \brief VPC_PEER WF_N_8
/// \ref WF_N_8
TEST_F(vpc_peer, DISABLED_vpc_peer_workflow_neg_8) {
    vpc_peer_feeder feeder1, feeder2;
    pds_vpc_peer_key_t key1 = {.id = 10}, key2 = {.id = 30};
    pds_vpc_key_t vpc1 = {.id = 1};
    pds_vpc_key_t vpc2 = {.id = 2};
    pds_vpc_key_t vpc3 = {.id = 1};
    pds_vpc_key_t vpc4 = {.id = 20};

    feeder1.init(key1, vpc1, vpc2, 10);
    feeder2.init(key2, vpc3, vpc4, 10);
    workflow_neg_8<vpc_peer_feeder>(feeder1, feeder2);
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
