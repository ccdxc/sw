//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/hal/iris/sysmgr/sysmgr.hpp"
#include "nic/hal/core/heartbeat/heartbeat.hpp"

namespace hal {
namespace sysmgr {

std::pair<delphi::error, std::string>
sysmgr_client::Heartbeat(void) {
    if (hal::hb::is_hal_healthy()) {
        return std::make_pair(delphi::error::OK(), "");
    } else {
        return std::make_pair(delphi::error::New("HAL heartbeat failure"), "");
    }
}

}    // namespace sysmgr
}    // namespace hal
