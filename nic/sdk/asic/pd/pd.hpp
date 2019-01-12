// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __SDK_ASIC_PD_HPP__
#define __SDK_ASIC_PD_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"

namespace sdk {
namespace asic {
namespace pd {

#define SDK_LOG_TABLE_WRITE 1

// TODO: please move this to sdk/lib/p4 later !!
typedef struct p4_table_mem_layout_ {
    uint16_t    entry_width;    /* In units of memory words.. 16b  in case of PIPE tables */
                                /* In units of bytes in case of HBM table */
    uint16_t    entry_width_bits;
    uint32_t    start_index;
    uint32_t    end_index;
    uint16_t    top_left_x;
    uint16_t    top_left_y;
    uint8_t     top_left_block;
    uint16_t    btm_right_x;
    uint16_t    btm_right_y;
    uint8_t     btm_right_block;
    uint8_t     num_buckets;
    uint32_t    tabledepth;
    char        *tablename;
} p4_table_mem_layout_t;

sdk_ret_t asicpd_program_table_constant(uint32_t tableid, uint64_t const_value);
sdk_ret_t asicpd_p4plus_table_mpu_base_init(p4pd_cfg_t *p4pd_cfg);
sdk_ret_t asicpd_program_table_mpu_pc(void);
sdk_ret_t asicpd_table_mpu_base_init(p4pd_cfg_t *p4pd_cfg);
sdk_ret_t asicpd_program_table_mpu_pc(void);
sdk_ret_t asicpd_deparser_init(void);
sdk_ret_t asicpd_program_hbm_table_base_addr(void);
uint8_t asicpd_get_action_pc(uint32_t tableid, uint8_t actionid);
uint8_t asicpd_get_action_id(uint32_t tableid, uint8_t actionpc);
int asicpd_table_entry_write(uint32_t tableid, uint32_t index,
                             uint8_t  *hwentry, uint16_t hwentry_bit_len,
                             uint8_t  *hwentry_mask);
int asicpd_table_entry_read(uint32_t tableid, uint32_t index,
                            uint8_t  *hwentry, uint16_t *hwentry_bit_len);
int asicpd_tcam_table_entry_write(uint32_t tableid, uint32_t index,
                                  uint8_t  *trit_x, uint8_t  *trit_y,
                                  uint16_t hwentry_bit_len);
int asicpd_tcam_table_entry_read(uint32_t tableid, uint32_t index,
                                 uint8_t  *trit_x, uint8_t  *trit_y,
                                 uint16_t *hwentry_bit_len);
int asicpd_hbm_table_entry_write (uint32_t tableid, uint32_t index,
                                  uint8_t *hwentry, uint16_t entry_size);
int asicpd_hbm_table_entry_write (uint32_t tableid, uint32_t index,
                                  uint8_t *hwentry, uint16_t *entry_size);
int asicpd_table_hw_entry_read(uint32_t tableid, uint32_t index,
                               uint8_t  *hwentry, uint16_t *hwentry_bit_len);
int asicpd_tcam_table_hw_entry_read(uint32_t tableid, uint32_t index,
                                    uint8_t  *trit_x, uint8_t  *trit_y,
                                    uint16_t *hwentry_bit_len);
void asicpd_copy_capri_table_info(p4_table_mem_layout_t *out,
                                  p4pd_table_mem_layout_t *in,
                                  p4pd_table_properties_t *tbl_ctx);
int asicpd_hbm_table_entry_read(uint32_t tableid, uint32_t index,
                                uint8_t *hwentry, uint16_t *entry_size);
int asicpd_hbm_table_entry_write(uint32_t tableid, uint32_t index,
                                 uint8_t *hwentry, uint16_t entry_size);

}    // namespace pd
}    // namespace asic
}    // namespace sdk

using sdk::asic::pd::p4_table_mem_layout_t;

#endif    // __SDK_ASIC_PD_HPP__
