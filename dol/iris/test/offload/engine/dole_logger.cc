// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#include <cstdlib>
#include <vector>

#include "dole_logger.hpp"

namespace dole {
namespace logger {

const auto LOG_FILENAME = "./dole.log";
const auto LOG_MAX_FILESIZE = 2*1024*1024; // 2 MiB
const auto LOG_MAX_FILES = 5;
const auto LOG_OVERFLOW_POLICY = spdlog::async_overflow_policy::discard_log_msg;
const auto LOG_PATTERN = "%L [%Y-%m-%d %H:%M:%S.%e%z] %v";

static std::shared_ptr<spdlog::logger> _logger = NULL;
static std::shared_ptr<spdlog::logger> _syslogger = NULL;

spdlog::level::level_enum level = spdlog::level::debug;

void
init(bool log_to_console,
     const char *requested_level)
{
    std::string logfile;
    std::string level_str;
    char        *logdir;
    struct stat st = { 0 };

    if (std::getenv("DOLE_LOG_LEVEL") != NULL) {
        level_str.assign(std::getenv("DOLE_LOG_LEVEL"));
    } else if (requested_level) {
        level_str.assign(requested_level);
    }
    if (level_str == "trace") {
        level = spdlog::level::trace;
    } else if (level_str == "debug") {
        level = spdlog::level::debug;
    } else if (level_str == "info") {
        level = spdlog::level::info;
    } else if (level_str == "warn") {
        level = spdlog::level::warn;
    } else if (level_str == "error") {
        level = spdlog::level::err;
    } else if (level_str == "critical") {
        level = spdlog::level::critical;
    } else if (level_str == "off") {
        level = spdlog::level::off;
    } else {
        // nop
    }

    logdir = std::getenv("LOG_DIR");
    if (!logdir) {
        // log in the current dir
        logfile = LOG_FILENAME;
    } else {
        // check if this log dir exists
        if (stat(logdir, &st) == -1) {
            // doesn't exist, try to create
            if (mkdir(logdir, 0755) < 0) {
                fprintf(stderr,
                        "Log directory %s/ doesn't exist, failed to create one\n",
                        logdir);
                return;
            }
        } else {
            // log dir exists, check if we have write permissions
            if (access(logdir, W_OK) < 0) {
                // don't have permissions to create this directory
                fprintf(stderr,
                        "No permissions to create log file in %s\n",
                        logdir);
                return;
            }
        }
        logfile = std::string(logdir) + "/" + LOG_FILENAME;
    }

    spdlog::set_level(level);
    spdlog::set_pattern(LOG_PATTERN);

    std::vector<spdlog::sink_ptr> sinks;
    if (log_to_console) {
        sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_st>());
    }
    sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logfile,
        LOG_MAX_FILESIZE, LOG_MAX_FILES));

    _logger = std::make_shared<spdlog::logger>("dole", begin(sinks), end(sinks));
    assert(_logger != NULL);
    _logger->set_level(level);
    // messages at this level or higher should be flushed immediately
    _logger->flush_on(spdlog::level::err);
    _logger->info("Logger initialized... Log level: {}, {}",
                  level, spdlog::level::level_names[level]);

    _syslogger = spdlog::syslog_logger("dole-syslog", "syslog", LOG_PID);
    assert(_syslogger != NULL);
    _syslogger->info("Syslogger initialized!");
}

std::shared_ptr<spdlog::logger>
logger(void)
{
    return _logger;
}

std::shared_ptr<spdlog::logger>
syslogger(void)
{
    return _syslogger;
}

#define NUM_DASHES 20
void
dole_api_trace (const char *trace)
{
    fmt::MemoryWriter   buf;

    if (!trace) return;

    for (int i = 0; i < NUM_DASHES; i++) {
        buf.write("{}", "-");
    }
    buf.write(" {} ", trace);
    for (int i = 0; i < NUM_DASHES; i++) {
        buf.write("{}", "-");
    }
    DOLE_LOG_DEBUG("{}", buf.c_str());
}

}   // namespace logger
}   // namespace dole
