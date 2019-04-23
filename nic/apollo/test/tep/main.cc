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
};

//----------------------------------------------------------------------------
// TEP test cases implementation
//----------------------------------------------------------------------------

/// \defgroup TEP
/// @{

/// \brief Create and delete maximum TEPs in the same batch
/// The operation should be de-duped by framework and is a NO-OP
/// from hardware perspective
/// [ Create SetMax, Delete SetMax ] - Read
TEST_F(tep_test, tep_workflow1) {
    pds_batch_params_t batch_params = {0};
    std::string tep_first_ip_str = "50.50.1.1";
    pds_encap_t encap = {PDS_ENCAP_TYPE_MPLSoUDP, 0};

    // trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_create(k_max_tep, tep_first_ip_str,
        PDS_TEP_TYPE_WORKLOAD, encap) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_delete(k_max_tep, tep_first_ip_str,
        PDS_TEP_TYPE_WORKLOAD, encap) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(tep_util::many_read(
        k_max_tep, tep_first_ip_str, PDS_TEP_TYPE_WORKLOAD, encap,
        sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

/// \brief Create, delete and create max TEPs in the same batch
/// Create and delete should be de-deduped by framework and subsequent
/// create should result in successful creation
/// [ Create SetMax - Delete SetMax - Create SetMax ] - Read
TEST_F(tep_test, tep_workflow2) {
    pds_batch_params_t batch_params = {0};
    std::string tep_first_ip_str = "50.50.1.1";
    pds_encap_t encap = {PDS_ENCAP_TYPE_MPLSoUDP, 0};

    // trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_create(k_max_tep, tep_first_ip_str,
        PDS_TEP_TYPE_WORKLOAD, encap) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_delete(k_max_tep, tep_first_ip_str,
        PDS_TEP_TYPE_WORKLOAD, encap) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_create(k_max_tep, tep_first_ip_str,
        PDS_TEP_TYPE_WORKLOAD, encap) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(tep_util::many_read(
        k_max_tep, tep_first_ip_str, PDS_TEP_TYPE_WORKLOAD, encap,
        sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    // cleanup
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_delete(k_max_tep, tep_first_ip_str,
        PDS_TEP_TYPE_WORKLOAD, encap) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create, delete some and create another set of TEPs in the same batch
/// [ Create Set1, Set2 - Delete Set1 - Create Set3 ] - Read
TEST_F(tep_test, tep_workflow3) {
    pds_batch_params_t batch_params = {0};
    std::string set1_first_ip_str = "10.10.1.1";
    std::string set2_first_ip_str = "20.20.1.1";
    std::string set3_first_ip_str = "30.30.1.1";
    pds_encap_t encap = {PDS_ENCAP_TYPE_MPLSoUDP, 0};

    // trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_create(10, set1_first_ip_str,
        PDS_TEP_TYPE_WORKLOAD, encap) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_create(20, set2_first_ip_str,
        PDS_TEP_TYPE_WORKLOAD, encap) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_delete(10, set1_first_ip_str,
        PDS_TEP_TYPE_WORKLOAD, encap) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_create(30, set3_first_ip_str,
        PDS_TEP_TYPE_WORKLOAD, encap) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(tep_util::many_read(
        10, set1_first_ip_str, PDS_TEP_TYPE_WORKLOAD, encap,
        sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_read(
        20, set2_first_ip_str, PDS_TEP_TYPE_WORKLOAD, encap,
        sdk::SDK_RET_OK) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_read(
        30, set3_first_ip_str, PDS_TEP_TYPE_WORKLOAD, encap,
        sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    // cleanup
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_delete(20, set2_first_ip_str,
        PDS_TEP_TYPE_WORKLOAD, encap) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_delete(30, set3_first_ip_str,
        PDS_TEP_TYPE_WORKLOAD, encap) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(tep_util::many_read(
        20, set2_first_ip_str, PDS_TEP_TYPE_WORKLOAD, encap,
        sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_read(
        30, set3_first_ip_str, PDS_TEP_TYPE_WORKLOAD, encap,
        sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

/// \brief Create and delete max TEPs in different batches
/// [ Create SetMax ] - Read - [ Delete SetMax ] - Read
TEST_F(tep_test, tep_workflow4) {
    pds_batch_params_t batch_params = {0};
    std::string tep_first_ip_str = "50.50.1.1";
    pds_encap_t encap = {PDS_ENCAP_TYPE_MPLSoUDP, 0};

    // trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_create(k_max_tep, tep_first_ip_str,
        PDS_TEP_TYPE_WORKLOAD, encap) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(tep_util::many_read(
        k_max_tep, tep_first_ip_str, PDS_TEP_TYPE_WORKLOAD, encap,
        sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_delete(k_max_tep, tep_first_ip_str,
        PDS_TEP_TYPE_WORKLOAD, encap) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(tep_util::many_read(
        k_max_tep, tep_first_ip_str, PDS_TEP_TYPE_WORKLOAD, encap,
        sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

/// \brief Create and delete mix and match of TEPs in different batches
/// [ Create Set1, Set2 ] - Read - [Delete Set1 - Create Set3 ] - Read
TEST_F(tep_test, tep_workflow5) {
    pds_batch_params_t batch_params = {0};
    std::string set1_first_ip_str = "10.10.1.1";
    std::string set2_first_ip_str = "20.20.1.1";
    std::string set3_first_ip_str = "30.30.1.1";
    pds_encap_t encap = {PDS_ENCAP_TYPE_MPLSoUDP, 0};

    // trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_create(10, set1_first_ip_str,
        PDS_TEP_TYPE_WORKLOAD, encap) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_create(20, set2_first_ip_str,
        PDS_TEP_TYPE_WORKLOAD, encap) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(tep_util::many_read(
        10, set1_first_ip_str, PDS_TEP_TYPE_WORKLOAD, encap,
        sdk::SDK_RET_OK) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_read(
        20, set2_first_ip_str, PDS_TEP_TYPE_WORKLOAD, encap,
        sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_delete(10, set1_first_ip_str,
        PDS_TEP_TYPE_WORKLOAD, encap) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_create(30, set3_first_ip_str,
        PDS_TEP_TYPE_WORKLOAD, encap) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(tep_util::many_read(
        10, set1_first_ip_str, PDS_TEP_TYPE_WORKLOAD, encap,
        sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_read(
        20, set2_first_ip_str, PDS_TEP_TYPE_WORKLOAD, encap,
        sdk::SDK_RET_OK) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_read(
        30, set3_first_ip_str, PDS_TEP_TYPE_WORKLOAD, encap,
        sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    // cleanup
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_delete(20, set2_first_ip_str,
        PDS_TEP_TYPE_WORKLOAD, encap) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_delete(30, set3_first_ip_str,
        PDS_TEP_TYPE_WORKLOAD, encap) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(tep_util::many_read(
        20, set2_first_ip_str, PDS_TEP_TYPE_WORKLOAD, encap,
        sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_read(
        30, set3_first_ip_str, PDS_TEP_TYPE_WORKLOAD, encap,
        sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

/// \brief Create maximum number of VPCs in two batches
/// [ Create SetMax ] - [ Create SetMax ] - Read
TEST_F(tep_test, tep_workflow_neg_1) {
    pds_batch_params_t batch_params = {0};
    std::string tep_first_ip_str = "50.50.1.1";
    pds_encap_t encap = {PDS_ENCAP_TYPE_MPLSoUDP, 0};

    // trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_create(k_max_tep, tep_first_ip_str,
        PDS_TEP_TYPE_WORKLOAD, encap) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(tep_util::many_read(
        k_max_tep, tep_first_ip_str, PDS_TEP_TYPE_WORKLOAD, encap,
        sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_create(k_max_tep, tep_first_ip_str,
        PDS_TEP_TYPE_WORKLOAD, encap) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_INVALID_OP);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    ASSERT_TRUE(tep_util::many_read(
        k_max_tep, tep_first_ip_str, PDS_TEP_TYPE_WORKLOAD, encap,
        sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    // cleanup
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_delete(k_max_tep, tep_first_ip_str,
        PDS_TEP_TYPE_WORKLOAD, encap) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(tep_util::many_read(
        k_max_tep, tep_first_ip_str, PDS_TEP_TYPE_WORKLOAD, encap,
        sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

/// \brief Create more than maximum number of TEPs supported.
/// [ Create SetMax+1 ] - Read
TEST_F(tep_test, tep_workflow_neg_2) {
    pds_batch_params_t batch_params = {0};
    std::string tep_first_ip_str = "50.50.1.1";
    pds_encap_t encap = {PDS_ENCAP_TYPE_MPLSoUDP, 0};

    // trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_create(k_max_tep + 1, tep_first_ip_str,
        PDS_TEP_TYPE_WORKLOAD, encap) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_NO_RESOURCE);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    ASSERT_TRUE(tep_util::many_read(
        k_max_tep + 1, tep_first_ip_str, PDS_TEP_TYPE_WORKLOAD, encap,
        sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

/// \brief Read and delete non existing TEPs
/// Read NonEx, [ Delete NonExMax ]
TEST_F(tep_test, tep_workflow_neg_3) {
    pds_batch_params_t batch_params = {0};
    std::string tep_invalid_ip_str = "150.150.1.1";
    pds_encap_t encap = {PDS_ENCAP_TYPE_MPLSoUDP, 0};

    // trigger
    ASSERT_TRUE(tep_util::many_read(
        k_max_tep, tep_invalid_ip_str, PDS_TEP_TYPE_WORKLOAD, encap,
        sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);

    // trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_delete(k_max_tep, tep_invalid_ip_str,
        PDS_TEP_TYPE_WORKLOAD, encap) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_ENTRY_NOT_FOUND);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);
}

/// \brief Invalid batch shouldn't affect entries of previous batch
/// [ Create Set1 ] - [Delete Set1, Set2 ] - Read
TEST_F(tep_test, tep_workflow_neg_4) {
    pds_batch_params_t batch_params = {0};
    uint32_t num_teps = 10;
    pds_encap_t encap = {PDS_ENCAP_TYPE_MPLSoUDP, 0};
    std::string set1_first_ip_str = "50.50.1.1";
    std::string set2_first_ip_str = "60.60.1.1";

    // trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_create(num_teps, set1_first_ip_str,
        PDS_TEP_TYPE_WORKLOAD, encap) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(tep_util::many_read(
        num_teps, set1_first_ip_str, PDS_TEP_TYPE_WORKLOAD, encap,
        sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_delete(num_teps, set1_first_ip_str,
        PDS_TEP_TYPE_WORKLOAD, encap) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_delete(num_teps, set2_first_ip_str,
        PDS_TEP_TYPE_WORKLOAD, encap) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_ENTRY_NOT_FOUND);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    ASSERT_TRUE(tep_util::many_read(
        num_teps, set1_first_ip_str, PDS_TEP_TYPE_WORKLOAD, encap,
        sdk::SDK_RET_OK) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_read(
        num_teps, set2_first_ip_str, PDS_TEP_TYPE_WORKLOAD, encap,
        sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);

    // cleanup
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_delete(num_teps, set1_first_ip_str,
        PDS_TEP_TYPE_WORKLOAD, encap) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(tep_util::many_read(
        num_teps, set1_first_ip_str, PDS_TEP_TYPE_WORKLOAD, encap,
        sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

/// \brief TEP workflow corner case 4
/// [ Create SetCorner ] - Read
TEST_F(tep_test, tep_workflowcorner_case_4) {}

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
