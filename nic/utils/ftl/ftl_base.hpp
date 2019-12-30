//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __FTL_BASE_HPP__
#define __FTL_BASE_HPP__

// #undef __FTL_INCLUDES_HPP__
#include "ftl_includes.hpp"

namespace sdk {
namespace table {

class ftl_base {
private:
    sdk::table::properties_t *props_;
    void *main_table_;
    apistats api_stats_;
    tablestats tstats_;
    Apictx apictx_[FTL_MAX_API_CONTEXTS + 1];
    uint32_t thread_id_;

private:
    sdk_ret_t init_(sdk_table_factory_params_t *params);
    sdk_ret_t genhash_(sdk_table_api_params_t *params);
    sdk_ret_t ctxinit_(sdk_table_api_op_t op,
                       sdk_table_api_params_t *params);

public:
    static ftl_base *factory(sdk_table_factory_params_t *params);
    static void destroy(ftl_base *f);

    ftl_base() {}
    ~ftl_base() {}

    sdk_ret_t txn_start();
    sdk_ret_t txn_end();
    sdk_ret_t insert(sdk_table_api_params_t *params);
    sdk_ret_t update(sdk_table_api_params_t *params);
    sdk_ret_t remove(sdk_table_api_params_t *params);
    sdk_ret_t get(sdk_table_api_params_t *params);
    sdk_ret_t stats_get(sdk_table_api_stats_t *api_stats,
                        sdk_table_stats_t *table_stats);
    sdk_ret_t iterate(sdk_table_api_params_t *params);
    sdk_ret_t clear(bool clear_global_state, bool clear_thread_local_state);
    sdk_ret_t clear_stats(void);
};

}   // namespace table
}   // namespace sdk

#endif    // __FTL_BASE_HPP__
