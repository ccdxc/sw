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
#include "nic/include/hal_cfg.hpp"
#include "nic/include/hal.hpp"
#include "nic/hal/pd/capri/capri_txs_scheduler.hpp"
#include "nic/asic/capri/model/cap_psp/cap_psp_csr.h"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/hal/src/qos.hpp"
#include "nic/hal/pd/pd_api.hpp"
#include "nic/hal/pd/capri/capri.hpp"
#include "nic/hal/pd/capri/capri_state.hpp"

#ifndef HAL_GTEST
#include "nic/asic/capri/model/utils/cap_blk_reg_model.h"
#include "nic/asic/capri/model/cap_top/cap_top_csr.h"
#include "nic/asic/capri/model/cap_txs/cap_txs_csr.h"
#include "nic/asic/capri/verif/apis/cap_txs_api.h"
#endif

#define NUM_MAX_COSES 16
#define CHECK_BIT(var,pos) ((var) & (1 << (pos)))
#define DTDM_CALENDAR_SIZE 64

extern class capri_state_pd *g_capri_state_pd;

hal_ret_t
capri_txs_scheduler_init (uint32_t admin_cos)
{

    cap_top_csr_t       &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_txs_csr_t       &txs_csr = cap0.txs.txs;
    cap_psp_csr_t       &psp_csr = cap0.pt.pt.psp;
    uint64_t            txs_sched_hbm_base_addr;
    uint16_t            dtdm_lo_map, dtdm_hi_map;
    uint32_t            control_cos;
    //hal::qos_class_t    *control_qos_class;
    //hal_ret_t            ret = HAL_RET_OK;

    hal::hal_cfg_t *hal_cfg =
                (hal::hal_cfg_t *)hal::hal_get_current_thread()->data();
    HAL_ASSERT(hal_cfg);

    txs_sched_hbm_base_addr = (uint64_t) get_start_offset(CAPRI_HBM_REG_TXS_SCHEDULER);

    // Update HBM base addr.  
    txs_csr.cfw_scheduler_static.read();
    txs_csr.cfw_scheduler_static.hbm_base(txs_sched_hbm_base_addr);

    // Init sram.
    txs_csr.cfw_scheduler_glb.read();
    // skip init on RTL/Model.
    if (hal_cfg->platform_mode == hal::HAL_PLATFORM_MODE_HAPS) {
        txs_csr.cfw_scheduler_glb.hbm_hw_init(1);
    }
    txs_csr.cfw_scheduler_glb.sram_hw_init(1);

    txs_csr.cfw_scheduler_static.write();
    txs_csr.cfw_scheduler_glb.write();

#if 0
    // Find admin_cos and program it in dtdmhi-calendar for higher priority.
    // NOTE TODO: Init of admin-qos-class should be done before this.
    // Find control_cos and program it for higher-priority in dtdmlo-calendar.
    if ((control_qos_class = find_qos_class_by_group(hal::QOS_GROUP_CONTROL)) != NULL) {
        hal::pd::pd_qos_class_get_qos_class_id_args_t args = {0};
        args.qos_class = control_qos_class;
        args.dest_if = NULL;
        args.qos_class_id = &control_cos;
        ret = hal::pd::pd_qos_class_get_qos_class_id(&args);
        // ret = hal::pd::qos_class_get_qos_class_id(control_qos_class, NULL, &control_cos);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error deriving qos-class-id for admin Qos class "
                          "{} ret {}",
                          control_qos_class->key, ret);
            control_cos = 0;
        }
    } else {
        HAL_TRACE_DEBUG("control qos class not init'ed!! Setting it to default\n");
        control_cos = 0;
    }
