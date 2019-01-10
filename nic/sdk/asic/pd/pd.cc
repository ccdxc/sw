// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/sdk/platform/capri/capri_p4.hpp"
#include "nic/sdk/platform/capri/capri_tbl_rw.hpp"
#include "nic/sdk/asic/pd/pd.hpp"

using namespace sdk::platform::capri;

namespace sdk {
namespace asic {
namespace pd {

static uint64_t capri_table_asm_base[P4TBL_ID_MAX];
static uint64_t capri_table_asm_err_offset[P4TBL_ID_MAX];

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

void
asicpd_copy_capri_table_info (p4_table_mem_layout_t *out,
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

sdk_ret_t
asicpd_program_table_constant (uint32_t tableid, uint64_t const_value)
{
    p4pd_table_properties_t       tbl_ctx;

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
asicpd_p4plus_table_mpu_base_init (p4pd_cfg_t *p4pd_cfg)
{
    sdk_ret_t ret = SDK_RET_OK;
    char action_name[P4ACTION_NAME_MAX_LEN] = {0};
    char progname[P4ACTION_NAME_MAX_LEN] = {0};
    uint64_t capri_action_rxdma_asm_base;
    uint64_t capri_action_txdma_asm_base;
    uint64_t capri_table_rxdma_asm_base;
    uint64_t capri_table_txdma_asm_base;

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
            // action base is in byte and 64B aligned
            capri_action_rxdma_asm_base >>= 6;
            SDK_TRACE_DEBUG("program name %s, action name %s, action pc 0x%x",
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
            SDK_TRACE_DEBUG("program name %s, action name %s, action pc 0x%x",
                            progname, action_name, capri_action_txdma_asm_base);
            capri_set_action_txdma_asm_base(i, j, capri_action_txdma_asm_base);
        }
    }
    p4pd_table_properties_t tbl_info;

    // P4+ MPU PC initialize
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

    for (uint32_t i = p4pd_tableid_min_get(); i < p4pd_tableid_max_get(); i++) {
        snprintf(progname, P4ACTION_NAME_MAX_LEN, "%s%s", p4pd_tbl_names[i], ".bin");
        sdk::platform::p4_program_to_base_addr(p4pd_cfg->p4pd_pgm_name, progname,
                                   &capri_table_asm_base[i]);
        for (int j = 0; j < p4pd_get_max_action_id(i); j++) {
            p4pd_get_action_name(i, j, action_name);
            capri_action_asm_base = 0;
            sdk::platform::p4_program_label_to_offset(p4pd_cfg->p4pd_pgm_name, progname,
                                          action_name, &capri_action_asm_base);
            // action base is in byte and 64B aligned
            SDK_ASSERT((capri_action_asm_base & 0x3f) == 0);
            capri_action_asm_base >>= 6;
            SDK_TRACE_DEBUG("program name %s, action name %s, action pc 0x%x",
                            progname, action_name, capri_action_asm_base);
            capri_set_action_asm_base(i, j, capri_action_asm_base);
        }

        // compute error program offset for each table
        snprintf(action_name, P4ACTION_NAME_MAX_LEN, "%s_error",
                 p4pd_tbl_names[i]);
        sdk::platform::p4_program_label_to_offset(p4pd_cfg->p4pd_pgm_name, progname,
                                      action_name,
                                      &capri_table_asm_err_offset[i]);
        SDK_ASSERT((capri_table_asm_err_offset[i] & 0x3f) == 0);
        capri_table_asm_err_offset[i] >>= 6;
        SDK_TRACE_DEBUG("program name %s, action name %s, action pc 0x%x",
                        progname, action_name, capri_table_asm_err_offset[i]);
    }
    return SDK_RET_OK;
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

int
asicpd_table_hw_entry_read (uint32_t tableid,
                            uint32_t index,
                            uint8_t  *hwentry,
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
    asicpd_copy_capri_table_info(&cap_tbl_info, &tbl_ctx.sram_layout, &tbl_ctx);
    ret = capri_table_hw_entry_read(tableid, index,
                                    hwentry, hwentry_bit_len,
                                    cap_tbl_info, tbl_ctx.gress,
                                    tbl_ctx.is_oflow_table,
                                    (tbl_ctx.gress == P4_GRESS_INGRESS),
                                    oflow_parent_tbl_depth);
    return ret;
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
    p4_table_mem_layout_t cap_tbl_info = {0};

    p4pd_global_table_properties_get(tableid, &tbl_ctx);
    assert(tbl_ctx.table_location != P4_TBL_LOCATION_HBM);
    asicpd_copy_capri_table_info(&cap_tbl_info, &tbl_ctx.tcam_layout, &tbl_ctx);
    ret = capri_tcam_table_hw_entry_read(tableid, index, trit_x, trit_y,
                                         hwentry_bit_len, cap_tbl_info,
                                         (tbl_ctx.gress == P4_GRESS_INGRESS));
    return ret;
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
    p4_table_mem_layout_t cap_tbl_info = {0};


    p4pd_global_table_properties_get(tableid, &tbl_ctx);
    assert(tbl_ctx.table_location != P4_TBL_LOCATION_HBM);
    if (tbl_ctx.is_oflow_table) {
        p4pd_table_properties_t ofl_tbl_parent_ctx;
        p4pd_table_properties_get(tbl_ctx.oflow_table_id, &ofl_tbl_parent_ctx);
        oflow_parent_tbl_depth = ofl_tbl_parent_ctx.tabledepth;
    }
    sdk::asic::pd::asicpd_copy_capri_table_info(&cap_tbl_info,
                                                &tbl_ctx.sram_layout, &tbl_ctx);
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
        p4pd_global_entry_read(tableid, index, (void*)key,
                               (void*)keymask, (void*)data);
        p4pd_global_table_ds_decoded_string_get(tableid, index, (void*)key,
                                                (void*)keymask, (void*)data,
                                                buffer, sizeof(buffer));
        SDK_TRACE_DEBUG("%s", buffer);
    }
#endif

    return ret;
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
    p4_table_mem_layout_t cap_tbl_info = {0};


    p4pd_global_table_properties_get(tableid, &tbl_ctx);
    assert(tbl_ctx.table_location != P4_TBL_LOCATION_HBM);
    if (tbl_ctx.is_oflow_table) {
        p4pd_table_properties_t ofl_tbl_parent_ctx;
        p4pd_table_properties_get(tbl_ctx.oflow_table_id, &ofl_tbl_parent_ctx);
        oflow_parent_tbl_depth = ofl_tbl_parent_ctx.tabledepth;
    }
    sdk::asic::pd::asicpd_copy_capri_table_info(&cap_tbl_info,
                                                &tbl_ctx.sram_layout, &tbl_ctx);
    ret = capri_table_entry_read(tableid, index,
                                 hwentry, hwentry_bit_len,
                                 cap_tbl_info, tbl_ctx.gress,
                                 tbl_ctx.is_oflow_table,
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
    p4_table_mem_layout_t cap_tbl_info = {0};

    p4pd_global_table_properties_get(tableid, &tbl_ctx);
    assert(tbl_ctx.table_location != P4_TBL_LOCATION_HBM);
    sdk::asic::pd::asicpd_copy_capri_table_info(&cap_tbl_info,
                                                &tbl_ctx.tcam_layout, &tbl_ctx);
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
        p4pd_global_entry_read(tableid, index, (void*)key,
                               (void*)keymask, (void*)data);
        p4pd_global_table_ds_decoded_string_get(tableid, index,
                                                (void*)key, (void*)keymask,
                                                (void*)data, buffer,
                                                sizeof(buffer));
        SDK_TRACE_DEBUG("{}", buffer);
    }
#endif

    return ret;
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
    p4_table_mem_layout_t cap_tbl_info = {0};

    p4pd_global_table_properties_get(tableid, &tbl_ctx);
    assert(tbl_ctx.table_location != P4_TBL_LOCATION_HBM);
    sdk::asic::pd::asicpd_copy_capri_table_info(&cap_tbl_info,
                                                &tbl_ctx.tcam_layout, &tbl_ctx);
    ret = capri_tcam_table_entry_read(tableid, index, trit_x, trit_y,
                                      hwentry_bit_len, cap_tbl_info,
                                      tbl_ctx.gress);
    return ret;
}

int
asicpd_hbm_table_entry_write (uint32_t tableid,
                              uint32_t index,
                              uint8_t *hwentry,
                              uint16_t entry_size)
{
    int ret;
    p4pd_table_properties_t tbl_ctx;
    p4_table_mem_layout_t cap_tbl_info = {0};

    p4pd_global_table_properties_get(tableid, &tbl_ctx);
    sdk::asic::pd::asicpd_copy_capri_table_info(&cap_tbl_info,
                                                &tbl_ctx.hbm_layout, &tbl_ctx);
    ret = capri_hbm_table_entry_write(tableid, index, hwentry,
                                      entry_size, cap_tbl_info);
    uint64_t    entry_addr = (index * cap_tbl_info.entry_width);
    ret |= capri_hbm_table_entry_cache_invalidate(tbl_ctx.gress == P4_GRESS_INGRESS,
                                                  entry_addr, cap_tbl_info);

#if SDK_LOG_TABLE_WRITE
    char    buffer[2048];
    memset(buffer, 0, sizeof(buffer));
    uint8_t key[128] = {0}; // Atmost key is 64B. Assuming each
                            // key byte has worst case byte padding
    uint8_t keymask[128] = {0};
    uint8_t data[128] = {0};
    SDK_TRACE_DEBUG("%s", "read last installed hbm table entry back into table "
                          "key and action structures");
    p4pd_global_entry_read(tableid, index,
                           (void*)key, (void*)keymask, (void*)data);
    p4pd_global_table_ds_decoded_string_get(tableid, index,
                                            (void*)key, (void*)keymask,
                                            (void*)data, buffer,
                                            sizeof(buffer));
    SDK_TRACE_DEBUG("%s", buffer);
#endif

    return ret;
}

int
asicpd_hbm_table_entry_read (uint32_t tableid,
                             uint32_t index,
                             uint8_t *hwentry,
                             uint16_t *entry_size)
{
    int ret;
    p4pd_table_properties_t tbl_ctx;
    p4_table_mem_layout_t cap_tbl_info = {0};

    p4pd_global_table_properties_get(tableid, &tbl_ctx);
    sdk::asic::pd::asicpd_copy_capri_table_info(&cap_tbl_info,
                                                &tbl_ctx.hbm_layout, &tbl_ctx);
    ret = capri_hbm_table_entry_read(tableid, index, hwentry,
                                     entry_size, cap_tbl_info);
    return ret;
}


}    // namespace pd
}    // namespace asic
}    // namespace sdk
