// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __HAL_PD_ASIC_PD_COMMON_HPP__
#define __HAL_PD_ASIC_PD_COMMON_HPP__

#include "nic/include/base.hpp"
#include "nic/include/hal.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "include/sdk/platform/capri/capri_hbm_rw.hpp"
#include "gen/proto/debug.pb.h"

#define HAL_LOG_TBL_UPDATES 1

typedef struct asicpd_stats_region_info_t_ {
    int tblid;
    int tbldepthshift;
} __PACK__ asicpd_stats_region_info_t;

namespace hal {
namespace pd {

// sw phv pipeline type
enum asicpd_swphv_type_t {
    ASICPD_SWPHV_TYPE_RXDMA   = 0,    // P4+ RxDMA
    ASICPD_SWPHV_TYPE_TXDMA   = 1,    // P4+ TxDMA
    ASICPD_SWPHV_TYPE_INGRESS = 2,    // P4 Ingress
    ASICPD_SWPHV_TYPE_EGRESS  = 3,    // P4 Egress
};

// sw phv injection state
typedef struct asicpd_sw_phv_state_ {
    bool        enabled;
    bool        done;
    uint32_t    current_cntr;
    uint32_t    no_data_cntr;
    uint32_t    drop_no_data_cntr;
} asicpd_sw_phv_state_t;

typedef struct pd_hbm_bw_get_args_s {
    uint32_t      num_samples;
    uint32_t      sleep_interval;
    asic_hbm_bw_t *hbm_bw;
} __PACK__ pd_hbm_bw_get_args_t;

typedef struct pd_llc_get_args_s {
    uint32_t mask;
    uint32_t data[16];
} pd_llc_get_args_t;

typedef struct pd_scheduler_stats_get_args_s {
    debug::SchedulerStatsResponse *response;
} pd_scheduler_stats_get_args_t;

int asicpd_table_entry_write(uint32_t tableid, uint32_t index,
                             uint8_t  *hwentry, uint16_t hwentry_bit_len, 
                             uint8_t  *hwentry_mask);
int asicpd_table_entry_read(uint32_t tableid, uint32_t index,
                            uint8_t  *hwentry, uint16_t *hwentry_bit_len);
int asicpd_table_hw_entry_read(uint32_t tableid, uint32_t index,
                               uint8_t  *hwentry, uint16_t *hwentry_bit_len);
int asicpd_tcam_table_entry_write(uint32_t tableid, uint32_t index,
                                  uint8_t  *trit_x, uint8_t  *trit_y,
                                  uint16_t hwentry_bit_len);
int asicpd_tcam_table_entry_read(uint32_t tableid, uint32_t index,
                                 uint8_t  *trit_x, uint8_t  *trit_y,
                                 uint16_t *hwentry_bit_len);
int asicpd_tcam_table_hw_entry_read(uint32_t tableid, uint32_t index,
                                    uint8_t  *trit_x, uint8_t  *trit_y,
                                    uint16_t *hwentry_bit_len);
int asicpd_hbm_table_entry_write (uint32_t tableid, uint32_t index,
                                  uint8_t *hwentry, uint16_t entry_size);
int asicpd_hbm_table_entry_write (uint32_t tableid, uint32_t index,
                                  uint8_t *hwentry, uint16_t *entry_size);
uint8_t asicpd_get_action_pc(uint32_t tableid, uint8_t actionid);
uint8_t asicpd_get_action_id(uint32_t tableid, uint8_t actionpc);
hal_ret_t asicpd_toeplitz_init(void);
hal_ret_t asicpd_p4plus_table_init(hal::hal_cfg_t *hal_cfg);
hal_ret_t asicpd_p4plus_recirc_init(void);
sdk_ret_t asicpd_program_table_mpu_pc(void);
hal_ret_t asicpd_program_table_constant(uint32_t tableid,
                                        uint64_t const_value);
hal_ret_t asicpd_program_table_thread_constant(uint32_t tableid,
                                               uint8_t table_thread,
                                               uint64_t const_value);
sdk_ret_t asicpd_table_mpu_base_init(p4pd_cfg_t *p4pd_cfg);
sdk_ret_t asicpd_deparser_init(void);
sdk_ret_t asicpd_program_hbm_table_base_addr(void);
hal_ret_t asicpd_stats_addr_get (int tblid, uint32_t index,
                       asicpd_stats_region_info_t *region_arr, int arrlen,
                       hbm_addr_t *stats_addr_p);

hal_ret_t asicpd_stats_region_init(asicpd_stats_region_info_t *region_arr,
                                   int arrlen);
sdk_ret_t asicpd_p4plus_table_mpu_base_init(p4pd_cfg_t *p4pd_cfg);


hal_ret_t asicpd_sw_phv_inject(asicpd_swphv_type_t type, uint8_t prof_num, 
		uint8_t start_idx, uint8_t num_flits, void *data);
hal_ret_t asicpd_sw_phv_get (asicpd_swphv_type_t type, uint8_t prof_num, 
	asicpd_sw_phv_state_t *state);

hal_ret_t
asic_pd_hbm_bw_get(pd_hbm_bw_get_args_t *hbm_bw_args);

hal_ret_t
asic_pd_llc_setup (pd_llc_get_args_t *llc_args);

hal_ret_t
asic_pd_llc_get (pd_llc_get_args_t *llc_args);

hal_ret_t
asic_pd_scheduler_stats_get (pd_scheduler_stats_get_args_t *scheduler_stats_args);
hal_ret_t xcvr_valid_check_enable(bool enable);

}    // namespace pd
}    // namespace hal

#endif    // __HAL_PD_ASIC_PD_COMMON_HPP__

