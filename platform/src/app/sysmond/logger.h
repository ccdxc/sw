// {C} Copyright 201 Pensando Systems Inc. All rights reserved.

#ifndef __SYSMOND_LOGGER_H__
#define __SYSMOND_LOGGER_H__

#include "nic/sdk/lib/logger/logger.hpp"
#include "nic/utils/trace/trace.hpp"

const auto LOG_FILENAME = "/var/log/pensando/sysmond.log";
const auto ASICERR_LOG_FILENAME = "/var/log/pensando/asicerrord.log";
const auto LOG_MAX_FILESIZE = 1*1024*1024;
const auto LOG_MAX_FILES = 5;
const auto OBFL_LOG_FILENAME = "/obfl/sysmond_err.log";
const auto ASICERR_OBFL_LOG_FILENAME = "/obfl/asicerrord_err.log";
const auto ASICERR_OBFL_LOG_ONETIME_FILENAME = "/obfl/asicerrord_onetime_err.log";
const auto OBFL_LOG_MAX_FILESIZE = 1*1024*1024;

void initializeLogger(void);
int sysmond_logger(sdk_trace_level_e tracel_level, const char *format, ...);
int sysmond_obfl_logger(sdk_trace_level_e tracel_level, const char *format, ...);
void sysmond_flush_logger(void);
std::shared_ptr<spdlog::logger> GetLogger(void);
std::shared_ptr<spdlog::logger> GetObflLogger(void);
std::shared_ptr<spdlog::logger> GetAsicErrLogger(void);
std::shared_ptr<spdlog::logger> GetAsicErrObflLogger(void);
std::shared_ptr<spdlog::logger> GetAsicErrObflOnetimeLogger(void);

#endif    // __SYSMOND_LOGGER_H__
