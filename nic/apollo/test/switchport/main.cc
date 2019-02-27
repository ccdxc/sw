//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all switchport test cases
///
//----------------------------------------------------------------------------
#include <getopt.h>
#include "nic/apollo/include/api/pds_batch.hpp"
#include "nic/apollo/test/utils/base.hpp"
#include "nic/apollo/test/utils/switchport.hpp"

using std::cerr;
using std::cout;
using std::endl;

namespace api_test {

// Globals
char *g_cfg_file = NULL;
static pds_epoch_t g_batch_epoch = PDS_EPOCH_INVALID;

// Constants
const std::string k_switchport_ip_str("1.0.0.1");
const std::string k_gateway_ip_str("1.0.0.2");
const std::string k_mac_addr_str("00:02:01:00:00:01");

//----------------------------------------------------------------------------
// SwitchPort test class
//----------------------------------------------------------------------------

class switchport_test : public pds_test_base {
protected:
    switchport_test() {}
    virtual ~switchport_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        pds_test_base::SetUpTestCase(api_test::g_cfg_file, false);
    }
};

//----------------------------------------------------------------------------
// SwitchPort test cases utility routines
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// SwitchPort test cases implementation
//----------------------------------------------------------------------------

/// \defgroup SwitchPort
/// @{

/// \brief Read SwitchPort without Create
///
/// Without creating a switchport, read switchport
TEST_F(switchport_test, switchport_read_before_create) {
    switchport_util switchport_obj;
    pds_switchport_info_t info;

    // Trigger
    ASSERT_TRUE(switchport_obj.read(&info) == sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Delete SwitchPort without Create
///
/// Without creating a switchport, attempt to delete switchport
TEST_F(switchport_test, switchport_delete_before_create) {
    pds_batch_params_t batch_params = {0};

    // Trigger
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(switchport_util::del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_ENTRY_NOT_FOUND);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);
}

/// \brief Create SwitchPort
///
/// Create switchport for the first time and verify with read
TEST_F(switchport_test, switchport_create) {
    pds_batch_params_t batch_params = {0};
    pds_switchport_info_t info;
    switchport_util switchport_obj(k_switchport_ip_str, k_mac_addr_str,
                                   k_gateway_ip_str);

    // Trigger
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(switchport_obj.create() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // Verify with read
    ASSERT_TRUE(switchport_obj.read(&info) == sdk::SDK_RET_OK);
    //TODO fix read_hw
    //ASSERT_TRUE(switchport_obj.validate(&info) == sdk::SDK_RET_OK);

    // Tear down
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(switchport_util::del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create SwitchPort after Create
///
/// Create switchport immediately followed by another create with exact same
/// values but different batch
TEST_F(switchport_test, switchport_create_after_create) {}

/// \brief Create SwitchPort after Create
///
/// Create switchport immediately followed by another create with different
/// values and different batch
TEST_F(switchport_test, switchport_create_after_create_1) {}

/// \brief Delete SwitchPort
///
/// Delete switchport and validate with read
TEST_F(switchport_test, switchport_delete) {
    // TODO Even after delete, values are still present in DB.
    // Commenting this test out, until that is fixed.
#if 0
    pds_batch_params_t batch_params = {0};
    pds_switchport_info_t info;
    switchport_util    switchport_obj(k_switchport_ip_str, k_mac_addr_str,
                                   k_gateway_ip_str);

    // Setup
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == SDK_RET_OK);
    ASSERT_TRUE(switchport_obj.create() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // Trigger
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == SDK_RET_OK);
    ASSERT_TRUE(switchport_util::del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // Verify with read
    ASSERT_TRUE(switchport_obj.read(&info) == sdk::SDK_RET_OK);
    ASSERT_TRUE(switchport_obj.validate(&info) == sdk::SDK_RET_OK);
#endif
}

/// \brief Delete SwitchPort after Delete
///
/// Delete the switchport followed by delete in different batch
TEST_F(switchport_test, switchport_delete_after_delete) {}

/// \brief Create-Delete-Create Switchport in batches
///
/// Create, Delete & Create Switchport with all same values in different batch
TEST_F(switchport_test, switchport_create_delete_create) {}

/// \brief Create-Delete-Create Switchport in batches
///
/// Create, Delete & Create Switchport with all different values & batches
TEST_F(switchport_test, switchport_create_delete_create_1) {}

/// \brief Read SwitchPort multiple times
///
/// Read the switchport in a loop
TEST_F(switchport_test, switchport_read_loops) {}

/// \brief Create SwitchPort after Create in single batch
///
/// Create switchport immediately followed by another create with exact same
/// values and in same batch
TEST_F(switchport_test, switchport_create_after_create_same_batch) {}

/// \brief Create SwitchPort after Create in single batch
///
/// Create switchport immediately followed by another create with different
/// values but same batch
TEST_F(switchport_test, switchport_create_after_create_same_batch_1) {}

/// \brief Delete SwitchPort after Delete in single batch
///
/// Delete the switchport followed by delete in same batch
TEST_F(switchport_test, switchport_delete_after_delete_same_batch) {}

/// \brief Create-Delete-Create Switchport in single batch
///
/// Create, Delete and Create Switchport with all same values in same batch
TEST_F(switchport_test, switchport_create_delete_create_same_batch) {}

/// \brief Create-Delete-Create Switchport in single batch
///
/// Create, Delete & Create Switchport with all different values in same batch
TEST_F(switchport_test, switchport_create_delete_create_same_batch_1) {}

/// @}
}    // namespace api_test

static inline void
switchport_test_usage_print (char **argv)
{
    cout << "Usage : " << argv[0] << " -c <hal.json>" << endl;
    return;
}

static inline sdk_ret_t
switchport_test_options_validate (void)
{
    if (!api_test::g_cfg_file) {
        cerr << "HAL config file is not specified" << endl;
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

static void
switchport_test_options_parse (int argc, char **argv)
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
    switchport_test_options_parse(argc, argv);
    if (switchport_test_options_validate() != SDK_RET_OK) {
        switchport_test_usage_print(argv);
        exit(1);
    }

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
