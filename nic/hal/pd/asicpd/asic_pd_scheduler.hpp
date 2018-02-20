#ifndef __ASICPD_SCHEDULER_HPP_
#define __ASICPD_SCHEDULER_HPP_

#include "nic/include/base.h"
#include "nic/include/pd.hpp"
#include "nic/hal/pd/capri/capri_txs_scheduler.hpp"

typedef struct asicpd_scheduler_lif_params_s_ {
    uint32_t lif_id;
    uint32_t tx_sched_table_offset;
    uint32_t tx_sched_num_table_entries; 
    uint32_t total_qcount;
    uint32_t hw_lif_id;
    uint16_t cos_bmp;
} __PACK__ asicpd_scheduler_lif_params_t;

namespace hal {
namespace pd {

hal_ret_t asicpd_scheduler_tx_pd_alloc(asicpd_scheduler_lif_params_t *lif);
hal_ret_t asicpd_scheduler_tx_pd_dealloc(asicpd_scheduler_lif_params_t *lif);
hal_ret_t asicpd_scheduler_tx_pd_program_hw(asicpd_scheduler_lif_params_t *lif);
hal_ret_t asicpd_scheduler_tx_pd_deprogram_hw(asicpd_scheduler_lif_params_t *lif);
hal_ret_t asicpd_policer_tx_pd_program_hw(asicpd_scheduler_lif_params_t *lif);

}   // namespace pd
}   // namespace hal

#endif 
