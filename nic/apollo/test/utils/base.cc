//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include <iostream>
#include <getopt.h>
#include <stdarg.h>
#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/api/include/pds_init.hpp"
#include "nic/apollo/test/utils/base.hpp"

// Callback invoked for debug traces
//
// This is sample implementation, hence doesn't check whether
// user enabled traces at what level, it always prints the traces
// but with a simple header prepended that tells what level the
// trace is spwed at ... in reality, you call your favorite logger here
sdk_trace_level_e g_trace_level = sdk::lib::SDK_TRACE_LEVEL_DEBUG;
static int
trace_cb (sdk_trace_level_e trace_level, const char *format, ...)
{
    va_list args;
    const char *pfx;
    struct timespec tp_;
    char logbuf[1024];

    if (trace_level == sdk::lib::SDK_TRACE_LEVEL_NONE) {
        return 0;
    }

    if (trace_level > g_trace_level) {
        return 0;
    }

    switch (trace_level) {
    case sdk::lib::SDK_TRACE_LEVEL_ERR:
        pfx = "[E]";
        break;

    case sdk::lib::SDK_TRACE_LEVEL_WARN:
        pfx = "[W]";
        break;

    case sdk::lib::SDK_TRACE_LEVEL_INFO:
        pfx = "[I]";
        break;

    case sdk::lib::SDK_TRACE_LEVEL_DEBUG:
        pfx = "[D]";
        break;

    case sdk::lib::SDK_TRACE_LEVEL_VERBOSE:
    default:
        // pfx = "[V]";
        // fprintf(stdout, "[V] %s\n", logbuf);
        return 0;
    }

    clock_gettime(CLOCK_MONOTONIC, &tp_);
    va_start(args, format);
    vsnprintf(logbuf, sizeof(logbuf), format, args);
    va_end(args);
    fprintf(stdout, "%s [%lu.%9lu] %s\n", pfx, tp_.tv_sec, tp_.tv_nsec, logbuf);
    fflush(stdout);
    return 0;
}

// Called at the beginning of all test cases in this class, initialize PDS HAL
void
pds_test_base::SetUpTestCase(test_case_params_t& params)
{
    pds_init_params_t init_params;

    memset(&init_params, 0, sizeof(init_params));
    init_params.init_mode = PDS_INIT_MODE_COLD_START;
    init_params.trace_cb  = trace_cb;
    init_params.pipeline  = ::pipeline_get();
    init_params.cfg_file  = std::string(params.cfg_file);
    init_params.scale_profile = PDS_SCALE_PROFILE_DEFAULT;
    if (!params.profile.empty()) {
        if (params.profile.compare("p1") == 0) {
            init_params.scale_profile = PDS_SCALE_PROFILE_P1;
        } else if (params.profile.compare("p2") == 0) {
            init_params.scale_profile = PDS_SCALE_PROFILE_P2;
        }
    }
    pds_init(&init_params);
}

// Called at the end of all test cases in this class, cleanup PDS HAL and quit
void
pds_test_base::TearDownTestCase(void)
{
    pds_teardown();
}

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
