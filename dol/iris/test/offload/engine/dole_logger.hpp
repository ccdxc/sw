// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __DOLE_LOGGER_HPP__
#define __DOLE_LOGGER_HPP__

#include "spdlog/spdlog.h"


namespace dole {
namespace logger {
    extern spdlog::level::level_enum level;
    void init(bool log_to_console=false,
              const char *requested_level=nullptr);
    std::shared_ptr<spdlog::logger> logger(void);
    std::shared_ptr<spdlog::logger> syslogger(void);
    void dole_api_trace(const char *trace);
    static inline bool is_level_debug(void) { return level == spdlog::level::debug; }
}
}

#define DOLE_IS_LOG_LEVEL_DEBUG()  \
    dole::logger::is_level_debug()

#define DOLE_HEADER_TRACE(arg_str) \
    dole::logger::dole_api_trace(arg_str);

// syslog macros
#define DOLE_SYSLOG_CRIT(args...) \
do { \
    if (dole::logger::syslogger()) { \
        dole::logger::syslogger()->critical(args); \
    } \
} while (0)

#define DOLE_SYSLOG_ERR(args...) \
do { \
    if (dole::logger::syslogger()) { \
        dole::logger::syslogger()->error(args); \
    } \
} while (0)

#define DOLE_SYSLOG_WARN(args...) \
do { \
    if (dole::logger::syslogger()) { \
        dole::logger::syslogger()->warn(args); \
    } \
} while (0)

#define DOLE_SYSLOG_INFO(args...) \
do { \
    if (dole::logger::syslogger()) { \
        dole::logger::syslogger()->info(args); \
    } \
} while (0)

// log macros
#define DOLE_LOG_CRIT(args...) \
do { \
    if (dole::logger::logger()) { \
        dole::logger::logger()->error(args); \
    } \
} while (0)

#define DOLE_LOG_ERR(args...) \
do { \
    if (dole::logger::logger()) { \
        dole::logger::logger()->error(args); \
    } \
} while (0)

#define DOLE_LOG_WARN(args...) \
do { \
    if (dole::logger::logger()) { \
        dole::logger::logger()->warn(args); \
    } \
} while (0)

#define DOLE_LOG_INFO(args...) \
do { \
    if (dole::logger::logger()) { \
        dole::logger::logger()->info(args); \
        dole::logger::logger()->flush(); \
    } \
} while (0)

#define DOLE_LOG_DEBUG(args...) \
do { \
    if (dole::logger::logger()) { \
        dole::logger::logger()->debug(args); \
        dole::logger::logger()->flush(); \
    } \
} while (0)

#define DOLE_LOG_TRACE(args...) \
do { \
    if (dole::logger::logger()) { \
        dole::logger::logger()->trace(args); \
    } \
} while (0)

// function logging macros
#define DOLE_FUNC_CRIT(fmt, args...) \
do { \
    if (dole::logger::logger()) { \
        dole::logger::logger()->critical("[{}:{}] " fmt, __FUNCTION__, __LINE__, args); \
    } \
} while (0)

#define DOLE_FUNC_ERR(fmt, ...) \
do { \
    if (dole::logger::logger()) { \
        dole::logger::logger()->error("[{}:{}] " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } \
} while (0)

#define DOLE_FUNC_WARN(fmt, ...) \
do { \
    if (dole::logger::logger()) { \
        dole::logger::logger()->warn("[{}:{}] " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } \
} while (0)

#define DOLE_FUNC_INFO(fmt, ...) \
do { \
    if (dole::logger::logger()) { \
        dole::logger::logger()->info("[{}:{}] " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } \
} while (0)

#define DOLE_FUNC_DEBUG(fmt, ...) \
do { \
    if (dole::logger::logger()) { \
        dole::logger::logger()->debug("[{}:{}] " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } \
} while (0)

#define DOLE_FUNC_TRACE(fmt, ...) \
do { \
    if (dole::logger::logger()) { \
        dole::logger::logger()->trace("[{}:{}] " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } \
} while (0)

#endif  /* __DOLE_LOGGER_HPP__ */
