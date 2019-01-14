//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __MEM_HASH_HPP__
#define __MEM_HASH_HPP__

#include <string>

#include "include/sdk/base.hpp"
#include "include/sdk/mem.hpp"
#include "lib/table/common/table.hpp"
#include "lib/utils/crc_fast.hpp"

#include "mem_hash_stats.hpp"

using namespace std;

namespace sdk {
namespace table {
typedef bool (*iterate_t)(void *key, void *data, const void *cb_data);
typedef char* (*key2str_t)(void *key);
typedef char* (*appdata2str_t)(void *data);

using sdk::utils::crcFast;
using sdk::table::memhash::mem_hash_api_stats;
using sdk::table::memhash::mem_hash_table_stats;

typedef union mem_hash_handle_ {
    struct {
        uint32_t hint_table_entry:1;
        uint32_t main_table_index:28;
        uint32_t hint_table_index:24;
        uint32_t spare:11;
    };
    uint64_t value;
} __attribute__((__packed__)) mem_hash_handle_t;

typedef struct mem_hash_factory_params_ {
    uint32_t table_id;
    uint32_t num_hints;
    uint32_t max_recircs;
    table_health_monitor_func_t health_monitor_func;
    key2str_t key2str;
    appdata2str_t appdata2str;
} mem_hash_factory_params_t;

typedef struct mem_hash_api_params_ {
    void *key; // Input Param
    void *appdata; // Input Param
    bool hash_valid; // Input Param
    uint32_t hash_32b; // Input/Output Param
    mem_hash_handle_t handle; // Input/Output Param
} mem_hash_api_params_t;

typedef struct mem_hash_properties_ {
    std::string name;
    uint32_t main_table_id;
    uint32_t main_table_size;
    uint32_t hint_table_id;
    uint32_t hint_table_size;
    uint32_t num_hints;
    uint32_t key_len;
    uint32_t data_len;
    uint32_t appdata_len;
    uint32_t max_recircs;
    uint32_t hash_poly;
    table_health_monitor_func_t health_monitor_func;
    key2str_t key2str;
    appdata2str_t appdata2str;
} mem_hash_properties_t;

class mem_hash {
private:
    mem_hash_properties_t *props_;
    void *main_table_;
    void *hint_table_;
    crcFast *crc32gen_;
    mem_hash_api_stats api_stats_;
    mem_hash_table_stats table_stats_;

private:
    sdk_ret_t init_(mem_hash_factory_params_t *params);
    sdk_ret_t genhash_(mem_hash_api_params_t *params);

public:
    static mem_hash *factory(mem_hash_factory_params_t *params);
    static void destroy(mem_hash *memhash);

    mem_hash() {
    }

    ~mem_hash() {
    }

    sdk_ret_t insert(mem_hash_api_params_t *params);
    sdk_ret_t update(mem_hash_api_params_t *params);
    sdk_ret_t remove(mem_hash_api_params_t *params);
    sdk_ret_t get(mem_hash_api_params_t *params);
    sdk_ret_t reserve(mem_hash_api_params_t *params);
    sdk_ret_t release(mem_hash_api_params_t *params);
    sdk_ret_t getstats();
};

}   // namespace table
}   // namespace sdk

using sdk::table::mem_hash;

#endif // __MEM_HASH_HPP__
