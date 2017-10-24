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

#define NUM_MAX_COSES 16
#define CHECK_BIT(var,pos) ((var) & (1 << (pos)))

hal_ret_t
capri_txs_scheduler_init ()
{

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

    HAL_TRACE_DEBUG("CAPRI-TXS::{}: Set hbm base addr for TXS sched to {:#x}",
                    __func__, txs_sched_hbm_base_addr);
    return HAL_RET_OK;
}

hal_ret_t
capri_txs_scheduler_lif_params_update(uint32_t hw_lif_id, txs_sched_lif_params_t *txs_hw_params) 
{

    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_txs_csr_t &txs_csr = cap0.txs.txs;
    uint32_t      i = 0, j = 0, table_offset = 0, num_cos_val = 0, lif_cos_index = 0;
    uint16_t      lif_cos_bmp = 0x0;

    lif_cos_bmp = txs_hw_params->cos_bmp;
    if ((hw_lif_id >= TXS_MAX_TABLE_ENTRIES) ||  
        (txs_hw_params->sched_table_offset >= TXS_MAX_TABLE_ENTRIES)) {
        HAL_TRACE_ERR("CAPRI-TXS::{}: Invalid parameters to function {},{}",__func__, hw_lif_id, 
                       txs_hw_params->sched_table_offset);
        return HAL_RET_INVALID_ARG;
    }

    //Program mapping from (lif,queue,cos) to scheduler table entries.
    txs_csr.dhs_sch_lif_map_sram.entry[hw_lif_id].sg_start(txs_hw_params->sched_table_offset);
    txs_csr.dhs_sch_lif_map_sram.entry[hw_lif_id].sg_per_cos(txs_hw_params->num_entries_per_cos);
    txs_csr.dhs_sch_lif_map_sram.entry[hw_lif_id].sg_act_cos(lif_cos_bmp);
    txs_csr.dhs_sch_lif_map_sram.entry[hw_lif_id].write();

    //Program reverse mapping from scheduler table entry to (lif,queue,cos). 
    for (i = 0; i < NUM_MAX_COSES; i++) {
        if (CHECK_BIT(lif_cos_bmp, i)) {
            lif_cos_index = (num_cos_val * txs_hw_params->num_entries_per_cos);
            //Program all entries for this cos.
            for (j = 0; j < txs_hw_params->num_entries_per_cos; j++) {
                table_offset = txs_hw_params->sched_table_offset + lif_cos_index + j;
                txs_csr.dhs_sch_grp_entry.entry[table_offset].lif(hw_lif_id);
                txs_csr.dhs_sch_grp_entry.entry[table_offset].qid_offset(j);
                txs_csr.dhs_sch_grp_entry.entry[table_offset].rr_sel(i);
                txs_csr.dhs_sch_grp_entry.entry[table_offset].write();
            }
            num_cos_val++;
        }
    }

    HAL_TRACE_DEBUG("CAPRI-TXS::{}: Programmed sched-table-offset {} and entries-per-cos {}"
                    "and cos-bmp {:#x} for hw-lif-id {}", __func__, txs_hw_params->sched_table_offset,
                     txs_hw_params->num_entries_per_cos, lif_cos_bmp, hw_lif_id);

    return HAL_RET_OK;
}
