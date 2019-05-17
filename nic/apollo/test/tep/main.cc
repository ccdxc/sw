//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all tep test cases
///
//----------------------------------------------------------------------------

#include <getopt.h>
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/test/utils/base.hpp"
#include "nic/apollo/test/utils/tep.hpp"

using std::cerr;
using std::cout;
using std::endl;

namespace api_test {

// Globals
char *g_cfg_file = NULL;
static pds_epoch_t g_batch_epoch = PDS_EPOCH_INVALID;
constexpr int k_max_tep = PDS_MAX_TEP;
constexpr pds_encap_t k_mplsoudp_encap = {PDS_ENCAP_TYPE_MPLSoUDP, 11};
constexpr pds_encap_t k_vxlan_encap = {PDS_ENCAP_TYPE_VXLAN, 22};
constexpr bool k_nat = TRUE;

//----------------------------------------------------------------------------
// TEP test class
//----------------------------------------------------------------------------

class tep_test : public pds_test_base {
protected:
    tep_test() {}
    virtual ~tep_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        test_case_params_t params;
        params.cfg_file = api_test::g_cfg_file;
        params.enable_fte = false;
        pds_test_base::SetUpTestCase(params);
    }
    static void TearDownTestCase() {
        pds_test_base::TearDownTestCase();
    }
};

//----------------------------------------------------------------------------
// TEP test cases implementation
//----------------------------------------------------------------------------

/// \defgroup TEP
/// @{

