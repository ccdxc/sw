#ifndef __OPERD_LOGGER_H__
#define __OPERD_LOGGER_H__

#include <memory>
#include <stdarg.h>
#include <string>

#include "operd.hpp"

namespace sdk {
namespace operd {

enum level {
    NONE    = 0,
    ERROR   = 1,
    WARNING = 2,
    INFO    = 3,
    DEBUG   = 4,
    TRACE   = 5,
};

class logger {
public:
    static std::shared_ptr<logger> create(std::string name);
    logger(std::string name);
    void err(const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));
    void warn(const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));
    void info(const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));
    void debug(const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));
    void trace(const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));
private:
    producer_ptr producer;
};
typedef std::shared_ptr<logger> logger_ptr;

} // namespace operd
} // namespace sdk

#endif
