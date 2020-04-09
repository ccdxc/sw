// {C} Copyright 2020 Pensando Systems Inc. All rights reserved

#include "asic/port.hpp"
#include "platform/capri/capri_hbm_rw.hpp"
#include "platform/capri/capri_tbl_rw.hpp"
#include "platform/capri/capri_txs_scheduler.hpp"
#include "platform/capri/capri_qstate.hpp"
#include "platform/capri/capri_state.hpp"
#include "platform/capri/capri_mon.hpp"
#include "platform/capri/capri_tm_utils.hpp"
#include "platform/capri/capri_sw_phv.hpp"
#include "platform/capri/capri_quiesce.hpp"
#include "platform/capri/capri_pxb_pcie.hpp"
#include "platform/capri/csrint/csr_init.hpp"
#include "asic/asic.hpp"
#include "asic/pd/pd.hpp"
#include "asic/cmn/asic_state.hpp"
#include "asic/pd/pd_internal.hpp"
#include "lib/utils/time_profile.hpp"
#include "platform/utils/mpartition.hpp"
#include "platform/capri/capri_toeplitz.hpp"
#include "platform/capri/capri_pxb_pcie.hpp"
#include "include/sdk/crypto_apis.hpp"
#include "platform/capri/capri_barco_rings.hpp"
#include "platform/capri/capri_barco_sym_apis.hpp"
#include "platform/capri/capri_barco_asym_apis.hpp"
#include "third-party/asic/capri/model/cap_top/cap_top_csr.h"
// TODO: move out pipeline related code out of sdk
#if defined(APOLLO) || defined(ARTEMIS) || defined(APULU) || defined(ATHENA)
#include "gen/p4gen/p4plus_rxdma/include/p4plus_rxdma_p4pd.h"
#include "gen/p4gen/p4plus_txdma/include/p4plus_txdma_p4pd.h"
#endif

using namespace sdk::platform::capri;

