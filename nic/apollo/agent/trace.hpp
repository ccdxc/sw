// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __PDS_AGENT_TRACE_HPP__
#define __PDS_AGENT_TRACE_HPP__

#include "nic/include/trace.hpp"

namespace core {

extern utils::log *g_trace_logger;
extern utils::log *g_link_trace_logger;
extern utils::log *g_hmon_trace_logger;
extern utils::log *g_intr_trace_logger;
void trace_init(const char *name, uint64_t cpu_mask, bool sync_mode,
                const char *err_file, const char *trace_file, size_t file_size,
                size_t max_files, utils::trace_level_e trace_level);
void link_trace_init(const char *name, uint64_t cpu_mask, bool sync_mode,
                     const char *err_file, const char *trace_file,
                     size_t file_size, size_t max_files,
                     utils::trace_level_e trace_level);
void hmon_trace_init(const char *name, uint64_t cpu_mask, bool sync_mode,
                     const char *err_file, const char *trace_file,
                     size_t file_size, size_t max_files,
                     utils::trace_level_e trace_level);
void intr_trace_init(const char *name, uint64_t cpu_mask, bool sync_mode,
                     const char *err_file, const char *trace_file,
                     size_t file_size, size_t max_files,
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

static inline std::shared_ptr<logger>
link_trace_logger (void)
{
    if (g_link_trace_logger) {
        return g_link_trace_logger->logger();
    }
    return NULL;
}

static inline std::shared_ptr<logger>
hmon_trace_logger (void)
{
    if (g_hmon_trace_logger) {
        return g_hmon_trace_logger->logger();
    }
    return NULL;
}

static inline std::shared_ptr<logger>
intr_trace_logger (void)
{
    if (g_intr_trace_logger) {
        return g_intr_trace_logger->logger();
    }
    return NULL;
}

void trace_update(utils::trace_level_e trace_level);
void flush_logs(void);

}    // namespace core

//------------------------------------------------------------------------------
// trace macros
// NOTE: we can't use printf() here if g_trace_logger is NULL, because printf()
// won't understand spdlog friendly formatters
// TODO: Remove call to trace_logger flush from all the macros below
//------------------------------------------------------------------------------

#define PDS_TRACE_FLUSH()                                                      \
    if (likely(core::trace_logger())) {                                        \
        core::core::trace_logger()->flush();                                   \
    }                                                                          \
    if (likely(core::link_trace_logger())) {                                   \
        core::core::link_trace_logger()->flush();                              \
    }                                                                          \
    if (likely(core::hmon_trace_logger())) {                                   \
        core::core::hmon_trace_logger()->flush();                              \
    }                                                                          \
    if (likely(core::intr_trace_logger())) {                                   \
        core::core::intr_trace_logger()->flush();                              \
    }

#define PDS_MOD_TRACE_ERR(mod_id, fmt, ...) {                                  \
    switch (mod_id) {                                                          \
    case sdk_mod_id_t::SDK_MOD_ID_LINK:                                        \
        PDS_LINK_TRACE_ERR(fmt, ##__VA_ARGS__);                                \
        break;                                                                 \
    case sdk_mod_id_t::SDK_MOD_ID_HMON:                                        \
        PDS_HMON_TRACE_ERR(fmt, ##__VA_ARGS__);                                \
        break;                                                                 \
    case sdk_mod_id_t::SDK_MOD_ID_INTR:                                        \
        PDS_INTR_TRACE_ERR(fmt, ##__VA_ARGS__);                                \
        break;                                                                 \
    default:                                                                   \
        PDS_TRACE_ERR(fmt, ##__VA_ARGS__);                                     \
        break;                                                                 \
    }                                                                          \
}

#define PDS_MOD_TRACE_ERR_NO_META(mod_id, fmt...) {                            \
    switch (mod_id) {                                                          \
    case sdk_mod_id_t::SDK_MOD_ID_LINK:                                        \
        PDS_LINK_TRACE_ERR_NO_META(fmt);                                       \
        break;                                                                 \
    case sdk_mod_id_t::SDK_MOD_ID_HMON:                                        \
        PDS_HMON_TRACE_ERR_NO_META(fmt);                                       \
        break;                                                                 \
    case sdk_mod_id_t::SDK_MOD_ID_INTR:                                        \
        PDS_INTR_TRACE_ERR_NO_META(fmt);                                       \
        break;                                                                 \
    default:                                                                   \
        PDS_TRACE_ERR_NO_META(fmt);                                            \
        break;                                                                 \
    }                                                                          \
}

#define PDS_MOD_TRACE_WARN(mod_id, fmt, ...) {                                 \
    switch (mod_id) {                                                          \
    case sdk_mod_id_t::SDK_MOD_ID_LINK:                                        \
        PDS_LINK_TRACE_WARN(fmt, ##__VA_ARGS__);                               \
        break;                                                                 \
    case sdk_mod_id_t::SDK_MOD_ID_HMON:                                        \
        PDS_HMON_TRACE_WARN(fmt, ##__VA_ARGS__);                               \
        break;                                                                 \
    case sdk_mod_id_t::SDK_MOD_ID_INTR:                                        \
        PDS_INTR_TRACE_WARN(fmt, ##__VA_ARGS__);                               \
        break;                                                                 \
    default:                                                                   \
        PDS_TRACE_WARN(fmt, ##__VA_ARGS__);                                    \
        break;                                                                 \
    }                                                                          \
}

#define PDS_MOD_TRACE_WARN_NO_META(mod_id, fmt...) {                           \
    switch (mod_id) {                                                          \
    case sdk_mod_id_t::SDK_MOD_ID_LINK:                                        \
        PDS_LINK_TRACE_WARN_NO_META(fmt);                                      \
        break;                                                                 \
    case sdk_mod_id_t::SDK_MOD_ID_HMON:                                        \
        PDS_HMON_TRACE_WARN_NO_META(fmt);                                      \
        break;                                                                 \
    case sdk_mod_id_t::SDK_MOD_ID_INTR:                                        \
        PDS_INTR_TRACE_WARN_NO_META(fmt);                                      \
        break;                                                                 \
    default:                                                                   \
        PDS_TRACE_WARN_NO_META(fmt);                                           \
        break;                                                                 \
    }                                                                          \
}

#define PDS_MOD_TRACE_INFO(mod_id, fmt, ...) {                                 \
    switch (mod_id) {                                                          \
    case sdk_mod_id_t::SDK_MOD_ID_LINK:                                        \
        PDS_LINK_TRACE_INFO(fmt, ##__VA_ARGS__);                               \
        break;                                                                 \
    case sdk_mod_id_t::SDK_MOD_ID_HMON:                                        \
        PDS_HMON_TRACE_INFO(fmt, ##__VA_ARGS__);                               \
        break;                                                                 \
    case sdk_mod_id_t::SDK_MOD_ID_INTR:                                        \
        PDS_INTR_TRACE_INFO(fmt, ##__VA_ARGS__);                               \
        break;                                                                 \
    default:                                                                   \
        PDS_TRACE_INFO(fmt, ##__VA_ARGS__);                                    \
        break;                                                                 \
    }                                                                          \
}

#define PDS_MOD_TRACE_INFO_NO_META(mod_id, fmt...) {                           \
    switch (mod_id) {                                                          \
    case sdk_mod_id_t::SDK_MOD_ID_LINK:                                        \
        PDS_LINK_TRACE_INFO_NO_META(fmt);                                      \
        break;                                                                 \
    case sdk_mod_id_t::SDK_MOD_ID_HMON:                                        \
        PDS_HMON_TRACE_INFO_NO_META(fmt);                                      \
        break;                                                                 \
    case sdk_mod_id_t::SDK_MOD_ID_INTR:                                        \
        PDS_INTR_TRACE_INFO_NO_META(fmt);                                      \
        break;                                                                 \
    default:                                                                   \
        PDS_TRACE_INFO_NO_META(fmt);                                           \
        break;                                                                 \
    }                                                                          \
}

#define PDS_MOD_TRACE_DEBUG(mod_id, fmt, ...) {                                \
    switch (mod_id) {                                                          \
    case sdk_mod_id_t::SDK_MOD_ID_LINK:                                        \
        PDS_LINK_TRACE_DEBUG(fmt, ##__VA_ARGS__);                              \
        break;                                                                 \
    case sdk_mod_id_t::SDK_MOD_ID_HMON:                                        \
        PDS_HMON_TRACE_DEBUG(fmt, ##__VA_ARGS__);                              \
        break;                                                                 \
    case sdk_mod_id_t::SDK_MOD_ID_INTR:                                        \
        PDS_INTR_TRACE_DEBUG(fmt, ##__VA_ARGS__);                              \
        break;                                                                 \
    default:                                                                   \
        PDS_TRACE_DEBUG(fmt, ##__VA_ARGS__);                                   \
        break;                                                                 \
    }                                                                          \
}

#define PDS_MOD_TRACE_DEBUG_NO_META(mod_id, fmt...) {                          \
    switch (mod_id) {                                                          \
    case sdk_mod_id_t::SDK_MOD_ID_LINK:                                        \
        PDS_LINK_TRACE_DEBUG_NO_META(fmt);                                     \
        break;                                                                 \
    case sdk_mod_id_t::SDK_MOD_ID_HMON:                                        \
        PDS_HMON_TRACE_DEBUG_NO_META(fmt);                                     \
        break;                                                                 \
    case sdk_mod_id_t::SDK_MOD_ID_INTR:                                        \
        PDS_INTR_TRACE_DEBUG_NO_META(fmt);                                     \
        break;                                                                 \
    default:                                                                   \
        PDS_TRACE_DEBUG_NO_META(fmt);                                          \
        break;                                                                 \
    }                                                                          \
}

#define PDS_MOD_TRACE_VERBOSE(mod_id, fmt, ...) {                              \
    switch (mod_id) {                                                          \
    case sdk_mod_id_t::SDK_MOD_ID_LINK:                                        \
        PDS_LINK_TRACE_VERBOSE(fmt, ##__VA_ARGS__);                            \
        break;                                                                 \
    case sdk_mod_id_t::SDK_MOD_ID_HMON:                                        \
        PDS_HMON_TRACE_VERBOSE(fmt, ##__VA_ARGS__);                            \
        break;                                                                 \
    case sdk_mod_id_t::SDK_MOD_ID_INTR:                                        \
        PDS_INTR_TRACE_VERBOSE(fmt, ##__VA_ARGS__);                            \
        break;                                                                 \
    default:                                                                   \
        PDS_TRACE_VERBOSE(fmt, ##__VA_ARGS__);                                 \
        break;                                                                 \
    }                                                                          \
}

#define PDS_MOD_TRACE_VERBOSE_NO_META(mod_id, fmt...) {                        \
    switch (mod_id) {                                                          \
    case sdk_mod_id_t::SDK_MOD_ID_LINK:                                        \
        PDS_LINK_TRACE_VERBOSE_NO_META(fmt);                                   \
        break;                                                                 \
    case sdk_mod_id_t::SDK_MOD_ID_HMON:                                        \
        PDS_HMON_TRACE_VERBOSE_NO_META(fmt);                                   \
        break;                                                                 \
    case sdk_mod_id_t::SDK_MOD_ID_INTR:                                        \
        PDS_INTR_TRACE_VERBOSE_NO_META(fmt);                                   \
        break;                                                                 \
    default:                                                                   \
        PDS_TRACE_VERBOSE_NO_META(fmt);                                        \
        break;                                                                 \
    }                                                                          \
}

// pds trace macros
#define PDS_TRACE_ERR(fmt, ...)                                                \
    if (likely(core::trace_logger()) &&                                        \
        (core::trace_level() >= utils::trace_err)) {                           \
        core::trace_logger()->error("[{}:{}] " fmt, __func__, __LINE__,        \
                                    ##__VA_ARGS__);                            \
        core::trace_logger()->flush();                                         \
    }                                                                          \

#define PDS_TRACE_ERR_NO_META(fmt...)                                          \
    if (likely(core::trace_logger()) &&                                        \
        (core::trace_level() >= utils::trace_err)) {                           \
        core::trace_logger()->error(fmt);                                      \
        core::trace_logger()->flush();                                         \
    }                                                                          \

#define PDS_TRACE_WARN(fmt, ...)                                               \
    if (likely(core::trace_logger()) &&                                        \
        (core::trace_level() >= utils::trace_warn)) {                          \
        core::trace_logger()->warn("[{}:{}] " fmt, __func__, __LINE__,         \
                                   ##__VA_ARGS__);                             \
        core::trace_logger()->flush();                                         \
    }                                                                          \

#define PDS_TRACE_WARN_NO_META(fmt...)                                         \
    if (likely(core::trace_logger()) &&                                        \
        (core::trace_level() >= utils::trace_warn)) {                          \
        core::trace_logger()->warn(fmt);                                       \
        core::trace_logger()->flush();                                         \
    }

#define PDS_TRACE_INFO(fmt, ...)                                               \
    if (likely(core::trace_logger()) &&                                        \
        (core::trace_level() >= utils::trace_info)) {                          \
        core::trace_logger()->info("[{}:{}] " fmt, __func__, __LINE__,         \
                                   ##__VA_ARGS__);                             \
        core::trace_logger()->flush();                                         \
    }                                                                          \

#define PDS_TRACE_INFO_NO_META(fmt...)                                         \
    if (likely(core::trace_logger()) &&                                        \
        (core::trace_level() >= utils::trace_info)) {                          \
        core::trace_logger()->info(fmt);                                       \
        core::trace_logger()->flush();                                         \
    }

#define PDS_TRACE_DEBUG(fmt, ...)                                              \
    if (likely(core::trace_logger()) &&                                        \
        (core::trace_level() >= utils::trace_debug)) {                         \
        core::trace_logger()->debug("[{}:{}] " fmt, __func__, __LINE__,        \
                                    ##__VA_ARGS__);                            \
        core::trace_logger()->flush();                                         \
    }                                                                          \

#define PDS_TRACE_DEBUG_NO_META(fmt...)                                        \
    if (likely(core::trace_logger()) &&                                        \
        (core::trace_level() >= utils::trace_debug)) {                         \
        core::trace_logger()->debug(fmt);                                      \
        core::trace_logger()->flush();                                         \
    }                                                                          \

#define PDS_TRACE_VERBOSE(fmt, ...)                                            \
    if (likely(core::trace_logger())) {                                        \
        core::trace_logger()->trace("[{}:{}] " fmt, __func__, __LINE__,        \
                                    ##__VA_ARGS__);                            \
        core::trace_logger()->flush();                                         \
    }                                                                          \

#define PDS_TRACE_VERBOSE_NO_META(fmt...)                                      \
    if (likely(core::trace_logger())) {                                        \
        core::trace_logger()->trace(fmt);                                      \
        core::trace_logger()->flush();                                         \
    }                                                                          \

#define PDS_ERR_IF(cond, fmt, ...)                                             \
    if (likely(core::trace_logger() &&                                         \
               (core::trace_level() >= utils::trace_err) && (cond))) {         \
        core::trace_logger()->error("[{}:{}] "  fmt,  __func__, __LINE__,      \
                                    ##__VA_ARGS__);                            \
        core::trace_logger()->flush();                                         \
    }                                                                          \

#define PDS_WARN_IF(cond, fmt, ...)                                            \
    if (likely(core::trace_logger() &&                                         \
               (core::trace_level() >= utils::trace_warn) && (cond))) {        \
        core::trace_logger()->warn("[{}:{}] "  fmt, __func__, __LINE__,        \
                                   ##__VA_ARGS__);                             \
        core::trace_logger()->flush();                                         \
    }                                                                          \

#define PDS_INFO_IF(cond, fmt, ...)                                            \
    if (likely(core::trace_logger() &&                                         \
               (core::trace_level() >= utils::trace_info) && (cond))) {        \
        core::trace_logger()->info("[{}:{}] "  fmt, __func__, __LINE__,        \
                                   ##__VA_ARGS__);                             \
        core::trace_logger()->flush();                                         \
    }                                                                          \

#define PDS_DEBUG_IF(cond, fmt, ...)                                           \
    if (likely(core::trace_logger() &&                                         \
               (core::trace_level() >= utils::trace_debug) && (cond))) {       \
        core::trace_logger()->debug("[{}:{}] "  fmt, __func__, __LINE__,       \
                                    ##__VA_ARGS__);                            \
        core::trace_logger()->flush();                                         \
    }                                                                          \

// pds link trace macros
#define PDS_LINK_TRACE_ERR(fmt, ...)                                           \
    if (likely(core::link_trace_logger()) &&                                   \
        (core::trace_level() >= utils::trace_err)) {                           \
        core::link_trace_logger()->error("[{}:{}] " fmt, __func__, __LINE__,   \
                                         ##__VA_ARGS__);                       \
        core::link_trace_logger()->flush();                                    \
    }                                                                          \

#define PDS_LINK_TRACE_ERR_NO_META(fmt...)                                     \
    if (likely(core::link_trace_logger()) &&                                   \
        (core::trace_level() >= utils::trace_err)) {                           \
        core::link_trace_logger()->error(fmt);                                 \
        core::link_trace_logger()->flush();                                    \
    }                                                                          \

#define PDS_LINK_TRACE_WARN(fmt, ...)                                          \
    if (likely(core::link_trace_logger()) &&                                   \
        (core::trace_level() >= utils::trace_warn)) {                          \
        core::link_trace_logger()->warn("[{}:{}] " fmt, __func__, __LINE__,    \
                                         ##__VA_ARGS__);                       \
        core::link_trace_logger()->flush();                                    \
    }                                                                          \

#define PDS_LINK_TRACE_WARN_NO_META(fmt...)                                    \
    if (likely(core::link_trace_logger()) &&                                   \
        (core::trace_level() >= utils::trace_warn)) {                          \
        core::link_trace_logger()->warn(fmt);                                  \
        core::link_trace_logger()->flush();                                    \
    }                                                                          \

#define PDS_LINK_TRACE_INFO(fmt, ...)                                          \
    if (likely(core::link_trace_logger()) &&                                   \
        (core::trace_level() >= utils::trace_info)) {                          \
        core::link_trace_logger()->info("[{}:{}] " fmt, __func__, __LINE__,    \
                                         ##__VA_ARGS__);                       \
        core::link_trace_logger()->flush();                                    \
    }                                                                          \

#define PDS_LINK_TRACE_INFO_NO_META(fmt...)                                    \
    if (likely(core::link_trace_logger()) &&                                   \
        (core::trace_level() >= utils::trace_info)) {                          \
        core::link_trace_logger()->info(fmt);                                  \
        core::link_trace_logger()->flush();                                    \
    }                                                                          \

#define PDS_LINK_TRACE_DEBUG(fmt, ...)                                         \
    if (likely(core::link_trace_logger()) &&                                   \
        (core::trace_level() >= utils::trace_debug)) {                         \
        core::link_trace_logger()->debug("[{}:{}] " fmt, __func__, __LINE__,   \
                                         ##__VA_ARGS__);                       \
        core::link_trace_logger()->flush();                                    \
    }                                                                          \

#define PDS_LINK_TRACE_DEBUG_NO_META(fmt...)                                   \
    if (likely(core::link_trace_logger()) &&                                   \
        (core::trace_level() >= utils::trace_debug)) {                         \
        core::link_trace_logger()->debug(fmt);                                 \
        core::link_trace_logger()->flush();                                    \
    }                                                                          \

#define PDS_LINK_TRACE_VERBOSE(fmt, ...)                                       \
    if (likely(core::link_trace_logger())) {                                   \
        core::link_trace_logger()->trace("[{}:{}] " fmt, __func__, __LINE__,   \
                                    ##__VA_ARGS__);                            \
        core::link_trace_logger()->flush();                                    \
    }                                                                          \

#define PDS_LINK_TRACE_VERBOSE_NO_META(fmt...)                                 \
    if (likely(core::link_trace_logger())) {                                   \
        core::link_trace_logger()->trace(fmt);                                 \
        core::link_trace_logger()->flush();                                    \
    }                                                                          \

// pds hmon trace macros
#define PDS_HMON_TRACE_ERR(fmt, ...)                                           \
    if (likely(core::hmon_trace_logger()) &&                                   \
        (core::trace_level() >= utils::trace_err)) {                           \
        core::hmon_trace_logger()->error("[{}:{}] " fmt, __func__, __LINE__,   \
                                         ##__VA_ARGS__);                       \
        core::hmon_trace_logger()->flush();                                    \
    }                                                                          \

#define PDS_HMON_TRACE_ERR_NO_META(fmt...)                                     \
    if (likely(core::hmon_trace_logger()) &&                                   \
        (core::trace_level() >= utils::trace_err)) {                           \
        core::hmon_trace_logger()->error(fmt);                                 \
        core::hmon_trace_logger()->flush();                                    \
    }                                                                          \

#define PDS_HMON_TRACE_WARN(fmt, ...)                                          \
    if (likely(core::hmon_trace_logger()) &&                                   \
        (core::trace_level() >= utils::trace_warn)) {                          \
        core::hmon_trace_logger()->warn("[{}:{}] " fmt, __func__, __LINE__,    \
                                         ##__VA_ARGS__);                       \
        core::hmon_trace_logger()->flush();                                    \
    }                                                                          \

#define PDS_HMON_TRACE_WARN_NO_META(fmt...)                                    \
    if (likely(core::hmon_trace_logger()) &&                                   \
        (core::trace_level() >= utils::trace_warn)) {                          \
        core::hmon_trace_logger()->warn(fmt);                                  \
        core::hmon_trace_logger()->flush();                                    \
    }                                                                          \

#define PDS_HMON_TRACE_INFO(fmt, ...)                                          \
    if (likely(core::hmon_trace_logger()) &&                                   \
        (core::trace_level() >= utils::trace_info)) {                          \
        core::hmon_trace_logger()->info("[{}:{}] " fmt, __func__, __LINE__,    \
                                         ##__VA_ARGS__);                       \
        core::hmon_trace_logger()->flush();                                    \
    }                                                                          \

#define PDS_HMON_TRACE_INFO_NO_META(fmt...)                                    \
    if (likely(core::hmon_trace_logger()) &&                                   \
        (core::trace_level() >= utils::trace_info)) {                          \
        core::hmon_trace_logger()->info(fmt);                                  \
        core::hmon_trace_logger()->flush();                                    \
    }                                                                          \

#define PDS_HMON_TRACE_DEBUG(fmt, ...)                                         \
    if (likely(core::hmon_trace_logger()) &&                                   \
        (core::trace_level() >= utils::trace_debug)) {                         \
        core::hmon_trace_logger()->debug("[{}:{}] " fmt, __func__, __LINE__,   \
                                         ##__VA_ARGS__);                       \
        core::hmon_trace_logger()->flush();                                    \
    }                                                                          \

#define PDS_HMON_TRACE_DEBUG_NO_META(fmt...)                                   \
    if (likely(core::hmon_trace_logger()) &&                                   \
        (core::trace_level() >= utils::trace_debug)) {                         \
        core::hmon_trace_logger()->debug(fmt);                                 \
        core::hmon_trace_logger()->flush();                                    \
    }                                                                          \

#define PDS_HMON_TRACE_VERBOSE(fmt, ...)                                       \
    if (likely(core::hmon_trace_logger())) {                                   \
        core::hmon_trace_logger()->trace("[{}:{}] " fmt, __func__, __LINE__,   \
                                    ##__VA_ARGS__);                            \
        core::hmon_trace_logger()->flush();                                    \
    }                                                                          \

#define PDS_HMON_TRACE_VERBOSE_NO_META(fmt...)                                 \
    if (likely(core::hmon_trace_logger())) {                                   \
        core::hmon_trace_logger()->trace(fmt);                                 \
        core::hmon_trace_logger()->flush();                                    \
    }                                                                          \

// pds interrupt trace macros
#define PDS_INTR_TRACE_ERR(fmt, ...)                                           \
    if (likely(core::intr_trace_logger()) &&                                   \
        (core::trace_level() >= utils::trace_err)) {                           \
        core::intr_trace_logger()->error("[{}:{}] " fmt, __func__, __LINE__,   \
                                         ##__VA_ARGS__);                       \
        core::intr_trace_logger()->flush();                                    \
    }                                                                          \

#define PDS_INTR_TRACE_ERR_NO_META(fmt...)                                     \
    if (likely(core::intr_trace_logger()) &&                                   \
        (core::trace_level() >= utils::trace_err)) {                           \
        core::intr_trace_logger()->error(fmt);                                 \
        core::intr_trace_logger()->flush();                                    \
    }                                                                          \

#define PDS_INTR_TRACE_WARN(fmt, ...)                                          \
    if (likely(core::intr_trace_logger()) &&                                   \
        (core::trace_level() >= utils::trace_warn)) {                          \
        core::intr_trace_logger()->warn("[{}:{}] " fmt, __func__, __LINE__,    \
                                         ##__VA_ARGS__);                       \
        core::intr_trace_logger()->flush();                                    \
    }                                                                          \

#define PDS_INTR_TRACE_WARN_NO_META(fmt...)                                    \
    if (likely(core::intr_trace_logger()) &&                                   \
        (core::trace_level() >= utils::trace_warn)) {                          \
        core::intr_trace_logger()->warn(fmt);                                  \
        core::intr_trace_logger()->flush();                                    \
    }                                                                          \

#define PDS_INTR_TRACE_INFO(fmt, ...)                                          \
    if (likely(core::intr_trace_logger()) &&                                   \
        (core::trace_level() >= utils::trace_info)) {                          \
        core::intr_trace_logger()->info("[{}:{}] " fmt, __func__, __LINE__,    \
                                         ##__VA_ARGS__);                       \
        core::intr_trace_logger()->flush();                                    \
    }                                                                          \

#define PDS_INTR_TRACE_INFO_NO_META(fmt...)                                    \
    if (likely(core::intr_trace_logger()) &&                                   \
        (core::trace_level() >= utils::trace_info)) {                          \
        core::intr_trace_logger()->info(fmt);                                  \
        core::intr_trace_logger()->flush();                                    \
    }                                                                          \

#define PDS_INTR_TRACE_DEBUG(fmt, ...)                                         \
    if (likely(core::intr_trace_logger()) &&                                   \
        (core::trace_level() >= utils::trace_debug)) {                         \
        core::intr_trace_logger()->debug("[{}:{}] " fmt, __func__, __LINE__,   \
                                         ##__VA_ARGS__);                       \
        core::intr_trace_logger()->flush();                                    \
    }                                                                          \

#define PDS_INTR_TRACE_DEBUG_NO_META(fmt...)                                   \
    if (likely(core::intr_trace_logger()) &&                                   \
        (core::trace_level() >= utils::trace_debug)) {                         \
        core::intr_trace_logger()->debug(fmt);                                 \
        core::intr_trace_logger()->flush();                                    \
    }                                                                          \

#define PDS_INTR_TRACE_VERBOSE(fmt, ...)                                       \
    if (likely(core::intr_trace_logger())) {                                   \
        core::intr_trace_logger()->trace("[{}:{}] " fmt, __func__, __LINE__,   \
                                    ##__VA_ARGS__);                            \
        core::intr_trace_logger()->flush();                                    \
    }                                                                          \

#define PDS_INTR_TRACE_VERBOSE_NO_META(fmt...)                                 \
    if (likely(core::intr_trace_logger())) {                                   \
        core::intr_trace_logger()->trace(fmt);                                 \
        core::intr_trace_logger()->flush();                                    \
    }                                                                          \

#endif    // __PDS_AGENT_TRACE_HPP__
