//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// - Common definitions for tables
//------------------------------------------------------------------------------

#ifndef __TABLE_MONITOR_HPP__
#define __TABLE_MONITOR_HPP__

namespace sdk {
namespace table {

typedef enum table_health_state_s {
    TABLE_HEALTH_GREEN,
    TABLE_HEALTH_YELLOW,
    TABLE_HEALTH_RED
} table_health_state_t;

/*
 * Callback on every INSERT and DELETE of a table lib.
 * Callback has to be implemented to set new state based on capacity & usage.
 */
typedef void (*table_health_monitor_func_t)(uint32_t table_id,
                                            char *name,
                                            table_health_state_t curr_state,
                                            uint32_t capacity,
                                            uint32_t usage,
                                            table_health_state_t *new_state);


}    // namespace table
}    // namespace sdk

#endif    // __TABLE_MONITOR_HPP__
