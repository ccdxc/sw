//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __FTL_HPP__
#define __FTL_HPP__

#include <string>

#include "include/sdk/lock.hpp"
#include "include/sdk/base.hpp"
#include "include/sdk/mem.hpp"
#include "include/sdk/table.hpp"
#include "lib/table/common/table.hpp"
#include "lib/utils/crc_fast.hpp"

#include "ftl_stats.hpp"
#include "ftl_apictx.hpp"
#include "ftl_structs.hpp"

using namespace std;

namespace sdk {
namespace table {

using sdk::utils::crcFast;
using sdk::table::ftlint::ftl_api_stats;
using sdk::table::ftlint::ftl_table_stats;
using sdk::table::ftlint::ftl_apictx;

class ftl {
private:
    sdk::table::properties_t *props_;
    void *main_table_;
    crcFast *crc32gen_;
    ftl_api_stats api_stats_;
    ftl_table_stats table_stats_;
    sdk_spinlock_t slock_;
    ftl_apictx apictx_;

private:
    sdk_ret_t init_(sdk_table_factory_params_t *params);
    sdk_ret_t genhash_(sdk_table_api_params_t *params);
    sdk_ret_t ctxinit_(sdk_table_api_op_t op,
                       sdk_table_api_params_t *params);

public:
    static ftl *factory(sdk_table_factory_params_t *params);
    static void destroy(ftl *ftl);

    ftl() {
        props_ = NULL;
        main_table_ = NULL;
        crc32gen_ = NULL;
        SDK_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE);
    }

    ~ftl() {
        SDK_SPINLOCK_DESTROY(&slock_);
    }

    sdk_ret_t txn_start();
    sdk_ret_t txn_end();
    sdk_ret_t insert(sdk_table_api_params_t *params);
    sdk_ret_t update(sdk_table_api_params_t *params);
    sdk_ret_t remove(sdk_table_api_params_t *params);
    sdk_ret_t get(sdk_table_api_params_t *params);
    sdk_ret_t stats_get(sdk_table_api_stats_t *api_stats,
                        sdk_table_stats_t *table_stats);
    sdk_ret_t iterate(sdk_table_api_params_t *params);
};

}   // namespace table
}   // namespace sdk

using sdk::table::ftl;

#endif // __FTL_HPP__