#endif
   
    // Init scheduler calendar  for all cos.
    for (uint32_t i = 0; i < DTDM_CALENDAR_SIZE ; i++) {
        txs_csr.dhs_dtdmlo_calendar.entry[i].read();
        txs_csr.dhs_dtdmhi_calendar.entry[i].read();
        
        // Program only admin_cos in hi-calendar to mimic strict-priority.
        // Program control_cos multiple times in lo-calendar for higher priority than other coses.
        txs_csr.dhs_dtdmhi_calendar.entry[i].dtdm_calendar(admin_cos);
        if ((i % 16) == admin_cos) {
            txs_csr.dhs_dtdmlo_calendar.entry[i].dtdm_calendar(control_cos);
        } else {
            txs_csr.dhs_dtdmlo_calendar.entry[i].dtdm_calendar(i % 16);
        }
        txs_csr.dhs_dtdmlo_calendar.entry[i].write();
        txs_csr.dhs_dtdmhi_calendar.entry[i].write();
    }

    // Init all cos except admin-cos to be part of lo-calendar. admin-cos will be in hi-calendar (strict priority).
    dtdm_lo_map = (0xffff & ~(1 << admin_cos));
    dtdm_hi_map = 1 << admin_cos;
    txs_csr.cfg_sch.read();
    txs_csr.cfg_sch.dtdm_lo_map(dtdm_lo_map);
    txs_csr.cfg_sch.dtdm_hi_map(dtdm_hi_map);
    txs_csr.cfg_sch.timeout(0);
    txs_csr.cfg_sch.pause(0);
    txs_csr.cfg_sch.enable(1);
    txs_csr.cfg_sch.write();

    // Asic polling routine to check if init is done and kickstart scheduler.
    cap_txs_init_done(0, 0);

    //Init PSP block to enable mapping tm_oq value in PHV.
    psp_csr.cfg_npv_values.read();
    psp_csr.cfg_npv_values.tm_oq_map_enable(1);
    psp_csr.cfg_npv_values.write();

    //Program one-to-one mapping from cos to tm_oq.
    for (int i = 0; i < NUM_MAX_COSES ; i++) {
        psp_csr.cfg_npv_cos_to_tm_oq_map[i].tm_oq(i);
        psp_csr.cfg_npv_cos_to_tm_oq_map[i].write();
    }

    HAL_TRACE_DEBUG("Set hbm base addr for TXS sched to {:#x}, dtdm_lo_map {:#x}, dtdm_hi_map {:#x}",
                    txs_sched_hbm_base_addr, dtdm_lo_map, dtdm_hi_map);
    return HAL_RET_OK;
}

hal_ret_t
capri_txs_scheduler_lif_params_update(uint32_t hw_lif_id, capri_txs_sched_lif_params_t *txs_hw_params) 
{

    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_txs_csr_t &txs_csr = cap0.txs.txs;
    uint32_t      i = 0, j = 0, table_offset = 0, num_cos_val = 0, lif_cos_index = 0;
    uint16_t      lif_cos_bmp = 0x0;

    lif_cos_bmp = txs_hw_params->cos_bmp;
    if ((hw_lif_id >= CAPRI_TXS_MAX_TABLE_ENTRIES) ||  
        (txs_hw_params->sched_table_offset >= CAPRI_TXS_MAX_TABLE_ENTRIES)) {
        HAL_TRACE_ERR("CAPRI-TXS::{}: Invalid parameters to function {},{}",__func__, hw_lif_id, 
                       txs_hw_params->sched_table_offset);
        return HAL_RET_INVALID_ARG;
    }

    //Program mapping from (lif,queue,cos) to scheduler table entries.
    txs_csr.dhs_sch_lif_map_sram.entry[hw_lif_id].read();
    txs_csr.dhs_sch_lif_map_sram.entry[hw_lif_id].sg_start(txs_hw_params->sched_table_offset);
    txs_csr.dhs_sch_lif_map_sram.entry[hw_lif_id].sg_per_cos(txs_hw_params->num_entries_per_cos);
    txs_csr.dhs_sch_lif_map_sram.entry[hw_lif_id].sg_act_cos(lif_cos_bmp);

    if (lif_cos_bmp == 0x0 && !txs_hw_params->num_entries_per_cos) 
        txs_csr.dhs_sch_lif_map_sram.entry[hw_lif_id].sg_active(0); // lif delete case. Make the entry invalid.
    else
        txs_csr.dhs_sch_lif_map_sram.entry[hw_lif_id].sg_active(1);
        
    txs_csr.dhs_sch_lif_map_sram.entry[hw_lif_id].write();

    //Program reverse mapping from scheduler table entry to (lif,queue,cos). 
    for (i = 0; i < NUM_MAX_COSES; i++) {
        if (CHECK_BIT(lif_cos_bmp, i)) {
            lif_cos_index = (num_cos_val * txs_hw_params->num_entries_per_cos);
            //Program all entries for this cos.
            for (j = 0; j < txs_hw_params->num_entries_per_cos; j++) {
                table_offset = txs_hw_params->sched_table_offset + lif_cos_index + j;
                txs_csr.dhs_sch_grp_entry.entry[table_offset].read();
                txs_csr.dhs_sch_grp_entry.entry[table_offset].lif(hw_lif_id);
                txs_csr.dhs_sch_grp_entry.entry[table_offset].qid_offset(j);
                txs_csr.dhs_sch_grp_entry.entry[table_offset].rr_sel(i);
                txs_csr.dhs_sch_grp_entry.entry[table_offset].write();
            }
            num_cos_val++;
        }
    }

    HAL_TRACE_DEBUG("Programmed sched-table-offset {} and entries-per-cos {}"
                    "and cos-bmp {:#x} for hw-lif-id {}", txs_hw_params->sched_table_offset,
                     txs_hw_params->num_entries_per_cos, lif_cos_bmp, hw_lif_id);

    return HAL_RET_OK;
}

