#ifndef __HAL_PD_SYSTEM_HPP__
#define __HAL_PD_SYSTEM_HPP__

#include "nic/include/base.hpp"
#include "nic/include/pd_api.hpp"

using sys::DropStatsEntry;
using sys::EgressDropStatsEntry;
namespace hal {
namespace pd {

hal_ret_t
pd_system_decode(drop_stats_swkey *key, drop_stats_swkey_mask *key_mask, 
        drop_stats_actiondata *data, DropStatsEntry *stats_entry);
hal_ret_t
pd_conv_hw_clock_to_sw_clock(pd_func_args_t *pd_func_args);
hal_ret_t
pd_conv_sw_clock_to_hw_clock(pd_func_args_t *pd_func_args);

}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_SYSTEM_HPP__
