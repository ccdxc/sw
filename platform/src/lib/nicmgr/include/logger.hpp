// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __LOGGER_HPP__
#define __LOGGER_HPP__

#include "spdlog/spdlog.h"


namespace utils {
namespace logger {
    void init(void);
    std::shared_ptr<spdlog::logger> logger(void);
    std::shared_ptr<spdlog::logger> syslogger(void);
    void nicmgr_api_trace(const char *trace);
}
}

#define NIC_HEADER_TRACE(arg_str) \
    utils::logger::nicmgr_api_trace(arg_str);

// syslog macros
#define NIC_SYSLOG_CRIT(args...) \
do { \
    if (utils::logger::syslogger()) { \
        utils::logger::syslogger()->critical(args); \
    } \
} while (0)

#define NIC_SYSLOG_ERR(args...) \
do { \
    if (utils::logger::syslogger()) { \
        utils::logger::syslogger()->error(args); \
    } \
} while (0)

#define NIC_SYSLOG_WARN(args...) \
do { \
    if (utils::logger::syslogger()) { \
        utils::logger::syslogger()->warn(args); \
    } \
} while (0)

#define NIC_SYSLOG_INFO(args...) \
do { \
    if (utils::logger::syslogger()) { \
        utils::logger::syslogger()->info(args); \
    } \
} while (0)

// log macros
#define NIC_LOG_CRIT(args...) \
do { \
    if (utils::logger::logger()) { \
        utils::logger::logger()->error(args); \
    } \
} while (0)

#define NIC_LOG_ERR(args...) \
do { \
    if (utils::logger::logger()) { \
        utils::logger::logger()->error(args); \
    } \
} while (0)

#define NIC_LOG_WARN(args...) \
do { \
    if (utils::logger::logger()) { \
        utils::logger::logger()->warn(args); \
    } \
} while (0)

#define NIC_LOG_INFO(args...) \
do { \
    if (utils::logger::logger()) { \
        utils::logger::logger()->info(args); \
    } \
} while (0)

#define NIC_LOG_DEBUG(args...) \
do { \
    if (utils::logger::logger()) { \
        utils::logger::logger()->debug(args); \
    } \
} while (0)

#define NIC_LOG_TRACE(args...) \
do { \
    if (utils::logger::logger()) { \
        utils::logger::logger()->trace(args); \
    } \
} while (0)

// function logging macros
#define NIC_FUNC_CRIT(fmt, args...) \
do { \
    if (utils::logger::logger()) { \
        utils::logger::logger()->critical("[{}:{}] " fmt, __FUNCTION__, __LINE__, args); \
    } \
} while (0)

#define NIC_FUNC_ERR(fmt, ...) \
do { \
    if (utils::logger::logger()) { \
        utils::logger::logger()->error("[{}:{}] " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } \
} while (0)

#define NIC_FUNC_WARN(fmt, ...) \
do { \
    if (utils::logger::logger()) { \
        utils::logger::logger()->warn("[{}:{}] " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } \
} while (0)

#define NIC_FUNC_INFO(fmt, ...) \
do { \
    if (utils::logger::logger()) { \
        utils::logger::logger()->info("[{}:{}] " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } \
} while (0)

#define NIC_FUNC_DEBUG(fmt, ...) \
do { \
    if (utils::logger::logger()) { \
        utils::logger::logger()->debug("[{}:{}] " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } \
} while (0)

#define NIC_FUNC_TRACE(fmt, ...) \
do { \
    if (utils::logger::logger()) { \
        utils::logger::logger()->trace("[{}:{}] " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } \
} while (0)

#endif  /* __LOGGERR_HPP__ */
