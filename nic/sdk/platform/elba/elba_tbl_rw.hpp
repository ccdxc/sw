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
#include "asic/pd/pd.hpp"

using namespace sdk::asic;
using namespace sdk::asic::pd;

namespace sdk {
namespace platform {
namespace elba {

#define ELBA_P4_NUM_STAGES           6
#define ELBA_P4PLUS_NUM_STAGES       8
#define ELBA_P4PLUS_SXDMA_NUM_STAGES 4

#define ELBA_OK (0)
#define ELBA_FAIL (-1)

sdk_ret_t elba_table_rw_init(asic_cfg_t *elba_cfg);
sdk_ret_t elba_table_rw_soft_init(asic_cfg_t *elba_cfg);
sdk_ret_t elba_p4plus_table_rw_init(void);

typedef struct pf_tcam_l2_ {
    uint16_t eth_type;
    uint8_t  payload[6];
} pf_tcam_l2_t;

typedef struct pf_tcam_ip_ {
    uint8_t  l4_prot;
    uint16_t l4_src_port;
    uint16_t l4_dst_port;
    uint8_t  dscp:6;
    uint8_t  ecn:2;
    uint8_t  v6:1;
    uint8_t  opt:1;
    uint8_t  mf:1;
    uint8_t  frag_ofst:1;
    uint8_t  frag:1;
    uint8_t  res3:3;
} pf_tcam_ip_t;


typedef union pf_tcam_l2_ip_key {
    pf_tcam_l2_t pf_tcam_l2_key;
    pf_tcam_ip_t pf_tcam_ip_key;
} pf_tcam_l2_ip_key_t;


typedef struct pf_tcam_key_ {
    uint8_t mac_da[6];
    uint8_t mac_sa[6];
    uint8_t port:3;
    uint8_t cos:3;
    uint8_t qiq_vld:1;
    uint8_t de:1;
    uint16_t vlan_id:12;
    uint8_t qtag_vlan_vld:1;
    uint8_t qtag_vlan:1;
    uint8_t lkup_ip:1;
    uint8_t res1:1;
    uint8_t pyld_len:3;
    uint8_t res2:5;
    pf_tcam_l2_ip_key_t pf_tcam_l2_ip_key;
} pf_tcam_key_t;


typedef struct pf_tcam_rslt_ {
    uint8_t tc_vld:1;
    uint8_t out_pb:1;
    uint8_t out_mnic:1;
    uint8_t out_bx:1;
    uint8_t out_mx:1;
    uint8_t tc:3;
    uint16_t mnic_vlan_id:12;
    uint8_t  mnic_vlan_op:2;
    uint8_t mx_port:2;
    uint8_t pb_port:3;
    uint8_t spare2:5;
} pf_tcam_rslt_t;

#define PF_TCAM_SET_FLD(flags, pos) (flags |= (1 << pos))
#define PF_TCAM_CLR_FLD(flags, pos) (flags &= ~(1 << pos))
#define PF_TCAM_FLD_IS_SET(flags, pos) (flags & (1 << pos))

typedef enum pf_tcam_key_fld_pos {
    PF_TCAM_FLD_MAC_DA = 0,
    PF_TCAM_FLD_MAC_SA,
    PF_TCAM_FLD_PORT,
    PF_TCAM_FLD_QID_VLD,
    PF_TCAM_FLD_QTAG_VLAN_VLD,
    PF_TCAM_FLD_QTAG_VLAN,
    PF_TCAM_FLD_COS,
    PF_TCAM_FLD_DE,
    PF_TCAM_FLD_VLAN_ID,
    PF_TCAM_FLD_PYLD_LEN,
    PF_TCAM_FLD_LKUP_IP,
    PF_TCAM_FLD_ETH_TYPE,
    PF_TCAM_FLD_PYLD,
    PF_TCAM_FLD_V6,
    PF_TCAM_FLD_DSCP,
    PF_TCAM_FLD_ECN,
    PF_TCAM_FLD_OPT,
    PF_TCAM_FLD_MF,
    PF_TCAM_FLD_FRAG_OF,
    PF_TCAM_FLD_FRAG,
    PF_TCAM_FLD_PROT,
    PF_TCAM_FLD_L4_SRC_PORT,
    PF_TCAM_FLD_L4_DST_PORT,
    PF_TCAM_FLD_MAX_FIELDS
} pf_tcam_key_fld_pos_t;


void elba_table_rw_cleanup(void);

sdk_ret_t elba_table_entry_write(uint32_t tableid, uint32_t index,
                                 uint8_t  *hwentry, uint8_t  *hwentry_mask,
                                 uint16_t hwentry_bit_len,
                                 p4_table_mem_layout_t &tbl_info,
                                 int gress, bool is_oflow_table, bool ingress,
                                 uint32_t ofl_parent_tbl_depth);

sdk_ret_t elba_table_entry_read(uint32_t tableid, uint32_t index,
                                uint8_t  *hwentry, uint16_t *hwentry_bit_len,
                                p4_table_mem_layout_t &tbl_info, int gress,
                                bool is_oflow_table,
                                uint32_t ofl_parent_tbl_depth);

sdk_ret_t elba_table_hw_entry_read(uint32_t tableid, uint32_t index,
                                   uint8_t  *hwentry,
                                   uint16_t *hwentry_bit_len,
                                   p4_table_mem_layout_t &tbl_info, int gress,
                                   bool is_oflow_table, bool ingress,
                                   uint32_t ofl_parent_tbl_depth);

sdk_ret_t elba_tcam_table_entry_write (uint32_t tableid, uint32_t index,
                                       uint8_t  *trit_x, uint8_t  *trit_y,
                                       uint16_t hwentry_bit_len,
                                       p4_table_mem_layout_t &tbl_info,
                                       int gress, bool ingress);

sdk_ret_t elba_tcam_table_entry_read(uint32_t tableid, uint32_t index,
                                     uint8_t  *trit_x, uint8_t  *trit_y,
                                     uint16_t *hwentry_bit_len,
                                     p4_table_mem_layout_t &tbl_info,
                                     int gress);

sdk_ret_t elba_tcam_table_hw_entry_read(uint32_t tableid, uint32_t index,
                                        uint8_t  *trit_x, uint8_t  *trit_y,
                                        uint16_t *hwentry_bit_len,
                                        p4_table_mem_layout_t &tbl_info,
                                        bool ingress);

sdk_ret_t elba_hbm_table_entry_write(uint32_t tableid, uint32_t index,
                                     uint8_t *hwentry, uint16_t entry_size,
                                     uint16_t entry_width,
                                     p4pd_table_properties_t *tbl_info);

sdk_ret_t elba_hbm_table_entry_cache_invalidate(p4pd_table_cache_t cache,
                                                uint64_t entry_addr,
                                                uint16_t entry_width,
                                                mem_addr_t base_mem_pa);

sdk_ret_t elba_hbm_table_entry_read(uint32_t tableid, uint32_t index,
                                    uint8_t *hwentry, uint16_t *entry_size,
                                    p4_table_mem_layout_t &tbl_info,
                                    bool read_thru);

sdk_ret_t elba_table_constant_write(uint64_t val, uint32_t stage,
                                    uint32_t stage_tableid, bool ingress);

sdk_ret_t elba_table_constant_read(uint64_t *val, uint32_t stage,
                                   int stage_tableid, bool ingress);

void elba_set_action_asm_base(int tableid, int actionid, uint64_t asm_base);

void elba_set_action_rxdma_asm_base(int tableid, int actionid,
                                    uint64_t asm_base);

void elba_set_action_txdma_asm_base(int tableid, int actionid,
                                    uint64_t asm_base);

void elba_set_table_rxdma_asm_base(int tableid, uint64_t asm_base);

void elba_set_table_txdma_asm_base(int tableid, uint64_t asm_base);

void elba_program_p4plus_sram_table_mpu_pc(int tableid, int stage_tbl_id,
                                           int stage);

void elba_program_table_mpu_pc(int tableid, bool gress, int stage,
                               int stage_tableid,
                               uint64_t elba_table_asm_err_offset,
                               uint64_t elba_table_asm_base);

sdk_ret_t elba_toeplitz_init(const char *handle, int stage, int stage_tableid);

sdk_ret_t elba_p4plus_table_init(platform_type_t platform_type,
                                 int stage_apphdr, int stage_tableid_apphdr,
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

void elba_deparser_init(int tm_port_ingress, int tm_port_egress);

void elba_program_hbm_table_base_addr(int tableid, int stage_tableid,
                                      char *tablename, int stage, int pipe);

void elba_p4plus_recirc_init(void);

uint8_t elba_get_action_pc(uint32_t tableid, uint8_t actionid);

void p4_invalidate_cache(uint64_t addr, uint32_t size_in_bytes,
                         p4pd_table_cache_t cache);

bool p4plus_invalidate_cache(uint64_t addr, uint32_t size_in_bytes,
                             p4plus_cache_action_t action);

uint8_t elba_get_action_id(uint32_t tableid, uint8_t actionpc);

sdk_ret_t elba_te_enable_capri_mode(void);
sdk_ret_t elba_ipsec_inline_enable(void);

sdk_ret_t elba_pf_tcam_write_entry(uint8_t index, uint8_t *mac_da,
                                   uint8_t port, uint8_t out_port);

sdk_ret_t elba_pf_tcam_write(uint8_t entry_idx, pf_tcam_key_fld_pos_t fields,
                             pf_tcam_key_t* tcam_key,
                             pf_tcam_rslt_t* rslt_entry);

sdk_ret_t elba_pf_l2_multi_dest_tcam_entry_write(uint8_t index, uint8_t *mac_da,
                                                 uint8_t port, uint8_t port_cnt,
                                                 uint8_t *out_port_list,
                                                 uint8_t vlan_op,
                                                 uint16_t vlan_id);

void elba_table_csr_cache_inval_init(void);
void elba_program_p4plus_table_mpu_pc(int tableid, int stage_tbl_id, int stage);
uint64_t elba_get_p4plus_table_mpu_pc(int tableid);

sdk_ret_t elba_tbl_eng_cfg_modify(p4pd_pipeline_t pipeline,
                                  p4_tbl_eng_cfg_t *cfg, uint32_t ncfgs);
sdk_ret_t elba_pgm_init(void);

}    // namespace elba
}    // namespace platform
}    // namespace sdk

#endif   // __ELBA_TBL_RW_HPP__
