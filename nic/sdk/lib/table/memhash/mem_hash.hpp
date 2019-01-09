//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __MEM_HASH_HPP__
#define __MEM_HASH_HPP__

#include <string>

#include "include/sdk/base.hpp"
#include "include/sdk/indexer.hpp"
#include "include/sdk/mem.hpp"
#include "include/sdk/crc_fast.hpp"
#include "include/sdk/table_monitor.hpp"
#include "include/sdk/slab.hpp"

#include "mem_hash_api_context.hpp"

using namespace std;

namespace sdk {
namespace table {

using sdk::table::memhash::mem_hash_api_context;

typedef bool (*mem_hash_iterate_func_t)(uint32_t gl_index, const void *cb_data);

class mem_hash {
public:
    enum HashPoly {
        // CRC_32,
        HASH_POLY0,
        HASH_POLY1,
        HASH_POLY2,
        HASH_POLY3,
        HASH_POLY_MAX = HASH_POLY3
    };

    enum stats {
        STATS_INS_SUCCESS,
        STATS_INS_MEM_HASH_COLL, // STATS_INS_SUCCESS will not be incr.
        STATS_INS_FAIL_DUP_INS,
        STATS_INS_FAIL_NO_RES,
        STATS_INS_FAIL_HW,
        STATS_UPD_SUCCESS,
        STATS_UPD_FAIL_ENTRY_NOT_FOUND,
        STATS_REM_SUCCESS,
        STATS_REM_FAIL_ENTRY_NOT_FOUND,
        STATS_REM_FAIL_HW,
        STATS_MAX
    };

private:
    // Private Data
    std::string                 name_;          // Name
    uint32_t                    main_table_id_; // Main Table ID
    uint32_t                    main_table_size_; // Main Table Size
    uint32_t                    hint_table_id_; // Hint Table ID
    uint32_t                    hint_table_size_; // Hint Table Size
    void                        *main_table_;   // Main Table
    void                        *hint_table_;   // Hint Table
    uint32_t                    key_len_;
    uint32_t                    data_len_;
    uint32_t                    max_recircs_;
    mem_hash::HashPoly           hash_poly_;
    table_health_monitor_func_t health_monitor_func_;

private:
    sdk_ret_t   init_(uint32_t table_id,
                      uint32_t max_recircs,
                      table_health_monitor_func_t health_monitor_func);

    uint32_t    genhash_(void *key);
    sdk_ret_t   defragment_(mem_hash_api_context *ctx);

public:
    static mem_hash *factory(uint32_t table_id,
                            uint32_t max_recircs = 8,
                            table_health_monitor_func_t health_monitor_func = NULL);

    static void destroy(mem_hash *memhash);

    mem_hash() {
    }
    
    ~mem_hash() {
    }

    sdk_ret_t   insert(void *key, void *data);
    sdk_ret_t   insert(void *key, void *data, uint32_t crc32);
    sdk_ret_t   update(void *key, void *data);
    sdk_ret_t   update(void *key, void *data, uint32_t crc32);
    sdk_ret_t   remove(void *key);
    sdk_ret_t   remove(void *key, uint32_t crc32);
};

}   // namespace table
}   // namespace sdk

using sdk::table::mem_hash;

#endif // __MEM_HASH_HPP__
