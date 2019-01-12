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
typedef char* (*data2str_t)(void *data);

using sdk::utils::crcFast;
using sdk::table::memhash::mem_hash_api_stats;
using sdk::table::memhash::mem_hash_table_stats;

class mem_hash {
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
    uint32_t                    hash_poly_;
    crcFast                     *crc32gen_;
    table_health_monitor_func_t health_monitor_func_;
    key2str_t                   key2str_;
    data2str_t                  data2str_;
    mem_hash_api_stats          api_stats_;
    mem_hash_table_stats        table_stats_;

private:
    sdk_ret_t   init_(uint32_t table_id,
                      uint32_t max_recircs,
                      table_health_monitor_func_t health_monitor_func,
                      key2str_t key2str, data2str_t data2str);

    uint32_t    genhash_(void *key);

public:
    static mem_hash *factory(uint32_t table_id,
                            uint32_t max_recircs = 8,
                            table_health_monitor_func_t health_monitor_func = NULL,
                            key2str_t key2str = NULL, 
                            data2str_t data2str = NULL);

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
    sdk_ret_t   getstats();
};

}   // namespace table
}   // namespace sdk

using sdk::table::mem_hash;

#endif // __MEM_HASH_HPP__
