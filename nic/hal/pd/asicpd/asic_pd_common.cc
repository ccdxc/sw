// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#include "sdk/thread.hpp"
#include "nic/hal/pd/asic_pd.hpp"
#include "sdk/pal.hpp"
#include "nic/hal/pd/p4pd_api.hpp"
// TODO: Need to remove capri references and use lib symbols instead
#include "nic/hal/pd/capri/capri_tbl_rw.hpp"

#define HAL_LOG_TBL_UPDATES

namespace hal {
namespace pd {

static void asicpd_copy_capri_table_info (capri_table_mem_layout_t *out,
                                          p4pd_table_mem_layout_t *in)
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
    return;
}

int asicpd_table_entry_write(uint32_t tableid,
                            uint32_t index,
                            uint8_t  *hwentry,
                            uint16_t hwentry_bit_len)
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
    asicpd_copy_capri_table_info (&cap_tbl_info, &tbl_ctx.sram_layout);
    ret = capri_table_entry_write(tableid, index,
                                  hwentry, hwentry_bit_len,
                                  cap_tbl_info, tbl_ctx.gress,
                                  tbl_ctx.is_oflow_table,
                                  oflow_parent_tbl_depth);
#ifdef HAL_LOG_TBL_UPDATES
    if (tbl_ctx.table_type == P4_TBL_TYPE_HASH || tbl_ctx.table_type == P4_TBL_TYPE_INDEX) {
        char    buffer[2048];
        memset(buffer, 0, sizeof(buffer));

        uint8_t key[128] = {0}; /* Atmost key is 64B. Assuming each
                                 * key byte has worst case byte padding
                                 */
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

}    // namespace pd
}    // namespace hal
