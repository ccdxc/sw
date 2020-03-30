// {C} Copyright 2020 Pensando Systems Inc. All rights reserved

#ifndef __OPERD_ALERTS_HPP__
#define __OPERD_ALERTS_HPP__

#include <memory>
#include <stdarg.h>
#include <string>

#include "alert_defs.h"
#include "nic/sdk/lib/operd/operd.hpp"
#include "nic/sdk/lib/operd/region.hpp"

namespace operd {
namespace alerts {

class alert_recorder {
public:
    static std::shared_ptr<alert_recorder> get(void);
    void alert(operd_alerts_t alert, const char *fmt, ...)
        __attribute__ ((format (printf, 3, 4)));
private:
    static std::shared_ptr<alert_recorder> instance_;
    sdk::operd::region_ptr region_;
};
typedef std::shared_ptr<alert_recorder> alert_recorder_ptr;

} // namespace alerts
} // namespace operd

#endif
