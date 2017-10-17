/*
 * capri_txs_scheduler.cc
 * Vishwas Danivas (Pensando Systems)
 */


#include <stdio.h>
#include <string>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>
#include <map>
#include <cmath>

#include "nic/include/base.h"
#include "nic/hal/pd/capri/capri_txs_scheduler.hpp"
#include "nic/hal/pd/capri/capri_hbm.hpp"

#ifndef HAL_GTEST
#include "nic/asic/capri/model/utils/cap_blk_reg_model.h"
#include "nic/asic/capri/model/cap_top/cap_top_csr.h"
#include "nic/asic/capri/model/cap_txs/cap_txs_csr.h"
#endif

hal_ret_t
capri_txs_scheduler_init ()
{

#ifndef HAL_GTEST
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_txs_csr_t &txs_csr = cap0.txs.txs;
    uint64_t       txs_sched_hbm_base_addr;

    txs_sched_hbm_base_addr = (uint64_t) get_start_offset(CAPRI_HBM_REG_TXS_SCHEDULER);

    // Update HBM base addr.  
    txs_csr.cfw_scheduler_static.hbm_base(txs_sched_hbm_base_addr);

    // Init the txs scheduler.
    txs_csr.cfw_scheduler_glb.enable(1);    
    txs_csr.cfw_scheduler_glb.hbm_hw_init(1);
    txs_csr.cfw_scheduler_glb.sram_hw_init(1);

    txs_csr.cfw_scheduler_static.write();
    txs_csr.cfw_scheduler_glb.write();

#endif 
    HAL_TRACE_DEBUG("CAPRI-TXS::{}: Set hbm base addr for TXS sched to {:#x}",
                    __func__, txs_sched_hbm_base_addr);
    return HAL_RET_OK;
}
