// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

#ifndef _COREMGR_H_
#define _COREMGR_H_

#include <spdlog/spdlog.h>
#include "nic/sdk/lib/logger/logger.hpp"

#define LOG_DIRECTORY "/var/log/pensando"

typedef std::shared_ptr<spdlog::logger> Logger;
extern Logger g_coremgr_logger;

static inline Logger
coremgr_logger (void)
{
    return g_coremgr_logger;
}

#define COREMGR_TRACE_ERR_NO_META(fmt...)                                  \
    if (coremgr_logger()) {                                                \
        coremgr_logger()->error(fmt);                                      \
        coremgr_logger()->flush();                                         \
    }                                                                      \

#define COREMGR_TRACE_WARN_NO_META(fmt...)                                 \
    if (coremgr_logger()) {                                                \
        coremgr_logger()->warn(fmt);                                       \
        coremgr_logger()->flush();                                         \
    }                                                                      \

#define COREMGR_TRACE_DEBUG_NO_META(fmt...)                                \
    if (coremgr_logger()) {                                                \
        coremgr_logger()->debug(fmt);                                      \
        coremgr_logger()->flush();                                         \
    }                                                                      \

#define COREMGR_TRACE_INFO_NO_META(fmt...)                                 \
    if (coremgr_logger()) {                                                \
        coremgr_logger()->info(fmt);                                       \
        coremgr_logger()->flush();                                         \
    }                                                                      \

#define COREMGR_TRACE_VERBOSE_NO_META(fmt...)                              \
    if (coremgr_logger()) {                                                \
        coremgr_logger()->trace(fmt);                                      \
        coremgr_logger()->flush();                                         \
    }

#endif
