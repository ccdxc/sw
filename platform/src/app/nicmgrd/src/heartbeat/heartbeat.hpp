//------------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#ifndef __HEARTBEAT_HPP__
#define __HEARTBEAT_HPP__

#include "nic/include/base.hpp"

namespace nicmgr {
namespace hb {

// initialization function to register callback with periodic thread
uint32_t heartbeat_init(void);

// is_nicmgr_healthy return true if NICMgr is healthy or else false
bool is_nicmgr_healthy(void);

}    // namespace hb
}    // namespace nicmgr

#endif    // __HEARTBEAT_HPP__

