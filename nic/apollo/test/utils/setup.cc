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
    const char *pfx;
    struct timespec tp_;

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
        } else if (params.profile.compare("p2") == 0) {
            init_params.scale_profile = PDS_SCALE_PROFILE_P2;
        }
    }
    init_params.batching_en = true;
    pds_init(&init_params);
}

/// Called at the end of all test cases in this class,
/// cleanup PDS HAL and quit
void
pds_test_base::TearDownTestCase(void)
{
    pds_teardown();
}
