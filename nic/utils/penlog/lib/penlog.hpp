#ifndef __PENLOG_H__
#define __PENLOG_H__

#include <memory>

#include <spdlog/sinks/file_sinks.h>
#include <spdlog/spdlog.h>

#include "nic/delphi/sdk/delphi_sdk.hpp"

#define PENLOG_LOCATION "/var/log"
#define PENLOG_LOGFILE_SIZE (1024 * 1024)
#define PENLOG_ROTATE_COUNT 5

namespace penlog {
    typedef std::shared_ptr<spdlog::sinks::rotating_file_sink_mt> sinkptr;
    
    class Logger {
    protected:
        std::shared_ptr<spdlog::logger> spd;
        spdlog::level::level_enum level = spdlog::level::info;
    public:
        const std::string name;
    public:
        Logger(const std::string &name);
        void spd_init(sinkptr sink, spdlog::level::level_enum level);
        template<typename... Args>
        void trace(const char *fmt, const Args &... args) {
            if (this->spd != nullptr)
            {
                this->spd->trace(fmt, args...);
            }   
        };

        template<typename... Args>
        void debug(const char *fmt, const Args &... args) {
            if (this->spd != nullptr)
            {
                this->spd->debug(fmt, args...);
            }   
        };

        template<typename... Args>
        void info(const char *fmt, const Args &... args) {
            if (this->spd != nullptr)
            {
                this->spd->info(fmt, args...);
            }   
        };

        template<typename... Args>
        void warn(const char *fmt, const Args &... args) {
            if (this->spd != nullptr)
            {
                this->spd->warn(fmt, args...);
            }   
        };

        template<typename... Args>
        void error(const char *fmt, const Args &... args) {
            if (this->spd != nullptr)
            {
                this->spd->error(fmt, args...);
            }   
        };
        
        template<typename... Args>
        void critical(const char *fmt, const Args &... args) {
            if (this->spd != nullptr)
            {
                this->spd->critical(fmt, args...);
            }   
        };
        
        void set_level(spdlog::level::level_enum level);

    };
    
    typedef std::shared_ptr<Logger> LoggerPtr;

    // Not thread safe
    // *MUST* be called before delphi mount complete
    LoggerPtr logger_init(delphi::SdkPtr delphi, const std::string &name);

    // Not thread safe
    LoggerPtr logger_init_for_lib(const std::string &name);

    LoggerPtr log();
};

#endif
