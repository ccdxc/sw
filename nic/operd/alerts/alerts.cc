// {C} Copyright 2020 Pensando Systems Inc. All rights reserved

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <string>

#include "alerts.hpp"
#include "nic/sdk/lib/operd/decoder.h"
#include "nic/sdk/lib/operd/operd.hpp"

namespace operd {
namespace alerts {

const int MAX_PRINTF_SIZE = 1024;

alert_recorder_ptr alert_recorder::instance_ = nullptr;

alert_recorder_ptr
alert_recorder::get(void) {
    if (instance_ == nullptr) {
        instance_ = std::make_shared<alert_recorder>();
        instance_->region_ = std::make_shared<sdk::operd::region>("alerts");
    }

    return instance_;
}

void
alert_recorder::alert(operd_alerts_t alert, const char *fmt, ...) {
    va_list ap;
    char buffer[MAX_PRINTF_SIZE + sizeof(int)];
    char *print_buffer = &buffer[sizeof(int)];
    int n;

    assert(sizeof(alert) == sizeof(int));
    
    va_start(ap, fmt);
    n = vsnprintf(print_buffer, MAX_PRINTF_SIZE, fmt, ap);
    memcpy(buffer, &alert, sizeof(int));
    va_end(ap);

    this->region_->write(OPERD_DECODER_ALERTS, 1, buffer, n + 1 + sizeof(int));
}

} // namespace alerts
} // namespace operd
