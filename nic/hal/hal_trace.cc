//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/hal/hal_trace.hpp"

namespace hal {
namespace utils {

// HAL specific globals
::utils::log *g_trace_logger;
::utils::log *g_syslog_logger;
uint64_t g_cpu_mask;

// wrapper for HAL trace init function
void
trace_init (const char *name, uint64_t cores_mask, bool sync_mode,
            const char *trace_file, size_t file_size, size_t num_files,
            ::utils::trace_level_e trace_level)
{
    if ((name == NULL) || (trace_file == NULL)) {
        return;
    }
    g_trace_logger =
        ::utils::log::factory(name, cores_mask,
            sync_mode ? ::utils::log_mode_sync : ::utils::log_mode_async,
            false, trace_file, file_size, num_files,
            ::utils::trace_debug, ::utils::log_none);
}

void
trace_deinit (void)
{
    if (g_trace_logger) {
        // TODO destory spdlog instance in g_trace_logger?
        ::utils::log::destroy(g_trace_logger);
    }

    g_trace_logger = NULL;
    return;
}

}    // utils
}    // hal

