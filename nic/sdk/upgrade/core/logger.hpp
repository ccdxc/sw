//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __UPGRADE_CORE_LOGGER_HPP__
#define __UPGRADE_CORE_LOGGER_HPP__

#include <sys/stat.h>
#include "include/sdk/base.hpp"
#include "lib/operd/logger.hpp"

namespace sdk {
namespace upg {

// operd logging
extern sdk::operd::logger_ptr g_upg_log;
extern const char *g_upg_log_pfx;

#define UPGRADE_LOG_NAME "upgradelog"

// enable this after fixing the operd dump for hitless upgrade
#define UPG_TRACE_ERR(fmt, ...)                                                \
{                                                                              \
    if (g_upg_log_pfx) {                                                       \
        g_upg_log->err("[%s:%u:%s] " fmt, __FNAME__, __LINE__, g_upg_log_pfx,  \
                       ##__VA_ARGS__);                                         \
    } else {                                                                   \
        g_upg_log->err("[%s:%u] " fmt, __FNAME__, __LINE__, ##__VA_ARGS__);    \
    }                                                                          \
}

#define UPG_TRACE_WARN(fmt, ...)                                               \
{                                                                              \
    if (g_upg_log_pfx) {                                                       \
        g_upg_log->warn("[%s:%u:%s] " fmt, __FNAME__, __LINE__, g_upg_log_pfx, \
                       ##__VA_ARGS__);                                         \
    } else {                                                                   \
        g_upg_log->warn("[%s:%u] " fmt, __FNAME__, __LINE__, ##__VA_ARGS__);   \
    }                                                                          \
}

#define UPG_TRACE_INFO(fmt, ...)                                               \
{                                                                              \
    if (g_upg_log_pfx) {                                                       \
        g_upg_log->info("[%s:%u:%s] " fmt, __FNAME__, __LINE__, g_upg_log_pfx, \
                       ##__VA_ARGS__);                                         \
    } else {                                                                   \
        g_upg_log->info("[%s:%u] " fmt, __FNAME__, __LINE__, ##__VA_ARGS__);   \
    }                                                                          \
}

#define UPG_TRACE_DEBUG(fmt, ...)                                              \
{                                                                              \
    if (g_upg_log_pfx) {                                                       \
        g_upg_log->debug("[%s:%u:%s] " fmt, __FNAME__, __LINE__, g_upg_log_pfx,\
                       ##__VA_ARGS__);                                         \
    } else {                                                                   \
        g_upg_log->debug("[%s:%u] " fmt, __FNAME__, __LINE__, ##__VA_ARGS__);  \
    }                                                                          \
}

#define UPG_TRACE(fmt, ...)                                                    \
{                                                                              \
    if (g_upg_log_pfx) {                                                       \
        g_upg_log->trace("[%s:%u:%s] " fmt, __FNAME__, __LINE__, g_upg_log_pfx,\
                       ##__VA_ARGS__);                                         \
    } else {                                                                   \
        g_upg_log->trace("[%s:%u] " fmt, __FNAME__, __LINE__, ##__VA_ARGS__);  \
    }                                                                          \
}

#define UPG_TRACE_VERBOSE(fmt, ...)                                       \
{                                                                         \
}

}    // namespace upg
}    // namespace sdk

using sdk::upg::g_upg_log;
using sdk::upg::g_upg_log_pfx;

#endif     // __UPGRADE_CORE_LOGGER_HPP__
