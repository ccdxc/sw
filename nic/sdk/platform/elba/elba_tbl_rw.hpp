// {C} Copyright 2020 Pensando Systems Inc. All rights reserved

#ifndef __ELBA_TBL_RW_HPP__
#define __ELBA_TBL_RW_HPP__

#include <stdio.h>
#include <string>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include "include/sdk/base.hpp"
#include "asic/asic.hpp"
#include "asic/cmn/asic_cfg.hpp"

using namespace sdk::asic;

namespace sdk {
namespace platform {
namespace elba {

#define ELBA_P4_NUM_STAGES           6
#define ELBA_P4PLUS_NUM_STAGES       8
#define ELBA_P4PLUS_SXDMA_NUM_STAGES 4

sdk_ret_t elba_table_rw_init(asic_cfg_t *elba_cfg);
sdk_ret_t elba_p4plus_table_rw_init(void);

void elba_table_rw_cleanup(void);

extern "C" sdk_ret_t elba_table_entry_write(uint32_t tableid,
                                            uint32_t index,
                                            uint8_t  *hwentry,
                                            uint8_t  *hwentry_mask,
                                            uint16_t hwentry_bit_len,
                                            p4_table_mem_layout_t &tbl_info,
                                            int gress,
                                            bool is_oflow_table,
                                            bool ingress,
                                            uint32_t ofl_parent_tbl_depth);

extern "C" sdk_ret_t elba_table_entry_read(uint32_t tableid,
                                           uint32_t index,
                                           uint8_t  *hwentry,
                                           uint16_t *hwentry_bit_len,
                                           p4_table_mem_layout_t &tbl_info,
                                           int gress,
                                           bool is_oflow_table,
                                           uint32_t ofl_parent_tbl_depth);

extern "C" sdk_ret_t elba_table_hw_entry_read(uint32_t tableid,
                                              uint32_t index,
                                              uint8_t  *hwentry,
                                              uint16_t *hwentry_bit_len,
                                              p4_table_mem_layout_t &tbl_info,
                                              int gress,
                                              bool is_oflow_table, bool ingress,
                                              uint32_t ofl_parent_tbl_depth);

extern "C" sdk_ret_t elba_tcam_table_entry_write (uint32_t tableid,
                                                  uint32_t index,
                                                  uint8_t  *trit_x,
                                                  uint8_t  *trit_y,
                                                  uint16_t hwentry_bit_len,
                                                  p4_table_mem_layout_t &tbl_info,
                                                  int gress, bool ingress);

extern "C" sdk_ret_t elba_tcam_table_entry_read(uint32_t tableid,
                                                uint32_t index,
                                                uint8_t  *trit_x,
                                                uint8_t  *trit_y,
                                                uint16_t *hwentry_bit_len,
                                                p4_table_mem_layout_t &tbl_info,
                                                int gress);

extern "C" sdk_ret_t elba_tcam_table_hw_entry_read(uint32_t tableid,
                                                   uint32_t index,
                                                   uint8_t  *trit_x,
                                                   uint8_t  *trit_y,
                                                   uint16_t *hwentry_bit_len,
                                                   p4_table_mem_layout_t &tbl_info,
                                                   bool ingress);

extern "C" sdk_ret_t elba_hbm_table_entry_write(uint32_t tableid,
                                                uint32_t index,
                                                uint8_t *hwentry,
                                                uint16_t entry_size,
                                                p4_table_mem_layout_t &tbl_info);

extern "C" sdk_ret_t elba_hbm_table_entry_cache_invalidate (bool ingress,
                                                            uint64_t entry_addr,
                                                            p4_table_mem_layout_t &tbl_info);

extern "C" sdk_ret_t elba_hbm_table_entry_read(uint32_t tableid,
                                               uint32_t index,
                                               uint8_t *hwentry,
                                               uint16_t *entry_size,
                                               p4_table_mem_layout_t &tbl_info);

extern "C" sdk_ret_t elba_table_constant_write(uint64_t val, uint32_t stage,
                                               uint32_t stage_tableid,
                                               bool ingress);

extern "C" sdk_ret_t elba_table_constant_read(uint64_t *val, uint32_t stage,
                                              int stage_tableid, bool ingress);

extern "C" void elba_set_action_asm_base(int tableid, int actionid,
                                         uint64_t asm_base);

extern "C" void elba_set_action_rxdma_asm_base(int tableid, int actionid,
                                               uint64_t asm_base);

extern "C" void elba_set_action_txdma_asm_base(int tableid, int actionid,
                                               uint64_t asm_base);

extern "C" void elba_set_table_rxdma_asm_base(int tableid, uint64_t asm_base);

extern "C" void elba_set_table_txdma_asm_base(int tableid, uint64_t asm_base);

extern "C" void elba_program_p4plus_sram_table_mpu_pc(int tableid,
                                                      int stage_tbl_id,
                                                      int stage);

extern "C" void elba_program_table_mpu_pc(int tableid, bool gress, int stage,
                                          int stage_tableid,
                                          uint64_t elba_table_asm_err_offset,
                                          uint64_t elba_table_asm_base);

extern "C" sdk_ret_t elba_toeplitz_init(const char *handle,
                                        int stage,
                                        int stage_tableid);

extern "C" sdk_ret_t elba_p4plus_table_init(platform_type_t platform_type,
                                            int stage_apphdr,
                                            int stage_tableid_apphdr,
                                            int stage_apphdr_ext,
                                            int stage_tableid_apphdr_ext,
                                            int stage_apphdr_off,
                                            int stage_tableid_apphdr_off,
                                            int stage_apphdr_ext_off,
                                            int stage_tableid_apphdr_ext_off,
                                            int stage_txdma_act,
                                            int stage_tableid_txdma_act,
                                            int stage_txdma_act_ext,
                                            int stage_tableid_txdma_act_ext,
                                            int stage_sxdma_act,
                                            int stage_tableid_sxdma_act);

extern "C" void elba_deparser_init(int tm_port_ingress, int tm_port_egress);

extern "C" void elba_program_hbm_table_base_addr(int stage_tableid, char *tablename,
                                                 int stage, int pipe);

extern "C" void elba_p4plus_recirc_init(void);


extern "C" uint8_t elba_get_action_pc(uint32_t tableid, uint8_t actionid);

extern "C" void p4_invalidate_cache(uint64_t addr, uint32_t size_in_bytes,
                                    p4pd_table_cache_t cache);

extern "C" bool p4plus_invalidate_cache(uint64_t addr, uint32_t size_in_bytes,
                                        p4plus_cache_action_t action);

extern "C" uint8_t elba_get_action_id(uint32_t tableid, uint8_t actionpc);

sdk_ret_t elba_te_enable_capri_mode(void);
sdk_ret_t elba_ipsec_inline_enable(void);

extern "C" sdk_ret_t elba_pf_tcam_write_entry(uint8_t index,
                                              uint8_t *mac_da,
                                              uint8_t port,
                                              uint8_t out_port);

extern "C" sdk_ret_t elba_pf_tcam_write(uint8_t entry_idx,
                                        pf_tcam_key_fld_pos_t fields,
                                        pf_tcam_key_t* tcam_key,
                                        pf_tcam_rslt_t* rslt_entry);

extern "C" sdk_ret_t elba_pf_l2_multi_dest_tcam_entry_write(uint8_t index,
                                                            uint8_t *mac_da,
                                                            uint8_t port,
                                                            uint8_t port_cnt,
                                                            uint8_t *out_port_list,
                                                            uint8_t vlan_op,
                                                            uint16_t vlan_id);

}    // namespace elba
}    // namespace platform
}    // namespace sdk



#endif   // __ELBA_TBL_RW_HPP__
