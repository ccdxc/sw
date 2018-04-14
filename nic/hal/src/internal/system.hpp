// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __SYSTEM_HPP__
#define __SYSTEM_HPP__

#include "nic/include/base.h"
#include "nic/gen/proto/hal/system.pb.h"

using sys::ApiCounter;
using sys::ApiStatsResponse;
using sys::ApiStatsEntry;
using sys::SystemResponse;

namespace hal {

hal_ret_t api_stats_get(ApiStatsResponse *rsp);
hal_ret_t system_get(SystemResponse *rsp);

}    // namespace hal

#endif    // __SYSTEM_HPP__

