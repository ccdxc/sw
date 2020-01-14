#ifndef __SDK_OPERD_H__
#define __SDK_OPERD_H__

#include <memory>
#include <stdarg.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

namespace sdk {
namespace operd {

class log {
public:
    virtual uint8_t severity(void) = 0;
    virtual pid_t pid(void) = 0;
    virtual const struct timeval *timestamp(void) = 0;
    virtual uint8_t encoder(void) = 0;
    virtual const char *data(void) = 0;
    virtual size_t data_length(void) = 0;
};
typedef std::shared_ptr<log> log_ptr;

class producer {
public:
    static std::shared_ptr<producer> create(std::string name);
    virtual void write(uint8_t encoder, uint8_t severity, const char *data,
                       size_t data_length) = 0;
    int vprintf(uint8_t severity, const char *fmt, va_list ap);
    int printf(uint8_t severity, const char *fmt, ...)
        __attribute__ ((format (printf, 3, 4)));
};
typedef std::shared_ptr<producer> producer_ptr;

class consumer {
public:
    static std::shared_ptr<consumer> create(std::string name);
    // Read is NOT thread safe
    virtual log_ptr read(void) = 0;
    virtual void reset(void) = 0;
};
typedef std::shared_ptr<consumer> consumer_ptr;


} // namespace operd
} // namespace sdk

#endif
