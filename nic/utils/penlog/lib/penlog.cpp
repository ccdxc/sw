#include <map>
#include <memory>

#include <spdlog/sinks/file_sinks.h>
#include <spdlog/spdlog.h>

#include "main_logger.hpp"
#include "penlog.hpp"

using namespace penlog;

std::shared_ptr<MainLogger> g_logger;
std::map<std::string, LoggerPtr> g_libloggers;

LoggerPtr penlog::logger_init(const std::string &name)
{
    if (g_logger == nullptr)
    {
        g_logger = std::make_shared<MainLogger>(name);
        for (auto p: g_libloggers)
        {
            g_logger->register_lib_logger(p.second);
        }
    }

    return g_logger;
}

LoggerPtr penlog::logger_init_for_lib(const std::string &name)
{
    auto logger = std::make_shared<Logger>(name);
    if (g_logger == nullptr)
    {
        g_libloggers.insert(std::pair<std::string, LoggerPtr>(
                logger->name, logger));
    }
    else
    {
        g_logger->register_lib_logger(logger);
    }

    return logger;
}

LoggerPtr penlog::log()
{
    return g_logger;
}

Logger::Logger(const std::string &name) : name(name)
{
    this->spd = nullptr;
    this->level = spdlog::level::info;
}

void Logger::spd_init(sinkptr sink, spdlog::level::level_enum level)
{
    this->spd = std::make_shared<spdlog::logger>(this->name, sink);
    this->spd->set_pattern("%L [%Y-%m-%d %H:%M:%S.%f] %P/%n: %v");
    this->spd->flush_on(spdlog::level::trace);
    this->set_level(level);
}

void Logger::set_level(spdlog::level::level_enum level)
{
    this->level = level;
    this->spd->set_level(level);
}

std::shared_ptr<spdlog::logger> Logger::get_spd()
{
    return this->spd;
}
