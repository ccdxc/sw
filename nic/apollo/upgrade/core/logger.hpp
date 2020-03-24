//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __UPGRADE_LOGGER_HPP__
#define __UPGRADE_LOGGER_HPP__

#include <sys/stat.h>
#include <nic/sdk/include/sdk/base.hpp>
#include "nic/sdk/lib/operd/logger.hpp"

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

#define UPG_TRACE_VERBOSE(str)                                            \
{                                                                         \
    std::string s_ = str;                                                 \
    UPG_TRACE("%s", s_.c_str());                                          \
}

}    // namespace upg

using upg::g_upg_log;

#endif     // __UPGRADE_LOGGER_HPP__
