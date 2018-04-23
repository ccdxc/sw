// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef _DELPHI_UTILS_LOG_H_
#define _DELPHI_UTILS_LOG_H_

#include <stdio.h>
#include <iostream>
#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>

namespace delphi {
typedef std::shared_ptr<spdlog::logger> Logger;

// GetLogger returns the current logger instance
Logger GetLogger();

#ifdef DEBUG_ENABLE
#define LogDebug(args...) delphi::GetLogger()->debug(args)
#else
#define LogDebug(args...) while (0) { delphi::GetLogger()->debug(args); }
#endif // DEBUG_ENABLE

#define LogInfo(args...) delphi::GetLogger()->info(args)
#define LogWarn(args...) delphi::GetLogger()->warn(args)
#define LogError(args...) delphi::GetLogger()->error(args)
#define LogFatal(args...) { delphi::GetLogger()->error(args); assert(0); }

} // namespace delphi

#endif // _DELPHI_UTILS_LOG_H_
