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

// wrapper APIs to get logger and syslogger
std::shared_ptr<logger>
hal_logger (void)
{
    if (g_trace_logger) {
        return g_trace_logger->logger();
    }
    return NULL;
}

std::shared_ptr<logger>
hal_syslogger (void)
{
    if (g_syslog_logger) {
        return g_syslog_logger->logger();
    }
    return NULL;
}

::utils::trace_level_e
hal_trace_level (void)
{
    if (g_trace_logger) {
        return g_trace_logger->trace_level();
    }
    return ::utils::trace_none;
}

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

