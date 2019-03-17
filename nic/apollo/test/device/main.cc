//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all device test cases
///
//----------------------------------------------------------------------------
#include <getopt.h>
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/test/utils/base.hpp"
#include "nic/apollo/test/utils/device.hpp"

using std::cerr;
using std::cout;
using std::endl;

namespace api_test {

// Globals
char *g_cfg_file = NULL;
static pds_epoch_t g_batch_epoch = PDS_EPOCH_INVALID;

// Constants
const std::string k_device_ip_str("1.0.0.1");
const std::string k_gateway_ip_str("1.0.0.2");
const std::string k_mac_addr_str("00:02:01:00:00:01");

//----------------------------------------------------------------------------
// Device test class
//----------------------------------------------------------------------------

class device_test : public pds_test_base {
protected:
    device_test() {}
    virtual ~device_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        pds_test_base::SetUpTestCase(api_test::g_cfg_file, false);
    }
};

//----------------------------------------------------------------------------
// Device test cases utility routines
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Device test cases implementation
//----------------------------------------------------------------------------

/// \defgroup Device
/// @{

/// \brief Create and delete a device in the same batch
/// The operation should be de-duped by framework and effectively
/// a NO-OP from hardware perspective
TEST_F(device_test, DISABLED_device_workflow_1) {
    pds_batch_params_t batch_params = {0};
    pds_device_info_t info;
    device_util device_obj(k_device_ip_str, k_mac_addr_str,
                                   k_gateway_ip_str);

    // Trigger
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(device_obj.create() == sdk::SDK_RET_OK);
    ASSERT_TRUE(device_util::del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // Verify with read
    ASSERT_TRUE(device_obj.read(&info, true) == sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create, delete and create a device in the same batch
/// The operation should be program and unprogram device in hardware
/// and return successful afetr create
TEST_F(device_test, DISABLED_device_workflow_2) {
    pds_batch_params_t batch_params = {0};
    pds_device_info_t info;
    device_util device_obj(k_device_ip_str, k_mac_addr_str,
                                   k_gateway_ip_str);

    // Trigger
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(device_obj.create() == sdk::SDK_RET_OK);
    ASSERT_TRUE(device_util::del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(device_obj.create() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // Verify with read
    ASSERT_TRUE(device_obj.read(&info, true) == sdk::SDK_RET_OK);

    // Teardown
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(device_util::del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create and delete device in two batches
/// The hardware should create device correctly
/// and return entry not found after delete
TEST_F(device_test, DISABLED_device_workflow_4) {
    pds_batch_params_t batch_params = {0};
    pds_device_info_t info;
    device_util device_obj(k_device_ip_str, k_mac_addr_str,
                                   k_gateway_ip_str);

    // Trigger: create a device in first batch
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(device_obj.create() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // Verify with read
    ASSERT_TRUE(device_obj.read(&info, true) == sdk::SDK_RET_OK);

    // Trigger: delete a device in next batch
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(device_util::del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // Verify with read
    ASSERT_TRUE(device_obj.read(&info, true) == sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create device in two batches
/// The hardware should program device correctly in case of
/// first create and return error in second create operation
TEST_F(device_test, DISABLED_device_workflow_neg_1) {
    pds_batch_params_t batch_params = {0};
    pds_device_info_t info;
    device_util device_obj(k_device_ip_str, k_mac_addr_str,
                                   k_gateway_ip_str);

    // Trigger: create a device in first batch
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(device_obj.create() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // Verify with read
    ASSERT_TRUE(device_obj.read(&info, true) == sdk::SDK_RET_OK);

    // Trigger: create a device gaain in next batch
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(device_obj.create() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() != sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_abort() == sdk:: SDK_RET_OK);

    // Teardown
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(device_util::del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Read a device without creating it
/// The hardware should return entry not found
TEST_F(device_test, device_workflow_neg_3a) {
    device_util device_obj;
    pds_device_info_t info;

    // Trigger
    ASSERT_TRUE(device_obj.read(&info, false) == sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Delete a device without creating it
/// The hardware should return entry not found
TEST_F(device_test, device_workflow_neg_3b) {
    pds_batch_params_t batch_params = {0};

    // Trigger
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(device_util::del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_ENTRY_NOT_FOUND);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);
}


/// @}

}    // namespace api_test

static inline void
device_test_usage_print (char **argv)
{
    cout << "Usage : " << argv[0] << " -c <hal.json>" << endl;
    return;
}

static inline sdk_ret_t
device_test_options_validate (void)
{
    if (!api_test::g_cfg_file) {
        cerr << "HAL config file is not specified" << endl;
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

static void
device_test_options_parse (int argc, char **argv)
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
// Entry point
//----------------------------------------------------------------------------
int
main (int argc, char **argv)
{
    device_test_options_parse(argc, argv);
    if (device_test_options_validate() != SDK_RET_OK) {
        device_test_usage_print(argv);
        exit(1);
    }

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
