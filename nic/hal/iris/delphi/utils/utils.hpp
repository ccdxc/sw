// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __DELPHIC_UTILS_HPP__
#define __DELPHIC_UTILS_HPP__

#include "nic/hal/svc/hal_ext.hpp"

namespace hal {
namespace svc {

void
set_hal_status(hal::hal_status_t hal_status, delphi::SdkPtr sdk);

}    // namespace svc
}    // namespace hal

#endif    // __DELPHIC_UTILS_HPP__
