#ifndef __NULL_LOGGER_H__
#define __NULL_LOGGER_H__

#include "penlog.hpp"

namespace penlog {
    class NullLogger : public Logger {
    public:
        NullLogger(): Logger("") {};
        void spd_init(sinkptr sink) {};
        template<typename... Args>
        void trace(const char *fmt, const Args &... args) {
        };

        template<typename... Args>
        void debug(const char *fmt, const Args &... args) {
        };

        template<typename... Args>
        void info(const char *fmt, const Args &... args) {
        };

        template<typename... Args>
        void warn(const char *fmt, const Args &... args) {
        };

        template<typename... Args>
        void error(const char *fmt, const Args &... args) {
        };
    };
};

#endif