hal_ret_t
capri_txs_policer_lif_params_update (uint32_t hw_lif_id,
                            capri_txs_policer_lif_params_t *txs_hw_params)
{
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_txs_csr_t &txs_csr = cap0.txs.txs;

    if ((hw_lif_id >= CAPRI_TXS_MAX_TABLE_ENTRIES) ||
        (txs_hw_params->sched_table_end_offset >= CAPRI_TXS_MAX_TABLE_ENTRIES)) {
        HAL_TRACE_ERR("CAPRI-TXS::{}: Invalid parameters to function {},{}",__func__, hw_lif_id,
                       txs_hw_params->sched_table_end_offset);
        return HAL_RET_INVALID_ARG;
    }

    // Program mapping from rate-limiter-table entry (indexed by hw-lif-id) to scheduler table entries.
    // The scheduler table entries (also called Rate-limiter-group, RLG)  will be paused when rate-limiter entry goes red.
    txs_csr.dhs_sch_rlid_map_sram.entry[hw_lif_id].read();
    txs_csr.dhs_sch_rlid_map_sram.entry[hw_lif_id].sg_start(txs_hw_params->sched_table_start_offset);
    txs_csr.dhs_sch_rlid_map_sram.entry[hw_lif_id].sg_end(txs_hw_params->sched_table_end_offset);
    txs_csr.dhs_sch_rlid_map_sram.entry[hw_lif_id].write();

    HAL_TRACE_DEBUG("Programmed sched-table-start-offset {} and sched-table-end-offset {}"
                    "for hw-lif-id {}", txs_hw_params->sched_table_start_offset,
                     txs_hw_params->sched_table_end_offset, hw_lif_id);

    return HAL_RET_OK;
}

hal_ret_t
capri_txs_scheduler_tx_alloc (capri_txs_sched_lif_params_t *tx_params,
                              uint32_t *alloc_offset, uint32_t *alloc_units)
{
    hal_ret_t     ret = HAL_RET_OK;
    uint32_t      total_qcount = 0;
    
    *alloc_offset = INVALID_INDEXER_INDEX;
    *alloc_units = 0;
    // Sched table can hold 8K queues per index and mandates new index for each cos.
    total_qcount = tx_params->total_qcount;
    *alloc_units  =  (total_qcount / CAPRI_TXS_SCHEDULER_NUM_QUEUES_PER_ENTRY);
    *alloc_units += ((total_qcount % CAPRI_TXS_SCHEDULER_NUM_QUEUES_PER_ENTRY) ? 1 : 0);
    *alloc_units *=   hal::get_num_set_bits(tx_params->cos_bmp);

    if (*alloc_units > 0) {
        //Allocate consecutive alloc_unit num of entries in sched table.
        *alloc_offset = g_capri_state_pd->txs_scheduler_map_idxr()->Alloc(*alloc_units);
        if (*alloc_offset < 0) {
            ret = HAL_RET_NO_RESOURCE;
        }
    }
    return ret;
}

hal_ret_t
capri_txs_scheduler_tx_dealloc (uint32_t alloc_offset, uint32_t alloc_units)
{
    hal_ret_t     ret = HAL_RET_OK;
    g_capri_state_pd->txs_scheduler_map_idxr()->Free(alloc_offset, alloc_units);
    return ret;
}

