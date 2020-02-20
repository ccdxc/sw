//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/metaswitch/pegasus_trace.hpp"

namespace core {

//------------------------------------------------------------------------------
// globals
//------------------------------------------------------------------------------
utils::log *g_trace_logger;

//------------------------------------------------------------------------------
// initialize trace lib
//------------------------------------------------------------------------------
void
trace_init (const char *name, uint64_t cores_mask, bool sync_mode,
            const char *err_file, const char *trace_file, size_t file_size,
            size_t num_files, utils::trace_level_e trace_level)
{
    if ((name == NULL) || (trace_file == NULL)) {
        return;
    }
    g_trace_logger =
        utils::log::factory(name, cores_mask,
            sync_mode ? utils::log_mode_sync : utils::log_mode_async,
            false, err_file, trace_file, file_size, num_files,
            utils::trace_err, trace_level, utils::log_none);
}

//------------------------------------------------------------------------------
// cleanup trace lib
//------------------------------------------------------------------------------
void
trace_deinit (void)
{
    if (g_trace_logger) {
        utils::log::destroy(g_trace_logger);
    }
    g_trace_logger = NULL;
    return;
}

//------------------------------------------------------------------------------
// change trace level
//------------------------------------------------------------------------------
void
trace_update (utils::trace_level_e trace_level)
{
    g_trace_logger->set_trace_level(trace_level);
    return;
}


//------------------------------------------------------------------------------
// flush logs
//------------------------------------------------------------------------------
void
flush_logs (void)
{
    trace_logger()->flush();
    return;
}

}    // namespace core
