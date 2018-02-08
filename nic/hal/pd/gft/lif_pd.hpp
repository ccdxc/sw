// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __HAL_LIF_PD_HPP__
#define __HAL_LIF_PD_HPP__

#include "nic/include/base.h"
#include "nic/include/pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/globalpd/gpd_utils.hpp"

namespace hal {
namespace pd {

struct pd_lif_s {
    uint32_t    hw_lif_id;                  // Id used to carve out buffers
    uint32_t    tx_sched_table_offset;
    uint32_t    tx_sched_num_table_entries; 

    // pi ptr
    void        *pi_lif;
} __PACK__;


pd_lif_t *lif_pd_alloc ();
pd_lif_t *lif_pd_init (pd_lif_t *lif);
pd_lif_t *lif_pd_alloc_init ();
hal_ret_t lif_pd_free (pd_lif_t *lif);
hal_ret_t lif_pd_mem_free (pd_lif_t *lif);
hal_ret_t lif_pd_alloc_res(pd_lif_t *pd_lif, 
                           pd_lif_create_args_t *args);
hal_ret_t lif_pd_program_hw(pd_lif_t *pd_lif);
hal_ret_t lif_pd_deprogram_hw (pd_lif_t *pd_lif);

hal_ret_t lif_pd_rx_policer_program_hw (pd_lif_t *pd_lif, bool update);
hal_ret_t lif_pd_rx_policer_deprogram_hw (pd_lif_t *pd_lif);
hal_ret_t lif_pd_tx_policer_program_hw (pd_lif_t *pd_lif);
hal_ret_t lif_pd_tx_policer_deprogram_hw (pd_lif_t *pd_lif);

void link_pi_pd(pd_lif_t *pd_lif, lif_t *pi_lif);
void delink_pi_pd(pd_lif_t *pd_lif, lif_t *pi_lif);
hal_ret_t lif_pd_cleanup(pd_lif_t *lif_pd);
hal_ret_t pd_lif_make_clone(lif_t *ten, lif_t *clone);

}   // namespace pd
}   // namespace hal

#endif    // __HAL_LIF_PD_HPP__

