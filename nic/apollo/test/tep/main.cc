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
        pds_test_base::SetUpTestCase(api_test::g_cfg_file, false);
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
/// Read a non-existing TEP. First read after table init
TEST_F(tep_test, tep_invalid_read) {
    pds_tep_info_t info;
    tep_util tep_obj("10.1.2.3/8");

    ASSERT_TRUE(tep_obj.read(&info) == sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Delete a non-existing TEP
/// Delete a TEP before first ever create
TEST_F(tep_test, tep_invalid_delete) {
    pds_batch_params_t batch_params = {0};
    tep_util tep_obj("10.1.1.1/8", PDS_TEP_ENCAP_TYPE_VNIC);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_ENTRY_NOT_FOUND);
    // abort the batch as it failed as expected
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);
}

/// \brief Delete a non-existing TEP multiple times
/// Delete same non-existing TEP multiple times in separate batches
TEST_F(tep_test, tep_invalid_delete_1) {
    pds_batch_params_t batch_params = {0};
    tep_util tep_obj("10.1.1.1/8", PDS_TEP_ENCAP_TYPE_VNIC);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_ENTRY_NOT_FOUND);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_ENTRY_NOT_FOUND);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_ENTRY_NOT_FOUND);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);
}

/// \brief Delete a non-existing TEP multiple times
/// Delete same non-existing TEP multiple times in a single batch
TEST_F(tep_test, tep_invalid_delete_2) {
    pds_batch_params_t batch_params = {0};
    tep_util tep_obj("10.1.1.1/8", PDS_TEP_ENCAP_TYPE_VNIC);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_ENTRY_NOT_FOUND);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);
}

/// \brief Delete different non-existing TEPs
/// Delete non-existing but unique TEPs in separate batches
TEST_F(tep_test, tep_invalid_delete_3) {
    pds_batch_params_t batch_params = {0};
    tep_util tep_obj1("10.1.1.1/8", PDS_TEP_ENCAP_TYPE_VNIC);
    tep_util tep_obj2("10.1.1.2/8", PDS_TEP_ENCAP_TYPE_VNIC);
    tep_util tep_obj3("10.1.1.3/8", PDS_TEP_ENCAP_TYPE_VNIC);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj1.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_ENTRY_NOT_FOUND);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj2.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_ENTRY_NOT_FOUND);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj3.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_ENTRY_NOT_FOUND);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);
}

/// \brief Delete different non-existing TEPs
/// Delete non-existing but unique TEPs in a single batch
TEST_F(tep_test, tep_invalid_delete_4) {
    pds_batch_params_t batch_params = {0};
    tep_util tep_obj1("10.1.1.1/8", PDS_TEP_ENCAP_TYPE_VNIC);
    tep_util tep_obj2("10.1.1.2/8", PDS_TEP_ENCAP_TYPE_VNIC);
    tep_util tep_obj3("10.1.1.3/8", PDS_TEP_ENCAP_TYPE_VNIC);
    tep_util tep_obj4("10.1.1.4/8", PDS_TEP_ENCAP_TYPE_VNIC);
    tep_util tep_obj5("10.1.1.5/8", PDS_TEP_ENCAP_TYPE_VNIC);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj1.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj2.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj3.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj4.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj5.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_ENTRY_NOT_FOUND);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);
}

