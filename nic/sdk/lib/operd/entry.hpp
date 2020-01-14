#ifndef __OPERD_ENTRY_H__
#define __OPERD_ENTRY_H__

#include <assert.h>
#include <map>
#include <memory>
#include <stdint.h>
#include <string>
#include <sys/time.h>

#include "operd.hpp"

namespace sdk {
namespace operd {

class entry : public log {
public:
    pid_t pid(void) override;
    const struct timeval *timestamp(void) override;
    uint8_t encoder(void) override;
    const char *data(void) override;
    size_t data_length(void) override;
    uint8_t severity(void) override;
public:
    entry(size_t data_size);
    ~entry();
    uint8_t severity_;
    pid_t pid_;
    struct timeval timestamp_;
    uint8_t encoder_;
    char *data_;
    size_t data_length_;
};
typedef std::shared_ptr<entry> entry_ptr;

} // namespace operd
} // namespace sdk

#endif // __OPERD_ENTRY_H__
