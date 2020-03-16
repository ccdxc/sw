//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This provides api for rss table modification API's to HAL
///
//----------------------------------------------------------------------------

#include "nic/include/base.hpp"
#include "nic/include/pd_api.hpp"
#include "platform/src/lib/eth_p4plus/eth_p4pd.hpp"

namespace hal {
namespace pd {

hal_ret_t
pd_rss_params_table_entry_add(pd_func_args_t *pd_func_args)
{
    sdk_ret_t sdk_ret;
    pd_rss_params_table_entry_add_args_t *args = pd_func_args->pd_rss_params_table_entry_add;

    sdk_ret = p4plus_rxdma_rss_params_update(args->hw_lif_id,
                                             args->rss_type,
                                             args->rss_key,
                                             false);
    return hal_sdk_ret_to_hal_ret(sdk_ret);
}

hal_ret_t
pd_rss_indir_table_entry_add(pd_func_args_t *pd_func_args)
{
    sdk_ret_t sdk_ret;
    pd_rss_indir_table_entry_add_args_t *args = pd_func_args->pd_rss_indir_table_entry_add;

    sdk_ret = p4plus_rxdma_rss_indir_update(args->hw_lif_id,
                                            args->index,
                                            args->enable,
                                            args->qid);
    return hal_sdk_ret_to_hal_ret(sdk_ret);
}

}    // namespace pd
}    // namespace hal
