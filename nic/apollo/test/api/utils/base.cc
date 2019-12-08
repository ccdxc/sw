//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include <iostream>
#include <getopt.h>
#include <stdarg.h>
#include "nic/apollo/test/api/utils/base.hpp"

//----------------------------------------------------------------------------
// API test program routines, called from main()
//----------------------------------------------------------------------------

test_case_params_t g_tc_params;

static inline void
api_test_usage_print (char **argv)
{
    std::cout << "Usage : " << argv[0] << " -c <hal.json>" << std::endl;
}

static void
api_test_options_parse (int argc, char **argv)
{
    int oc;
    struct option longopts[] = {{"config", required_argument, NULL, 'c'},
                                {"help", no_argument, NULL, 'h'},
                                {0, 0, 0, 0}};

    g_tc_params.enable_fte = false;

    while ((oc = getopt_long(argc, argv, ":hc:", longopts, NULL)) != -1) {
        switch (oc) {
        case 'c':
            g_tc_params.cfg_file = optarg;
            break;
        default:    // ignore all other options
            break;
        }
    }
}

static inline sdk_ret_t
api_test_options_validate (void)
{
    if (!g_tc_params.cfg_file) {
        std::cerr << "HAL config file is not specified" << std::endl;
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

int
api_test_program_run (int argc, char **argv)
{
    api_test_options_parse(argc, argv);
    if (api_test_options_validate() != SDK_RET_OK) {
        api_test_usage_print(argv);
        exit(1);
    }

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
