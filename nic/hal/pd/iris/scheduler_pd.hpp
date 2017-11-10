#ifndef __HAL_SCHEDULER_PD_HPP__
#define __HAL_SCHEDULER_PD_HPP__

#include "nic/include/base.h"
#include "nic/include/pd.hpp"

namespace hal {
namespace pd {

// 2K * 8K scheduler
#define TXS_SCHEDULER_MAP_MAX_ENTRIES 2048
#define TXS_SCHEDULER_NUM_QUEUES_PER_ENTRY 8192

hal_ret_t scheduler_tx_pd_alloc (pd_lif_t *pd_lif);
hal_ret_t scheduler_tx_pd_dealloc (pd_lif_t *pd_lif);
hal_ret_t scheduler_tx_pd_program_hw(pd_lif_t *pd_lif);
hal_ret_t scheduler_tx_pd_deprogram_hw(pd_lif_t *pd_lif);

}   // namespace pd
}   // namespace hal

#endif    // HAL_SCHEDULER_PD_HPP__
