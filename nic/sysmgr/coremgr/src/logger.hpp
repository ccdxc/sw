// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <spdlog/spdlog.h>

#define LOG_DIRECTORY "/var/log/pensando"

typedef std::shared_ptr<spdlog::logger> Logger;

// GetLogger returns the current logger instance
Logger GetLogger();

#define DEBUG_ENABLE
#ifdef DEBUG_ENABLE
#define DEBUG(args...) GetLogger()->debug(args)
#else
#define DEBUG(args...) while (0) { GetLogger()->debug(args); }
#endif // DEBUG_ENABLE

#define INFO(args...) GetLogger()->info(args)
#define WARN(args...) GetLogger()->warn(args)
#define ERR(args...) GetLogger()->error(args)
#define FATAL(args...) { GetLogger()->error(args); assert(0); }

#endif
