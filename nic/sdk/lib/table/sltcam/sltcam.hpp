//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// stateless tcam table management library
//------------------------------------------------------------------------------
#ifndef __SDK_SLTCAM_HPP__
#define __SDK_SLTCAM_HPP__

#include "include/sdk/table.hpp"
#include "include/sdk/mem.hpp"
#include "include/sdk/base.hpp"
#include "lib/indexer/indexer.hpp"

#include "sltcam_internal.hpp"

using sdk::lib::indexer;
using sdk::table::table_health_state_t;
using sdk::table::table_health_monitor_func_t;

using sdk::table::sltcam::handle_t;

namespace sdk {
namespace table {

typedef struct sltcam_properties_ {
    std::string name; // table name
    uint32_t table_id; // table id
    uint32_t table_size; // size of tcam table
    uint32_t swkey_len; // sw key len
    uint32_t swdata_len_; // sw data len
    uint32_t hwkey_len; // hw key len
    uint32_t hwkeymask_len; // hw key mask len
    uint32_t hwdata_len; // hw data len
    bool entry_trace_en_; // enable entry tracing
    table_health_state_t health_state_; // health state
    table_health_monitor_func_t health_monitor_func; // health mon. cb
} sltcam_properties_t;

class sltcam {
private:
    sltcam_properties_t props_;
    indexer *indexer_;
    sltcam_stats_t stats_;

    sltcam() {}
    ~sltcam() {}
    sdk_ret_t init_(sdk_table_factory_params_t *params);
    sdk_ret_t read_(tcam_api_context_t *ctx);
    sdk_ret_t write_(tcam_api_context_t *ctx, bool del);
    sdk_ret_t alloc_(tcam_api_context_t *ctx);
    sdk_ret_t dealloc_(tcam_api_context_t *ctx);

public:
    static sltcam* factory(sdk_table_factory_params_t *params);
    static void destroy(sltcam *sltcam);

    // api methods
    sdk_ret_t insert(sdk_table_api_params_t *params);
    sdk_ret_t remove(sdk_table_api_params_t *params);
    sdk_ret_t get(sdk_table_api_params_t *params);
    sdk_ret_t update(sdk_table_api_params_t *params);
    sdk_ret_t txn_start();
    sdk_ret_t txn_end();
    sdk_ret_t reserve(sdk_table_api_params_t *params);
    sdk_ret_t release(sdk_table_api_params_t *params);
    sdk_ret_t stats_get(sdk_table_api_stats_t *api_stats,
                        sdk_table_stats_t *table_stats);
};

}    // namespace table
}    // namespace sdk

#endif    // __SDK_TCAM_HPP__
