// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

/*
 * capri_txs_scheduler.cc
 * Vishwas Danivas (Pensando Systems)
 */

#include "platform/capri/capri_txs_scheduler.hpp"
#include "asic/cmn/asic_common.hpp"
#include "platform/capri/capri_state.hpp"
#include "lib/utils/utils.hpp"
#include "platform/utils/mpartition.hpp"

#include "third-party/asic/capri/model/cap_psp/cap_psp_csr.h"
#include "third-party/asic/capri/model/utils/cap_blk_reg_model.h"
#include "third-party/asic/capri/model/cap_top/cap_top_csr.h"
#include "third-party/asic/capri/model/cap_txs/cap_txs_csr.h"
#include "third-party/asic/capri/verif/apis/cap_txs_api.h"
#include "third-party/asic/capri/model/cap_wa/cap_wa_csr.h"

namespace sdk {
namespace platform {
namespace capri {

#define CHECK_BIT(var,pos) ((var) & (1 << (pos)))
#define DTDM_CALENDAR_SIZE 64

uint32_t
capri_get_coreclk_freq(platform_type_t platform_type)
{

    cap_top_csr_t &cap0 = g_capri_state_pd->cap_top();
    cap_ms_csr_t &ms_csr = cap0.ms.ms;

    static const uint32_t core_freq[] = {
        CORECLK_FREQ_ASIC_00, CORECLK_FREQ_ASIC_01,
        CORECLK_FREQ_ASIC_10, CORECLK_FREQ_ASIC_11
    };

    // Below status register is not modelled in Model. So return 833 MHz always.
    if (platform_type == platform_type_t::PLATFORM_TYPE_SIM) {
        return CORECLK_FREQ_ASIC_10;
    }

    ms_csr.sta_pll_cfg.read();
    return core_freq[((ms_csr.sta_pll_cfg.core_muldiv().convert_to<uint8_t>()) & 0x3)];
}

void
capri_txs_timer_init_hsh_depth (uint32_t key_lines)
{
    uint64_t timer_key_hbm_base_addr;
    cap_top_csr_t & cap0 = g_capri_state_pd->cap_top();
    cap_txs_csr_t *txs_csr = &cap0.txs.txs;

    timer_key_hbm_base_addr =
        g_capri_state_pd->mempartition()->start_addr(MEM_REGION_TIMERS_NAME);

    txs_csr->cfg_timer_static.read();
    SDK_TRACE_DEBUG("hbm_base 0x%llx",
                    (uint64_t)txs_csr->cfg_timer_static.hbm_base());
    SDK_TRACE_DEBUG("timer hash depth %u",
                    txs_csr->cfg_timer_static.tmr_hsh_depth());
    SDK_TRACE_DEBUG("timer wheel depth %u",
                    txs_csr->cfg_timer_static.tmr_wheel_depth());
    txs_csr->cfg_timer_static.hbm_base(timer_key_hbm_base_addr);
    txs_csr->cfg_timer_static.tmr_hsh_depth(key_lines - 1);
    txs_csr->cfg_timer_static.tmr_wheel_depth(CAPRI_TIMER_WHEEL_DEPTH - 1);
    txs_csr->cfg_timer_static.write();

}

// pre init and call timer hbm and sram init
static void
capri_txs_timer_init_pre (uint32_t key_lines, asic_cfg_t *capri_cfg)
{
    cap_top_csr_t & cap0 = g_capri_state_pd->cap_top();
    cap_txs_csr_t *txs_csr = &cap0.txs.txs;

    // Set timer_hsh_depth to actual value + 1
    // Per Cino we need to add 1 for an ASIC bug workaround
    capri_txs_timer_init_hsh_depth(key_lines + 1);

    // timer hbm and sram init

    // sram_hw_init is not implemented in the C++ model, so skip it there
    SDK_ASSERT(capri_cfg);
    txs_csr->cfw_timer_glb.read();
    if (capri_cfg->platform != platform_type_t::PLATFORM_TYPE_SIM) {
        SDK_TRACE_DEBUG("timer sram init");
        txs_csr->cfw_timer_glb.sram_hw_init(1);
    }

    // skip hbm init in model (C++ and RTL) as memory is 0 there and this
    // takes a long time
    if (capri_cfg->platform != platform_type_t::PLATFORM_TYPE_SIM &&
          capri_cfg->platform != platform_type_t::PLATFORM_TYPE_RTL) {
        SDK_TRACE_DEBUG("timer hbm init");
        txs_csr->cfw_timer_glb.hbm_hw_init(1);
    }
    txs_csr->cfw_timer_glb.write();

    SDK_TRACE_DEBUG("Done timer pre init");
}

// This is called after hbm and sram init
static void
capri_txs_timer_init_post (uint32_t key_lines, asic_cfg_t *capri_cfg)
{
    cap_top_csr_t & cap0 = g_capri_state_pd->cap_top();
    cap_txs_csr_t *txs_csr = &cap0.txs.txs;
    uint32_t capri_coreclk_freq;

    // 0 the last element of sram
    // Per Cino this is needed for an ASIC bug workaround
    txs_csr->dhs_tmr_cnt_sram.entry[CAPRI_TIMER_WHEEL_DEPTH - 1].read();
    txs_csr->dhs_tmr_cnt_sram.entry[CAPRI_TIMER_WHEEL_DEPTH - 1].slow_cbcnt(0);
    txs_csr->dhs_tmr_cnt_sram.entry[CAPRI_TIMER_WHEEL_DEPTH - 1].slow_bcnt(0);
    txs_csr->dhs_tmr_cnt_sram.entry[CAPRI_TIMER_WHEEL_DEPTH - 1].slow_lcnt(0);
    txs_csr->dhs_tmr_cnt_sram.entry[CAPRI_TIMER_WHEEL_DEPTH - 1].fast_cbcnt(0);
    txs_csr->dhs_tmr_cnt_sram.entry[CAPRI_TIMER_WHEEL_DEPTH - 1].fast_bcnt(0);
    txs_csr->dhs_tmr_cnt_sram.entry[CAPRI_TIMER_WHEEL_DEPTH - 1].fast_lcnt(0);
    txs_csr->dhs_tmr_cnt_sram.entry[CAPRI_TIMER_WHEEL_DEPTH - 1].write();

    // Set timer_hsh_depth back to original size
    // Per Cino this is needed for an ASIC bug workaround
    capri_txs_timer_init_hsh_depth(key_lines);

    capri_coreclk_freq =
        (uint32_t)(capri_get_coreclk_freq(capri_cfg->platform) / 1000000);

    // Set the tick resolution
    txs_csr->cfg_fast_timer.read();
    txs_csr->cfg_fast_timer.tick(capri_coreclk_freq);
    txs_csr->cfg_fast_timer.write();

    txs_csr->cfg_slow_timer.read();
    txs_csr->cfg_slow_timer.tick(capri_coreclk_freq * 1000);
    txs_csr->cfg_slow_timer.write();

    // Timer doorbell config
    txs_csr->cfg_fast_timer_dbell.read();
    txs_csr->cfg_fast_timer_dbell.addr_update(DB_IDX_UPD_PIDX_INC |
                                              DB_SCHED_UPD_EVAL);
    txs_csr->cfg_fast_timer_dbell.write();

    txs_csr->cfg_slow_timer_dbell.read();
    txs_csr->cfg_slow_timer_dbell.addr_update(DB_IDX_UPD_PIDX_INC |
                                              DB_SCHED_UPD_EVAL);
    txs_csr->cfg_slow_timer_dbell.write();

    // Enable slow and fast timers
    txs_csr->cfw_timer_glb.read();
    txs_csr->cfw_timer_glb.ftmr_enable(1);
    txs_csr->cfw_timer_glb.stmr_enable(1);
    txs_csr->cfw_timer_glb.write();

    SDK_TRACE_DEBUG("Done timer post init");
}

sdk_ret_t
capri_txs_scheduler_init (uint32_t admin_cos, asic_cfg_t *capri_cfg)
{

    cap_top_csr_t       &cap0 = g_capri_state_pd->cap_top();
    cap_txs_csr_t       &txs_csr = cap0.txs.txs;
    cap_psp_csr_t       &psp_pt_csr = cap0.pt.pt.psp,
                        &psp_pr_csr = cap0.pr.pr.psp;
    uint64_t            txs_sched_hbm_base_addr;
    uint16_t            dtdm_lo_map, dtdm_hi_map;
    uint32_t            control_cos = 0;

    txs_csr.cfw_timer_glb.read();
    txs_csr.cfw_timer_glb.ftmr_enable(0);
    txs_csr.cfw_timer_glb.stmr_enable(0);
    txs_csr.cfw_timer_glb.hbm_hw_init(0);
    txs_csr.cfw_timer_glb.sram_hw_init(0);
    txs_csr.cfw_timer_glb.show();
    txs_csr.cfw_timer_glb.write();
    txs_csr.cfw_scheduler_glb.read();
    txs_csr.cfw_scheduler_glb.enable(0);
    txs_csr.cfw_scheduler_glb.hbm_hw_init(0);
    txs_csr.cfw_scheduler_glb.sram_hw_init(0);
    txs_csr.cfw_scheduler_glb.show();
    txs_csr.cfw_scheduler_glb.write();
    txs_csr.cfg_sch.read();
    txs_csr.cfg_sch.enable(0);
    txs_csr.cfg_sch.write();

    cap_wa_csr_cfg_wa_sched_hint_t &wa_sched_hint_csr =
        cap0.db.wa.cfg_wa_sched_hint;
    wa_sched_hint_csr.read();
    /* 5 bit value: bit 0=host, 1=local, 2=32b, 3=timer, 4=arm4kremap" */
    wa_sched_hint_csr.enable_src_mask(0x0);
    wa_sched_hint_csr.write();

    txs_sched_hbm_base_addr =
        g_capri_state_pd->mempartition()->start_addr(MEM_REGION_TX_SCHEDULER_NAME);

    // Update HBM base addr.
    txs_csr.cfw_scheduler_static.read();
    txs_csr.cfw_scheduler_static.hbm_base(txs_sched_hbm_base_addr);

    // Init sram.
    txs_csr.cfw_scheduler_glb.read();
    // skip init on RTL/Model.
    SDK_ASSERT(capri_cfg);
    if (capri_cfg->platform != platform_type_t::PLATFORM_TYPE_SIM &&
        capri_cfg->platform != platform_type_t::PLATFORM_TYPE_RTL) {
        txs_csr.cfw_scheduler_glb.hbm_hw_init(1);
    }
    txs_csr.cfw_scheduler_glb.sram_hw_init(1);

    // Disabling scheduler bypass/collapse setting for QoS functionality to work.
    txs_csr.cfw_scheduler_glb.enable_set_byp(0);

    txs_csr.cfw_scheduler_static.write();
    txs_csr.cfw_scheduler_glb.write();

    //TxDMA
    psp_pt_csr.cfg_profile.read();
    psp_pt_csr.cfg_profile.collapse_enable(0);
    psp_pt_csr.cfg_profile.write();

    // RxDMA
    psp_pr_csr.cfg_profile.read();
    psp_pr_csr.cfg_profile.collapse_enable(0);
    psp_pr_csr.cfg_profile.write();


    // init timer
    capri_txs_timer_init_pre(CAPRI_TIMER_NUM_KEY_CACHE_LINES, capri_cfg);

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
        if (ret != SDK_RET_OK) {
            SDK_TRACE_ERR("Error deriving qos-class-id for admin Qos class "
                          "{} ret {}",
                          control_qos_class->key, ret);
            control_cos = 0;
        }
    } else {
        SDK_TRACE_DEBUG("control qos class not init'ed!! Setting it to default\n");
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
    psp_pt_csr.cfg_npv_values.read();
    psp_pt_csr.cfg_npv_values.tm_oq_map_enable(1);
    psp_pt_csr.cfg_npv_values.write();

    //Program one-to-one mapping from cos to tm_oq.
    for (int i = 0; i < NUM_MAX_COSES ; i++) {
        psp_pt_csr.cfg_npv_cos_to_tm_oq_map[i].tm_oq(i);
        psp_pt_csr.cfg_npv_cos_to_tm_oq_map[i].write();
    }

    // init timer post init done
    capri_txs_timer_init_post(CAPRI_TIMER_NUM_KEY_CACHE_LINES, capri_cfg);
    if(capri_cfg->completion_func) {
        capri_cfg->completion_func(sdk_status_t::SDK_STATUS_SCHEDULER_INIT_DONE);
    }

    // disable txs scheduler fifo cache as a workaround for asic bug
    // where fifo cache is out of sync with hbm scheduler table
    txs_csr.cfg_scheduler_dbg.read();
    txs_csr.cfg_scheduler_dbg.fifo_mode_thr(0);
    txs_csr.cfg_scheduler_dbg.write();

    SDK_TRACE_DEBUG("Set hbm base addr for TXS sched to 0x%lx, dtdm_lo_map 0x%lx, dtdm_hi_map 0x%lx",
                    txs_sched_hbm_base_addr, dtdm_lo_map, dtdm_hi_map);
    return SDK_RET_OK;
}

sdk_ret_t
capri_txs_scheduler_lif_params_update (uint32_t hw_lif_id,
                                       capri_txs_sched_lif_params_t *txs_hw_params)
{

    cap_top_csr_t &cap0 = g_capri_state_pd->cap_top();
    cap_txs_csr_t &txs_csr = cap0.txs.txs;
    uint32_t      i = 0, j = 0, table_offset = 0,
                  num_cos_val = 0, lif_cos_index = 0;
    uint16_t      lif_cos_bmp = 0x0;

    lif_cos_bmp = txs_hw_params->cos_bmp;
    if ((hw_lif_id >= CAPRI_TXS_MAX_TABLE_ENTRIES) ||
        (txs_hw_params->sched_table_offset >= CAPRI_TXS_MAX_TABLE_ENTRIES)) {
        SDK_TRACE_ERR("CAPRI-TXS::%s: Invalid parameters to function %u,%u",__func__, hw_lif_id,
                       txs_hw_params->sched_table_offset);
        return SDK_RET_INVALID_ARG;
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
                table_offset = txs_hw_params->sched_table_offset +
                    lif_cos_index + j;
                txs_csr.dhs_sch_grp_entry.entry[table_offset].read();
                txs_csr.dhs_sch_grp_entry.entry[table_offset].lif(hw_lif_id);
                txs_csr.dhs_sch_grp_entry.entry[table_offset].qid_offset(j);
                txs_csr.dhs_sch_grp_entry.entry[table_offset].rr_sel(i);
                txs_csr.dhs_sch_grp_entry.entry[table_offset].write();
            }
            num_cos_val++;
        }
    }

