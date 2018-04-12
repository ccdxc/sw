//------------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#ifndef __STATS_HPP__
#define __STATS_HPP__

#include "nic/include/base.h"

namespace hal {

hal_ret_t hal_stats_init_cb(void);
hal_ret_t hal_stats_cleanup_cb(void);

}    // namespace

#endif    // __STATS_HPP__
