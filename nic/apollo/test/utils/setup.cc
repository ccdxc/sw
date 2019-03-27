//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the base class implementation of test classes
///
//----------------------------------------------------------------------------

#include <stdarg.h>
#include "base.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/api/include/pds_init.hpp"

/// Callback invoked for debug traces
///
/// This is sample implementation, hence doesn't check whether
//  user enabled traces at what level, it always prints the traces
/// but with a simple header prepended that tells what level the
/// trace is spwed at ... in reality, you call your favorite logger here
static char logbuf[2048];
static int
trace_cb (sdk_trace_level_e trace_level, const char *format, ...)
{
    va_list args;

    va_start(args, format);
    vsnprintf(logbuf, sizeof(logbuf), format, args);
    va_end(args);

    switch (trace_level) {
    case sdk::lib::SDK_TRACE_LEVEL_NONE:
        return 0;
        break;
    case sdk::lib::SDK_TRACE_LEVEL_ERR:
        fprintf(stderr, "[E] %s\n", logbuf);
        break;
    case sdk::lib::SDK_TRACE_LEVEL_WARN:
        fprintf(stderr, "[W] %s\n", logbuf);
        break;
    case sdk::lib::SDK_TRACE_LEVEL_INFO:
        fprintf(stdout, "[I] %s\n", logbuf);
        break;
    case sdk::lib::SDK_TRACE_LEVEL_DEBUG:
        fprintf(stdout, "[D] %s\n", logbuf);
        break;
    case sdk::lib::SDK_TRACE_LEVEL_VERBOSE:
        // fprintf(stdout, "[V] %s\n", logbuf);
        break;
    default:
        break;
    }
    fflush(stdout);
    return 0;
}

/// Called at the beginning of all test cases in this class,
/// initialize PDS HAL
void
pds_test_base::SetUpTestCase(test_case_params_t &params)
{
    pds_init_params_t init_params;

    memset(&init_params, 0, sizeof(init_params));
    init_params.init_mode = PDS_INIT_MODE_COLD_START;
    init_params.trace_cb  = trace_cb;
    init_params.pipeline  = "apollo";
    init_params.cfg_file  = std::string(params.cfg_file);
    init_params.scale_profile = PDS_SCALE_PROFILE_DEFAULT;
    if (!params.profile.empty()) {
        if (params.profile.compare("p1") == 0) {
            init_params.scale_profile = PDS_SCALE_PROFILE_P1;
        }
    }
    pds_init(&init_params);
}

/// Called at the end of all test cases in this class,
/// cleanup PDS HAL and quit
void
pds_test_base::TearDownTestCase(void)
{
    // pds_teardown();
}
