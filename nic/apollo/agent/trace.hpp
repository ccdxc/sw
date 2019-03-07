// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __PDS_AGENT_TRACE_HPP__
#define __PDS_AGENT_TRACE_HPP__

#include "nic/include/trace.hpp"

namespace core {

extern utils::log *g_trace_logger;
void trace_init(const char *name, uint64_t cpu_mask, bool sync_mode,
                const char *trace_file, size_t file_size, size_t max_files,
                utils::trace_level_e trace_level);
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

}    // namespace core

//------------------------------------------------------------------------------
// trace macros
// NOTE: we can't use printf() here if g_trace_logger is NULL, because printf()
// won't understand spdlog friendly formatters
// TODO: Remove call to trace_logger flush from all the macros below
//------------------------------------------------------------------------------
#define PDS_TRACE_FLUSH()                                                      \
    if (likely(trace_logger())) {                                              \
        core::trace_logger()->flush();                                         \
    }

#define PDS_TRACE_ERR(fmt, ...)                                                \
    if (likely(trace_logger()) && (trace_level() >= utils::trace_err)) {       \
        trace_logger()->error("[{}:{}] " fmt, __func__, __LINE__,              \
                              ##__VA_ARGS__);                                  \
        trace_logger()->flush();                                               \
    }                                                                          \

#define PDS_TRACE_ERR_NO_META(fmt...)                                          \
    if (likely(trace_logger()) && (trace_level() >= utils::trace_err)) {       \
        trace_logger()->error(fmt);                                            \
        trace_logger()->flush();                                               \
    }                                                                          \

#define PDS_TRACE_WARN(fmt, ...)                                               \
    if (likely(trace_logger()) && (trace_level() >= utils::trace_warn)) {      \
        trace_logger()->warn("[{}:{}] " fmt, __func__, __LINE__,               \
                             ##__VA_ARGS__);                                   \
        trace_logger()->flush();                                               \
    }                                                                          \

#define PDS_TRACE_WARN_NO_META(fmt, ...)                                       \
    if (likely(trace_logger()) && (trace_level() >= utils::trace_warn)) {      \
        trace_logger()->warn(fmt);                                             \
        trace_logger()->flush();                                               \
    }

#define PDS_TRACE_INFO(fmt, ...)                                               \
    if (likely(core::trace_logger()) &&                                        \
        (core::trace_level() >= utils::trace_info)) {                          \
        core::trace_logger()->info("[{}:{}] " fmt, __func__, __LINE__,         \
                             ##__VA_ARGS__);                                   \
        core::trace_logger()->flush();                                         \
    }                                                                          \

#define PDS_TRACE_INFO_NO_META(fmt, ...)                                       \
    if (likely(trace_logger()) && (trace_level() >= utils::trace_info)) {      \
        trace_logger()->info(fmt);                                             \
        trace_logger()->flush();                                               \
    }

#define PDS_TRACE_DEBUG(fmt, ...)                                              \
    if (likely(trace_logger()) && (trace_level() >= utils::trace_debug)) {     \
        trace_logger()->debug("[{}:{}] " fmt, __func__, __LINE__,              \
                              ##__VA_ARGS__);                                  \
        trace_logger()->flush();                                               \
    }                                                                          \

#define PDS_TRACE_DEBUG_NO_META(fmt...)                                        \
    if (likely(trace_logger()) && (trace_level() >= utils::trace_debug)) {     \
        trace_logger()->debug(fmt);                                            \
        trace_logger()->flush();                                               \
    }                                                                          \

#define PDS_TRACE_VERBOSE(fmt, ...)                                            \
    if (likely(trace_logger())) {                                              \
        trace_logger()->trace("[{}:{}] " fmt, __func__, __LINE__,              \
                              ##__VA_ARGS__);                                  \
        trace_logger()->flush();                                               \
    }                                                                          \

#define PDS_TRACE_VERBOSE_NO_META(fmt...)                                      \
    if (likely(trace_logger())) {                                              \
        trace_logger()->trace(fmt);                                            \
        trace_logger()->flush();                                               \
    }                                                                          \

#define PDS_ERR_IF(cond, fmt, ...)                                             \
    if (likely(trace_logger() &&                                               \
               (trace_level() >= utils::trace_err) && (cond))) {               \
        trace_logger()->error("[{}:{}] "  fmt,  __func__, __LINE__,            \
                              ##__VA_ARGS__);                                  \
        trace_logger()->flush();                                               \
    }                                                                          \

#define PDS_WARN_IF(cond, fmt, ...)                                            \
    if (likely(trace_logger() &&                                               \
               (trace_level() >= utils::trace_warn) && (cond))) {              \
        trace_logger()->warn("[{}:{}] "  fmt, __func__, __LINE__,              \
                             ##__VA_ARGS__);                                   \
        trace_logger()->flush();                                               \
    }                                                                          \

#define PDS_INFO_IF(cond, fmt, ...)                                            \
    if (likely(trace_logger() &&                                               \
               (trace_level() >= utils::trace_info) && (cond))) {              \
        trace_logger()->info("[{}:{}] "  fmt, __func__, __LINE__,              \
                             ##__VA_ARGS__);                                   \
        trace_logger()->flush();                                               \
    }                                                                          \

#define PDS_DEBUG_IF(cond, fmt, ...)                                           \
    if (likely(trace_logger() &&                                               \
               (trace_level() >= utils::trace_debug) && (cond))) {             \
        trace_logger()->debug("[{}:{}] "  fmt, __func__, __LINE__,             \
                                        ##__VA_ARGS__);                        \
        trace_logger()->flush();                                               \
    }                                                                          \

#endif    // __PDS_AGENT_TRACE_HPP__
