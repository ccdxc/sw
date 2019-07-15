// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __OFFL_LOGGER_HPP__
#define __OFFL_LOGGER_HPP__

#include "spdlog/spdlog.h"


namespace offl {
namespace logger {
    extern spdlog::level::level_enum level;
    void init(bool log_to_console=false,
              const char *requested_level=nullptr);
    std::shared_ptr<spdlog::logger> logger(void);
    std::shared_ptr<spdlog::logger> syslogger(void);
    void offl_api_trace(const char *trace);
    static inline bool is_level_debug(void) { return level == spdlog::level::debug; }
}
}

#define OFFL_IS_LOG_LEVEL_DEBUG()  \
    offl::logger::is_level_debug()

#define OFFL_HEADER_TRACE(arg_str) \
    offl::logger::offl_api_trace(arg_str);

// syslog macros
#define OFFL_SYSLOG_CRIT(args...) \
do { \
    if (offl::logger::syslogger()) { \
        offl::logger::syslogger()->critical(args); \
    } \
} while (0)

#define OFFL_SYSLOG_ERR(args...) \
do { \
    if (offl::logger::syslogger()) { \
        offl::logger::syslogger()->error(args); \
    } \
} while (0)

#define OFFL_SYSLOG_WARN(args...) \
do { \
    if (offl::logger::syslogger()) { \
        offl::logger::syslogger()->warn(args); \
    } \
} while (0)

#define OFFL_SYSLOG_INFO(args...) \
do { \
    if (offl::logger::syslogger()) { \
        offl::logger::syslogger()->info(args); \
    } \
} while (0)

// log macros
#define OFFL_LOG_CRIT(args...) \
do { \
    if (offl::logger::logger()) { \
        offl::logger::logger()->error(args); \
    } \
} while (0)

#define OFFL_LOG_ERR(args...) \
do { \
    if (offl::logger::logger()) { \
        offl::logger::logger()->error(args); \
    } \
} while (0)

#define OFFL_LOG_WARN(args...) \
do { \
    if (offl::logger::logger()) { \
        offl::logger::logger()->warn(args); \
    } \
} while (0)

#define OFFL_LOG_INFO(args...) \
do { \
    if (offl::logger::logger()) { \
        offl::logger::logger()->info(args); \
        offl::logger::logger()->flush(); \
    } \
} while (0)

#define OFFL_LOG_DEBUG(args...) \
do { \
    if (offl::logger::logger()) { \
        offl::logger::logger()->debug(args); \
        offl::logger::logger()->flush(); \
    } \
} while (0)

#define OFFL_LOG_TRACE(args...) \
do { \
    if (offl::logger::logger()) { \
        offl::logger::logger()->trace(args); \
    } \
} while (0)

// function logging macros
#define OFFL_FUNC_CRIT(fmt, args...) \
do { \
    if (offl::logger::logger()) { \
        offl::logger::logger()->critical("[{}:{}] " fmt, __FUNCTION__, __LINE__, args); \
    } \
} while (0)

#define OFFL_FUNC_ERR(fmt, ...) \
do { \
    if (offl::logger::logger()) { \
        offl::logger::logger()->error("[{}:{}] " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } \
} while (0)

#define OFFL_FUNC_WARN(fmt, ...) \
do { \
    if (offl::logger::logger()) { \
        offl::logger::logger()->warn("[{}:{}] " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } \
} while (0)

#define OFFL_FUNC_INFO(fmt, ...) \
do { \
    if (offl::logger::logger()) { \
        offl::logger::logger()->info("[{}:{}] " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } \
} while (0)

#define OFFL_FUNC_DEBUG(fmt, ...) \
do { \
    if (offl::logger::logger()) { \
        offl::logger::logger()->debug("[{}:{}] " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } \
} while (0)

#define OFFL_FUNC_TRACE(fmt, ...) \
do { \
    if (offl::logger::logger()) { \
        offl::logger::logger()->trace("[{}:{}] " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } \
} while (0)

#endif  /* __OFFL_LOGGER_HPP__ */
