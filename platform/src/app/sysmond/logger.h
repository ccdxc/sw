// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef _LOGGER_H_
#define _LOGGER_H_

#include "nic/utils/trace/trace.hpp"

const auto LOG_FILENAME = "/var/log/pensando/sysmond.log";
const auto LOG_MAX_FILESIZE = 2*1024*1024;
const auto LOG_MAX_FILES = 5;
extern ::utils::log *g_trace_logger;

// wrapper APIs to get logger
static inline std::shared_ptr<logger>
GetLogger (void)
{
    if (g_trace_logger) {
        return g_trace_logger->logger();
    }
    return NULL;
}

#endif
