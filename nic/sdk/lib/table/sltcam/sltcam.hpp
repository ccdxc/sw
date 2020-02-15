//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// stateless tcam table management library
//------------------------------------------------------------------------------
#ifndef __SDK_SLTCAM_HPP__
#define __SDK_SLTCAM_HPP__

#include "include/sdk/table.hpp"
#include "include/sdk/mem.hpp"
#include "include/sdk/base.hpp"
#include "lib/indexer/indexer.hpp"

#include "sltcam_properties.hpp"
#include "sltcam_api_context.hpp"
#include "sltcam_stats.hpp"
#include "sltcam_db.hpp"
#include "sltcam_txn.hpp"

using sdk::lib::indexer;
using sltctx = sdk::table::sltcam_internal::sltctx;
using sdk::table::sdk_table_api_params_t;

namespace sdk {
namespace table {

class sltcam {
public:
    static sltcam* factory(sdk_table_factory_params_t *params);
    static void destroy(sltcam *sltcam);

    // public APIs
    sdk_ret_t insert(sdk_table_api_params_t *params);
    sdk_ret_t remove(sdk_table_api_params_t *params);
    sdk_ret_t get(sdk_table_api_params_t *params);
    sdk_ret_t update(sdk_table_api_params_t *params);
    sdk_ret_t iterate(sdk_table_api_params_t *params);
    sdk_ret_t reserve(sdk_table_api_params_t *params);
    sdk_ret_t release(sdk_table_api_params_t *params);
    sdk_ret_t stats_get(sdk_table_api_stats_t *api_stats,
                        sdk_table_stats_t *table_stats);
    sdk_ret_t txn_start(void);
    sdk_ret_t txn_end(void);

    // DEV USAGE ONLY:
    // Api to check sanity of the internal state
    sdk_ret_t sanitize(void);

private:
    sltcam() {
        SDK_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE);
    }
    ~sltcam() {
        SDK_SPINLOCK_DESTROY(&slock_);
    }
    sdk_ret_t init_(sdk_table_factory_params_t *params);
    sdk_ret_t read_(sltctx *ctx);
    sdk_ret_t write_(sltctx *ctx);
    sdk_ret_t find_(sltctx *ctx);
    sdk_ret_t alloc_(sltctx *ctx);
    sdk_ret_t dealloc_(sltctx *ctx);

private:
    sdk::lib::indexer *indexer_;
    sdk::table::sltcam_internal::properties props_;
    sdk::table::sltcam_internal::stats stats_;
    sdk::table::sltcam_internal::db db_;
    sdk::table::sltcam_internal::txn txn_;
    sdk_spinlock_t slock_;
};

}    // namespace table
}    // namespace sdk

using sdk::table::sltcam;

#endif    // __SDK_TCAM_HPP__