namespace sdk {
namespace asic {
namespace pd {

extern bool g_mock_mode_;

void
asic_program_hbm_table_base_addr (int tableid, int stage_tableid,
                                  char *tablename, int stage,
                                  int pipe, bool hw_init)
{
    capri_program_hbm_table_base_addr(tableid, stage_tableid, tablename,
                                      stage, pipe, hw_init);
}

void
asicpd_p4_invalidate_cache (uint64_t addr, uint32_t size_in_bytes,
                            p4pd_table_cache_t cache)
{
    return p4_invalidate_cache(addr, size_in_bytes, cache);
}

bool
asicpd_p4plus_invalidate_cache (uint64_t addr, uint32_t size_in_bytes,
                                p4plus_cache_action_t action)
{
    return p4plus_invalidate_cache(addr, size_in_bytes, action);
}

uint8_t
asicpd_get_action_pc (uint32_t tableid, uint8_t actionid)
{
    return capri_get_action_pc(tableid, actionid);
}

uint8_t
asicpd_get_action_id (uint32_t tableid, uint8_t actionpc)
{
    return capri_get_action_id(tableid, actionpc);
}

int
asicpd_table_hw_entry_read (uint32_t tableid, uint32_t index, uint8_t  *hwentry,
                            uint16_t *hwentry_bit_len)
{
    int ret;
    uint32_t oflow_parent_tbl_depth = 0;
    p4pd_table_properties_t tbl_ctx;
    p4_table_mem_layout_t cap_tbl_info = {0};

    p4pd_global_table_properties_get(tableid, &tbl_ctx);
    assert(tbl_ctx.table_location != P4_TBL_LOCATION_HBM);
    if (tbl_ctx.is_oflow_table) {
        p4pd_table_properties_t ofl_tbl_parent_ctx;
        p4pd_table_properties_get(tbl_ctx.oflow_table_id, &ofl_tbl_parent_ctx);
        oflow_parent_tbl_depth = ofl_tbl_parent_ctx.tabledepth;
    }
    asicpd_copy_table_info(&cap_tbl_info, &tbl_ctx.sram_layout, &tbl_ctx);
    if (g_mock_mode_) {
        ret = asicpd_table_entry_read(tableid, index, hwentry, hwentry_bit_len);
    } else {
        ret = capri_table_hw_entry_read(tableid, index,
                                        hwentry, hwentry_bit_len,
                                        cap_tbl_info, tbl_ctx.gress,
                                        tbl_ctx.is_oflow_table,
                                        (tbl_ctx.gress == P4_GRESS_INGRESS),
                                        oflow_parent_tbl_depth);
    }

    return ret;
}

int
asicpd_tcam_table_hw_entry_read (uint32_t tableid, uint32_t index,
                                 uint8_t  *trit_x, uint8_t  *trit_y,
                                 uint16_t *hwentry_bit_len)
{
    int ret;
    p4pd_table_properties_t tbl_ctx;
    p4_table_mem_layout_t cap_tbl_info = {0};

    p4pd_global_table_properties_get(tableid, &tbl_ctx);
    assert(tbl_ctx.table_location != P4_TBL_LOCATION_HBM);
    asicpd_copy_table_info(&cap_tbl_info, &tbl_ctx.tcam_layout, &tbl_ctx);
    if (g_mock_mode_) {
        ret = asicpd_tcam_table_entry_read(tableid, index, trit_x, trit_y,
                                           hwentry_bit_len);
    } else {
        ret = capri_tcam_table_hw_entry_read(tableid, index, trit_x, trit_y,
                                             hwentry_bit_len, cap_tbl_info,
                                             (tbl_ctx.gress ==
                                              P4_GRESS_INGRESS));
    }
    return ret;
}

int
asicpd_table_entry_write (uint32_t tableid, uint32_t index, uint8_t  *hwentry,
                          uint16_t hwentry_bit_len, uint8_t  *hwentry_mask)
{
    int ret;
    uint32_t oflow_parent_tbl_depth = 0;
    p4pd_table_properties_t tbl_ctx;
    p4_table_mem_layout_t cap_tbl_info = {0};

    p4pd_global_table_properties_get(tableid, &tbl_ctx);
    assert(tbl_ctx.table_location != P4_TBL_LOCATION_HBM);
    if (tbl_ctx.is_oflow_table) {
        p4pd_table_properties_t ofl_tbl_parent_ctx;
        p4pd_table_properties_get(tbl_ctx.oflow_table_id, &ofl_tbl_parent_ctx);
        oflow_parent_tbl_depth = ofl_tbl_parent_ctx.tabledepth;
    }
    asicpd_copy_table_info(&cap_tbl_info, &tbl_ctx.sram_layout, &tbl_ctx);
    ret = capri_table_entry_write(tableid, index,
                                  hwentry, hwentry_mask, hwentry_bit_len,
                                  cap_tbl_info, tbl_ctx.gress,
                                  tbl_ctx.is_oflow_table,
                                  (tbl_ctx.gress == P4_GRESS_INGRESS),
                                  oflow_parent_tbl_depth);
#if SDK_LOG_TABLE_WRITE
    if ((tbl_ctx.table_type == P4_TBL_TYPE_HASH)     ||
        (tbl_ctx.table_type == P4_TBL_TYPE_HASHTCAM) ||
        (tbl_ctx.table_type == P4_TBL_TYPE_INDEX)) {
        char    buffer[2048];
        memset(buffer, 0, sizeof(buffer));

        uint8_t key[128] = {0}; // Atmost key is 64B. Assuming each
                                // key byte has worst case byte padding
        uint8_t keymask[128] = {0};
        uint8_t data[128] = {0};
        SDK_TRACE_DEBUG("%s", "Read last installed table entry back into table "
                        "key and action structures");
        p4pd_global_entry_read(tableid, index, (void *) key,
                               (void *) keymask, (void *)data);
        p4pd_global_table_ds_decoded_string_get(tableid, index, (void *)key,
                                                (void *)keymask, (void *)data,
                                                buffer, sizeof(buffer));
        SDK_TRACE_DEBUG("%s", buffer);
    }
#endif

    return ret;
}

sdk_ret_t
asicpd_read_table_constant (uint32_t tableid, uint64_t *value)
{
    p4pd_table_properties_t       tbl_ctx;

    p4pd_table_properties_get(tableid, &tbl_ctx);
    capri_table_constant_read(value, tbl_ctx.stage, tbl_ctx.stage_tableid,
                              (tbl_ctx.gress == P4_GRESS_INGRESS));

    return SDK_RET_OK;
}

sdk_ret_t
asicpd_program_table_constant (uint32_t tableid, uint64_t const_value)
{
    p4pd_table_properties_t tbl_ctx;

    p4pd_table_properties_get(tableid, &tbl_ctx);
    capri_table_constant_write(const_value, tbl_ctx.stage,
                               tbl_ctx.stage_tableid,
                               (tbl_ctx.gress == P4_GRESS_INGRESS));

    return SDK_RET_OK;
}

sdk_ret_t
asicpd_program_table_thread_constant (uint32_t tableid, uint8_t table_thread_id,
                                      uint64_t const_value)
{
    p4pd_table_properties_t       tbl_ctx;

    p4pd_table_properties_get(tableid, &tbl_ctx);
    if (table_thread_id < tbl_ctx.table_thread_count) {
        uint8_t tid = 0;
        if (table_thread_id != 0) {
            tid = tbl_ctx.thread_table_id[table_thread_id];
        } else {
            tid = tbl_ctx.stage_tableid;
        }
        capri_table_constant_write(const_value, tbl_ctx.stage, tid,
                                   (tbl_ctx.gress == P4_GRESS_INGRESS));
    }
    return SDK_RET_OK;
}

sdk_ret_t
asicpd_deparser_init(void)
{
    capri_deparser_init(TM_PORT_INGRESS, TM_PORT_EGRESS);
    return SDK_RET_OK;
}

int
asicpd_table_entry_read (uint32_t tableid, uint32_t index, uint8_t  *hwentry,
                         uint16_t *hwentry_bit_len)
{
    int ret;
    uint32_t oflow_parent_tbl_depth = 0;
    p4pd_table_properties_t tbl_ctx;
    p4_table_mem_layout_t cap_tbl_info = {0};

    p4pd_global_table_properties_get(tableid, &tbl_ctx);
    assert(tbl_ctx.table_location != P4_TBL_LOCATION_HBM);
    if (tbl_ctx.is_oflow_table) {
        p4pd_table_properties_t ofl_tbl_parent_ctx;
        p4pd_table_properties_get(tbl_ctx.oflow_table_id, &ofl_tbl_parent_ctx);
        oflow_parent_tbl_depth = ofl_tbl_parent_ctx.tabledepth;
    }
    asicpd_copy_table_info(&cap_tbl_info, &tbl_ctx.sram_layout, &tbl_ctx);
    ret = capri_table_entry_read(tableid, index, hwentry, hwentry_bit_len,
                                 cap_tbl_info, tbl_ctx.gress,
                                 tbl_ctx.is_oflow_table,
                                 oflow_parent_tbl_depth);
    return (ret);
}

int
asicpd_tcam_table_entry_write (uint32_t tableid, uint32_t index,
                               uint8_t  *trit_x, uint8_t *trit_y,
                               uint16_t hwentry_bit_len)
{
    int ret;
    p4pd_table_properties_t tbl_ctx;
    p4_table_mem_layout_t cap_tbl_info = {0};

    p4pd_global_table_properties_get(tableid, &tbl_ctx);
    assert(tbl_ctx.table_location != P4_TBL_LOCATION_HBM);
    asicpd_copy_table_info(&cap_tbl_info, &tbl_ctx.tcam_layout, &tbl_ctx);
    ret = capri_tcam_table_entry_write(tableid, index, trit_x, trit_y,
                                       hwentry_bit_len,
                                       cap_tbl_info, tbl_ctx.gress,
                                       (tbl_ctx.gress == P4_GRESS_INGRESS));
#if SDK_LOG_TABLE_WRITE
    if ((tbl_ctx.table_type != P4_TBL_TYPE_HASH) &&
        (tbl_ctx.table_type != P4_TBL_TYPE_INDEX)) {
        char    buffer[2048];
        memset(buffer, 0, sizeof(buffer));

        uint8_t key[128] = {0}; // Atmost key is 64B. Assuming each
                                // key byte has worst case byte padding
        uint8_t keymask[128] = {0};
        uint8_t data[128] = {0};
        SDK_TRACE_DEBUG("%s", "Read last installed table entry back into table "
                        "key and action structures");
        p4pd_global_entry_read(tableid, index, (void *) key,
                               (void *) keymask, (void *) data);
        p4pd_global_table_ds_decoded_string_get(tableid, index, (void *) key,
                                                (void *) keymask, (void *) data,
                                                buffer, sizeof(buffer));
        SDK_TRACE_DEBUG("%s", buffer);
    }
#endif

    return ret;
}

int
asicpd_tcam_table_entry_read (uint32_t tableid, uint32_t index,
                              uint8_t  *trit_x, uint8_t *trit_y,
                              uint16_t *hwentry_bit_len)
{
    int ret;
    p4pd_table_properties_t tbl_ctx;
    p4_table_mem_layout_t cap_tbl_info = {0};

    p4pd_global_table_properties_get(tableid, &tbl_ctx);
    assert(tbl_ctx.table_location != P4_TBL_LOCATION_HBM);
    asicpd_copy_table_info(&cap_tbl_info, &tbl_ctx.tcam_layout, &tbl_ctx);

    ret = capri_tcam_table_entry_read(tableid, index, trit_x, trit_y,
                                      hwentry_bit_len, cap_tbl_info,
                                      tbl_ctx.gress);
    return ret;
}

int
asicpd_hbm_table_entry_write (uint32_t tableid, uint32_t index,
                              uint8_t *hwentry, uint16_t entry_size)
{
    int ret;
    p4pd_table_properties_t tbl_ctx;

    time_profile_begin(sdk::utils::time_profile::ASICPD_HBM_TABLE_ENTRY_WRITE);
    p4pd_global_table_properties_get(tableid, &tbl_ctx);
    ret = capri_hbm_table_entry_write(tableid, index, hwentry, entry_size,
                                      tbl_ctx.hbm_layout.entry_width, &tbl_ctx);

    uint64_t entry_addr = (index * tbl_ctx.hbm_layout.entry_width);
    ret |= capri_hbm_table_entry_cache_invalidate(tbl_ctx.cache, entry_addr,
                tbl_ctx.hbm_layout.entry_width, tbl_ctx.base_mem_pa);

#if SDK_LOG_TABLE_WRITE
    char    buffer[2048];
    memset(buffer, 0, sizeof(buffer));
    uint8_t key[128] = {0}; // Atmost key is 64B. Assuming each
                            // key byte has worst case byte padding
    uint8_t keymask[128] = {0};
    uint8_t data[128] = {0};
    SDK_TRACE_DEBUG("%s", "read last installed hbm table entry back into table "
                          "key and action structures");

    p4pd_global_entry_read(tableid, index, (void *) key,
                           (void *) keymask, (void *) data);

    p4pd_global_table_ds_decoded_string_get(tableid, index,
                                            (void *) key, (void *) keymask,
                                            (void *) data, buffer,
                                            sizeof(buffer));
    SDK_TRACE_DEBUG("%s", buffer);
#endif

    time_profile_end(sdk::utils::time_profile::ASICPD_HBM_TABLE_ENTRY_WRITE);
    return ret;
}

int
asicpd_hbm_table_entry_read (uint32_t tableid, uint32_t index,
                             uint8_t *hwentry, uint16_t *entry_size)
{
    int ret;
    p4pd_table_properties_t tbl_ctx;
    p4_table_mem_layout_t cap_tbl_info = {0};

    time_profile_begin(sdk::utils::time_profile::ASICPD_HBM_TABLE_ENTRY_READ);
    p4pd_global_table_properties_get(tableid, &tbl_ctx);
    asicpd_copy_table_info(&cap_tbl_info, &tbl_ctx.hbm_layout, &tbl_ctx);
    ret = capri_hbm_table_entry_read(tableid, index, hwentry,
                                     entry_size, cap_tbl_info,
                                     tbl_ctx.read_thru_mode);
    time_profile_end(sdk::utils::time_profile::ASICPD_HBM_TABLE_ENTRY_READ);
    return ret;
}

sdk_ret_t
asicpd_p4plus_recirc_init (void)
{
    capri_p4plus_recirc_init();
    return SDK_RET_OK;
}

sdk_ret_t
asic_pd_hbm_bw_get (hbm_bw_samples_t *hbm_bw_samples)
{
    return capri_hbm_bw(hbm_bw_samples->num_samples,
                        hbm_bw_samples->sleep_interval, true,
                        hbm_bw_samples->hbm_bw);
}

sdk_ret_t
asic_pd_llc_setup (llc_counters_t *llc)
{
    return capri_nx_setup_llc_counters(llc->mask);
}

sdk_ret_t
asicpd_toeplitz_init (const char *handle, uint32_t tableid,
                      uint32_t rss_indir_tbl_entry_size)
{
    p4pd_table_properties_t tbl_ctx;

    p4pd_global_table_properties_get(tableid, &tbl_ctx);

    return capri_toeplitz_init(handle, tbl_ctx.stage, tbl_ctx.stage_tableid,
                               rss_indir_tbl_entry_size);
}

sdk_ret_t
asic_pd_llc_get (llc_counters_t *llc)
{
    return capri_nx_get_llc_counters(&llc->mask, llc->data);
}

sdk_ret_t
asic_pd_scheduler_stats_get (scheduler_stats_t *sch_stats)
{
    sdk_ret_t ret;
    capri_txs_scheduler_stats_t asic_stats = {};

    ret = capri_txs_scheduler_stats_get(&asic_stats);
    if (ret != SDK_RET_OK) {
        return ret;
    }
    if (SDK_ARRAY_SIZE(asic_stats.cos_stats) > ASIC_NUM_MAX_COSES) {
        return SDK_RET_ERR;
    }
    sch_stats->num_coses = SDK_ARRAY_SIZE(asic_stats.cos_stats);
    sch_stats->doorbell_set_count = asic_stats.doorbell_set_count;
    sch_stats->doorbell_clear_count = asic_stats.doorbell_clear_count;
    sch_stats->ratelimit_start_count = asic_stats.ratelimit_start_count;
    sch_stats->ratelimit_stop_count = asic_stats.ratelimit_stop_count;
    for (unsigned i = 0; i < SDK_ARRAY_SIZE(asic_stats.cos_stats); i++) {
        sch_stats->cos_stats[i].cos = asic_stats.cos_stats[i].cos;
        sch_stats->cos_stats[i].doorbell_count =
            asic_stats.cos_stats[i].doorbell_count;
        sch_stats->cos_stats[i].xon_status =
            asic_stats.cos_stats[i].xon_status;
    }
    return SDK_RET_OK;
}

sdk_ret_t
asicpd_qstate_map_clear (uint32_t lif_id)
{
    capri_clear_qstate_map(lif_id);
    return SDK_RET_OK;
}

sdk_ret_t
asicpd_qstate_map_write (lif_qstate_t *qstate, uint8_t enable)
{
    capri_program_qstate_map(qstate, enable);
    return SDK_RET_OK;
}

sdk_ret_t
asicpd_qstate_map_rewrite (uint32_t lif_id, uint8_t enable)
{
    capri_reprogram_qstate_map(lif_id, enable);
    return SDK_RET_OK;
}

sdk_ret_t
asicpd_qstate_map_read (lif_qstate_t *qstate)
{
    capri_read_qstate_map(qstate);
    return SDK_RET_OK;
}

sdk_ret_t
asicpd_qstate_write (uint64_t addr, const uint8_t *buf, uint32_t size)
{
    return capri_write_qstate(addr, buf, size);
}

sdk_ret_t
asicpd_qstate_read (uint64_t addr, uint8_t *buf, uint32_t size)
{
    return capri_read_qstate(addr, buf, size);
}

sdk_ret_t
asicpd_qstate_clear (lif_qstate_t *qstate)
{
    return capri_clear_qstate(qstate);
}

void
asicpd_reset_qstate_map (uint32_t lif_id)
{
    capri_reset_qstate_map(lif_id);
}

sdk_ret_t
asicpd_p4plus_invalidate_cache (mpartition_region_t *reg,
                                uint64_t q_addr, uint32_t size)
{
    p4plus_cache_action_t action = P4PLUS_CACHE_ACTION_NONE;

    if(is_region_cache_pipe_p4plus_all(reg)) {
        action = P4PLUS_CACHE_INVALIDATE_BOTH;
    } else if (is_region_cache_pipe_p4plus_rxdma(reg)) {
        action = P4PLUS_CACHE_INVALIDATE_RXDMA;
    } else if (is_region_cache_pipe_p4plus_txdma(reg)) {
        action = P4PLUS_CACHE_INVALIDATE_TXDMA;
    }

    if (action != P4PLUS_CACHE_ACTION_NONE) {
        p4plus_invalidate_cache(q_addr, size, action);
    }

    return SDK_RET_OK;
}

uint32_t
asic_pd_clock_freq_get(void)
{
    return capri_freq_get();
}

pd_adjust_perf_status_t
asic_pd_adjust_perf (int chip_id, int inst_id,
             pd_adjust_perf_index_t &idx,
             pd_adjust_perf_type_t perf_type)
{
    return (pd_adjust_perf_status_t) capri_adjust_perf(chip_id, inst_id,
                (pen_adjust_index_t&) idx, (pen_adjust_perf_type_t) perf_type);
}

void
asic_pd_set_half_clock (int chip_id, int inst_id)
{
    capri_set_half_clock(chip_id, inst_id);
}

sdk_ret_t
asic_pd_unravel_hbm_intrs (bool *iscattrip, bool *iseccerr, bool logging)
{
    return capri_unravel_hbm_intrs(iscattrip, iseccerr, logging);
}

int
asicpd_p4plus_table_init (platform_type_t platform_type,
                          int stage_apphdr, int stage_tableid_apphdr,
                          int stage_apphdr_ext, int stage_tableid_apphdr_ext,
                          int stage_apphdr_off, int stage_tableid_apphdr_off,
                          int stage_apphdr_ext_off,
                          int stage_tableid_apphdr_ext_off,
                          int stage_txdma_act, int stage_tableid_txdma_act,
                          int stage_txdma_act_ext,
                          int stage_tableid_txdma_act_ext,
                          int stage_sxdma_act, int stage_tableid_sxdma_act)
{
    return capri_p4plus_table_init(platform_type,
                                   stage_apphdr, stage_tableid_apphdr,
                                   stage_apphdr_ext, stage_tableid_apphdr_ext,
                                   stage_apphdr_off, stage_tableid_apphdr_off,
                                   stage_apphdr_ext_off,
                                   stage_tableid_apphdr_ext_off,
                                   stage_txdma_act, stage_tableid_txdma_act,
                                   stage_txdma_act_ext,
                                   stage_tableid_txdma_act_ext);
}

int
asicpd_p4plus_table_init (p4plus_prog_t *prog,
                          platform_type_t platform_type)
{
    return capri_p4plus_table_init(prog, platform_type);
}

sdk_ret_t
asicpd_tm_get_clock_tick (uint64_t *tick)
{
    return capri_tm_get_clock_tick(tick);
}

sdk_ret_t
asicpd_tm_debug_stats_get (tm_port_t port, tm_debug_stats_t *debug_stats,
                           bool reset)
{
    return capri_tm_debug_stats_get(port, debug_stats, reset);
}

sdk_ret_t
asicpd_sw_phv_init (void)
{
    return capri_sw_phv_init();
}

sdk_ret_t
asicpd_sw_phv_get (asic_swphv_type_t type, uint8_t prof_num,
                   asic_sw_phv_state_t *state)
{
    return capri_sw_phv_get(type, prof_num, state);
}

sdk_ret_t
asicpd_sw_phv_inject (asic_swphv_type_t type, uint8_t prof_num,
                      uint8_t start_idx, uint8_t num_flits, void *data)
{
    return capri_sw_phv_inject(type, prof_num, start_idx, num_flits, data);
}

uint32_t
asicpd_get_coreclk_freq (platform_type_t platform_type)
{
    return capri_get_coreclk_freq(platform_type);
}

void
asicpd_txs_timer_init_hsh_depth (uint32_t key_lines)
{
    return capri_txs_timer_init_hsh_depth(key_lines);
}

sdk_ret_t
queue_credits_get (queue_credits_get_cb_t cb, void *ctxt)
{
    return capri_queue_credits_get(cb, ctxt);
}

// called during upgrade in quiesced state
sdk_ret_t
asicpd_tbl_eng_cfg_modify (p4pd_pipeline_t pipeline, p4_tbl_eng_cfg_t *cfg,
                           uint32_t ncfgs)
{
    return capri_tbl_eng_cfg_modify(pipeline, cfg, ncfgs);
}

sdk_ret_t
asicpd_rss_tbl_eng_cfg_get (const char *handle, uint32_t tableid,
                            p4_tbl_eng_cfg_t *rss)
{
    p4pd_table_properties_t tbl_ctx;

    p4pd_global_table_properties_get(tableid, &tbl_ctx);
    rss->tableid = tableid;
    rss->stage = tbl_ctx.stage;
    rss->stage_tableid = tbl_ctx.stage_tableid;

    return capri_rss_table_base_pc_get(handle, &rss->mem_offset,
                                       &rss->asm_base);
}

void
asicpd_rss_tbl_eng_cfg_modify (p4_tbl_eng_cfg_t *rss)
{
    capri_rss_table_config(rss->stage, rss->stage_tableid, rss->mem_offset,
                           rss->asm_base);
}

sdk_ret_t
asicpd_init_hw_fifo (int fifo_num, uint64_t addr, int n, hw_fifo_prof_t *prof)
{
    return SDK_RET_OK;
}

sdk_ret_t
asicpd_get_hw_fifo_info (int fifo_num, hw_fifo_stats_t *stats)
{
    return SDK_RET_OK;
}

sdk_ret_t
asicpd_set_hw_fifo_info (int fifo_num, hw_fifo_stats_t *stats)
{
    return SDK_RET_OK;
}

inline bool
asicpd_tm_q_valid (int32_t tm_q)
{
    return (tm_q < 0) ? false : true;
}

uint64_t
asicpd_get_p4plus_table_mpu_pc (int table_id)
{
    return capri_get_p4plus_table_mpu_pc(table_id);
}

void
asicpd_program_p4plus_table_mpu_pc (int tableid, int stage_tbl_id, int stage)
{
    capri_program_p4plus_table_mpu_pc(tableid, stage_tbl_id, stage);
}

void
asicpd_program_tbl_mpu_pc (int tableid, bool gress, int stage,
                           int stage_tableid, uint64_t table_asm_err_offset,
                           uint64_t table_asm_base)
{
    capri_program_table_mpu_pc(tableid, gress, stage, stage_tableid,
                               table_asm_err_offset, table_asm_base);
}

void
asicpd_set_action_asm_base (int tableid, int actionid, uint64_t asm_base)
{
    capri_set_action_asm_base(tableid, actionid, asm_base);
}

void
asicpd_set_action_rxdma_asm_base (int tableid, int actionid, uint64_t asm_base)
{
    capri_set_action_rxdma_asm_base(tableid, actionid, asm_base);
}

void
asicpd_set_action_txdma_asm_base (int tableid, int actionid, uint64_t asm_base)
{
    capri_set_action_txdma_asm_base(tableid, actionid, asm_base);
}

void
asicpd_set_table_rxdma_asm_base (int tableid, uint64_t asm_base)
{
    capri_set_table_rxdma_asm_base(tableid, asm_base);
}

void
asicpd_set_table_txdma_asm_base (int tableid, uint64_t asm_base)
{
    capri_set_table_txdma_asm_base(tableid, asm_base);
}

uint64_t
asicpd_host_dbaddr_get (void)
{
    return capri_host_dbaddr();
}

uint64_t
asicpd_local_dbaddr_get (void)
{
    return capri_local_dbaddr();
}

uint64_t
asicpd_local_db32_addr_get (void)
{
    return capri_local_db32_addr();
}

//------------------------------------------------------------------------------
// perform all the CAPRI specific initialization
//------------------------------------------------------------------------------
sdk_ret_t
asicpd_init (asic_cfg_t *cfg)
{
    sdk_ret_t     ret;
    asic_cfg_t    capri_cfg;

    SDK_ASSERT(cfg != NULL);

    ret = sdk::asic::asic_state_init(cfg);
    SDK_ASSERT_TRACE_RETURN((ret == SDK_RET_OK), ret,
                            "asic_state_init failure, err : %d", ret);

    capri_cfg.default_config_dir = cfg->default_config_dir;
    capri_cfg.cfg_path = cfg->cfg_path;
    capri_cfg.admin_cos = cfg->admin_cos;
    capri_cfg.repl_entry_width = cfg->repl_entry_width;
    capri_cfg.catalog = cfg->catalog;
    capri_cfg.mempartition = cfg->mempartition;
    capri_cfg.p4_cache = true;
    capri_cfg.p4plus_cache = true;
    capri_cfg.llc_cache = true;
    capri_cfg.platform = cfg->platform;
    capri_cfg.num_pgm_cfgs = cfg->num_pgm_cfgs;
    capri_cfg.pgm_name = cfg->pgm_name;
    for (int i = 0; i < cfg->num_pgm_cfgs; i++) {
        capri_cfg.pgm_cfg[i].path = cfg->pgm_cfg[i].path;
    }
    capri_cfg.num_asm_cfgs = cfg->num_asm_cfgs;
    for (int i = 0; i < cfg->num_asm_cfgs; i++) {
        capri_cfg.asm_cfg[i].name = cfg->asm_cfg[i].name;
        capri_cfg.asm_cfg[i].path = cfg->asm_cfg[i].path;
        capri_cfg.asm_cfg[i].symbols_func = cfg->asm_cfg[i].symbols_func;
        capri_cfg.asm_cfg[i].base_addr = cfg->asm_cfg[i].base_addr;
        capri_cfg.asm_cfg[i].sort_func =
            cfg->asm_cfg[i].sort_func;
    }
    for (int i = 0; i < cfg->num_rings; i++) {
        sdk::platform::ring ring;
        ring.init(&cfg->ring_meta[i], cfg->mempartition);
    }

    capri_cfg.completion_func = cfg->completion_func;
    capri_cfg.device_profile = cfg->device_profile;

    //@@TODO - check and update redundant initializations within asic_state_init() and capri_init().
    return capri_init(&capri_cfg);
}

sdk_ret_t
asicpd_pgm_init (void)
{
    return capri_pgm_init();
}

sdk_ret_t
asicpd_p4plus_table_rw_init (void)
{
    return capri_p4plus_table_rw_init();
}

sdk_ret_t
asicpd_state_pd_init (asic_cfg_t *cfg)
{
    return capri_state_pd_init(cfg);
}

void
asicpd_csr_init (void)
{
    csr_init();
}

sdk_ret_t
asicpd_tm_uplink_lif_set (tm_port_t port, uint32_t lif)
{
    return capri_tm_uplink_lif_set(port, lif);
}

sdk_ret_t
asicpd_tm_enable_disable_uplink_port (tm_port_t port, bool enable)
{
    return capri_tm_enable_disable_uplink_port(port, enable);
}

sdk_ret_t
asicpd_tm_flush_uplink_port (tm_port_t port, bool enable)
{
    return capri_tm_flush_uplink_port(port, enable);
}

sdk_ret_t
asicpd_tm_write_control_uplink_port (tm_port_t port, bool enable)
{
    return capri_tm_write_control_uplink_port(port, enable);
}

sdk_ret_t
asicpd_tm_drain_uplink_port (tm_port_t port)
{
    return capri_tm_drain_uplink_port(port);
}

// Enable MPU tracing on p4plus rxdma
sdk_ret_t
asicpd_p4p_rxdma_mpu_trace_enable (uint32_t stage_id, uint32_t mpu,
                                   uint8_t  enable, uint8_t  trace_enable,
                                   uint8_t  phv_debug, uint8_t  phv_error,
                                   uint64_t watch_pc, uint64_t base_addr,
                                   uint8_t  table_key, uint8_t  instructions,
                                   uint8_t  wrap, uint8_t  reset,
                                   uint32_t buf_size)
{
    cap_top_csr_t & cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);

