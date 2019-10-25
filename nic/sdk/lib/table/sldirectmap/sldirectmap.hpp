//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
// stateless directmap (index) table management library
//
// - manages directmap tables
// - eventually calls P4 APIs to program.
//
//------------------------------------------------------------------------------

#ifndef __SDK_LIB_TABLE_SLDIRECTMAP_HPP__
#define __SDK_LIB_TABLE_SLDIRECTMAP_HPP__

#include "include/sdk/mem.hpp"
#include "include/sdk/base.hpp"
#include "include/sdk/table.hpp"
#include "lib/rte_indexer/rte_indexer.hpp"
#include "lib/table/common/table.hpp"
#include "lib/table/sldirectmap/sldirectmap_entry.hpp"
#include "sldirectmap_utils.hpp"

using sdk::lib::ht;
using sdk::lib::rte_indexer;
using sdk::table::sldirectmap_api_stats;

namespace sdk {
namespace table {

class sldirectmap {
public:
    // factory & destroy methods
    static sldirectmap *factory(sdk_table_factory_params_t *params);
    static void destroy(sldirectmap *dm);

    // utility apis
    std::string name() { return props_->name; }
    uint32_t table_id() {return props_->ptable_id; }
    uint32_t capacity() { return props_->ptable_size; }
    uint32_t inuse() { return in_use_; }
    sdk_ret_t entry_to_str(void *data, uint32_t index, char *buff,
                           uint32_t buff_size);

    // library APIs
    sdk_ret_t insert(sdk_table_api_params_t *params);
    sdk_ret_t insert_withid(sdk_table_api_params_t *params);
    sdk_ret_t insert_atid(sdk_table_api_params_t *params);
    sdk_ret_t reserve_index(sdk_table_api_params_t *params);
    sdk_ret_t reserve(sdk_table_api_params_t *params);
    sdk_ret_t release(sdk_table_api_params_t *params);
    sdk_ret_t update(sdk_table_api_params_t *params);
    sdk_ret_t remove(sdk_table_api_params_t *params);
    sdk_ret_t get(sdk_table_api_params_t *params);
    sdk_ret_t iterate(sdk_table_api_params_t *params);
    sdk_ret_t stats_get(sdk_table_api_stats_t *api_stats);

private:
    sldirectmap();
    ~sldirectmap();
    sdk_ret_t init_(sdk_table_factory_params_t *params);
    sdk_ret_t alloc_index_(uint32_t *idx);
    sdk_ret_t alloc_index_withid_(uint32_t idx);
    sdk_ret_t free_index_(uint32_t idx);
    sdk_ret_t entry_trace_(const char *fname, void *data, uint32_t index);

private:
    sdk::table::properties_t *props_;
    rte_indexer *indexer_;
    sldirectmap_api_stats api_stats_;
    uint32_t in_use_;
};

}    // namespace table
}    // namespace sdk

using sdk::table::sldirectmap;

#endif    // __SDK_LIB_TABLE_SLDIRECT_MAP_HPP__
