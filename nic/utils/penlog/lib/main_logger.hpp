#ifndef __MAIN_LOGGER_H__
#define __MAIN_LOGGER_H__

#include <memory>
#include <map>

#include "penlog.hpp"

namespace penlog {
    class MainLogger: public Logger,
                      public std::enable_shared_from_this<MainLogger> {
    private:
        sinkptr sink;
        std::map<std::string, LoggerPtr> libloggers;
    private:
        void update_level(Level level);
    public:
        MainLogger(const std::string &name);
        void register_lib_logger(LoggerPtr liblogger);
    };
};

#endif
