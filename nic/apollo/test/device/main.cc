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
#include "nic/apollo/include/api/pds_batch.hpp"
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

/// \brief Read Device without Create
///
/// Without creating a device, read device
TEST_F(device_test, device_read_before_create) {
    device_util device_obj;
    pds_device_info_t info;

    // Trigger
    ASSERT_TRUE(device_obj.read(&info) == sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Delete Device without Create
///
/// Without creating a device, attempt to delete device
TEST_F(device_test, device_delete_before_create) {
    pds_batch_params_t batch_params = {0};

    // Trigger
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(device_util::del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_ENTRY_NOT_FOUND);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);
}

/// \brief Create Device
///
/// Create device for the first time and verify with read
TEST_F(device_test, device_create) {
    pds_batch_params_t batch_params = {0};
    pds_device_info_t info;
    device_util device_obj(k_device_ip_str, k_mac_addr_str,
                                   k_gateway_ip_str);

    // Trigger
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(device_obj.create() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // Verify with read
    ASSERT_TRUE(device_obj.read(&info) == sdk::SDK_RET_OK);
    //TODO fix read_hw
    //ASSERT_TRUE(device_obj.validate(&info) == sdk::SDK_RET_OK);

    // Tear down
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(device_util::del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create Device after Create
///
/// Create device immediately followed by another create with exact same
/// values but different batch
TEST_F(device_test, device_create_after_create) {}

/// \brief Create Device after Create
///
/// Create device immediately followed by another create with different
/// values and different batch
TEST_F(device_test, device_create_after_create_1) {}

/// \brief Delete Device
///
/// Delete device and validate with read
TEST_F(device_test, device_delete) {
    pds_batch_params_t batch_params = {0};
    pds_device_info_t info;
    device_util    device_obj(k_device_ip_str, k_mac_addr_str,
                                   k_gateway_ip_str);

    // Setup
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == SDK_RET_OK);
    ASSERT_TRUE(device_obj.create() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // Trigger
    batch_params.epoch = ++api_test::g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == SDK_RET_OK);
    ASSERT_TRUE(device_util::del() == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    // Verify that the read operation fails
    ASSERT_TRUE(device_obj.read(&info) == sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Delete Device after Delete
///
/// Delete the device followed by delete in different batch
TEST_F(device_test, device_delete_after_delete) {}

/// \brief Create-Delete-Create Device in batches
///
/// Create, Delete & Create Device with all same values in different batch
TEST_F(device_test, device_create_delete_create) {}

/// \brief Create-Delete-Create Device in batches
///
/// Create, Delete & Create Device with all different values & batches
TEST_F(device_test, device_create_delete_create_1) {}

/// \brief Read Device multiple times
///
/// Read the device in a loop
TEST_F(device_test, device_read_loops) {}

/// \brief Create Device after Create in single batch
///
/// Create device immediately followed by another create with exact same
/// values and in same batch
TEST_F(device_test, device_create_after_create_same_batch) {}

/// \brief Create Device after Create in single batch
///
/// Create device immediately followed by another create with different
/// values but same batch
TEST_F(device_test, device_create_after_create_same_batch_1) {}

/// \brief Delete Device after Delete in single batch
///
/// Delete the device followed by delete in same batch
TEST_F(device_test, device_delete_after_delete_same_batch) {}

/// \brief Create-Delete-Create Device in single batch
///
/// Create, Delete and Create Device with all same values in same batch
TEST_F(device_test, device_create_delete_create_same_batch) {}

/// \brief Create-Delete-Create Device in single batch
///
/// Create, Delete & Create Device with all different values in same batch
TEST_F(device_test, device_create_delete_create_same_batch_1) {}

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
