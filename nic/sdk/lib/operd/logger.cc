#include <stdarg.h>
#include <string>

#include "logger.hpp"
#include "operd.hpp"

namespace sdk {
namespace operd {

logger_ptr
logger::create(std::string name) {
    return std::make_shared<logger>(name);
}

logger::logger(std::string name) {
    this->producer = producer::create(name);
}

void
logger::err(const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    this->producer->vprintf(ERROR, fmt, ap);
    va_end(ap);
}

void
logger::warn(const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    this->producer->vprintf(WARNING, fmt, ap);
    va_end(ap);
}

void
logger::info(const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    this->producer->vprintf(INFO, fmt, ap);
    va_end(ap);
}

void
logger::debug(const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    this->producer->vprintf(DEBUG, fmt, ap);
    va_end(ap);
}

void
logger::trace(const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    this->producer->vprintf(TRACE, fmt, ap);
    va_end(ap);
}


} // namespace operd
} // namespace sdk
