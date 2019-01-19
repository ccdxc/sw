//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __MEM_HASH_HPP__
#define __MEM_HASH_HPP__

#include <string>

#include "include/sdk/base.hpp"
#include "include/sdk/mem.hpp"
#include "include/sdk/table.hpp"
#include "lib/table/common/table.hpp"
#include "lib/utils/crc_fast.hpp"

#include "mem_hash_stats.hpp"

using namespace std;

namespace sdk {
namespace table {

using sdk::utils::crcFast;
using sdk::table::sdk_table_api_params_t;
using sdk::table::memhash::mem_hash_api_stats;
using sdk::table::memhash::mem_hash_table_stats;

typedef struct mem_hash_factory_params_ {
    uint32_t table_id;
    uint32_t num_hints;
    uint32_t max_recircs;
    table_health_monitor_func_t health_monitor_func;
    key2str_t key2str;
    appdata2str_t appdata2str;
} mem_hash_factory_params_t;

typedef struct mem_hash_properties_ {
    std::string name;
    uint32_t main_table_id;
    uint32_t main_table_size;
    uint32_t hint_table_id;
    uint32_t hint_table_size;
    uint32_t num_hints;
    uint32_t key_len;
    uint32_t data_len;
    uint32_t max_recircs;
    uint32_t hash_poly;
    table_health_monitor_func_t health_monitor_func;
    key2str_t key2str;
    appdata2str_t appdata2str;
} mem_hash_properties_t;

typedef struct mem_hash_api_txn_ {
    bool valid;
    uint32_t reserved_count;
} mem_hash_api_txn_t;

class mem_hash {
private:
    mem_hash_properties_t *props_;
    void *main_table_;
    crcFast *crc32gen_;
    mem_hash_api_stats api_stats_;
    mem_hash_table_stats table_stats_;
    mem_hash_api_txn_t txn_;

private:
    sdk_ret_t init_(mem_hash_factory_params_t *params);
    sdk_ret_t genhash_(sdk_table_api_params_t *params);
    sdk_ret_t create_api_context_(uint32_t op, sdk_table_api_params_t *params,
                                  void **retctx);

public:
    static mem_hash *factory(mem_hash_factory_params_t *params);
    static void destroy(mem_hash *memhash);

    mem_hash() {
    }

    ~mem_hash() {
    }

    sdk_ret_t txn_start();
    sdk_ret_t txn_end();
    sdk_ret_t insert(sdk_table_api_params_t *params);
    sdk_ret_t update(sdk_table_api_params_t *params);
    sdk_ret_t remove(sdk_table_api_params_t *params);
    sdk_ret_t get(sdk_table_api_params_t *params);
    sdk_ret_t reserve(sdk_table_api_params_t *params);
    sdk_ret_t release(sdk_table_api_params_t *params);
    sdk_ret_t stats_get(sdk_table_api_stats_t *stats,
                        sdk_table_stats_t *table_stats);
};

}   // namespace table
}   // namespace sdk

using sdk::table::mem_hash;

#endif // __MEM_HASH_HPP__