    SDK_TRACE_DEBUG ("RXDMA: stage {:d} mpu {:d} base_addr {:#x} enable {:d} reset {:d} wrap {:d}",
                     stage_id, mpu, base_addr, enable, reset, wrap);
    SDK_TRACE_DEBUG ("RXDMA: trace_enable {:d} phv_debug {:d} phv_error {:d} watch_pc {:d} table_key {:d}",
                     trace_enable, phv_debug, phv_error, watch_pc, table_key);
    SDK_TRACE_DEBUG ("RXDMA: instructions {:d} buf_size {:d}",
                     instructions, buf_size);

    // TODO max check on mpu and stage_id

    cap0.pcr.mpu[stage_id].trace[mpu].read();
    cap0.pcr.mpu[stage_id].trace[mpu].phv_debug(phv_debug);
    cap0.pcr.mpu[stage_id].trace[mpu].phv_error(phv_error);

    if (watch_pc != 0) {
        cap0.pcr.mpu[stage_id].trace[mpu].watch_pc(watch_pc >> 6); // TODO
        cap0.pcr.mpu[stage_id].trace[mpu].watch_enable(1);
    } else {
        cap0.pcr.mpu[stage_id].trace[mpu].watch_enable(0);
    }

    cap0.pcr.mpu[stage_id].trace[mpu].base_addr(base_addr >> 6);
    cap0.pcr.mpu[stage_id].trace[mpu].table_and_key(table_key);
    cap0.pcr.mpu[stage_id].trace[mpu].instructions(instructions);
    cap0.pcr.mpu[stage_id].trace[mpu].wrap(wrap);
    cap0.pcr.mpu[stage_id].trace[mpu].rst(reset);
    cap0.pcr.mpu[stage_id].trace[mpu].buf_size((uint32_t)log2(buf_size));
    cap0.pcr.mpu[stage_id].trace[mpu].enable(enable);
    cap0.pcr.mpu[stage_id].trace[mpu].trace_enable(trace_enable);
    cap0.pcr.mpu[stage_id].trace[mpu].write();

