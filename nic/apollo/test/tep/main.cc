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
#include "nic/apollo/include/api/oci_batch.hpp"
#include "nic/apollo/test/utils/base.hpp"
#include "nic/apollo/test/utils/tep.hpp"

using std::cerr;
using std::cout;
using std::endl;

namespace api_test {

// Globals
char *g_cfg_file = NULL;
static oci_epoch_t g_batch_epoch = OCI_EPOCH_INVALID;

//----------------------------------------------------------------------------
// TEP test class
//----------------------------------------------------------------------------

class tep_test : public oci_test_base {
protected:
    tep_test() {}
    virtual ~tep_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        oci_test_base::SetUpTestCase(api_test::g_cfg_file, false);
    }
};

//----------------------------------------------------------------------------
// TEP test cases utility routines
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// TEP test cases implementation
//----------------------------------------------------------------------------

/// \defgroup TEP
/// @{

/// \brief Read a non-existing TEP
///
/// Read a non-existing TEP. First read after table init
TEST_F(tep_test, tep_invalid_read) {
    oci_tep_info_t info;
    tep_util tunnel_obj("10.1.2.3/8");
    ASSERT_TRUE(tunnel_obj.read(&info) == sdk::SDK_RET_ENTRY_NOT_FOUND);
    // ASSERT_TRUE(tunnel_obj.validate(&info) == sdk::SDK_RET_INVALID_ARG);
}

