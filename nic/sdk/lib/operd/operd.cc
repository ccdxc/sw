#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>

#include "operd.hpp"
#include "region.hpp"

namespace sdk {
namespace operd {

const int MAX_PRINTF_SIZE = 1024;

int
producer::printf(uint8_t severity, const char *fmt, ...) {
    va_list ap;
    int n;

    va_start(ap, fmt);
    n = this->vprintf(severity, fmt, ap);
    va_end(ap);

    return n;
}

int
producer::vprintf(uint8_t severity, const char *fmt, va_list ap) {
    char buffer[MAX_PRINTF_SIZE];
    int n;

    n = vsnprintf(buffer, sizeof(buffer), fmt, ap);

    // n return the number of characters *not* including the final null
    // character, hence the +1
    this->write(0, severity, buffer, n + 1);

    return n;
}

producer_ptr
producer::create(std::string name) {
    return std::make_shared<region>(name);
}

consumer_ptr
consumer::create(std::string name) {
    return std::make_shared<region>(name);
}

} // namespace operd
} // namespace sdk