    return SDK_RET_OK;
}

// Enable MPU tracing on p4plus txdma
sdk_ret_t
asicpd_p4p_txdma_mpu_trace_enable (uint32_t stage_id, uint32_t mpu,
                                   uint8_t  enable, uint8_t  trace_enable,
                                   uint8_t  phv_debug, uint8_t  phv_error,
                                   uint64_t watch_pc, uint64_t base_addr,
                                   uint8_t  table_key, uint8_t  instructions,
                                   uint8_t  wrap, uint8_t  reset,
                                   uint32_t buf_size)
{
    cap_top_csr_t & cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);

    SDK_TRACE_DEBUG ("TXDMA: stage {:d} mpu {:d} base_addr {:#x} enable {:d} reset {:d} wrap {:d}",
                     stage_id, mpu, base_addr, enable, reset, wrap);
    SDK_TRACE_DEBUG ("TXDMA: trace_enable {:d} phv_debug {:d} phv_error {:d} watch_pc {:d} table_key {:d}",
                     trace_enable, phv_debug, phv_error, watch_pc, table_key);
    SDK_TRACE_DEBUG ("TXDMA: instructions {:d} buf_size {:d}",
                     instructions, buf_size);

    // TODO max check on mpu and stage_id

    cap0.pct.mpu[stage_id].trace[mpu].read();
    cap0.pct.mpu[stage_id].trace[mpu].phv_debug(phv_debug);
    cap0.pct.mpu[stage_id].trace[mpu].phv_error(phv_error);