/// \brief Create and delete maximum TEPs in the same batch
/// The operation should be de-duped by framework and is a NO-OP
/// from hardware perspective
/// [ Create SetMax - Delete SetMax ] - Read
TEST_F(tep_test, tep_workflow1) {
    pds_batch_params_t batch_params = {0};
    tep_stepper_seed_t seed = {};

    TEP_SEED_INIT(k_max_tep, "50.50.1.1", PDS_TEP_TYPE_WORKLOAD,
                  k_mplsoudp_encap, k_nat, &seed);

    // trigger
    BATCH_START();
    TEP_MANY_CREATE(&seed);
    TEP_MANY_DELETE(&seed);
    BATCH_COMMIT();

    TEP_MANY_READ_FAIL(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create, delete and create max TEPs in the same batch
/// Create and delete should be de-deduped by framework and subsequent
/// create should result in successful creation
/// [ Create SetMax - Delete SetMax - Create SetMax ] - Read
TEST_F(tep_test, tep_workflow2) {
    pds_batch_params_t batch_params = {0};
    tep_stepper_seed_t seed = {};

    TEP_SEED_INIT(k_max_tep, "50.50.1.1", PDS_TEP_TYPE_WORKLOAD,
                  k_mplsoudp_encap, k_nat, &seed);

    // trigger
    BATCH_START();
    TEP_MANY_CREATE(&seed);
    TEP_MANY_DELETE(&seed);
    TEP_MANY_CREATE(&seed);
    BATCH_COMMIT();

    TEP_MANY_READ(&seed);

    // cleanup
    BATCH_START();
    TEP_MANY_DELETE(&seed);
    BATCH_COMMIT();

    TEP_MANY_READ_FAIL(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create, delete some and create another set of TEPs in the same batch
/// [ Create Set1, Set2 - Delete Set1 - Create Set3 ] - Read
TEST_F(tep_test, tep_workflow3) {
    pds_batch_params_t batch_params = {0};
    tep_stepper_seed_t seedA = {}, seedB = {}, seedC = {};

    TEP_SEED_INIT(10, "10.10.1.1", PDS_TEP_TYPE_WORKLOAD,
                  k_mplsoudp_encap, k_nat, &seedA);
    TEP_SEED_INIT(20, "20.20.1.1", PDS_TEP_TYPE_WORKLOAD,
                  k_mplsoudp_encap, k_nat, &seedB);
    TEP_SEED_INIT(30, "30.30.1.1", PDS_TEP_TYPE_WORKLOAD,
                  k_mplsoudp_encap, k_nat, &seedC);

    // trigger
    BATCH_START();
    TEP_MANY_CREATE(&seedA);
    TEP_MANY_CREATE(&seedB);
    TEP_MANY_DELETE(&seedA);
    TEP_MANY_CREATE(&seedC);
    BATCH_COMMIT();

    TEP_MANY_READ_FAIL(&seedA, sdk::SDK_RET_ENTRY_NOT_FOUND);
    TEP_MANY_READ(&seedB);
    TEP_MANY_READ(&seedC);

    // cleanup
    BATCH_START();
    TEP_MANY_DELETE(&seedB);
    TEP_MANY_DELETE(&seedC);
    BATCH_COMMIT();

    TEP_MANY_READ_FAIL(&seedB, sdk::SDK_RET_ENTRY_NOT_FOUND);
    TEP_MANY_READ_FAIL(&seedC, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create and delete max TEPs in different batches
/// [ Create SetMax ] - Read - [ Delete SetMax ] - Read
TEST_F(tep_test, tep_workflow4) {
    pds_batch_params_t batch_params = {0};
    tep_stepper_seed_t seed = {};

    TEP_SEED_INIT(k_max_tep, "50.50.1.1", PDS_TEP_TYPE_WORKLOAD,
                  k_mplsoudp_encap, k_nat, &seed);

    // trigger
    BATCH_START();
    TEP_MANY_CREATE(&seed);
    BATCH_COMMIT();

    TEP_MANY_READ(&seed);

    BATCH_START();
    TEP_MANY_DELETE(&seed);
    BATCH_COMMIT();

    TEP_MANY_READ_FAIL(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create and delete mix and match of TEPs in different batches
/// [ Create Set1, Set2 ] - Read - [Delete Set1 - Create Set3 ] - Read
TEST_F(tep_test, tep_workflow5) {
    pds_batch_params_t batch_params = {0};
    tep_stepper_seed_t seedA = {}, seedB = {}, seedC = {};

    TEP_SEED_INIT(10, "10.10.1.1", PDS_TEP_TYPE_WORKLOAD,
                  k_mplsoudp_encap, k_nat, &seedA);
    TEP_SEED_INIT(20, "20.20.1.1", PDS_TEP_TYPE_WORKLOAD,
                  k_mplsoudp_encap, k_nat, &seedB);
    TEP_SEED_INIT(30, "30.30.1.1", PDS_TEP_TYPE_WORKLOAD,
                  k_mplsoudp_encap, k_nat, &seedC);

    // trigger
    BATCH_START();
    TEP_MANY_CREATE(&seedA);
    TEP_MANY_CREATE(&seedB);
    BATCH_COMMIT();

    TEP_MANY_READ(&seedA);
    TEP_MANY_READ(&seedB);

    BATCH_START();
    TEP_MANY_DELETE(&seedA);
    TEP_MANY_CREATE(&seedC);
    BATCH_COMMIT();

    TEP_MANY_READ_FAIL(&seedA, sdk::SDK_RET_ENTRY_NOT_FOUND);
    TEP_MANY_READ(&seedB);
    TEP_MANY_READ(&seedC);

    // cleanup
    BATCH_START();
    TEP_MANY_DELETE(&seedB);
    TEP_MANY_DELETE(&seedC);
    BATCH_COMMIT();

    TEP_MANY_READ_FAIL(&seedB, sdk::SDK_RET_ENTRY_NOT_FOUND);
    TEP_MANY_READ_FAIL(&seedC, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create, update and delete maximum TEPs in the same batch
/// The operation should be de-duped by framework and is a NO-OP
/// from hardware perspective
/// [ Create SetMax - Update SetMax - Update SetMax - Delete SetMax ] - Read
TEST_F(tep_test, tep_workflow6) {
    pds_batch_params_t batch_params = {0};
    tep_stepper_seed_t seedA = {}, seedA1 = {}, seedA2 = {};

    TEP_SEED_INIT(k_max_tep, "50.50.1.1", PDS_TEP_TYPE_WORKLOAD,
                  k_mplsoudp_encap, k_nat, &seedA);
    // seedA1 =  seedA + different encap
    TEP_SEED_INIT(k_max_tep, "50.50.1.1", PDS_TEP_TYPE_WORKLOAD,
                  k_vxlan_encap, k_nat, &seedA1);
    // seedA2 =  seedA1 + different tunnel type, encap, nat
    TEP_SEED_INIT(k_max_tep, "50.50.1.1", PDS_TEP_TYPE_IGW,
                  k_mplsoudp_encap, FALSE, &seedA2);

    // trigger
    BATCH_START();
    TEP_MANY_CREATE(&seedA);
    TEP_MANY_UPDATE(&seedA1);
    TEP_MANY_UPDATE(&seedA2);
    TEP_MANY_DELETE(&seedA2);
    BATCH_COMMIT();

    TEP_MANY_READ_FAIL(&seedA2, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create, delete, create, update, update max TEPs in the same batch
/// Create and delete should be de-deduped by framework and subsequent
/// create and double update should succeed
/// [ Create Max - Delete Max - Create Max - Update Max - Update Max]
TEST_F(tep_test, tep_workflow7) {
    pds_batch_params_t batch_params = {0};
    tep_stepper_seed_t seedA = {}, seedA1 = {}, seedA2 = {};

    TEP_SEED_INIT(k_max_tep, "50.50.1.1", PDS_TEP_TYPE_WORKLOAD,
                  k_mplsoudp_encap, k_nat, &seedA);
    // seedA1 =  seedA + different encap
    TEP_SEED_INIT(k_max_tep, "50.50.1.1", PDS_TEP_TYPE_WORKLOAD,
                  k_vxlan_encap, k_nat, &seedA1);
    // seedA2 =  seedA1 + different tunnel type, encap, nat
    TEP_SEED_INIT(k_max_tep, "50.50.1.1", PDS_TEP_TYPE_IGW,
                  k_mplsoudp_encap, FALSE, &seedA2);

    // trigger
    BATCH_START();
    TEP_MANY_CREATE(&seedA);
    TEP_MANY_DELETE(&seedA);
    TEP_MANY_CREATE(&seedA);
    TEP_MANY_UPDATE(&seedA1);
    TEP_MANY_UPDATE(&seedA2);
    BATCH_COMMIT();

    // verify
    TEP_MANY_READ(&seedA2);

    // cleanup
    BATCH_START();
    TEP_MANY_DELETE(&seedA2);
    BATCH_COMMIT();
    TEP_MANY_READ_FAIL(&seedA2, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create and update max TEPs in same batch
/// [ Create Max - Update Max ] - Read - [ Update Max ] - Read - [ Delete Max ]
TEST_F(tep_test, DISABLED_tep_workflow8) {
    pds_batch_params_t batch_params = {0};
    tep_stepper_seed_t seedA = {}, seedA1 = {}, seedA2 = {};

    TEP_SEED_INIT(k_max_tep, "50.50.1.1", PDS_TEP_TYPE_WORKLOAD,
                  k_mplsoudp_encap, k_nat, &seedA);
    // seedA1 =  seedA + different encap
    TEP_SEED_INIT(k_max_tep, "50.50.1.1", PDS_TEP_TYPE_WORKLOAD,
                  k_vxlan_encap, k_nat, &seedA1);
    // seedA2 =  seedA1 + different tunnel type, encap, nat
    TEP_SEED_INIT(k_max_tep, "50.50.1.1", PDS_TEP_TYPE_IGW,
                  k_mplsoudp_encap, FALSE, &seedA2);

    // trigger
    BATCH_START();
    TEP_MANY_CREATE(&seedA);
    TEP_MANY_UPDATE(&seedA1);
    BATCH_COMMIT();

    // TODO: tunnel type will be lost with vxlan encap
    TEP_MANY_READ(&seedA1);

    BATCH_START();
    TEP_MANY_UPDATE(&seedA2);
    BATCH_COMMIT();

    TEP_MANY_READ(&seedA2);

    BATCH_START();
    TEP_MANY_DELETE(&seedA2);
    BATCH_COMMIT();
    TEP_MANY_READ_FAIL(&seedA2, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Update and delete max TEPs in same batch
/// [ Create SetMax ] - Read - [ Update SetMax - Delete SetMax ] - Read
TEST_F(tep_test, tep_workflow9) {
    pds_batch_params_t batch_params = {0};
    tep_stepper_seed_t seedA = {}, seedA1 = {};

    TEP_SEED_INIT(k_max_tep, "50.50.1.1", PDS_TEP_TYPE_WORKLOAD,
                  k_mplsoudp_encap, k_nat, &seedA);
    // seedA1 =  seedA + different tunnel type, encap, nat
    TEP_SEED_INIT(k_max_tep, "50.50.1.1", PDS_TEP_TYPE_IGW,
                  k_vxlan_encap, FALSE, &seedA1);

    // trigger
    BATCH_START();
    TEP_MANY_CREATE(&seedA);
    BATCH_COMMIT();

    TEP_MANY_READ(&seedA);

    BATCH_START();
    TEP_MANY_UPDATE(&seedA1);
    TEP_MANY_DELETE(&seedA1);
    BATCH_COMMIT();

    TEP_MANY_READ_FAIL(&seedA1, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create, update and delete mix and match of TEPs in different batches
/// [ Create Set1, Set2, Set3 - Delete Set1 - Update Set2 ] - Read
/// - [ Update Set3 - Delete Set2 - Create Set4] - Read
TEST_F(tep_test, DISABLED_tep_workflow10) {
    pds_batch_params_t batch_params = {0};
    tep_stepper_seed_t seedA = {}, seedB = {}, seedC = {}, seedD = {};
    tep_stepper_seed_t seedB1 = {}, seedC1 = {};

    TEP_SEED_INIT(10, "10.10.1.1", PDS_TEP_TYPE_WORKLOAD,
                  k_mplsoudp_encap, k_nat, &seedA);
    TEP_SEED_INIT(20, "20.20.1.1", PDS_TEP_TYPE_WORKLOAD,
                  k_mplsoudp_encap, k_nat, &seedB);
    TEP_SEED_INIT(20, "20.20.1.1", PDS_TEP_TYPE_IGW,
                  k_mplsoudp_encap, FALSE, &seedB1);
    TEP_SEED_INIT(30, "30.30.1.1", PDS_TEP_TYPE_WORKLOAD,
                  k_mplsoudp_encap, k_nat, &seedC);
    TEP_SEED_INIT(30, "30.30.1.1", PDS_TEP_TYPE_WORKLOAD,
                  k_mplsoudp_encap, FALSE, &seedC1);
    TEP_SEED_INIT(40, "40.40.1.1", PDS_TEP_TYPE_WORKLOAD,
                  k_mplsoudp_encap, k_nat, &seedD);

    // trigger
    BATCH_START();
    TEP_MANY_CREATE(&seedA);
    TEP_MANY_CREATE(&seedB);
    TEP_MANY_CREATE(&seedC);
    TEP_MANY_DELETE(&seedA);
    // update setB to different tunnel type and nat
    TEP_MANY_UPDATE(&seedB1);
    BATCH_COMMIT();

    TEP_MANY_READ_FAIL(&seedA, sdk::SDK_RET_ENTRY_NOT_FOUND);
    TEP_MANY_READ(&seedB1);
    TEP_MANY_READ(&seedC);

    BATCH_START();
    // update setC to different nat
    TEP_MANY_UPDATE(&seedC1);
    TEP_MANY_DELETE(&seedB1);
    TEP_MANY_CREATE(&seedD);
    BATCH_COMMIT();

    TEP_MANY_READ_FAIL(&seedB1, sdk::SDK_RET_ENTRY_NOT_FOUND);
    TEP_MANY_READ(&seedC1);
    TEP_MANY_READ(&seedD);

    // cleanup
    BATCH_START();
    TEP_MANY_DELETE(&seedC1);
    TEP_MANY_DELETE(&seedD);
    BATCH_COMMIT();

    TEP_MANY_READ_FAIL(&seedC1, sdk::SDK_RET_ENTRY_NOT_FOUND);
    TEP_MANY_READ_FAIL(&seedD, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create maximum number of VPCs in two batches
/// [ Create SetMax ] - [ Create SetMax ] - Read
TEST_F(tep_test, tep_workflow_neg_1) {
    pds_batch_params_t batch_params = {0};
    tep_stepper_seed_t seed = {};

    TEP_SEED_INIT(k_max_tep, "50.50.1.1", PDS_TEP_TYPE_WORKLOAD,
                  k_mplsoudp_encap, k_nat, &seed);

    // trigger
    BATCH_START();
    TEP_MANY_CREATE(&seed);
    BATCH_COMMIT();

    TEP_MANY_READ(&seed);

    BATCH_START();
    TEP_MANY_CREATE(&seed);
    BATCH_COMMIT_FAILURE(sdk::SDK_RET_INVALID_OP);
    BATCH_ABORT();

    TEP_MANY_READ(&seed);

    // cleanup
    BATCH_START();
    TEP_MANY_DELETE(&seed);
    BATCH_COMMIT();

    TEP_MANY_READ_FAIL(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create more than maximum number of TEPs supported.
/// [ Create SetMax+1 ] - Read
TEST_F(tep_test, tep_workflow_neg_2) {
    pds_batch_params_t batch_params = {0};
    tep_stepper_seed_t seed = {};

    TEP_SEED_INIT(k_max_tep + 2, "50.50.1.1", PDS_TEP_TYPE_WORKLOAD,
                  k_mplsoudp_encap, k_nat, &seed);

    // trigger
    BATCH_START();
    TEP_MANY_CREATE(&seed);
    BATCH_COMMIT_FAILURE(sdk::SDK_RET_NO_RESOURCE);
    BATCH_ABORT();

    TEP_MANY_READ_FAIL(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Read and delete non existing TEPs
/// Read NonEx - [ Delete NonExMax ]
TEST_F(tep_test, tep_workflow_neg_3) {
    pds_batch_params_t batch_params = {0};
    tep_stepper_seed_t seed = {};

    TEP_SEED_INIT(k_max_tep, "150.150.1.1", PDS_TEP_TYPE_WORKLOAD,
                  k_mplsoudp_encap, k_nat, &seed);

    // trigger
    TEP_MANY_READ_FAIL(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);

    // trigger
    BATCH_START();
    TEP_MANY_DELETE(&seed);
    BATCH_COMMIT_FAILURE(sdk::SDK_RET_ENTRY_NOT_FOUND);
    BATCH_ABORT();
}

/// \brief Invalid batch shouldn't affect entries of previous batch
/// [ Create Set1 ] - [Delete Set1, Set2 ] - Read
TEST_F(tep_test, tep_workflow_neg_4) {
    pds_batch_params_t batch_params = {0};
    tep_stepper_seed_t seedA = {}, seedB = {};

    TEP_SEED_INIT(10, "50.50.1.1", PDS_TEP_TYPE_WORKLOAD,
                  k_mplsoudp_encap, k_nat, &seedA);
    TEP_SEED_INIT(10, "60.60.1.1", PDS_TEP_TYPE_WORKLOAD,
                  k_mplsoudp_encap, k_nat, &seedB);

    // trigger
    BATCH_START();
    TEP_MANY_CREATE(&seedA);
    BATCH_COMMIT();

    TEP_MANY_READ(&seedA);

    BATCH_START();
    TEP_MANY_DELETE(&seedA);
    TEP_MANY_DELETE(&seedB);
    BATCH_COMMIT_FAILURE(sdk::SDK_RET_ENTRY_NOT_FOUND);
    BATCH_ABORT();

    TEP_MANY_READ(&seedA);
    TEP_MANY_READ_FAIL(&seedB, sdk::SDK_RET_ENTRY_NOT_FOUND);

    // cleanup
    BATCH_START();
    TEP_MANY_DELETE(&seedA);
    BATCH_COMMIT();

    TEP_MANY_READ_FAIL(&seedA, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief TEP workflow corner case 4
/// [ Create SetCorner ] - Read
TEST_F(tep_test, tep_workflowcorner_case_4) {}

/// \brief Update the deleted max TEPs in same batch
/// [ Create SetMax ] - Read - [ Delete SetMax - Update SetMax ] - Read
TEST_F(tep_test, DISABLED_tep_workflow_neg_5) {
    pds_batch_params_t batch_params = {0};
    tep_stepper_seed_t seed = {}, seed1 = {};

    TEP_SEED_INIT(k_max_tep, "50.50.1.1", PDS_TEP_TYPE_WORKLOAD,
                  k_mplsoudp_encap, k_nat, &seed);
    // seed1 = seed + different tunnel type, nat
    TEP_SEED_INIT(k_max_tep, "50.50.1.1", PDS_TEP_TYPE_IGW,
                  k_mplsoudp_encap, FALSE, &seed1);

    // trigger
    BATCH_START();
    TEP_MANY_CREATE(&seed);
    BATCH_COMMIT();

    TEP_MANY_READ(&seed);

    // delete and then update the TEPs in same batch
    BATCH_START();
    TEP_MANY_DELETE(&seed);
    // update to different tunnel type, nat
    TEP_MANY_UPDATE(&seed1);
    BATCH_COMMIT_FAILURE(sdk::SDK_RET_ENTRY_NOT_FOUND);
    BATCH_ABORT();
    // TODO: Above batch commit crashes

    TEP_MANY_READ(&seed);

    // cleanup
    BATCH_START();
    TEP_MANY_DELETE(&seed);
    BATCH_COMMIT();

    TEP_MANY_READ_FAIL(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Update and read non existing TEPs
/// [ Update SetMax ] - Read
TEST_F(tep_test, tep_workflow_neg_6) {
    pds_batch_params_t batch_params = {0};
    tep_stepper_seed_t seed = {};

    TEP_SEED_INIT(k_max_tep, "150.150.1.1", PDS_TEP_TYPE_WORKLOAD,
                  k_mplsoudp_encap, k_nat, &seed);

    // trigger
    BATCH_START();
    TEP_MANY_UPDATE(&seed);
    BATCH_COMMIT_FAILURE(sdk::SDK_RET_ENTRY_NOT_FOUND);
    BATCH_ABORT();

    // trigger
    TEP_MANY_READ_FAIL(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create and then update max+1 TEPs in different batch
/// [ Create SetMax ] - Read - [ Update SetMax + 1 ] - Read
TEST_F(tep_test, tep_workflow_neg_7) {
    pds_batch_params_t batch_params = {0};
    tep_stepper_seed_t seed = {}, seed1 = {};

    TEP_SEED_INIT(k_max_tep, "50.50.1.1", PDS_TEP_TYPE_WORKLOAD,
                  k_mplsoudp_encap, k_nat, &seed);
    // seed1 = seed + different tunnel type, nat
    TEP_SEED_INIT(k_max_tep + 1, "50.50.1.1", PDS_TEP_TYPE_IGW,
                  k_mplsoudp_encap, FALSE, &seed1);

    // trigger
    BATCH_START();
    TEP_MANY_CREATE(&seed);
    BATCH_COMMIT();

    TEP_MANY_READ(&seed);

    BATCH_START();
    // update max+1 TEPs with different tunnel type, nat
    TEP_MANY_UPDATE(&seed1);
    BATCH_COMMIT_FAILURE(sdk::SDK_RET_ENTRY_NOT_FOUND);
    BATCH_ABORT();

    TEP_MANY_READ(&seed);

    // cleanup
    BATCH_START();
    TEP_MANY_DELETE(&seed);
    BATCH_COMMIT();

    TEP_MANY_READ_FAIL(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Update existing and non-existing TEPs in same batch
/// [ Create Set1 ] - Read - [ Update Set1 - Update Set2 ] - Read
TEST_F(tep_test, tep_workflow_neg_8) {
    pds_batch_params_t batch_params = {0};
    tep_stepper_seed_t seedA = {}, seedA1 = {}, seedB = {};

    TEP_SEED_INIT(10, "10.10.1.1", PDS_TEP_TYPE_IGW,
                  k_mplsoudp_encap, FALSE, &seedA);
    TEP_SEED_INIT(10, "10.10.1.1", PDS_TEP_TYPE_WORKLOAD,
                  k_mplsoudp_encap, k_nat, &seedA1);
    TEP_SEED_INIT(20, "20.20.1.1", PDS_TEP_TYPE_WORKLOAD,
                  k_mplsoudp_encap, k_nat, &seedB);

    // trigger
    BATCH_START();
    TEP_MANY_CREATE(&seedA);
    BATCH_COMMIT();

    TEP_MANY_READ(&seedA);

    BATCH_START();
    // update set1 to different tunnel type and nat
    TEP_MANY_UPDATE(&seedA1);
    // update set2 which does NOT exist
    TEP_MANY_UPDATE(&seedB);
    BATCH_COMMIT_FAILURE(sdk::SDK_RET_ENTRY_NOT_FOUND);
    BATCH_ABORT();

    // verify that set1 update is rolled back
    TEP_MANY_READ(&seedA);
    // verify that set2 stays non-existent
    TEP_MANY_READ_FAIL(&seedB, sdk::SDK_RET_ENTRY_NOT_FOUND);

    // cleanup
    BATCH_START();
    TEP_MANY_DELETE(&seedA);
    BATCH_COMMIT();

    TEP_MANY_READ_FAIL(&seedA, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Delete existing and Update non-existing TEPs in same batch
/// [ Create Set1 ] - Read - [ Delete Set1 - Update Set2 ] - Read
TEST_F(tep_test, tep_workflow_neg_9) {
    pds_batch_params_t batch_params = {0};
    tep_stepper_seed_t seedA = {}, seedB = {};

    TEP_SEED_INIT(10, "10.10.1.1", PDS_TEP_TYPE_IGW,
                  k_mplsoudp_encap, FALSE, &seedA);
    TEP_SEED_INIT(20, "20.20.1.1", PDS_TEP_TYPE_WORKLOAD,
                  k_mplsoudp_encap, k_nat, &seedB);

    // trigger
    BATCH_START();
    TEP_MANY_CREATE(&seedA);
    BATCH_COMMIT();

    TEP_MANY_READ(&seedA);

    BATCH_START();
    TEP_MANY_DELETE(&seedA);
    // update set2 which does NOT exist
    TEP_MANY_UPDATE(&seedB);
    BATCH_COMMIT_FAILURE(sdk::SDK_RET_ENTRY_NOT_FOUND);
    BATCH_ABORT();

    // verify that set1 deletion is rolled back
    TEP_MANY_READ(&seedA);
    // verify that set2 stays non-existent
    TEP_MANY_READ_FAIL(&seedB, sdk::SDK_RET_ENTRY_NOT_FOUND);

    // cleanup
    BATCH_START();
    TEP_MANY_DELETE(&seedA);
    BATCH_COMMIT();

    TEP_MANY_READ_FAIL(&seedA, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// @}

}    // namespace api_test

static inline void
tep_test_usage_print (char **argv)
{
    cout << "Usage : " << argv[0] << " -c <hal.json>" << endl;
    return;
}

static inline sdk_ret_t
tep_test_options_validate (void)
{
    if (!api_test::g_cfg_file) {
        cerr << "HAL config file is not specified" << endl;
        return sdk::SDK_RET_ERR;
    }
    return sdk::SDK_RET_OK;
}

static void
tep_test_options_parse (int argc, char **argv)
{
    int oc = -1;
    struct option longopts[] = {{"config", required_argument, NULL, 'c'},
                                {"help", no_argument, NULL, 'h'},
                                {0, 0, 0, 0}};

    while ((oc = getopt_long(argc, argv, ":hc:", longopts, NULL)) != -1) {
        switch (oc) {
        case 'c':
            api_test::g_cfg_file = optarg;
            break;
        default:    // ignore all other options
            break;
        }
    }
    return;
}

//----------------------------------------------------------------------------
//// Entry point
////----------------------------------------------------------------------------
int
main (int argc, char **argv)
{
    tep_test_options_parse(argc, argv);
    if (tep_test_options_validate() != sdk::SDK_RET_OK) {
        tep_test_usage_print(argv);
        exit(1);
    }

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
