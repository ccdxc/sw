//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __HAL_EXT_HPP__
#define __HAL_EXT_HPP__

#include "nic/hal/hal.hpp"

namespace hal {
namespace svc {

void hal_init_done(void);
void set_hal_status(hal::hal_status_t hal_status);

}    // namespace svc
}    // namespace hal

#endif    // __HAL_EXT_HPP__

