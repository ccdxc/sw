// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#include "nic/sdk/include/sdk/base.hpp"
#include "lib/thread/thread.hpp"
#include "nic/hal/pd/asic_pd.hpp"
#include "nic/sdk/include/sdk/pal.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "gen/p4gen/common_rxdma_actions/include/common_rxdma_actions_p4pd.h"
#include "gen/p4gen/common_txdma_actions/include/common_txdma_actions_p4pd.h"
#include "gen/p4gen/common_rxdma_actions/include/common_rxdma_actions_p4pd_table.h"
#include "gen/p4gen/common_txdma_actions/include/common_txdma_actions_p4pd_table.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/hal/pd/asicpd/asic_pd_common.hpp"
#include "platform/capri/capri_txs_scheduler.hpp"
#include "platform/capri/capri_tbl_rw.hpp"
#include "nic/hal/pd/capri/capri_sw_phv.hpp"
#include "nic/sdk/platform/drivers/xcvr.hpp"

using namespace sdk::platform::capri;

namespace hal {
namespace pd {

// Store base address for the table
static uint64_t capri_table_asm_err_offset[P4TBL_ID_MAX];
static uint64_t capri_table_asm_base[P4TBL_ID_MAX];

static void
asicpd_copy_capri_table_info (capri_table_mem_layout_t *out,
                              p4pd_table_mem_layout_t *in,
                              p4pd_table_properties_t *tbl_ctx)
{
    out->entry_width = in->entry_width;
    out->entry_width_bits = in->entry_width_bits;
    out->start_index = in->start_index;
    out->end_index = in->end_index;
    out->top_left_x = in->top_left_x;
    out->top_left_y = in->top_left_y;
    out->top_left_block = in->top_left_block;
    out->btm_right_x = in->btm_right_x;
    out->btm_right_y = in->btm_right_y;
    out->btm_right_block = in->btm_right_block;
    out->num_buckets = in->num_buckets;
    out->tabledepth = tbl_ctx->tabledepth;
    out->tablename = tbl_ctx->tablename;
    return;
}

int
asicpd_table_entry_write (uint32_t tableid,
                          uint32_t index,
                          uint8_t  *hwentry,
                          uint16_t hwentry_bit_len,
                          uint8_t  *hwentry_mask)
{
    int ret;
    uint32_t oflow_parent_tbl_depth = 0;
    p4pd_table_properties_t tbl_ctx;
    capri_table_mem_layout_t cap_tbl_info = {0};


    p4pd_global_table_properties_get(tableid, &tbl_ctx);
    assert(tbl_ctx.table_location != P4_TBL_LOCATION_HBM);
    if (tbl_ctx.is_oflow_table) {
        p4pd_table_properties_t ofl_tbl_parent_ctx;
        p4pd_table_properties_get(tbl_ctx.oflow_table_id, &ofl_tbl_parent_ctx);
        oflow_parent_tbl_depth = ofl_tbl_parent_ctx.tabledepth;
    }
    asicpd_copy_capri_table_info(&cap_tbl_info, &tbl_ctx.sram_layout, &tbl_ctx);
    ret = capri_table_entry_write(tableid, index,
                                  hwentry, hwentry_mask, hwentry_bit_len,
                                  cap_tbl_info, tbl_ctx.gress,
                                  tbl_ctx.is_oflow_table,
                                  (tbl_ctx.gress == P4_GRESS_INGRESS),
                                  oflow_parent_tbl_depth);
#if HAL_LOG_TBL_UPDATES
    if (tbl_ctx.table_type == P4_TBL_TYPE_HASH || tbl_ctx.table_type == P4_TBL_TYPE_HASHTCAM ||
        tbl_ctx.table_type == P4_TBL_TYPE_INDEX) {
        char    buffer[2048];
        memset(buffer, 0, sizeof(buffer));

        uint8_t key[128] = {0}; // Atmost key is 64B. Assuming each
                                // key byte has worst case byte padding
        uint8_t keymask[128] = {0};
        uint8_t data[128] = {0};
        HAL_TRACE_DEBUG("{}", "Read last installed table entry back into table key and action structures");
        p4pd_global_entry_read(tableid, index, (void*)key, (void*)keymask, (void*)data);
        p4pd_global_table_ds_decoded_string_get(tableid, index, (void*)key, (void*)keymask,
                                                (void*)data, buffer, sizeof(buffer));
        HAL_TRACE_DEBUG("{}", buffer);
    }
#endif
    return (ret);
}

int
asicpd_table_entry_read (uint32_t tableid,
                         uint32_t index,
                         uint8_t  *hwentry,
                         uint16_t *hwentry_bit_len)
{
    int ret;
    uint32_t oflow_parent_tbl_depth = 0;
    p4pd_table_properties_t tbl_ctx;
    capri_table_mem_layout_t cap_tbl_info = {0};


    p4pd_global_table_properties_get(tableid, &tbl_ctx);
    assert(tbl_ctx.table_location != P4_TBL_LOCATION_HBM);
    if (tbl_ctx.is_oflow_table) {
        p4pd_table_properties_t ofl_tbl_parent_ctx;
        p4pd_table_properties_get(tbl_ctx.oflow_table_id, &ofl_tbl_parent_ctx);
        oflow_parent_tbl_depth = ofl_tbl_parent_ctx.tabledepth;
    }
    asicpd_copy_capri_table_info(&cap_tbl_info, &tbl_ctx.sram_layout, &tbl_ctx);
    ret = capri_table_entry_read(tableid, index,
                                 hwentry, hwentry_bit_len,
                                 cap_tbl_info, tbl_ctx.gress,
                                 tbl_ctx.is_oflow_table,
                                 oflow_parent_tbl_depth);
    return (ret);
}

int
asicpd_table_hw_entry_read (uint32_t tableid,
                            uint32_t index,
                            uint8_t  *hwentry,
                            uint16_t *hwentry_bit_len)
{
    int ret;
    uint32_t oflow_parent_tbl_depth = 0;
    p4pd_table_properties_t tbl_ctx;
    capri_table_mem_layout_t cap_tbl_info = {0};

    p4pd_global_table_properties_get(tableid, &tbl_ctx);
    assert(tbl_ctx.table_location != P4_TBL_LOCATION_HBM);
    if (tbl_ctx.is_oflow_table) {
        p4pd_table_properties_t ofl_tbl_parent_ctx;
        p4pd_table_properties_get(tbl_ctx.oflow_table_id, &ofl_tbl_parent_ctx);
        oflow_parent_tbl_depth = ofl_tbl_parent_ctx.tabledepth;
    }
    asicpd_copy_capri_table_info(&cap_tbl_info, &tbl_ctx.sram_layout, &tbl_ctx);
    ret = capri_table_hw_entry_read(tableid, index,
                                    hwentry, hwentry_bit_len,
                                    cap_tbl_info, tbl_ctx.gress,
                                    tbl_ctx.is_oflow_table,
                                    (tbl_ctx.gress == P4_GRESS_INGRESS),
                                    oflow_parent_tbl_depth);
    return (ret);
}

int
asicpd_tcam_table_entry_write (uint32_t tableid,
                               uint32_t index,
                               uint8_t  *trit_x,
                               uint8_t  *trit_y,
                               uint16_t hwentry_bit_len)
{
    int ret;
    p4pd_table_properties_t tbl_ctx;
    capri_table_mem_layout_t cap_tbl_info = {0};

    p4pd_global_table_properties_get(tableid, &tbl_ctx);
    assert(tbl_ctx.table_location != P4_TBL_LOCATION_HBM);
    asicpd_copy_capri_table_info(&cap_tbl_info, &tbl_ctx.tcam_layout, &tbl_ctx);
    ret = capri_tcam_table_entry_write(tableid, index, trit_x, trit_y,
                                       hwentry_bit_len,
                                       cap_tbl_info, tbl_ctx.gress,
                                       (tbl_ctx.gress == P4_GRESS_INGRESS));
#if HAL_LOG_TBL_UPDATES
    if (tbl_ctx.table_type != P4_TBL_TYPE_HASH && tbl_ctx.table_type != P4_TBL_TYPE_INDEX) {
        char    buffer[2048];
        memset(buffer, 0, sizeof(buffer));

        uint8_t key[128] = {0}; // Atmost key is 64B. Assuming each
                                // key byte has worst case byte padding
        uint8_t keymask[128] = {0};
        uint8_t data[128] = {0};
        HAL_TRACE_DEBUG("{}", "Read last installed table entry back into table key and action structures");
        p4pd_global_entry_read(tableid, index, (void*)key, (void*)keymask, (void*)data);

        p4pd_global_table_ds_decoded_string_get(tableid, index, (void*)key, (void*)keymask,
                                                (void*)data, buffer, sizeof(buffer));
        HAL_TRACE_DEBUG("{}", buffer);
    }
#endif

    return (ret);
}

int
asicpd_tcam_table_entry_read (uint32_t tableid,
                              uint32_t index,
                              uint8_t  *trit_x,
                              uint8_t  *trit_y,
                              uint16_t *hwentry_bit_len)
{
    int ret;
    p4pd_table_properties_t tbl_ctx;
    capri_table_mem_layout_t cap_tbl_info = {0};

    p4pd_global_table_properties_get(tableid, &tbl_ctx);
    assert(tbl_ctx.table_location != P4_TBL_LOCATION_HBM);
    asicpd_copy_capri_table_info(&cap_tbl_info, &tbl_ctx.tcam_layout, &tbl_ctx);
    ret = capri_tcam_table_entry_read(tableid, index, trit_x, trit_y,
                                      hwentry_bit_len, cap_tbl_info,
                                      tbl_ctx.gress);
    return (ret);
}

int
asicpd_tcam_table_hw_entry_read (uint32_t tableid,
                                 uint32_t index,
                                 uint8_t  *trit_x,
                                 uint8_t  *trit_y,
                                 uint16_t *hwentry_bit_len)
{
    int ret;
    p4pd_table_properties_t tbl_ctx;
    capri_table_mem_layout_t cap_tbl_info = {0};

    p4pd_global_table_properties_get(tableid, &tbl_ctx);
    assert(tbl_ctx.table_location != P4_TBL_LOCATION_HBM);
    asicpd_copy_capri_table_info(&cap_tbl_info, &tbl_ctx.tcam_layout, &tbl_ctx);
    ret = capri_tcam_table_hw_entry_read(tableid, index, trit_x, trit_y,
                                         hwentry_bit_len, cap_tbl_info,
                                         (tbl_ctx.gress == P4_GRESS_INGRESS));
    return (ret);
}

int
asicpd_hbm_table_entry_write (uint32_t tableid,
                              uint32_t index,
                              uint8_t *hwentry,
                              uint16_t entry_size)
{
    int ret;
    p4pd_table_properties_t tbl_ctx;
    capri_table_mem_layout_t cap_tbl_info = {0};

    p4pd_global_table_properties_get(tableid, &tbl_ctx);
    asicpd_copy_capri_table_info(&cap_tbl_info, &tbl_ctx.hbm_layout, &tbl_ctx);
    ret = capri_hbm_table_entry_write(tableid, index, hwentry,
                                      entry_size, cap_tbl_info);
    uint64_t    entry_addr = (index * cap_tbl_info.entry_width);
    ret |= capri_hbm_table_entry_cache_invalidate(tbl_ctx.gress == P4_GRESS_INGRESS, entry_addr, cap_tbl_info);
#if HAL_LOG_TBL_UPDATES
    char    buffer[2048];
    memset(buffer, 0, sizeof(buffer));
    uint8_t key[128] = {0}; // Atmost key is 64B. Assuming each
                            // key byte has worst case byte padding
    uint8_t keymask[128] = {0};
    uint8_t data[128] = {0};
    HAL_TRACE_DEBUG("{}", "Read last installed hbm table entry back into table key and action structures");
    p4pd_global_entry_read(tableid, index, (void*)key, (void*)keymask, (void*)data);

    p4pd_global_table_ds_decoded_string_get(tableid, index, (void*)key, (void*)keymask,
                                            (void*)data, buffer, sizeof(buffer));
    HAL_TRACE_DEBUG("{}", buffer);
#endif
    return (ret);
}

int
asicpd_hbm_table_entry_read (uint32_t tableid,
                             uint32_t index,
                             uint8_t *hwentry,
                             uint16_t *entry_size)
{
    int ret;
    p4pd_table_properties_t tbl_ctx;
    capri_table_mem_layout_t cap_tbl_info = {0};

    p4pd_global_table_properties_get(tableid, &tbl_ctx);
    asicpd_copy_capri_table_info(&cap_tbl_info, &tbl_ctx.hbm_layout, &tbl_ctx);
    ret = capri_hbm_table_entry_read(tableid, index, hwentry,
                                     entry_size, cap_tbl_info);
    return (ret);

}

uint8_t
asicpd_get_action_pc (uint32_t tableid, uint8_t actionid)
{
    return (capri_get_action_pc(tableid, actionid));
}

uint8_t
asicpd_get_action_id (uint32_t tableid, uint8_t actionpc)
{
    return (capri_get_action_id(tableid, actionpc));
}

hal_ret_t
asicpd_toeplitz_init (void)
{
    p4pd_table_properties_t tbl_ctx;
    p4pd_global_table_properties_get(P4_COMMON_RXDMA_ACTIONS_TBL_ID_ETH_RX_RSS_INDIR,
                                     &tbl_ctx);
    capri_toeplitz_init(tbl_ctx.stage, tbl_ctx.stage_tableid);
    return HAL_RET_OK;
}

hal_ret_t
asicpd_p4plus_table_init (hal::hal_cfg_t *hal_cfg)
{
    p4pd_table_properties_t tbl_ctx_apphdr;
    p4pd_table_properties_t tbl_ctx_apphdr_ext;
    p4pd_table_properties_t tbl_ctx_apphdr_off;
    p4pd_table_properties_t tbl_ctx_apphdr_ext_off;
    p4pd_table_properties_t tbl_ctx_txdma_act;
    p4pd_table_properties_t tbl_ctx_txdma_act_ext;

    // P4 plus table inits
    p4pd_global_table_properties_get(P4_COMMON_RXDMA_ACTIONS_TBL_ID_COMMON_P4PLUS_STAGE0_APP_HEADER_TABLE,
                                     &tbl_ctx_apphdr);
    p4pd_global_table_properties_get(P4_COMMON_RXDMA_ACTIONS_TBL_ID_COMMON_P4PLUS_STAGE0_APP_HEADER_EXT_TABLE,
                                     &tbl_ctx_apphdr_ext);
    p4pd_global_table_properties_get(P4_COMMON_RXDMA_ACTIONS_TBL_ID_COMMON_P4PLUS_STAGE0_APP_HEADER_TABLE_OFFSET_64,
                                     &tbl_ctx_apphdr_off);
    p4pd_global_table_properties_get(P4_COMMON_RXDMA_ACTIONS_TBL_ID_COMMON_P4PLUS_STAGE0_APP_HEADER_TABLE_EXT_OFFSET_64,
                                     &tbl_ctx_apphdr_ext_off);
    p4pd_global_table_properties_get(P4_COMMON_TXDMA_ACTIONS_TBL_ID_TX_TABLE_S0_T0,
                                     &tbl_ctx_txdma_act);
    p4pd_global_table_properties_get(P4_COMMON_TXDMA_ACTIONS_TBL_ID_TX_TABLE_S0_T1,
                                     &tbl_ctx_txdma_act_ext);
    capri_p4plus_table_init(hal_cfg->platform,
                            tbl_ctx_apphdr.stage,
                            tbl_ctx_apphdr.stage_tableid,
                            tbl_ctx_apphdr_ext.stage,
                            tbl_ctx_apphdr_ext.stage_tableid,
                            tbl_ctx_apphdr_off.stage,
                            tbl_ctx_apphdr_off.stage_tableid,
                            tbl_ctx_apphdr_ext_off.stage,
                            tbl_ctx_apphdr_ext_off.stage_tableid,
                            tbl_ctx_txdma_act.stage,
                            tbl_ctx_txdma_act.stage_tableid,
                            tbl_ctx_txdma_act_ext.stage,
                            tbl_ctx_txdma_act_ext.stage_tableid);
    return HAL_RET_OK;
}

hal_ret_t
asicpd_p4plus_recirc_init (void)
{
    capri_p4plus_recirc_init();
    return HAL_RET_OK;
}

sdk_ret_t
asicpd_program_table_mpu_pc (void)
{
    p4pd_table_properties_t       tbl_ctx;

    for (uint32_t i = p4pd_tableid_min_get(); i < p4pd_tableid_max_get(); i++) {
        p4pd_table_properties_get(i, &tbl_ctx);
        if (tbl_ctx.is_oflow_table &&
            tbl_ctx.table_type == P4_TBL_TYPE_TCAM) {
            // OTCAM and hash table share the same table id
            // so mpu_pc shouldn't be overwritten
            continue;
        }
        capri_program_table_mpu_pc(tbl_ctx.tableid,
                                   (tbl_ctx.gress == P4_GRESS_INGRESS),
                                   tbl_ctx.stage,
                                   tbl_ctx.stage_tableid,
                                   capri_table_asm_err_offset[i],
                                   capri_table_asm_base[i]);
        if (tbl_ctx.table_thread_count > 1) {
            for (int j = 1; j < tbl_ctx.table_thread_count; j++) {
                capri_program_table_mpu_pc(tbl_ctx.tableid,
                                   (tbl_ctx.gress == P4_GRESS_INGRESS),
                                   tbl_ctx.stage,
                                   tbl_ctx.thread_table_id[j],
                                   capri_table_asm_err_offset[i],
                                   capri_table_asm_base[i]);
            }
        }
    }
    return SDK_RET_OK;
}

hal_ret_t
asicpd_program_table_constant (uint32_t tableid, uint64_t const_value)
{
    p4pd_table_properties_t       tbl_ctx;

    p4pd_table_properties_get(tableid, &tbl_ctx);
    capri_table_constant_write(const_value, tbl_ctx.stage,
                               tbl_ctx.stage_tableid,
                               (tbl_ctx.gress == P4_GRESS_INGRESS));
    return HAL_RET_OK;
}

hal_ret_t
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
    return HAL_RET_OK;
}

sdk_ret_t
asicpd_p4plus_table_mpu_base_init (p4pd_cfg_t *p4pd_cfg)
{
    sdk_ret_t ret = SDK_RET_OK;
    char action_name[P4ACTION_NAME_MAX_LEN] = {0};
    char progname[P4ACTION_NAME_MAX_LEN] = {0};
    uint64_t capri_action_rxdma_asm_base;
    uint64_t capri_action_txdma_asm_base;
    uint64_t capri_table_rxdma_asm_base;
    uint64_t capri_table_txdma_asm_base;

    HAL_TRACE_DEBUG("In asicpd_p4plus_table_mpu_base_init\n");
    for (uint32_t i = p4pd_rxdma_tableid_min_get();
         i < p4pd_rxdma_tableid_max_get(); i++) {
        snprintf(progname, P4ACTION_NAME_MAX_LEN, "%s%s",
                 p4pd_rxdma_tbl_names[i], ".bin");
        ret = sdk::platform::p4_program_to_base_addr(p4pd_cfg->p4pd_rxdma_pgm_name,
                                                     progname,
                                                     &capri_table_rxdma_asm_base);
        if (ret != SDK_RET_OK) {
            continue;
        }
        capri_set_table_rxdma_asm_base(i, capri_table_rxdma_asm_base);
        for (int j = 0; j < p4pd_rxdma_get_max_action_id(i); j++) {
            p4pd_rxdma_get_action_name(i, j, action_name);
            capri_action_rxdma_asm_base = 0;
            sdk::platform::p4_program_label_to_offset(p4pd_cfg->p4pd_rxdma_pgm_name,
                                          progname, action_name,
                                          &capri_action_rxdma_asm_base);
            /* Action base is in byte and 64B aligned... */
            capri_action_rxdma_asm_base >>= 6;
            HAL_TRACE_DEBUG("Program-Name {}, Action-Name {}, Action-Pc {:#x}",
                            progname, action_name, capri_action_rxdma_asm_base);
            capri_set_action_rxdma_asm_base(i, j, capri_action_rxdma_asm_base);
        }
    }

    for (uint32_t i = p4pd_txdma_tableid_min_get();
         i < p4pd_txdma_tableid_max_get(); i++) {
        snprintf(progname, P4ACTION_NAME_MAX_LEN, "%s%s",
                 p4pd_txdma_tbl_names[i], ".bin");
        ret = sdk::platform::p4_program_to_base_addr(p4pd_cfg->p4pd_txdma_pgm_name,
                                                     progname,
                                                     &capri_table_txdma_asm_base);
        if (ret != SDK_RET_OK) {
            continue;
        }
        capri_set_table_txdma_asm_base(i, capri_table_txdma_asm_base);
        for (int j = 0; j < p4pd_txdma_get_max_action_id(i); j++) {
            p4pd_txdma_get_action_name(i, j, action_name);
            capri_action_txdma_asm_base = 0;
            sdk::platform::p4_program_label_to_offset(p4pd_cfg->p4pd_txdma_pgm_name,
                                          progname, action_name,
                                          &capri_action_txdma_asm_base);
            /* Action base is in byte and 64B aligned... */
            capri_action_txdma_asm_base>>= 6;
            HAL_TRACE_DEBUG("Program-Name {}, Action-Name {}, Action-Pc {:#x}",
                            progname, action_name, capri_action_txdma_asm_base);
            capri_set_action_txdma_asm_base(i, j, capri_action_txdma_asm_base);
        }
    }
    p4pd_table_properties_t tbl_info;
    /* P4 plus - MPU PC initialize */
    for (uint32_t i = p4pd_rxdma_tableid_min_get();
         i < p4pd_rxdma_tableid_max_get(); i++) {
        p4pd_global_table_properties_get(i, &tbl_info);
        capri_program_p4plus_sram_table_mpu_pc(i,
                                               tbl_info.stage_tableid,
                                               tbl_info.stage);
    }

    for (uint32_t i = p4pd_txdma_tableid_min_get();
         i < p4pd_txdma_tableid_max_get(); i++) {
        p4pd_global_table_properties_get(i, &tbl_info);
        capri_program_p4plus_sram_table_mpu_pc(i,
                                               tbl_info.stage_tableid,
                                               tbl_info.stage);
    }
    return SDK_RET_OK;
}

sdk_ret_t
asicpd_table_mpu_base_init (p4pd_cfg_t *p4pd_cfg)
{
    char        action_name[P4ACTION_NAME_MAX_LEN] = {0};
    char        progname[P4ACTION_NAME_MAX_LEN] = {0};
    uint64_t    capri_action_asm_base;

    HAL_TRACE_DEBUG("In asicpd_table_mpu_base_init\n");
    for (uint32_t i = p4pd_tableid_min_get(); i < p4pd_tableid_max_get(); i++) {
        snprintf(progname, P4ACTION_NAME_MAX_LEN, "%s%s", p4pd_tbl_names[i], ".bin");
        sdk::platform::p4_program_to_base_addr(p4pd_cfg->p4pd_pgm_name, progname,
                                   &capri_table_asm_base[i]);
        for (int j = 0; j < p4pd_get_max_action_id(i); j++) {
            p4pd_get_action_name(i, j, action_name);
            capri_action_asm_base = 0;
            sdk::platform::p4_program_label_to_offset(p4pd_cfg->p4pd_pgm_name, progname,
                                          action_name, &capri_action_asm_base);
            // Action base is in byte and 64B aligned...
            HAL_ASSERT((capri_action_asm_base & 0x3f) == 0);
            capri_action_asm_base >>= 6;
            HAL_TRACE_DEBUG("Program-Name {}, Action-Name {}, Action-Pc {:#x}",
                            progname, action_name, capri_action_asm_base);
            capri_set_action_asm_base(i, j, capri_action_asm_base);
        }

        // compute error program offset for each table
        snprintf(action_name, P4ACTION_NAME_MAX_LEN, "%s_error",
                 p4pd_tbl_names[i]);
        sdk::platform::p4_program_label_to_offset(p4pd_cfg->p4pd_pgm_name, progname,
                                      action_name,
                                      &capri_table_asm_err_offset[i]);
        HAL_ASSERT((capri_table_asm_err_offset[i] & 0x3f) == 0);
        capri_table_asm_err_offset[i] >>= 6;
        HAL_TRACE_DEBUG("Program-Name {}, Action-Name {}, Action-Pc {:#x}",
                        progname, action_name, capri_table_asm_err_offset[i]);
    }
    return SDK_RET_OK;
}

sdk_ret_t
asicpd_deparser_init (void)
{
    capri_deparser_init(TM_PORT_INGRESS, TM_PORT_EGRESS);
    return SDK_RET_OK;
}

sdk_ret_t
asicpd_program_hbm_table_base_addr (void)
{
    p4pd_table_properties_t       tbl_ctx;
    p4pd_pipeline_t               pipe;
    for (uint32_t i = p4pd_tableid_min_get(); i < p4pd_tableid_max_get(); i++) {
        p4pd_global_table_properties_get(i, &tbl_ctx);
        if (tbl_ctx.table_location != P4_TBL_LOCATION_HBM) {
            continue;
        }
        if (tbl_ctx.gress == P4_GRESS_INGRESS) {
            pipe = P4_PIPELINE_INGRESS;
        } else {
            pipe = P4_PIPELINE_EGRESS;
        }
        capri_program_hbm_table_base_addr(tbl_ctx.stage_tableid,
                                          tbl_ctx.tablename,
                                          tbl_ctx.stage, pipe);
        if (tbl_ctx.table_thread_count > 1) {
            for (int j = 1; j < tbl_ctx.table_thread_count; j++) {
                capri_program_hbm_table_base_addr(tbl_ctx.thread_table_id[j],
                                                  tbl_ctx.tablename,
                                                  tbl_ctx.stage, pipe);
            }
        }
    }

    for (uint32_t i = p4pd_rxdma_tableid_min_get();
         i < p4pd_rxdma_tableid_max_get(); i++) {
        p4pd_global_table_properties_get(i, &tbl_ctx);
        if (tbl_ctx.table_location != P4_TBL_LOCATION_HBM) {
            continue;
        }
        capri_program_hbm_table_base_addr(tbl_ctx.stage_tableid,
                                          tbl_ctx.tablename,
                                          tbl_ctx.stage, P4_PIPELINE_RXDMA);
    }

    for (uint32_t i = p4pd_txdma_tableid_min_get();
         i < p4pd_txdma_tableid_max_get(); i++) {
        p4pd_global_table_properties_get(i, &tbl_ctx);
        if (tbl_ctx.table_location != P4_TBL_LOCATION_HBM) {
            continue;
        }
        capri_program_hbm_table_base_addr(tbl_ctx.stage_tableid,
                                          tbl_ctx.tablename,
                                          tbl_ctx.stage, P4_PIPELINE_TXDMA);
    }

    return SDK_RET_OK;
}

hal_ret_t
asicpd_stats_addr_get (int tblid, uint32_t index,
                       asicpd_stats_region_info_t *region_arr, int arrlen,
                       hbm_addr_t *stats_addr_p)
{
    p4pd_table_properties_t       tbl_ctx;
    hbm_addr_t                    stats_base_addr;

    stats_base_addr = get_start_offset(CAPRI_HBM_REG_P4_ATOMIC_STATS);

    for (int i = 0; i < arrlen; i++) {
        p4pd_table_properties_get(region_arr[i].tblid, &tbl_ctx);
        if (tblid == region_arr[i].tblid) {
            if (index < tbl_ctx.tabledepth) {
                *stats_addr_p = stats_base_addr +
                                (index << region_arr[i].tbldepthshift);
                return HAL_RET_OK;
            } else {
                return HAL_RET_ERR;
            }
        }
        stats_base_addr += (tbl_ctx.tabledepth << region_arr[i].tbldepthshift);
    }
    return HAL_RET_OK;
}

hal_ret_t
asicpd_stats_region_init (asicpd_stats_region_info_t *region_arr, int arrlen)
{
    p4pd_table_properties_t       tbl_ctx;
    hbm_addr_t                    stats_base_addr;
    uint64_t                      stats_region_start;
    uint64_t                      stats_region_size;
    uint64_t                      bit31_base = 0;

    stats_region_start = stats_base_addr =
        get_start_offset(CAPRI_HBM_REG_P4_ATOMIC_STATS);
    stats_region_size = (get_size_kb(CAPRI_HBM_REG_P4_ATOMIC_STATS) << 10);

    // reset bit 31 (saves one ASM instruction)
    bit31_base = stats_region_start & ((uint64_t)1<<31);
    stats_region_start &= ~((uint64_t)1 << 31);
    stats_base_addr &= ~((uint64_t)1 << 31);

    for (int i = 0; i < arrlen; i++) {
        p4pd_table_properties_get(region_arr[i].tblid, &tbl_ctx);
        HAL_TRACE_DEBUG("Table {} stats base addr {:#x}",
                        tbl_ctx.tablename, stats_base_addr | bit31_base);
        capri_table_constant_write(stats_base_addr,
                                   tbl_ctx.stage, tbl_ctx.stage_tableid,
                                   (tbl_ctx.gress == P4_GRESS_INGRESS));
        stats_base_addr += (tbl_ctx.tabledepth << region_arr[i].tbldepthshift);
    }
    assert(stats_base_addr <  (stats_region_start +  stats_region_size));
    return HAL_RET_OK;
}

// asicpd_sw_phv_inject
// Inject a software phv(maintenance PHV) into asic
hal_ret_t
asicpd_sw_phv_inject (asicpd_swphv_type_t type, uint8_t prof_num,
		uint8_t start_idx, uint8_t num_flits, void *data) {
    return capri_sw_phv_inject(type, prof_num, start_idx, num_flits, data);
}

// asicpd_sw_phv_get
// get software PHV state from asic
hal_ret_t
asicpd_sw_phv_get (asicpd_swphv_type_t type, uint8_t prof_num,
	asicpd_sw_phv_state_t *state) {
    return capri_sw_phv_get(type, prof_num, state);
}

hal_ret_t
asic_pd_hbm_bw_get (pd_hbm_bw_get_args_t *hbm_bw_args)
{
    sdk_ret_t sdk_ret;
    sdk_ret = capri_hbm_bw(hbm_bw_args->num_samples,
                        hbm_bw_args->sleep_interval,
                        true,
                        hbm_bw_args->hbm_bw);
    return hal_sdk_ret_to_hal_ret(sdk_ret);
}

hal_ret_t
asic_pd_llc_setup (pd_llc_get_args_t *llc_args)
{
    sdk_ret_t sdk_ret;
    sdk_ret = capri_nx_setup_llc_counters(llc_args->mask);
    return hal_sdk_ret_to_hal_ret(sdk_ret);
}

hal_ret_t
asic_pd_llc_get (pd_llc_get_args_t *llc_args)
{
    sdk_ret_t sdk_ret;
    sdk_ret = capri_nx_get_llc_counters(llc_args->data);
    return hal_sdk_ret_to_hal_ret(sdk_ret);
}

hal_ret_t
asic_pd_scheduler_stats_get (pd_scheduler_stats_get_args_t *scheduler_stats_args)
{
#if 0
    hal_ret_t ret;
    capri_txs_scheduler_stats_t asic_stats = {};
    debug::SchedulerStatsResponse *response = scheduler_stats_args->response;
    sdk_ret_t     sdk_ret;


    sdk_ret = capri_txs_scheduler_stats_get(&asic_stats);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    response->set_doorbell_set_count(asic_stats.doorbell_set_count);
    response->set_doorbell_clear_count(asic_stats.doorbell_clear_count);
    response->set_ratelimit_start_count(asic_stats.ratelimit_start_count);
    response->set_ratelimit_stop_count(asic_stats.ratelimit_stop_count);
    for (unsigned i = 0; i < HAL_ARRAY_SIZE(asic_stats.cos_stats); i++) {
        auto cos_entry = response->add_cos_entry();
        cos_entry->set_cos(asic_stats.cos_stats[i].cos);
        cos_entry->set_doorbell_count(asic_stats.cos_stats[i].doorbell_count);
        cos_entry->set_xon_status(asic_stats.cos_stats[i].xon_status);
    }
#endif
    return HAL_RET_OK;
}

hal_ret_t
xcvr_valid_check_enable (bool enable)
{
    sdk::platform::xcvr_set_valid_check(enable);
    return HAL_RET_OK;
}

}    // namespace pd
}    // namespace hal
