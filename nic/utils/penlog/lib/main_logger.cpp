#include <memory>

#include <assert.h>

#include <spdlog/spdlog.h>

#include "main_logger.hpp"
#include "penlog.hpp"

using namespace penlog;

MainLogger::MainLogger(const std::string &name) : Logger(name)
{
    const char *env_location = getenv("PENLOG_LOCATION");
    std::string location;
    if (env_location)
    {
        location = std::string(env_location);
    }
    else
    {
        location = std::string(PENLOG_LOCATION);
    }
    
    this->sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        location + "/" + name + ".log", PENLOG_LOGFILE_SIZE,
        PENLOG_ROTATE_COUNT);

    this->spd_init(this->sink, this->level);
}

void MainLogger::update_level(Level level)
{
    switch (level)
    {
    case Critical:
        this->level = spdlog::level::critical;
        break;
    case Error:
        this->level = spdlog::level::err;
        break;
    case Warn:
        this->level = spdlog::level::warn;
        break;
    case Info:
        this->level = spdlog::level::info;
        break;
    case Debug:
        this->level = spdlog::level::debug;
        break;
    case Trace:
        this->level = spdlog::level::trace;
        break;
    default:
        assert(false);
    }

    this->set_level(this->level);

    for (auto liblogger: libloggers)
    {
        liblogger.second->set_level(this->level);
    }    
}

void MainLogger::register_lib_logger(LoggerPtr liblogger)
{
    liblogger->spd_init(this->sink, this->level);
    this->libloggers.insert(std::pair<std::string, LoggerPtr>(
            liblogger->name, liblogger));
}