    if (watch_pc != 0) {
        cap0.pct.mpu[stage_id].trace[mpu].watch_pc(watch_pc >> 6); // TODO
        cap0.pct.mpu[stage_id].trace[mpu].watch_enable(1);
    } else {
        cap0.pct.mpu[stage_id].trace[mpu].watch_enable(0);
    }

    cap0.pct.mpu[stage_id].trace[mpu].base_addr(base_addr >> 6);
    cap0.pct.mpu[stage_id].trace[mpu].table_and_key(table_key);
    cap0.pct.mpu[stage_id].trace[mpu].instructions(instructions);
    cap0.pct.mpu[stage_id].trace[mpu].wrap(wrap);
    cap0.pct.mpu[stage_id].trace[mpu].rst(reset);
    cap0.pct.mpu[stage_id].trace[mpu].buf_size((uint32_t)log2(buf_size));
    cap0.pct.mpu[stage_id].trace[mpu].enable(enable);
    cap0.pct.mpu[stage_id].trace[mpu].trace_enable(trace_enable);
    cap0.pct.mpu[stage_id].trace[mpu].write();

    return SDK_RET_OK;
}

// Enable MPU tracing on p4 ingress
sdk_ret_t
asicpd_p4_ingress_mpu_trace_enable (uint32_t stage_id, uint32_t mpu,
                                    uint8_t  enable, uint8_t  trace_enable,
                                    uint8_t  phv_debug, uint8_t  phv_error,
                                    uint64_t watch_pc, uint64_t base_addr,
                                    uint8_t  table_key, uint8_t  instructions,
                                    uint8_t  wrap, uint8_t  reset,
                                    uint32_t buf_size)
{
    cap_top_csr_t & cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);

