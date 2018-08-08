//------------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#ifndef __HEARTBEAT_HPP__
#define __HEARTBEAT_HPP__

#include "nic/include/base.hpp"

namespace hal {
namespace hb {

// API invoked by all threads spawned by infra to punch heartbeat
// and declare that the thread is still alive
void heartbeat_punch(void);

// initialization function to register callback with periodic thread
hal_ret_t heartbeat_init(void);

}    // namespace hb
}    // namespace hal

#endif    // __HEARTBEAT_HPP__

