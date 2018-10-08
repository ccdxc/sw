//------------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#ifndef __HEARTBEAT_HPP__
#define __HEARTBEAT_HPP__

#include "nic/include/base.hpp"

namespace hal {
namespace hb {

// initialization function to register callback with periodic thread
hal_ret_t heartbeat_init(void);

// is_hal_healthy return true if HAL is healthy or else false
bool is_hal_healthy(void);

}    // namespace hb
}    // namespace hal

#endif    // __HEARTBEAT_HPP__