    SDK_TRACE_DEBUG ("INGRESS: stage {:d} mpu {:d} base_addr {:#x} enable {:d} reset {:d} wrap {:d}",
                     stage_id, mpu, base_addr, enable, reset, wrap);
    SDK_TRACE_DEBUG ("INGRESS: trace_enable {:d} phv_debug {:d} phv_error {:d} watch_pc {:d} table_key {:d}",
                     trace_enable, phv_debug, phv_error, watch_pc, table_key);
    SDK_TRACE_DEBUG ("INGRESS: instructions {:d} buf_size {:d}",
                     instructions, buf_size);

    // TODO max check on mpu and stage_id

    cap0.sgi.mpu[stage_id].trace[mpu].read();
    cap0.sgi.mpu[stage_id].trace[mpu].phv_debug(phv_debug);
    cap0.sgi.mpu[stage_id].trace[mpu].phv_error(phv_error);

    if (watch_pc != 0) {
        cap0.sgi.mpu[stage_id].trace[mpu].watch_pc(watch_pc >> 6); // TODO
        cap0.sgi.mpu[stage_id].trace[mpu].watch_enable(1);
    } else {
        cap0.sgi.mpu[stage_id].trace[mpu].watch_enable(0);
    }

    cap0.sgi.mpu[stage_id].trace[mpu].base_addr(base_addr >> 6);
    cap0.sgi.mpu[stage_id].trace[mpu].table_and_key(table_key);
    cap0.sgi.mpu[stage_id].trace[mpu].instructions(instructions);
    cap0.sgi.mpu[stage_id].trace[mpu].wrap(wrap);
    cap0.sgi.mpu[stage_id].trace[mpu].rst(reset);
    cap0.sgi.mpu[stage_id].trace[mpu].buf_size((uint32_t)log2(buf_size));
    cap0.sgi.mpu[stage_id].trace[mpu].enable(enable);
    cap0.sgi.mpu[stage_id].trace[mpu].trace_enable(trace_enable);
    cap0.sgi.mpu[stage_id].trace[mpu].write();

    return SDK_RET_OK;
}

