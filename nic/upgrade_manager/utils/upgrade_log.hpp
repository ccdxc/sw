// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef _UPGRADE_LOG_H_
#define _UPGRADE_LOG_H_

#include <stdio.h>
#include <iostream>
#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>
#include "nic/utils/trace/trace.hpp"
#include "nic/sdk/lib/logger/logger.hpp"

namespace upgrade {
extern ::utils::log *upgrade_obfl_trace_logger;

const auto OBFL_LOG_FILENAME = "/obfl/upgrade.log";
const auto OBFL_LOG_MAX_FILESIZE = 1*1024*1024;
const auto LOG_MAX_FILES = 5;

typedef std::shared_ptr<spdlog::logger> Logger;

// GetLogger returns the current logger instance
Logger GetLogger();

#define DEBUG_ENABLE
#ifdef DEBUG_ENABLE
#define UPG_LOG_DEBUG(args...) upgrade::GetLogger()->debug(args)
#else
#define UPG_LOG_DEBUG(args...) while (0) { upgrade::GetLogger()->debug(args); }
#endif // DEBUG_ENABLE

#define UPG_LOG_INFO(args...) upgrade::GetLogger()->info(args)
#define UPG_LOG_WARN(args...) upgrade::GetLogger()->warn(args)
#define UPG_LOG_ERROR(args...) upgrade::GetLogger()->error(args)
#define UPG_LOG_FATAL(args...) { upgrade::GetLogger()->error(args); assert(0); }


void initializeLogger();

static inline std::shared_ptr<logger>
GetUpgradeObflLogger (void)
{
    if (upgrade_obfl_trace_logger) {
        return upgrade_obfl_trace_logger->logger();
    }
    UPG_LOG_INFO("upgrade_obfl_trace_logger is null");
    return NULL;
}

#define UPG_OBFL_TRACE(args...) TRACE_INFO(GetUpgradeObflLogger(), args) \
                                TRACE_FLUSH(GetUpgradeObflLogger());

} // namespace upgrade

 #endif
