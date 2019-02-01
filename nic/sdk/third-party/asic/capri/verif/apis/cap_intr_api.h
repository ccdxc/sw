#ifndef CAP_INTR_API_H
#define CAP_INTR_API_H

#include "cpu.h"
#include "cap_base_api.h"
#include "cap_blk_reg_model.h"
#include "sknobs.h"
#include "LogMsg.h"
#include "cap_intr_csr.h"

const int intr_credit_size = 16;

// soft reset sequence
void cap_intr_soft_reset(int chip_id, int inst_id);
// soft reset, value = 1 means apply reset, value = 0 release soft reset
void cap_intr_set_soft_reset(int chip_id, int inst_id, int value);
// init start, no polling yet
void cap_intr_init_start(int chip_id, int inst_id);
// poll for init done
void cap_intr_init_done(int chip_id, int inst_id);
// use sknobs base load cfg
void cap_intr_load_from_cfg(int chip_id, int inst_id);
// eos check, this functions should call eos_cnt, eos_int, eos_sta
void cap_intr_eos(int chip_id, int inst_id);
void cap_intr_eos_cnt(int chip_id, int inst_id);
void cap_intr_eos_int(int chip_id, int inst_id);
void cap_intr_eos_sta(int chip_id, int inst_id);

void cap_intr_msixcfg_cmd_mask(int chip_id, int inst_id, int intr_index, int mask);
void cap_intr_fwcfg_cmd_mask(int chip_id, int inst_id, int intr_index, int mask);
void cap_intr_drvcfg_cmd_mask(int chip_id, int inst_id, int intr_index, int mask);
void cap_intr_drvcfg_cmd_credits(int chip_id, int inst_id, int intr_index, int16_t credits, int unmask, int restart_coal);
void cap_intr_get_drvcfg_cmd_credits(int chip_id,
                                     int inst_id,
                                     int intr_index,
                                     int16_t & credits,
                                     int & unmask,
                                     int & restart_coal);
#ifndef SWIG
void cap_intr_get_drvcfg_cmd_X(int chip_id, int inst_id, int intr_index,
 	                            uint32_t & data, int cmd_offset);
#endif
void cap_intr_get_fwcfg_cmd_X(int chip_id, int inst_id, int intr_index,
                            uint32_t & data, int cmd_offset);

void cap_intr_coal_timer_init(int chip_id, int inst_id, int intr_index, uint32_t timer_value);
void cap_intr_coal_timer_resolution(int chip_id, int inst_id, int resolution_us);
void cap_intr_csr_set_hdl_path(int chip_id, int inst_id, string path);

#endif // CAP_INTR_API_H