/// \brief Delete a non-existing TEP
///
/// Delete a TEP before first ever create
TEST_F(tep_test, tep_invalid_delete) {
    oci_batch_params_t batch_params = {0};
    tep_util tep_obj("10.1.1.1/8", OCI_ENCAP_TYPE_VNIC);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(oci_batch_start(&batch_params) == sdk::SDK_RET_OK);
    // TODO implement tep_impl::cleanup_hw() for del to work
    // ASSERT_TRUE(tep_obj.del() == sdk::SDK_RET_ENTRY_NOT_FOUND);
    ASSERT_TRUE(oci_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Delete a non-existing TEP multiple times
///
/// Delete same non-existing TEP multiple times in separate batches
TEST_F(tep_test, tep_invalid_delete_1) {}

/// \brief Delete a non-existing TEP multiple times
///
/// Delete same non-existing TEP multiple times in a single batch
TEST_F(tep_test, tep_invalid_delete_2) {}

/// \brief Delete different non-existing TEPs
///
/// Delete non-existing but unique TEPs in separate batches
TEST_F(tep_test, tep_invalid_delete_3) {}

/// \brief Delete different non-existing TEPs
///
/// Delete non-existing but unique TEPs in a single batch
TEST_F(tep_test, tep_invalid_delete_4) {}

/// \brief Create a new TEP
///
/// Create a new TEP and delete it
TEST_F(tep_test, tep_create) {
    oci_batch_params_t batch_params = {0};
    oci_tep_info_t info;
    tep_util tep_obj("10.1.1.1/8", OCI_ENCAP_TYPE_VNIC);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(oci_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj.create() == sdk::SDK_RET_OK);
    ASSERT_TRUE(oci_batch_commit() == sdk::SDK_RET_OK);

    // Teardown
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(oci_batch_start(&batch_params) == sdk::SDK_RET_OK);
    // TODO implement tep_impl::cleanup_hw() for del to work
    // ASSERT_TRUE(tep_obj.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(oci_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Read an existing TEP
///
/// Create a TEP and then read & validate
TEST_F(tep_test, tep_read) {
    oci_batch_params_t batch_params = {0};
    oci_tep_info_t info;
    tep_util tep_obj("10.1.1.2/8", OCI_ENCAP_TYPE_VNIC);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(oci_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj.create() == sdk::SDK_RET_OK);
    ASSERT_TRUE(oci_batch_commit() == sdk::SDK_RET_OK);

    tep_util tunnel_obj("10.1.1.2/8");
    ASSERT_TRUE(tunnel_obj.read(&info) == sdk::SDK_RET_OK);
    // Validate against original object
    ASSERT_TRUE(tep_obj.validate(&info) == sdk::SDK_RET_OK);

    // Teardown
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(oci_batch_start(&batch_params) == sdk::SDK_RET_OK);
    // TODO implement tep_impl::cleanup_hw() for del to work
    // ASSERT_TRUE(tep_obj.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(oci_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Delete an existing TEP
///
/// Create a TEP, delete it and then read & validate
TEST_F(tep_test, tep_delete) {
    oci_batch_params_t batch_params = {0};
    oci_tep_info_t info;
    tep_util tep_obj("10.1.1.3/8", OCI_ENCAP_TYPE_VNIC);

    // Setup
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(oci_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj.create() == sdk::SDK_RET_OK);
    ASSERT_TRUE(oci_batch_commit() == sdk::SDK_RET_OK);

    // Trigger (Delete)
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(oci_batch_start(&batch_params) == sdk::SDK_RET_OK);
    // TODO implement tep_impl::cleanup_hw() for del to work
    // ASSERT_TRUE(tep_obj.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(oci_batch_commit() == sdk::SDK_RET_OK);

    // Verify
    tep_util tunnel_obj("10.1.1.3/8");
    // TODO uncomment below when deletion works
    // ASSERT_TRUE(tunnel_obj.read(&info) == sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Double create in same batch
///
/// Create same TEP twice in single batch
TEST_F(tep_test, tep_double_create) {}

/// \brief Double create in separate batch
///
/// Create same TEP twice in separate batch
TEST_F(tep_test, tep_double_create_1) {}

/// \brief Double delete in same batch
///
/// Delete same TEP twice in single batch
TEST_F(tep_test, tep_double_delete) {}

/// \brief Double delete in separate batch
///
/// Delete same TEP twice in separate batch
TEST_F(tep_test, tep_double_delete_2) {}

/// \brief create delete in same batch
///
/// Create & Delete same TEP in single batch
TEST_F(tep_test, tep_create_delete) {}

/// \brief delete create in same batch
///
/// Delete & Create same TEP in single batch
TEST_F(tep_test, tep_delete_create) {}

/// \brief delete create delete in same batch
///
/// Delete, Create & Delete same TEP in single batch
TEST_F(tep_test, tep_delete_create_delete) {}

/// \brief delete create delete in same batch
///
/// Delete, Create & Delete unique valid TEPs in single batch
TEST_F(tep_test, tep_delete_create_delete_1) {}

/// \brief Create only 1 TEP in a single batch
///
/// Create 1 TEP when table has OCI_MAX_TEP-1 entries
TEST_F(tep_test, tep_max_create) {}

/// \brief Create 1 TEP after table full
///
/// Attempt to create only one TEP when table is already full
TEST_F(tep_test, tep_beyondmax_create) {}

/// \brief Create multiple TEPs in a single batch
///
/// Create multiple unique TEPs in a single batch
TEST_F(tep_test, tep_multi_create) {
    oci_batch_params_t batch_params = {0};
    oci_tep_info_t info;
    uint32_t num_teps = 5;

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(oci_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_create(num_teps, "10.1.2.1/8",
                                      OCI_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(oci_batch_commit() == sdk::SDK_RET_OK);

    // TODO Verify with read
    tep_util tunnel_obj("10.1.2.3/8");
    ASSERT_TRUE(tunnel_obj.read(&info) == sdk::SDK_RET_OK);

    // TODO Teardown
}

/// \brief Delete multiple TEPs in a single batch
///
/// Delete multiple unique TEPs in a single batch
TEST_F(tep_test, tep_multi_delete) {}

/// \brief Create max no of TEPs in a single batch
///
/// Create max supported no of unique TEPs in a single batch
TEST_F(tep_test, tep_multi_max_create) {}

/// \brief Create (OCI_MAX_TEP/2) no of TEPs in a single batch
///
/// Attempt to create (OCI_MAX_TEP/2) no of TEPs in a single batch
//  when the table is already half full
TEST_F(tep_test, tep_multi_max_create_1) {}

/// \brief Delete max no of TEPs in a single batch
///
/// Delete max supported no of TEPs in a single batch
TEST_F(tep_test, tep_multi_max_delete) {}

/// \brief Create 1 + OCI_MAX_TEP no of TEPs in a single batch
///
/// Attempt to create more than supported no of TEPs in a single batch
TEST_F(tep_test, tep_multi_beyondmax_create) {}

/// \brief Create 1 + (OCI_MAX_TEP/2) no of TEPs in a single batch
///
/// Attempt to create 1+(OCI_MAX_TEP/2) no of TEPs in a single batch
//  when the table is already half full
TEST_F(tep_test, tep_multi_beyondmax_create_1) {}

/// \brief Read a non existing TEP
///
/// Read a non existing TEP with ONLY one entry in HW
TEST_F(tep_test, tep_read_nonexisting_when_single) {}

/// \brief Read a non existing TEP
///
/// Read a non existing TEP with multiple entries in HW
TEST_F(tep_test, tep_read_nonexisting_when_multiple) {}

/// \brief Read an existing TEP
///
/// Read an existing TEP with ONLY that entry in HW
TEST_F(tep_test, tep_read_existing_when_single) {}

/// \brief Read an existing TEP
///
/// Read an existing TEP with multiple entries in HW
TEST_F(tep_test, tep_read_existing_when_multiple) {}

/// \brief Read last deleted TEP when table is empty
///
/// Read the last deleted TEP when nothing is programmed
TEST_F(tep_test, tep_read_last_deleted_when_empty) {}

/// \brief Read last created TEP when table is empty
///
/// Read the last created TEP when nothing is programmed
TEST_F(tep_test, tep_read_last_created_when_empty) {}

/// \brief Read last updated TEP when table is empty
///
/// Read the last updated TEP when nothing is programmed
TEST_F(tep_test, tep_read_last_updated_when_empty) {}

/// \brief Read never programmed TEP when table is empty
///
/// Read a never ever programmed TEP when nothing is programmed
TEST_F(tep_test, tep_read_never_programmed_when_empty) {}

/// \brief Read an existing TEP when table is full
///
/// Read an existing TEP (which is first HW entry) when table is full
TEST_F(tep_test, tep_read_existing_when_full) {}

/// \brief Read an existing TEP when table is full
///
/// Read an existing TEP (which is last HW entry) when table is full
TEST_F(tep_test, tep_read_existing_when_full_1) {}

/// \brief Read a non-existing TEP when table is full
///
/// Read a non existing TEP (never programmed anytime) when table is full
TEST_F(tep_test, tep_read_nonexisting_when_full) {}

/// \brief Read all TEPs
///
/// Read all currently programmed TEPs with single entry in HW
TEST_F(tep_test, tep_read_all_when_single) {}

/// \brief Read all TEPs
///
/// Read all currently programmed TEPs with multiple entries in HW
TEST_F(tep_test, tep_read_all_when_multiple) {}

/// \brief Read all TEPs on empty table
///
/// Read all currently programmed TEPs when nothing is programmed
TEST_F(tep_test, tep_read_all_when_empty) {}

/// \brief Read all TEPs after table full
///
/// Read all currently programmed TEPs when table is full
TEST_F(tep_test, tep_read_all_when_full) {}

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