// Enable MPU tracing on p4 egress
sdk_ret_t
asicpd_p4_egress_mpu_trace_enable (uint32_t stage_id, uint32_t mpu,
                                   uint8_t  enable, uint8_t  trace_enable,
                                   uint8_t  phv_debug, uint8_t  phv_error,
                                   uint64_t watch_pc, uint64_t base_addr,
                                   uint8_t  table_key, uint8_t  instructions,
                                   uint8_t  wrap, uint8_t  reset,
                                   uint32_t buf_size)
{
    cap_top_csr_t & cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);

    SDK_TRACE_DEBUG ("EGRESS: stage {:d} mpu {:d} base_addr {:#x} enable {:d} reset {:d} wrap {:d}",
                     stage_id, mpu, base_addr, enable, reset, wrap);
    SDK_TRACE_DEBUG ("EGRESS: trace_enable {:d} phv_debug {:d} phv_error {:d} watch_pc {:d} table_key {:d}",
                     trace_enable, phv_debug, phv_error, watch_pc, table_key);
    SDK_TRACE_DEBUG ("EGRESS: instructions {:d} buf_size {:d}",
                     instructions, buf_size);

    // TODO max check on mpu and stage_id

    cap0.sge.mpu[stage_id].trace[mpu].read();
    cap0.sge.mpu[stage_id].trace[mpu].phv_debug(phv_debug);
    cap0.sge.mpu[stage_id].trace[mpu].phv_error(phv_error);

    if (watch_pc != 0) {
        cap0.sge.mpu[stage_id].trace[mpu].watch_pc(watch_pc >> 6); // TODO
        cap0.sge.mpu[stage_id].trace[mpu].watch_enable(1);
    } else {
        cap0.sge.mpu[stage_id].trace[mpu].watch_enable(0);
    }

    cap0.sge.mpu[stage_id].trace[mpu].base_addr(base_addr >> 6);
    cap0.sge.mpu[stage_id].trace[mpu].table_and_key(table_key);
    cap0.sge.mpu[stage_id].trace[mpu].instructions(instructions);
    cap0.sge.mpu[stage_id].trace[mpu].wrap(wrap);
    cap0.sge.mpu[stage_id].trace[mpu].rst(reset);
    cap0.sge.mpu[stage_id].trace[mpu].buf_size((uint32_t)log2(buf_size));
    cap0.sge.mpu[stage_id].trace[mpu].enable(enable);
    cap0.sge.mpu[stage_id].trace[mpu].trace_enable(trace_enable);
    cap0.sge.mpu[stage_id].trace[mpu].write();

    return SDK_RET_OK;
}

sdk_ret_t
asicpd_quiesce_start (void)
{
    return capri_quiesce_start();
}

sdk_ret_t
asicpd_quiesce_stop (void)
{
    return capri_quiesce_stop();
}

sdk_ret_t
asicpd_barco_asym_ecc_point_mul (uint16_t key_size, uint8_t *p,
                                 uint8_t *n, uint8_t *xg, uint8_t *yg,
                                 uint8_t *a, uint8_t *b, uint8_t *x1,
                                 uint8_t *y1, uint8_t *k, uint8_t *x3,
                                 uint8_t *y3)
{
    return capri_barco_asym_ecc_point_mul(key_size, p, n, xg, yg, a, b, x1,
                                          y1, k, x3, y3);
}

sdk_ret_t
asicpd_barco_asym_ecdsa_p256_setup_priv_key (uint8_t *p, uint8_t *n,
                                             uint8_t *xg, uint8_t *yg,
                                             uint8_t *a, uint8_t *b,
                                             uint8_t *da, int32_t *key_idx)
{
    return capri_barco_asym_ecdsa_p256_setup_priv_key(p, n, xg, yg,
                                                       a, b, da, key_idx);
}

sdk_ret_t
asicpd_barco_asym_ecdsa_p256_sig_gen (int32_t key_idx, uint8_t *p,
                                      uint8_t *n, uint8_t *xg,
                                      uint8_t *yg, uint8_t *a,
                                      uint8_t *b, uint8_t *da,
                                      uint8_t *k, uint8_t *h,
                                      uint8_t *r, uint8_t *s,
                                      bool async_en,
                                      const uint8_t *unique_key)
{
    return capri_barco_asym_ecdsa_p256_sig_gen(key_idx, p, n, xg, yg, a,
                                               b, da, k, h, r, s,
                                               async_en, unique_key);
}

sdk_ret_t
asicpd_barco_asym_ecdsa_p256_sig_verify (uint8_t *p, uint8_t *n,
                                         uint8_t *xg, uint8_t *yg,
                                         uint8_t *a, uint8_t *b,
                                         uint8_t *xq, uint8_t *yq,
                                         uint8_t *r, uint8_t *s,
                                         uint8_t *h, bool async_en,
                                         const uint8_t *unique_key)
{
    return capri_barco_asym_ecdsa_p256_sig_verify(p, n, xg, yg, a, b,
                                                  xq, yq, r, s, h,
                                                  async_en, unique_key);
}

sdk_ret_t
asicpd_barco_asym_rsa2k_encrypt (uint8_t *n, uint8_t *e,
                                 uint8_t *m,  uint8_t *c,
                                 bool async_en,
                                 const uint8_t *unique_key)
{
    return capri_barco_asym_rsa2k_encrypt(n, e, m, c, async_en, unique_key);
}

sdk_ret_t
asicpd_barco_asym_rsa_encrypt (uint16_t key_size, uint8_t *n,
                               uint8_t *e, uint8_t *m,  uint8_t *c,
                               bool async_en, const uint8_t *unique_key)
{
    return capri_barco_asym_rsa_encrypt(key_size, n, e, m, c, async_en,
                                        unique_key);
}

sdk_ret_t
asicpd_barco_asym_rsa2k_decrypt (uint8_t *n, uint8_t *d, uint8_t *c, uint8_t *m)
{
    return capri_barco_asym_rsa2k_decrypt(n, d, c, m);
}

sdk_ret_t
asicpd_barco_asym_rsa2k_crt_decrypt (int32_t key_idx, uint8_t *p,
                                     uint8_t *q, uint8_t *dp,
                                     uint8_t *dq, uint8_t *qinv,
                                     uint8_t *c, uint8_t *m,
                                     bool async_en,
                                     const uint8_t *unique_key)
{
    return capri_barco_asym_rsa2k_crt_decrypt(key_idx, p, q, dp, dq, qinv,
                                              c, m, async_en, unique_key);
}

sdk_ret_t
asicpd_barco_asym_rsa2k_setup_sig_gen_priv_key (uint8_t *n, uint8_t *d,
                                                int32_t *key_idx)
{
    return capri_barco_asym_rsa2k_setup_sig_gen_priv_key(n, d, key_idx);
}

sdk_ret_t
asicpd_barco_asym_rsa2k_crt_setup_decrypt_priv_key (uint8_t *p, uint8_t *q,
                                                    uint8_t *dp, uint8_t *dq,
                                                    uint8_t *qinv,
                                                    int32_t* key_idx)
{
    return capri_barco_asym_rsa2k_crt_setup_decrypt_priv_key(p, q, dp, dq,
                                                             qinv, key_idx);
}

sdk_ret_t
asicpd_barco_asym_rsa_setup_priv_key (uint16_t key_size, uint8_t *n,
                                      uint8_t *d, int32_t* key_idx)
{
    return capri_barco_asym_rsa_setup_priv_key(key_size, n, d, key_idx);
}

