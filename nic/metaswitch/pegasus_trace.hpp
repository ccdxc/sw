//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//---------------------------------------------------------------

#ifndef __PEGASUS_TRACE_HPP__
#define __PEGASUS_TRACE_HPP__

#include "nic/include/trace.hpp"

namespace core {

extern utils::log *g_trace_logger;
void trace_init(const char *name, uint64_t cpu_mask, bool sync_mode,
                const char *err_file, const char *trace_file, size_t file_size,
                size_t max_files, utils::trace_level_e trace_level);
void trace_deinit(void);

// wrapper API to get logger
static inline std::shared_ptr<logger>
trace_logger (void)
{
    if (g_trace_logger) {
        return g_trace_logger->logger();
    }
    return NULL;
}

static inline utils::trace_level_e
trace_level (void)
{
    if (g_trace_logger) {
        return g_trace_logger->trace_level();
    }
    return utils::trace_none;
}

void trace_update(utils::trace_level_e trace_level);
void flush_logs(void);

}    // namespace core

#define PEGASUS_TRACE_ERR_NO_META(fmt...)                                      \
    if (likely(core::trace_logger()) &&                                        \
        (core::trace_level() >= utils::trace_err)) {                           \
        core::trace_logger()->error(fmt);                                      \
        core::trace_logger()->flush();                                         \
    }                                                                          \

#define PEGASUS_TRACE_WARN_NO_META(fmt...)                                     \
    if (likely(core::trace_logger()) &&                                        \
        (core::trace_level() >= utils::trace_warn)) {                          \
        core::trace_logger()->warn(fmt);                                       \
        core::trace_logger()->flush();                                         \
    }

#define PEGASUS_TRACE_INFO_NO_META(fmt...)                                     \
    if (likely(core::trace_logger()) &&                                        \
        (core::trace_level() >= utils::trace_info)) {                          \
        core::trace_logger()->info(fmt);                                       \
        core::trace_logger()->flush();                                         \
    }

#define PEGASUS_TRACE_DEBUG_NO_META(fmt...)                                    \
    if (likely(core::trace_logger()) &&                                        \
        (core::trace_level() >= utils::trace_debug)) {                         \
        core::trace_logger()->debug(fmt);                                      \
        core::trace_logger()->flush();                                         \
    }                                                                          \

#define PEGASUS_TRACE_VERBOSE_NO_META(fmt...)                                  \
    if (likely(core::trace_logger())) {                                        \
        core::trace_logger()->trace(fmt);                                      \
        core::trace_logger()->flush();                                         \
    }                                                                          \

#endif
