// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __SDK_ASIC_PD_HPP__
#define __SDK_ASIC_PD_HPP__

#include "include/sdk/base.hpp"
#include "lib/p4/p4_api.hpp"
#include "asic/asic.hpp"

namespace sdk {
namespace asic {
namespace pd {

// set this macro with max coses supported
#define ASIC_NUM_MAX_COSES   16

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

typedef struct llc_counters_s {
    uint32_t mask;
    uint32_t data[16];
} llc_counters_t;

typedef struct scheduler_stats_s {
    uint64_t doorbell_set_count;
    uint64_t doorbell_clear_count;
    uint32_t ratelimit_start_count;
    uint32_t ratelimit_stop_count;

    typedef struct txs_scheduler_cos_stats_s {
        uint32_t cos;
        bool xon_status;
        uint64_t doorbell_count;
    } txs_scheduler_cos_stats_t;

    // cos_count will be updated by the asic
    uint32_t num_coses;
    txs_scheduler_cos_stats_t cos_stats[ASIC_NUM_MAX_COSES];
} scheduler_stats_t;

typedef struct hbm_bw_samples_s {
    uint32_t      num_samples;
    uint32_t      sleep_interval;
    asic_hbm_bw_t *hbm_bw;
} __PACK__ hbm_bw_samples_t;

sdk_ret_t asicpd_read_table_constant(uint32_t tableid, uint64_t *value);
typedef struct lif_qtype_info_s {
    uint8_t entries;
    uint8_t size;
    uint8_t cosA;
    uint8_t cosB;
} __PACK__ lif_qtype_info_t;

const static uint32_t kNumQTypes = 8;
const static uint32_t kAllocUnit = 4096;
typedef struct lif_qstate_s {
    uint32_t lif_id;
    uint32_t allocation_size;
    uint64_t hbm_address;
    uint8_t hint_cos;
    uint8_t enable;
    struct {
        lif_qtype_info_t qtype_info;
        uint32_t hbm_offset;
        uint32_t qsize;
        uint32_t rsvd;
        uint32_t num_queues;
        uint8_t  coses;
    } type[kNumQTypes];
} __PACK__ lif_qstate_t;

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
sdk_ret_t asic_pd_scheduler_stats_get(
    scheduler_stats_t *sch_stats);
sdk_ret_t asic_pd_hbm_bw_get(hbm_bw_samples_t *hbm_bw_samples);
sdk_ret_t asic_pd_llc_setup(llc_counters_t *llc);
sdk_ret_t asic_pd_llc_get(llc_counters_t *llc);
sdk_ret_t asicpd_p4plus_recirc_init(void);
sdk_ret_t asic_pd_qstate_map_clear(uint32_t lif_id);
sdk_ret_t asic_pd_qstate_map_write(lif_qstate_t *qstate, uint8_t enable);
sdk_ret_t asic_pd_qstate_map_read (lif_qstate_t *qstate);
sdk_ret_t asic_pd_qstate_write (uint64_t addr, const uint8_t *buf, uint32_t size);
sdk_ret_t asic_pd_qstate_read (uint64_t addr, uint8_t *buf, uint32_t size);
sdk_ret_t asic_pd_qstate_clear (lif_qstate_t *qstate);
sdk_ret_t asic_pd_p4plus_invalidate_cache (mpartition_region_t *reg,
                                           uint64_t q_addr, uint32_t size);
uint32_t asic_pd_clock_freq_get(void);

}    // namespace pd
}    // namespace asic
}    // namespace sdk

using sdk::asic::pd::p4_table_mem_layout_t;
using sdk::asic::pd::llc_counters_t;
using sdk::asic::pd::scheduler_stats_t;
using sdk::asic::pd::hbm_bw_samples_t;
using sdk::asic::pd::lif_qstate_t;
using sdk::asic::pd::lif_qtype_info_t;
using sdk::asic::pd::kNumQTypes;
using sdk::asic::pd::kAllocUnit;


#endif    // __SDK_ASIC_PD_HPP__
