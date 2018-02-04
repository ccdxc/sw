#ifndef __HAL_PD_SYSTEM_HPP__
#define __HAL_PD_SYSTEM_HPP__

#include "nic/include/base.h"
#include "nic/include/pd_api.hpp"

using sys::DropStatsEntry;
namespace hal {
namespace pd {

hal_ret_t
pd_system_decode(drop_stats_swkey *key, drop_stats_swkey_mask *key_mask, 
        drop_stats_actiondata *data, DropStatsEntry *stats_entry);


}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_SYSTEM_HPP__
