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
#define UPGRADE_LOG_NAME "upgradelog"

#define UPG_TRACE_ERR(fmt, ...)                                           \
{                                                                         \
    g_upg_log->err("[%s:%u] " fmt, __FNAME__, __LINE__, ##__VA_ARGS__);   \
}

#define UPG_TRACE_WARN(fmt, ...)                                          \
{                                                                         \
    g_upg_log->warn("[%s:%u] " fmt, __FNAME__, __LINE__, ##__VA_ARGS__);  \
}

#define UPG_TRACE_INFO(fmt, ...)                                          \
{                                                                         \
    g_upg_log->info("[%s:%u] " fmt, __FNAME__, __LINE__, ##__VA_ARGS__);  \
}

#define UPG_TRACE_DEBUG(fmt, ...)                                         \
{                                                                         \
    g_upg_log->debug("[%s:%u] " fmt, __FNAME__, __LINE__, ##__VA_ARGS__); \
}

#define UPG_TRACE(fmt, ...)                                               \
{                                                                         \
    g_upg_log->trace("[%s:%u] " fmt, __FNAME__, __LINE__, ##__VA_ARGS__); \
}

#define UPG_TRACE_VERBOSE(fmt, ...)                                       \
{                                                                         \
}

}    // namespace upg
}    // namespace sdk

using sdk::upg::g_upg_log;

#endif     // __UPGRADE_CORE_LOGGER_HPP__