/// \brief Create a new TEP
/// Create a new TEP and delete it
TEST_F(tep_test, tep_create) {
    pds_batch_params_t batch_params = {0};
    pds_tep_info_t info;
    tep_util tep_obj("10.1.1.1/8", PDS_TEP_ENCAP_TYPE_VNIC);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj.create() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // Teardown
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Read an existing TEP
/// Create a TEP and then read & validate
TEST_F(tep_test, tep_read) {
    pds_batch_params_t batch_params = {0};
    pds_tep_info_t info;
    tep_util tep_obj("10.1.1.2/8", PDS_TEP_ENCAP_TYPE_VNIC);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj.create() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // turning off validation (sending 'FALSE') until then.
    ASSERT_TRUE(tep_obj.read(&info, FALSE) == sdk::SDK_RET_OK);

    // Teardown
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Delete an existing TEP
/// Create a TEP, delete it and then read & validate
TEST_F(tep_test, tep_delete) {
    pds_batch_params_t batch_params = {0};
    pds_tep_info_t info;
    tep_util tep_obj("10.1.1.3/8", PDS_TEP_ENCAP_TYPE_VNIC);

    // Setup
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj.create() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // Trigger (Delete)
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // Verify
    ASSERT_TRUE(tep_obj.read(&info) == sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Double create in same batch
/// Create same TEP twice in single batch
TEST_F(tep_test, tep_double_create) {
    pds_batch_params_t batch_params = {0};
    pds_tep_info_t info;
    tep_util tep_obj("10.1.1.4/8", PDS_TEP_ENCAP_TYPE_VNIC);

    // Trigger
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj.create() == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj.create() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_INVALID_OP);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);
}

/// \brief Double create in separate batch
/// Create same TEP twice in separate batch
TEST_F(tep_test, tep_double_create_1) {
    pds_batch_params_t batch_params = {0};
    pds_tep_info_t info;
    tep_util tep_obj("10.1.1.5/8", PDS_TEP_ENCAP_TYPE_VNIC);

    // Trigger
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj.create() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj.create() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_INVALID_OP);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    // Teardown
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Double delete in same batch
/// Delete same TEP twice in single batch
TEST_F(tep_test, tep_double_delete) {
    pds_batch_params_t batch_params = {0};
    pds_tep_info_t info;
    tep_util tep_obj("10.1.1.6/8", PDS_TEP_ENCAP_TYPE_VNIC);

    // Setup
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj.create() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // Trigger
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Double delete in separate batch
/// Delete same TEP twice in separate batch
TEST_F(tep_test, tep_double_delete_2) {
    pds_batch_params_t batch_params = {0};
    pds_tep_info_t info;
    tep_util tep_obj("10.1.1.7/8", PDS_TEP_ENCAP_TYPE_VNIC);

    // Setup
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj.create() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // Trigger 1
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // Trigger 2
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_ENTRY_NOT_FOUND);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);
}

/// \brief create delete in same batch
/// Create & Delete same TEP in single batch
TEST_F(tep_test, tep_create_delete) {
    pds_batch_params_t batch_params = {0};
    pds_tep_info_t info;
    tep_util tep_obj("10.1.1.8/8", PDS_TEP_ENCAP_TYPE_VNIC);

    // Trigger
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj.create() == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // Validate
    ASSERT_TRUE(tep_obj.read(&info, FALSE) == sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief delete create in same batch
/// Delete & Create same TEP in single batch
TEST_F(tep_test, tep_delete_create) {
    pds_batch_params_t batch_params = {0};
    pds_tep_info_t info;
    tep_util tep_obj("10.1.1.9/8", PDS_TEP_ENCAP_TYPE_VNIC);

    // Trigger
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj.create() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_ENTRY_NOT_FOUND);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    // Validate
    ASSERT_TRUE(tep_obj.read(&info, FALSE) == sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief delete create delete in same batch
/// Delete, Create & Delete same TEP in single batch
TEST_F(tep_test, tep_delete_create_delete) {
    pds_batch_params_t batch_params = {0};
    pds_tep_info_t info;
    tep_util tep_obj("10.1.1.10/8", PDS_TEP_ENCAP_TYPE_VNIC);

    // Trigger
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj.create() == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_ENTRY_NOT_FOUND);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    // Validate
    ASSERT_TRUE(tep_obj.read(&info, FALSE) == sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief delete create delete in same batch
/// Delete, Create & Delete multiple unique valid TEPs in single batch
TEST_F(tep_test, tep_delete_create_delete_1) {
    pds_batch_params_t batch_params = {0};
    pds_tep_info_t info;
    tep_util tep_obj1("10.1.1.11/8", PDS_TEP_ENCAP_TYPE_VNIC);
    tep_util tep_obj2("10.1.1.12/8", PDS_TEP_ENCAP_TYPE_VNIC);
    tep_util tep_obj3("10.1.1.13/8", PDS_TEP_ENCAP_TYPE_VNIC);

    // Trigger
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj1.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj1.create() == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj1.del() == sdk::SDK_RET_OK);

    ASSERT_TRUE(tep_obj2.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj2.create() == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj2.del() == sdk::SDK_RET_OK);

    ASSERT_TRUE(tep_obj3.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj3.create() == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj3.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_ENTRY_NOT_FOUND);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    // Validate
    ASSERT_TRUE(tep_obj1.read(&info, FALSE) == sdk::SDK_RET_ENTRY_NOT_FOUND);
    ASSERT_TRUE(tep_obj2.read(&info, FALSE) == sdk::SDK_RET_ENTRY_NOT_FOUND);
    ASSERT_TRUE(tep_obj3.read(&info, FALSE) == sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create only 1 TEP in a single batch
/// Create 1 TEP when table has PDS_MAX_TEP-1 entries
TEST_F(tep_test, DISABLED_tep_max_create) {
    pds_batch_params_t batch_params = {0};
    uint32_t num_teps = PDS_MAX_TEP - 1;
    std::string tep_first_ip_str = "10.10.1.2/8";
    tep_util tep_obj("10.10.5.1", PDS_TEP_ENCAP_TYPE_VNIC);

    // Setup
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_create(num_teps, tep_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // Trigger
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj.create() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // verify all PDS_MAX_TEP entries are programmed
    ASSERT_TRUE(tep_util::many_read(sdk::SDK_RET_OK, PDS_MAX_TEP,
        tep_first_ip_str, PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);

    // Teardown
    num_teps = PDS_MAX_TEP;
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_delete(
        num_teps, tep_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // verify all PDS_MAX_TEP entries are NOT programmed
    ASSERT_TRUE(tep_util::many_read(sdk::SDK_RET_ENTRY_NOT_FOUND, PDS_MAX_TEP,
        tep_first_ip_str, PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
}

/// \brief Create 1 TEP after table full
/// Attempt to create only one TEP when table is already full
TEST_F(tep_test, tep_beyondmax_create) {
    pds_batch_params_t batch_params = {0};
    pds_tep_info_t info;
    uint32_t num_teps = PDS_MAX_TEP;
    std::string tep_first_ip_str = "10.20.1.1/8";
    tep_util tep_obj("10.20.5.1", PDS_TEP_ENCAP_TYPE_VNIC);

    // Setup
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_create(num_teps, tep_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // Trigger
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj.create() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_NO_RESOURCE);
    // abort the batch as it failed as expected
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    ASSERT_TRUE(tep_obj.read(&info) == sdk::SDK_RET_ENTRY_NOT_FOUND);

    // Teardown
    num_teps = PDS_MAX_TEP;
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_delete(
        num_teps, tep_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(tep_obj.read(&info, FALSE) == sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create multiple TEPs in a single batch
/// Create multiple unique TEPs in a single batch
TEST_F(tep_test, DISABLED_tep_multi_create) {
    pds_batch_params_t batch_params = {0};
    uint32_t num_teps = 10;
    std::string tep_first_ip_str = "10.11.1.1/8";

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_create(num_teps, tep_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // verify all num_teps entries are programmed
    ASSERT_TRUE(tep_util::many_read(sdk::SDK_RET_OK, num_teps,
        tep_first_ip_str, PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);

    // Teardown
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_delete(
        num_teps, tep_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Delete multiple TEPs in a single batch
/// Delete multiple unique TEPs in a single batch
TEST_F(tep_test, tep_multi_delete) {
    pds_batch_params_t batch_params = {0};
    uint32_t num_teps = 10;
    std::string tep_first_ip_str = "10.12.1.1/8";

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_create(num_teps, tep_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // Teardown
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_delete(
        num_teps, tep_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // verify all num_teps entries are NOT programmed
    ASSERT_TRUE(tep_util::many_read(sdk::SDK_RET_ENTRY_NOT_FOUND, num_teps,
        tep_first_ip_str, PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
}

/// \brief Create max no of TEPs in a single batch
/// Create max supported no of unique TEPs in a single batch
TEST_F(tep_test, DISABLED_tep_multi_max_create) {
    pds_batch_params_t batch_params = {0};
    uint32_t num_teps = PDS_MAX_TEP;
    std::string tep_first_ip_str = "10.30.1.1/8";

    // Trigger
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_create(num_teps, tep_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // verify all PDS_MAX_TEP entries are programmed
    ASSERT_TRUE(tep_util::many_read(sdk::SDK_RET_OK, PDS_MAX_TEP,
        tep_first_ip_str, PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);

    // Teardown
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_delete(
        num_teps, tep_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create (PDS_MAX_TEP/2) no of TEPs in a single batch
/// Attempt to create (PDS_MAX_TEP/2) no of TEPs in a single batch
//  when the table is already half full
TEST_F(tep_test, DISABLED_tep_multi_max_create_1) {
    pds_batch_params_t batch_params = {0};
    uint32_t num_teps = PDS_MAX_TEP / 2;
    std::string tep_first_ip_str_1 = "10.40.1.1/8";
    std::string tep_first_ip_str_2 = "10.50.1.1/8";

    // Setup
    // make tep table half full
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_create(num_teps, tep_first_ip_str_1,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // Trigger
    // create (PDS_MAX_TEP/2) no of TEPs in a single batch
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_create(num_teps, tep_first_ip_str_2,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // verify all num_teps entries are programmed
    ASSERT_TRUE(tep_util::many_read(sdk::SDK_RET_OK, num_teps,
        tep_first_ip_str_2, PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);

    // Teardown
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_delete(
        num_teps, tep_first_ip_str_1,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_delete(
        num_teps, tep_first_ip_str_2,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Delete max no of TEPs in a single batch
TEST_F(tep_test, tep_multi_max_delete) {
    pds_batch_params_t batch_params = {0};
    uint32_t num_teps = PDS_MAX_TEP;
    std::string tep_first_ip_str = "10.60.1.1/8";

    // Setup
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_create(num_teps, tep_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // Trigger
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_delete(
        num_teps, tep_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // verify all num_teps entries are NOT programmed
    ASSERT_TRUE(tep_util::many_read(sdk::SDK_RET_ENTRY_NOT_FOUND, num_teps,
        tep_first_ip_str, PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
}

/// \brief Delete max no of TEPs - 1 in a single batch
/// Delete max supported no of TEPs but one in a single batch
TEST_F(tep_test, tep_multi_max_delete_1) {
    pds_batch_params_t batch_params = {0};
    pds_tep_info_t info;
    uint32_t num_teps = PDS_MAX_TEP;
    std::string tep_first_ip_str = "10.70.1.1/8";
    std::string tep_last_ip_str = "10.70.4.255";
    tep_util tep_obj(tep_last_ip_str, PDS_TEP_ENCAP_TYPE_VNIC);

    // Setup
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_create(num_teps, tep_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // Trigger
    num_teps = PDS_MAX_TEP - 1;
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_delete(num_teps, tep_first_ip_str,
                                      PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // verify all num_teps entries are NOT programmed
    ASSERT_TRUE(tep_util::many_read(sdk::SDK_RET_ENTRY_NOT_FOUND, num_teps,
        tep_first_ip_str, PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);

    // verify last tep exists
    ASSERT_TRUE(tep_obj.read(&info) == sdk::SDK_RET_OK);
}

/// \brief Create 1 + PDS_MAX_TEP no of TEPs in a single batch
/// Attempt to create more than supported no of TEPs in a single batch
TEST_F(tep_test, tep_multi_beyondmax_create) {
    pds_batch_params_t batch_params = {0};
    uint32_t num_teps = PDS_MAX_TEP + 1;
    std::string tep_first_ip_str = "10.80.1.1/8";

    // TODO: Even though this batch commit failed and got aborted,
    // resources do NOT seem to be released which is causing
    // further create to fail

    // Trigger
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_create(num_teps, tep_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_NO_RESOURCE);

    // abort the batch as it failed as expected
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    // verify all num_teps entries are NOT programmed
    ASSERT_TRUE(tep_util::many_read(sdk::SDK_RET_ENTRY_NOT_FOUND, num_teps,
        tep_first_ip_str, PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
}

/// \brief Create 1 + (PDS_MAX_TEP/2) no of TEPs in a single batch
/// Attempt to create 1+(PDS_MAX_TEP/2) no of TEPs in a single batch
//  when the table is already half full
TEST_F(tep_test, tep_multi_beyondmax_create_1) {
    pds_batch_params_t batch_params = {0};
    uint32_t num_teps = PDS_MAX_TEP / 2;
    std::string tep_first_ip_str_1 = "10.90.1.1/8";
    std::string tep_first_ip_str_2 = "10.100.1.1/8";

    // Setup
    // make tep table half full
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_create(num_teps, tep_first_ip_str_1,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // Trigger
    // create 1 more than the max no of TEPs in a single batch
    num_teps = 2 + (PDS_MAX_TEP / 2);
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_create(num_teps, tep_first_ip_str_2,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_NO_RESOURCE);

    // abort the batch as it failed as expected
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    // verify all num_teps entries are NOT programmed
    ASSERT_TRUE(tep_util::many_read(sdk::SDK_RET_ENTRY_NOT_FOUND, num_teps,
        tep_first_ip_str_2, PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);

    // Teardown
    num_teps = PDS_MAX_TEP / 2;
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_delete(
        num_teps, tep_first_ip_str_1,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Read a non existing TEP
/// Read a non existing TEP with ONLY one entry in HW
TEST_F(tep_test, DISABLED_tep_read_nonexisting_when_single) {
    pds_batch_params_t batch_params = {0};
    pds_tep_info_t info;
    tep_util tep_obj_1("10.1.1.15/8", PDS_TEP_ENCAP_TYPE_VNIC);
    tep_util tep_obj_2("10.1.1.20/8", PDS_TEP_ENCAP_TYPE_VNIC);

    // Setup
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj_1.create() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(tep_obj_2.read(&info) == sdk::SDK_RET_ENTRY_NOT_FOUND);

    // Teardown
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj_1.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Read a non existing TEP
/// Read a non existing TEP with multiple entries in HW
TEST_F(tep_test, tep_read_nonexisting_when_multiple) {
    pds_batch_params_t batch_params = {0};
    pds_tep_info_t info;
    tep_util tep_obj_1("10.1.1.21", PDS_TEP_ENCAP_TYPE_VNIC);
    tep_util tep_obj_2("10.1.1.22", PDS_TEP_ENCAP_TYPE_VNIC);
    tep_util tep_obj_3("10.1.1.23", PDS_TEP_ENCAP_TYPE_VNIC);
    tep_util tep_obj_4("10.1.1.24", PDS_TEP_ENCAP_TYPE_VNIC);
    tep_util tep_obj_5("10.1.1.25", PDS_TEP_ENCAP_TYPE_VNIC);
    tep_util tep_obj("10.1.1.30", PDS_TEP_ENCAP_TYPE_VNIC);

    // Setup
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj_1.create() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj_2.create() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj_3.create() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj_4.create() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj_5.create() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(tep_obj_1.read(&info) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj_2.read(&info) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj_3.read(&info) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj_4.read(&info) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj_5.read(&info) == sdk::SDK_RET_OK);

    // Trigger
    ASSERT_TRUE(tep_obj.read(&info) == sdk::SDK_RET_ENTRY_NOT_FOUND);

    // Teardown
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj_1.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj_2.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj_3.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj_4.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj_5.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Read an existing TEP
/// Read an existing TEP with ONLY that entry in HW
TEST_F(tep_test, tep_read_existing_when_single) {
    pds_batch_params_t batch_params = {0};
    pds_tep_info_t info;
    tep_util tep_obj("10.1.1.31", PDS_TEP_ENCAP_TYPE_VNIC);

    // Setup
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj.create() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(tep_obj.read(&info) == sdk::SDK_RET_OK);

    // Teardown
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_obj.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(tep_obj.read(&info) == sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Read an existing TEP
/// Read an existing TEP with multiple entries in HW
TEST_F(tep_test, DISABLED_tep_read_existing_when_multiple) {
    pds_batch_params_t batch_params = {0};
    uint32_t num_teps = 3;
    std::string tep_first_ip_str = "10.13.1.1/8";

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_create(num_teps, tep_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // verify all num_teps entries are programmed
    ASSERT_TRUE(tep_util::many_read(sdk::SDK_RET_OK, num_teps,
        tep_first_ip_str, PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);

    // Teardown
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_delete(
        num_teps, tep_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Read last deleted TEP when table is empty
/// Read the last deleted TEP when nothing is programmed
TEST_F(tep_test, tep_read_last_deleted_when_empty) {
    pds_batch_params_t batch_params = {0};
    uint32_t num_teps = 3;
    std::string tep_first_ip_str = "10.14.1.1/8";

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_create(num_teps, tep_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // Teardown
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_delete(
        num_teps, tep_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // verify all num_teps entries are NOT programmed
    ASSERT_TRUE(tep_util::many_read(sdk::SDK_RET_ENTRY_NOT_FOUND, num_teps,
        tep_first_ip_str, PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
}

/// \brief Read last created TEP when table is empty
/// Read the last created TEP when nothing is programmed
TEST_F(tep_test, tep_read_last_created_when_empty) {
    pds_batch_params_t batch_params = {0};
    uint32_t num_teps = 4;
    std::string tep_first_ip_str = "10.15.1.1/8";

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_create(num_teps, tep_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // Teardown
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_delete(
        num_teps, tep_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // verify all num_teps entries are NOT programmed
    ASSERT_TRUE(tep_util::many_read(sdk::SDK_RET_ENTRY_NOT_FOUND, num_teps,
        tep_first_ip_str, PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
}

/// \brief Read last updated TEP when table is empty
/// Read the last updated TEP when nothing is programmed
TEST_F(tep_test, DISABLED_tep_read_last_updated_when_empty) {}

/// \brief Read an existing TEP when table is full
/// Read an existing TEP (first/last HW entry) when table is full
TEST_F(tep_test, DISABLED_tep_read_existing_when_full) {
    pds_batch_params_t batch_params = {0};
    pds_tep_info_t info;
    uint32_t num_teps = PDS_MAX_TEP;
    std::string tep_first_ip_str = "10.110.1.1/8";
    tep_util tep_obj("10.1.1.1", PDS_TEP_ENCAP_TYPE_VNIC);

    // Trigger
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_create(num_teps, tep_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // verify all num_teps entries are programmed
    ASSERT_TRUE(tep_util::many_read(sdk::SDK_RET_OK, num_teps,
        tep_first_ip_str, PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);

    // read non programmed tep
    ASSERT_TRUE(tep_obj.read(&info) == sdk::SDK_RET_ENTRY_NOT_FOUND);

    // Teardown
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_delete(
        num_teps, tep_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Read all TEPs
/// Read all currently programmed TEPs with single entry in HW
TEST_F(tep_test, DISABLED_tep_read_all_when_single) {}

/// \brief Read all TEPs
/// Read all currently programmed TEPs with multiple entries in HW
TEST_F(tep_test, DISABLED_tep_read_all_when_multiple) {}

/// \brief Read all TEPs on empty table
/// Read all currently programmed TEPs when nothing is programmed
TEST_F(tep_test, DISABLED_tep_read_all_when_empty) {}

/// \brief Read all TEPs after table full
/// Read all currently programmed TEPs when table is full
TEST_F(tep_test, DISABLED_tep_read_all_when_full) {}

/// \brief Create and delete max TEPs in the same batch
/// The operation should be de-duped by framework and effectively
/// a NO-OP from hardware perspective
TEST_F(tep_test, tep_workflow_1) {
    pds_batch_params_t batch_params = {0};
    uint32_t num_teps = PDS_MAX_TEP - 1;
    std::string tep_first_ip_str = "50.50.1.1/8";

    // [ Create SetMax, Delete SetMax ] - Read

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_create(num_teps, tep_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_delete(num_teps, tep_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // verify all num_teps entries are NOT programmed
    ASSERT_TRUE(tep_util::many_read(sdk::SDK_RET_ENTRY_NOT_FOUND, num_teps,
        tep_first_ip_str, PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
}

/// \brief Create, delete and create max TEPs in the same batch
/// The operation should be program and unprogram device in hardware
/// and return successful after create
TEST_F(tep_test, DISABLED_tep_workflow_2) {
    pds_batch_params_t batch_params = {0};
    uint32_t num_teps = PDS_MAX_TEP - 1;
    std::string tep_first_ip_str = "50.50.1.1/8";

    // [ Create SetMax - Delete SetMax - Create SetMax ] - Read

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_create(num_teps, tep_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_delete(num_teps, tep_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_create(num_teps, tep_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // verify all num_teps entries are programmed
    ASSERT_TRUE(tep_util::many_read(sdk::SDK_RET_OK, num_teps,
        tep_first_ip_str, PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);

    // teardown
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_delete(num_teps, tep_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create, delete some and create another set of TEPs in the same batch
/// The operation should be program and unprogram device in hardware
/// and return successful after create
TEST_F(tep_test, DISABLED_tep_workflow_3) {
    pds_batch_params_t batch_params = {0};
    std::string set1_first_ip_str = "10.10.1.1/8";
    std::string set2_first_ip_str = "20.20.1.1/8";
    std::string set3_first_ip_str = "30.30.1.1/8";

    // [ Create Set1, Set2 - Delete Set1 - Create Set3 ] - Read

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    // create set1 of 10 TEPs
    ASSERT_TRUE(tep_util::many_create(10, set1_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    // create set2 of 20 TEPs
    ASSERT_TRUE(tep_util::many_create(20, set2_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    // dreate set1 of 10 TEPs
    ASSERT_TRUE(tep_util::many_delete(10, set1_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    // create set3 of 30 TEPs
    ASSERT_TRUE(tep_util::many_create(30, set3_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // verify TEPs from set1 are NOT programmed
    ASSERT_TRUE(tep_util::many_read(sdk::SDK_RET_ENTRY_NOT_FOUND, 10,
        set1_first_ip_str, PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    // verify TEPs from set2 are programmed
    ASSERT_TRUE(tep_util::many_read(sdk::SDK_RET_OK, 20,
        set2_first_ip_str, PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    // verify TEPs from set3 are programmed
    ASSERT_TRUE(tep_util::many_read(sdk::SDK_RET_OK, 30,
        set3_first_ip_str, PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);

    // teardown
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    // delete set2 of 20 TEPs
    ASSERT_TRUE(tep_util::many_delete(20, set2_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    // delete set3 of 30 TEPs
    ASSERT_TRUE(tep_util::many_delete(30, set3_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create and delete max TEPs in different batches
/// The hardware should create device correctly and return entry not found
/// after delete
TEST_F(tep_test, DISABLED_tep_workflow_4) {
    pds_batch_params_t batch_params = {0};
    uint32_t num_teps = PDS_MAX_TEP - 1;
    std::string tep_first_ip_str = "50.50.1.1/8";

    // [ Create SetMax ] - Read - [ Delete SetMax ] - Read

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_create(num_teps, tep_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // verify all num_teps entries are programmed
    ASSERT_TRUE(tep_util::many_read(sdk::SDK_RET_OK, num_teps,
        tep_first_ip_str, PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_delete(num_teps, tep_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // verify all num_teps entries are NOT programmed
    ASSERT_TRUE(tep_util::many_read(sdk::SDK_RET_ENTRY_NOT_FOUND, num_teps,
        tep_first_ip_str, PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
}

/// \brief Create and delete different sets of TEPs in different batches
/// The hardware should create TEPs correctly and return entry not found
/// after delete for deleted ones
TEST_F(tep_test, DISABLED_tep_workflow_5) {
    pds_batch_params_t batch_params = {0};
    std::string set1_first_ip_str = "10.10.1.1/8";
    std::string set2_first_ip_str = "20.20.1.1/8";
    std::string set3_first_ip_str = "30.30.1.1/8";

    // [ Create Set1, Set2 ] - Read - [Delete Set1 - Create Set3 ] - Read

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    // create set1 of 10 TEPs
    ASSERT_TRUE(tep_util::many_create(10, set1_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    // create set2 of 20 TEPs
    ASSERT_TRUE(tep_util::many_create(20, set2_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // verify TEPs from set1 are programmed
    ASSERT_TRUE(tep_util::many_read(sdk::SDK_RET_OK, 10,
        set1_first_ip_str, PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    // verify TEPs from set2 are programmed
    ASSERT_TRUE(tep_util::many_read(sdk::SDK_RET_OK, 20,
        set2_first_ip_str, PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    // create set1 of 10 TEPs
    ASSERT_TRUE(tep_util::many_delete(10, set1_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    // create set3 of 30 TEPs
    ASSERT_TRUE(tep_util::many_create(30, set3_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // verify TEPs from set1 are NOT programmed
    ASSERT_TRUE(tep_util::many_read(sdk::SDK_RET_ENTRY_NOT_FOUND, 10,
        set1_first_ip_str, PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    // verify TEPs from set2 are programmed
    ASSERT_TRUE(tep_util::many_read(sdk::SDK_RET_OK, 20,
        set2_first_ip_str, PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    // verify TEPs from set3 are programmed
    ASSERT_TRUE(tep_util::many_read(sdk::SDK_RET_OK, 30,
        set3_first_ip_str, PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);

    // teardown
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    // delete set2 of 20 TEPs
    ASSERT_TRUE(tep_util::many_delete(20, set2_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    // delete set3 of 30 TEPs
    ASSERT_TRUE(tep_util::many_delete(30, set3_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create and recreate max TEPs in two batches
/// The hardware should program device correctly in case of
/// first create and return error in second create operation
TEST_F(tep_test, DISABLED_tep_workflow_neg_1) {
    pds_batch_params_t batch_params = {0};
    uint32_t num_teps = PDS_MAX_TEP - 1;
    std::string tep_first_ip_str = "50.50.1.1/8";

    // [ Create SetMax ] - [ Create SetMax ] - Read

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_create(num_teps, tep_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_create(num_teps, tep_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_INVALID_OP);
    // abort the batch as it failed as expected
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    // verify all num_teps entries are programmed
    ASSERT_TRUE(tep_util::many_read(sdk::SDK_RET_OK, num_teps,
        tep_first_ip_str, PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);

    // teardown
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_delete(num_teps, tep_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create more than max TEPs
TEST_F(tep_test, tep_workflow_neg_2) {
    pds_batch_params_t batch_params = {0};
    uint32_t num_teps = PDS_MAX_TEP + 1;
    std::string tep_first_ip_str = "50.50.1.1/8";

    // [ Create SetMax+1] - Read

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_create(num_teps, tep_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_NO_RESOURCE);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    // verify all num_teps entries are NOT programmed
    ASSERT_TRUE(tep_util::many_read(sdk::SDK_RET_ENTRY_NOT_FOUND, num_teps,
        tep_first_ip_str, PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
}

/// \brief Read and delete non existing TEPs
/// The hardware should return entry not found
TEST_F(tep_test, tep_workflow_neg_3) {
    pds_batch_params_t batch_params = {0};
    pds_tep_info_t info;
    uint32_t num_teps = PDS_MAX_TEP;
    std::string tep_invalid_ip_str = "150.150.1.1/8";
    tep_util tep_obj("192.168.9.1", PDS_TEP_ENCAP_TYPE_VNIC);

    // Read NonEx, [ Delete NonExMax ]

    // read a non existing TEP entry
    ASSERT_TRUE(tep_obj.read(&info) == sdk::SDK_RET_ENTRY_NOT_FOUND);

    // read many non existing TEP entries
    ASSERT_TRUE(tep_util::many_read(sdk::SDK_RET_ENTRY_NOT_FOUND, num_teps,
        tep_invalid_ip_str, PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);

    // delete non existing TEP entries
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_delete(num_teps, tep_invalid_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_ENTRY_NOT_FOUND);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);
}

/// \brief Create and delete different TEPs in different batches
/// The hardware should create TEPs correctly and
/// delete batch (containing valid & invalid entries) is expected to fail
/// because of lookup failure.
TEST_F(tep_test, DISABLED_tep_workflow_neg_4) {
    pds_batch_params_t batch_params = {0};
    pds_tep_info_t info;
    uint32_t num_teps = 10;
    std::string tep_first_ip_str = "50.50.1.1/8";
    tep_util tep_obj("99.99.99.99", PDS_TEP_ENCAP_TYPE_VNIC);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_create(num_teps, tep_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // verify all TEPs are programmed
    ASSERT_TRUE(tep_util::many_read(sdk::SDK_RET_OK, num_teps,
        tep_first_ip_str, PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);

    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(tep_util::many_delete(num_teps, tep_first_ip_str,
        PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
    // TODO: rollback config causing crash in this scenario.
    // keeping the testcase DISABLED
    ASSERT_TRUE(tep_obj.del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_ENTRY_NOT_FOUND);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    // verify all TEPs are still programmed
    ASSERT_TRUE(tep_util::many_read(sdk::SDK_RET_OK, num_teps,
        tep_first_ip_str, PDS_TEP_ENCAP_TYPE_VNIC) == sdk::SDK_RET_OK);
}

/// \brief TEP workflow corner case 4
///
/// [ Create SetCorner ] - Read
TEST_F(tep_test, DISABLED_tep_workflow_corner_case_4) {}

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
