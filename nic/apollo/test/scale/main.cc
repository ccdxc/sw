//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the base class implementation of test classes
///
//----------------------------------------------------------------------------

#include <gtest/gtest.h>
#include "nic/apollo/test/scale/test.hpp"
#include "nic/apollo/test/scale/pkt.hpp"
#include "nic/apollo/test/utils/base.hpp"
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/api/include/pds_device.hpp"
#include "nic/sdk/platform/capri/capri_p4.hpp"
#include "nic/sdk/model_sim/include/lib_model_client.h"

char *g_input_cfg_file = NULL;
char *g_cfg_file = NULL;
bool g_daemon_mode = false;
string g_profile;

class scale_test : public pds_test_base {
protected:
    scale_test() {}
    virtual ~scale_test() {}
    /**< called immediately after the constructor before each test */
    virtual void SetUp() {}
    /**< called immediately after each test before the destructor */
    virtual void TearDown() {}
    /**< called at the beginning of all test cases in this class */
    static void SetUpTestCase() {
        /**< call base class function */
        test_case_params_t params;
        params.cfg_file = g_cfg_file;
        params.enable_fte = false;
        params.profile = g_profile;
        pds_test_base::SetUpTestCase(params);
    }
    static void TearDownTestCase() {
        pds_test_base::TearDownTestCase();
    }
};

/// \defgroup Scale
/// @{

/// Scale test
TEST_F(scale_test, scale_test_create)
{
    sdk_ret_t rv;
    pds_batch_params_t batch_params = {0};

    batch_params.epoch = 1;
    rv = pds_batch_start(&batch_params);
    ASSERT_TRUE(rv == SDK_RET_OK);
    rv = create_objects();
    ASSERT_TRUE(rv == SDK_RET_OK);
    rv = pds_batch_commit();
    ASSERT_TRUE(rv == SDK_RET_OK);

#if 0
#ifdef SIM
    send_packet();
#endif
#endif

    printf("DONE pushing configs!\n");

    if (g_daemon_mode) {
        printf("Entering forever loop ...\n");
        fflush(stdout);
        while (1)
            ;
    }
}

/// @}

// print help message showing usage of this gtest
static void inline print_usage(char **argv)
{
    fprintf(stdout, "Usage : %s -c <hal.json> -i <test-config.json>\n",
            argv[0]);
}

int
main (int argc, char **argv)
{
    int oc;
    struct option longopts[] = {
        {"config",  required_argument, NULL, 'c'},
        {"daemon",  required_argument, NULL, 'd'},
        {"profile", required_argument, NULL, 'p'},
        {"help", no_argument, NULL, 'h'},
        {0, 0, 0, 0}
    };

    // parse CLI options
    while ((oc = getopt_long(argc, argv, ":hdc:i:p:W;", longopts, NULL)) != -1) {
        switch (oc) {
        case 'c':
            g_cfg_file = optarg;
            if (!g_cfg_file) {
                fprintf(stderr, "HAL config file is not specified\n");
                print_usage(argv);
                exit(1);
            }
            break;

        case 'p':
            if (optarg) {
                g_profile = std::string(optarg);
            } else {
                fprintf(stderr, "profile is not specified\n");
                print_usage(argv);
                exit(1);
            }
            break;

        case 'd':
            g_daemon_mode = true;
            break;

        case 'i':
            g_input_cfg_file = optarg;
            if (!g_input_cfg_file) {
                fprintf(stderr, "test config file is not specified\n");
                print_usage(argv);
                exit(1);
            }
            break;

        default:
            // ignore all other options
            break;
        }
    }

    // make sure cfg files exist
    if (access(g_input_cfg_file, R_OK) < 0) {
        fprintf(stderr, "Config file %s doesn't exist or not accessible\n",
                g_input_cfg_file);
        exit(1);
    }

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
