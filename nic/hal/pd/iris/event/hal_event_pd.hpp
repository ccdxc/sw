// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __HAL_PD_EVENT_HPP__
#define __HAL_PD_EVENT_HPP__

#include "nic/include/base.hpp"
#include "include/sdk/table_monitor.hpp"

using sdk::table::table_health_state_t;


void table_health_monitor(uint32_t table_id,
                          char *name,
                          table_health_state_t curr_state,
                          uint32_t capacity,
                          uint32_t usage,
                          table_health_state_t *new_state);

#endif // __HAL_PD_EVENT_HPP__