sdk_ret_t
asicpd_barco_asym_rsa2k_sig_gen (int32_t key_idx, uint8_t *n,
                                 uint8_t *d, uint8_t *h, uint8_t *s,
                                 bool async_en, const uint8_t *unique_key)
{
    return capri_barco_asym_rsa2k_sig_gen(key_idx, n, d, h, s, async_en,
                                          unique_key);
}

sdk_ret_t
asicpd_barco_asym_rsa_sig_gen (uint16_t key_size, int32_t key_idx,
                               uint8_t *n, uint8_t *d,
                               uint8_t *h, uint8_t *s,
                               bool async_en, const uint8_t *unique_key)
{
    return capri_barco_asym_rsa_sig_gen(key_size, key_idx, n, d, h, s,
                                        async_en, unique_key);
}

sdk_ret_t
asicpd_barco_asym_fips_rsa_sig_gen (uint16_t key_size, int32_t key_idx,
                                    uint8_t *n, uint8_t *e, uint8_t *msg,
                                    uint16_t msg_len, uint8_t *s,
                                    uint8_t hash_type, uint8_t sig_scheme,
                                    bool async_en, const uint8_t *unique_key)
{
    return capri_barco_asym_fips_rsa_sig_gen(key_size, key_idx, n, e, msg,
                                          msg_len, s,
                                          (hash_type_t) hash_type,
                                          (rsa_signature_scheme_t) sig_scheme,
                                          async_en, unique_key);
}

sdk_ret_t
asicpd_barco_asym_fips_rsa_sig_verify (uint16_t key_size, uint8_t *n,
                                       uint8_t *e, uint8_t *msg,
                                       uint16_t msg_len, uint8_t *s,
                                       uint8_t hash_type, uint8_t sig_scheme,
                                       bool async_en,
                                       const uint8_t *unique_key)
{
    return capri_barco_asym_fips_rsa_sig_verify(key_size, n, e, msg, msg_len, s,
                                          (hash_type_t) hash_type,
                                          (rsa_signature_scheme_t) sig_scheme,
                                          async_en, unique_key);
}

sdk_ret_t
asicpd_barco_asym_rsa2k_sig_verify (uint8_t *n, uint8_t *e,
                                    uint8_t *h, uint8_t *s)
{
    return capri_barco_asym_rsa2k_sig_verify(n, e, h, s);
}

// Enable MPU tracing on p4 ingress
sdk_ret_t
asicpd_dpp_int_credit (uint32_t instance, uint32_t value)
{
    cap_top_csr_t & cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);

    SDK_TRACE_DEBUG ("INGRESS: instance {:d} value {:d}", instance, value);

    cap0.dpp.dpp[instance].int_credit.int_test_set.ptr_credit_ovflow_interrupt(value);
    cap0.dpp.dpp[instance].int_credit.int_test_set.ptr_credit_undflow_interrupt(value);
    cap0.dpp.dpp[instance].int_credit.int_test_set.pkt_credit_ovflow_interrupt(value);
    cap0.dpp.dpp[instance].int_credit.int_test_set.pkt_credit_undflow_interrupt(value);
    cap0.dpp.dpp[instance].int_credit.int_test_set.framer_credit_ovflow_interrupt(value);
    cap0.dpp.dpp[instance].int_credit.int_test_set.framer_credit_undflow_interrupt(value);
    cap0.dpp.dpp[instance].int_credit.int_test_set.framer_hdrfld_vld_ovfl_interrupt(value);
    cap0.dpp.dpp[instance].int_credit.int_test_set.framer_hdrfld_offset_ovfl_interrupt(value);
    cap0.dpp.dpp[instance].int_credit.int_test_set.err_framer_hdrsize_zero_ovfl_interrupt(value);
    cap0.dpp.dpp[instance].int_credit.write();

    return SDK_RET_OK;
}

sdk_ret_t
asicpd_pxb_cfg_lif_bdf (uint32_t lif, uint16_t bdf)
{
    return capri_pxb_cfg_lif_bdf(lif, bdf);
}

sdk_ret_t
asicpd_barco_asym_req_descr_get (uint32_t slot_index, void *asym_req_descr)
{
    return capri_barco_asym_req_descr_get(slot_index,
                                          (barco_asym_descr_t *)asym_req_descr);
}

sdk_ret_t
asicpd_barco_symm_req_descr_get (uint8_t ring_type, uint32_t slot_index,
                                 void *symm_req_descr)
{
    return capri_barco_symm_req_descr_get((barco_rings_t) ring_type, slot_index,
                                          (barco_symm_descr_t *)symm_req_descr);
}

sdk_ret_t
asicpd_barco_ring_meta_get (uint8_t ring_type, uint32_t *pi, uint32_t *ci)
{
    return capri_barco_ring_meta_get((barco_rings_t) ring_type, pi, ci);
}

sdk_ret_t
asicpd_barco_get_meta_config_info (uint8_t ring_type, void *meta)
{
    return capri_barco_get_meta_config_info((barco_rings_t) ring_type,
                                            (barco_ring_meta_config_t *)meta);
}

sdk_ret_t
asicpd_barco_asym_add_pend_req (uint32_t hw_id, uint32_t sw_id)
{
    return capri_barco_asym_add_pend_req(hw_id, sw_id);
}

sdk_ret_t
asicpd_barco_asym_poll_pend_req (uint32_t batch_size, uint32_t* id_count,
                                 uint32_t *ids)
{
    return capri_barco_asym_poll_pend_req(batch_size, id_count, ids);
}

sdk_ret_t
asicpd_barco_sym_hash_process_request (uint8_t hash_type, bool generate,
                                       unsigned char *key, int key_len,
                                       unsigned char *data, int data_len,
                                       uint8_t *output_digest, int digest_len)
{
    return capri_barco_sym_hash_process_request((CryptoApiHashType) hash_type,
                                                generate, key, key_len, data,
                                                data_len, output_digest,
                                                digest_len);
}

sdk_ret_t
asicpd_barco_get_capabilities (uint8_t ring_type, bool *sw_reset_capable,
                               bool *sw_enable_capable)
{
    return capri_barco_get_capabilities((barco_rings_t) ring_type,
                                         sw_reset_capable,
                                         sw_enable_capable);
}

sdk_ret_t
asicpd_table_rw_init (asic_cfg_t *cfg)
{
    return capri_table_rw_init(cfg);
}

void
asicpd_table_constant_write (uint64_t val, uint32_t stage,
                             uint32_t stage_tableid, bool ingress)
{
    capri_table_constant_write(val, stage, stage_tableid, ingress);
}

sdk_ret_t
asicpd_tm_set_span_threshold (uint32_t span_threshold)
{
    return capri_tm_set_span_threshold(span_threshold);
}

}    // namespace pd
}    // namespace asic
}    // namespace sdk