    SDK_TRACE_DEBUG("Programmed sched-table-offset %u and entries-per-cos %u"
                    "and cos-bmp 0x%lx for hw-lif-id %u",
                    txs_hw_params->sched_table_offset,
                     txs_hw_params->num_entries_per_cos, lif_cos_bmp,
                     hw_lif_id);

    return SDK_RET_OK;
}

sdk_ret_t
capri_txs_policer_lif_params_update (uint32_t hw_lif_id,
                                     capri_txs_policer_lif_params_t *txs_hw_params)
{
    cap_top_csr_t &cap0 = g_capri_state_pd->cap_top();
    cap_txs_csr_t &txs_csr = cap0.txs.txs;

    if ((hw_lif_id >= CAPRI_TXS_MAX_TABLE_ENTRIES) ||
        (txs_hw_params->sched_table_end_offset >= CAPRI_TXS_MAX_TABLE_ENTRIES)) {
        SDK_TRACE_ERR("CAPRI-TXS::%s: Invalid parameters to function %u,%u",__func__, hw_lif_id,
                       txs_hw_params->sched_table_end_offset);
        return SDK_RET_INVALID_ARG;
    }

    // Program mapping from rate-limiter-table entry (indexed by hw-lif-id) to scheduler table entries.
    // The scheduler table entries (also called Rate-limiter-group, RLG)  will be paused when rate-limiter entry goes red.
    txs_csr.dhs_sch_rlid_map_sram.entry[hw_lif_id].read();
    txs_csr.dhs_sch_rlid_map_sram.entry[hw_lif_id].sg_start(txs_hw_params->sched_table_start_offset);
    txs_csr.dhs_sch_rlid_map_sram.entry[hw_lif_id].sg_end(txs_hw_params->sched_table_end_offset);
    txs_csr.dhs_sch_rlid_map_sram.entry[hw_lif_id].write();

    SDK_TRACE_DEBUG("Programmed sched-table-start-offset %u and sched-table-end-offset %u"
                    "for hw-lif-id %u", txs_hw_params->sched_table_start_offset,
                     txs_hw_params->sched_table_end_offset, hw_lif_id);

    return SDK_RET_OK;
}

sdk_ret_t
capri_txs_scheduler_tx_alloc (capri_txs_sched_lif_params_t *tx_params,
                              uint32_t *alloc_offset, uint32_t *alloc_units)
{
    sdk_ret_t     ret = SDK_RET_OK;
    uint32_t      total_qcount = 0;

    *alloc_offset = INVALID_INDEXER_INDEX;
    *alloc_units = 0;
    // Sched table can hold 8K queues per index and mandates new index for each cos.
    total_qcount = tx_params->total_qcount;
    *alloc_units  =  (total_qcount / CAPRI_TXS_SCHEDULER_NUM_QUEUES_PER_ENTRY);
    *alloc_units +=
        ((total_qcount % CAPRI_TXS_SCHEDULER_NUM_QUEUES_PER_ENTRY) ? 1 : 0);
    *alloc_units *=   sdk::lib::count_bits_set(tx_params->cos_bmp);

    if (*alloc_units > 0) {
        //Allocate consecutive alloc_unit num of entries in sched table.
        *alloc_offset =
            g_capri_state_pd->txs_scheduler_map_idxr()->Alloc(*alloc_units);
        if (*alloc_offset < 0) {
            ret = SDK_RET_NO_RESOURCE;
        }
    }
    return ret;
}

sdk_ret_t
capri_txs_scheduler_tx_dealloc (uint32_t alloc_offset, uint32_t alloc_units)
{
    sdk_ret_t     ret = SDK_RET_OK;
    g_capri_state_pd->txs_scheduler_map_idxr()->Free(alloc_offset, alloc_units);
    return ret;
}

sdk_ret_t
capri_txs_scheduler_stats_get (capri_txs_scheduler_stats_t *scheduler_stats)
{
    cap_top_csr_t &cap0 = g_capri_state_pd->cap_top();
    cap_txs_csr_t &txs_csr = cap0.txs.txs;
    uint16_t xon_status;

    txs_csr.cnt_sch_doorbell_set.read();
    txs_csr.cnt_sch_doorbell_clr.read();
    txs_csr.cnt_sch_rlid_stop.read();
    txs_csr.cnt_sch_rlid_start.read();
    txs_csr.sta_glb.read();
    txs_csr.cnt_sch_txdma_cos0.read();
    txs_csr.cnt_sch_txdma_cos1.read();
    txs_csr.cnt_sch_txdma_cos2.read();
    txs_csr.cnt_sch_txdma_cos3.read();
    txs_csr.cnt_sch_txdma_cos4.read();
    txs_csr.cnt_sch_txdma_cos5.read();
    txs_csr.cnt_sch_txdma_cos6.read();
    txs_csr.cnt_sch_txdma_cos7.read();
    txs_csr.cnt_sch_txdma_cos8.read();
    txs_csr.cnt_sch_txdma_cos9.read();
    txs_csr.cnt_sch_txdma_cos10.read();
    txs_csr.cnt_sch_txdma_cos11.read();
    txs_csr.cnt_sch_txdma_cos12.read();
    txs_csr.cnt_sch_txdma_cos13.read();
    txs_csr.cnt_sch_txdma_cos14.read();
    txs_csr.cnt_sch_txdma_cos15.read();

    xon_status = txs_csr.sta_glb.pb_xoff().convert_to<uint16_t>();

    scheduler_stats->doorbell_set_count =
        txs_csr.cnt_sch_doorbell_set.val().convert_to<uint64_t>();
    scheduler_stats->doorbell_clear_count =
        txs_csr.cnt_sch_doorbell_clr.val().convert_to<uint64_t>();
    scheduler_stats->ratelimit_start_count =
        txs_csr.cnt_sch_rlid_stop.val().convert_to<uint32_t>();
    scheduler_stats->ratelimit_stop_count =
        txs_csr.cnt_sch_rlid_start.val().convert_to<uint32_t>();

    for (unsigned i = 0; i < SDK_ARRAY_SIZE(scheduler_stats->cos_stats); i++) {
        scheduler_stats->cos_stats[i].cos = i;
        scheduler_stats->cos_stats[i].xon_status =
                                        (xon_status >> i) & 0x1 ? true : false;
    }

    scheduler_stats->cos_stats[0].doorbell_count =
        txs_csr.cnt_sch_txdma_cos0.val().convert_to<uint64_t>();
    scheduler_stats->cos_stats[1].doorbell_count =
        txs_csr.cnt_sch_txdma_cos1.val().convert_to<uint64_t>();
    scheduler_stats->cos_stats[2].doorbell_count =
        txs_csr.cnt_sch_txdma_cos2.val().convert_to<uint64_t>();
    scheduler_stats->cos_stats[3].doorbell_count =
        txs_csr.cnt_sch_txdma_cos3.val().convert_to<uint64_t>();
    scheduler_stats->cos_stats[4].doorbell_count =
        txs_csr.cnt_sch_txdma_cos4.val().convert_to<uint64_t>();
    scheduler_stats->cos_stats[5].doorbell_count =
        txs_csr.cnt_sch_txdma_cos5.val().convert_to<uint64_t>();
    scheduler_stats->cos_stats[6].doorbell_count =
        txs_csr.cnt_sch_txdma_cos6.val().convert_to<uint64_t>();
    scheduler_stats->cos_stats[7].doorbell_count =
        txs_csr.cnt_sch_txdma_cos7.val().convert_to<uint64_t>();
    scheduler_stats->cos_stats[8].doorbell_count =
        txs_csr.cnt_sch_txdma_cos8.val().convert_to<uint64_t>();
    scheduler_stats->cos_stats[9].doorbell_count =
        txs_csr.cnt_sch_txdma_cos9.val().convert_to<uint64_t>();
    scheduler_stats->cos_stats[10].doorbell_count =
        txs_csr.cnt_sch_txdma_cos10.val().convert_to<uint64_t>();
    scheduler_stats->cos_stats[11].doorbell_count =
        txs_csr.cnt_sch_txdma_cos11.val().convert_to<uint64_t>();
    scheduler_stats->cos_stats[12].doorbell_count =
        txs_csr.cnt_sch_txdma_cos12.val().convert_to<uint64_t>();
    scheduler_stats->cos_stats[13].doorbell_count =
        txs_csr.cnt_sch_txdma_cos13.val().convert_to<uint64_t>();
    scheduler_stats->cos_stats[14].doorbell_count =
        txs_csr.cnt_sch_txdma_cos14.val().convert_to<uint64_t>();
    scheduler_stats->cos_stats[15].doorbell_count =
        txs_csr.cnt_sch_txdma_cos15.val().convert_to<uint64_t>();

    return SDK_RET_OK;
}

}    // namespace capri
}    // namespace platform
}    // namespace sdk
