//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
// SDK types header file
//------------------------------------------------------------------------------

#ifndef __SDK_TABLE_HPP__
#define __SDK_TABLE_HPP__

#include <stdint.h>

namespace sdk {
namespace table {

typedef bool (*iterate_t)(void *key, void *data, const void *cb_data);
typedef char* (*key2str_t)(void *key);
typedef char* (*appdata2str_t)(void *data);
typedef uint64_t sdk_table_handle_t;

typedef struct sdk_table_api_params_ {
    // Input Param
    void *key;
    // Input Param
    void *appdata;
    // Input Param
    uint8_t action_id;
    // Input/Output Param (Input is Optional)
    // valid only for HASH tables
    bool hash_valid;
    // Input/Output Param (Input is Optional)
    // valid only for HASH tables
    uint32_t hash_32b;
    // Input/Output Param
    sdk_table_handle_t handle;
} sdk_table_api_params_t;

typedef struct sdk_table_api_stats_ {
    uint32_t insert;
    uint32_t insert_duplicate;
    uint32_t insert_fail;
    uint32_t remove;
    uint32_t remove_not_found;
    uint32_t remove_fail;
    uint32_t update;
    uint32_t update_fail;
    uint32_t get;
    uint32_t get_fail;
    uint32_t reserve;
    uint32_t reserve_fail;
    uint32_t release;
    uint32_t release_fail;
} sdk_table_api_stats_t;

typedef struct sdk_table_stats_ {
    uint32_t entries;
    uint32_t collisions;
} sdk_table_stats_t;

} // namespace table
} // namespace sdk
#endif // __SDK_TABLE_HPP__
