#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "entry.hpp"

namespace sdk {
namespace operd {

pid_t
entry::pid(void) {
    return this->pid_;
}

const struct timeval *
entry::timestamp(void) {
    return &this->timestamp_;
}

uint8_t
entry::encoder(void) {
    return this->encoder_;
}

const char *
entry::data(void) {
    return this->data_;
}

size_t
entry::data_length(void) {
    return this->data_length_;
}

uint8_t
entry::severity(void) {
    return this->severity_;
}

entry::entry(size_t size) {
    this->data_ = (char *)malloc(size);
    this->data_length_ = size;
}

entry::~entry() {
    if (this->data_ != NULL)
        free(this->data_);
}

} // namespace operd
} // namespace sdk
