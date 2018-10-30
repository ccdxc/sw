//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __SYSTEM_HPP__
#define __SYSTEM_HPP__

#include "nic/include/base.hpp"
#include "gen/proto/system.pb.h"

using sys::ApiCounter;
using sys::ApiStatsResponse;
using sys::ApiStatsEntry;
using sys::SystemResponse;

namespace hal {

hal_ret_t api_stats_get(ApiStatsResponse *rsp);
hal_ret_t system_get(SystemResponse *rsp);
hal_ret_t system_uuid_get(SystemResponse *rsp);
hal_ret_t clear_pb_stats(void);

}    // namespace hal

#endif    // __SYSTEM_HPP__

