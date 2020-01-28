//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __FTL_BASE_HPP__
#define __FTL_BASE_HPP__

// #undef __FTL_INCLUDES_HPP__
#include <map>
#include "ftl_includes.hpp"

namespace sdk {
namespace table {

class ftl_table_info {
private:
    sdk::table::properties_t *props_;
    void *table_;
    int ref_count;


public:
    ftl_table_info() {
        ref_count = 0;
    }

    sdk::table::properties_t * get_props(void) { return props_; }
    void set_props(sdk::table::properties_t *val) { props_ = val; }
    void *get_table(void) { return table_; }
    void set_table(void *val) { table_ = val; }
    void increment_ref_count () { ref_count++;}
    void decrement_ref_count () { ref_count--;}
    int get_ref_count() {return ref_count;}
};

class ftl_base {
private:
    static thread_local Apictx apictx_[FTL_MAX_API_CONTEXTS + 1];
    static Apictx *get_apictx(int index) { return &apictx_[index]; }

    void *main_table_;
    apistats api_stats_;
    tablestats tstats_;
    uint32_t thread_id_;
    static std::map<int, ftl_table_info *> table_info_cache;

private:
    sdk_ret_t ctxinit_(sdk_table_api_op_t op,
                       sdk_table_api_params_t *params);
    void add_table_to_cache_(uint32_t table_id,
                             sdk::table::properties_t *props,
                             void *table);
    bool remove_table_from_cache_(uint32_t table_id);
    ftl_table_info *get_cached_table_(uint32_t table_id);

protected:
    sdk::table::properties_t *props_;

    virtual sdk_ret_t genhash_(sdk_table_api_params_t *params) = 0;

public:
    static void destroy(ftl_base *f);

    ftl_base() {}
    ~ftl_base() {}
    sdk_ret_t init_(sdk_table_factory_params_t *params);

    sdk_ret_t txn_start();
    sdk_ret_t txn_end();
    sdk_ret_t insert(sdk_table_api_params_t *params);
    sdk_ret_t update(sdk_table_api_params_t *params);
    sdk_ret_t remove(sdk_table_api_params_t *params);
    sdk_ret_t get(sdk_table_api_params_t *params);
    sdk_ret_t stats_get(sdk_table_api_stats_t *api_stats,
                        sdk_table_stats_t *table_stats);
    sdk_ret_t iterate(sdk_table_api_params_t *params);
    sdk_ret_t clear(bool clear_global_state, bool clear_thread_local_state,
                    sdk_table_api_params_t *params);
    sdk_ret_t clear_stats(void);

    virtual base_table_entry_t *get_entry(int index) = 0;
};

}   // namespace table
}   // namespace sdk

#endif    // __FTL_BASE_HPP__
