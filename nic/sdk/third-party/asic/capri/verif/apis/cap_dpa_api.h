#ifndef CAP_DPA_API_H
#define CAP_DPA_API_H

#include "cpu.h"
#include "cap_base_api.h"
#include "cap_blk_reg_model.h"
#include "cap_csr_util_api.h"
#include "sknobs.h"
#include "LogMsg.h"
#include "cap_dpp_csr.h"
#include "cap_dpr_csr.h"
#include "pknobs_reader.h"


// soft reset sequence 
void cap_dpa_soft_reset(int chip_id, int inst_id);
// soft reset, value = 1 means apply reset, value = 0 release soft reset
void cap_dpa_set_soft_reset(int chip_id, int inst_id, int value);
// init start, no polling yet
void cap_dpa_init_start(int chip_id, int inst_id);
// poll for init done
void cap_dpa_init_done(int chip_id, int inst_id);
// use sknobs base load cfg 
void cap_dpa_load_from_cfg(int chip_id, int inst_id);
// eos check, this functions should call eos_cnt, eos_int, eos_sta 
void cap_dpa_eos(int chip_id, int inst_id);
void cap_dpa_eos_cnt(int chip_id, int inst_id);
void cap_dpa_eos_int(int chip_id, int inst_id);
void cap_dpa_eos_sta(int chip_id, int inst_id);

//API for block level.
void cap_dpa_basic_config(int chip_id, int inst_id);
void cap_dpa_csum_config(int chip_id, int inst_id);
void cap_dpa_crc_config(int chip_id, int inst_id);

void cap_dpa_check_progr_sanity(int chip_id, int inst_id);
void cap_dpa_disable_backdoor_paths(int chip_id, int inst_id);
void cap_dpp_csr_set_hdl_path(int chip_id, int inst_id, string path);
void cap_dpr_csr_set_hdl_path(int chip_id, int inst_id, string path);

//This api writes random values to CFG & CNT type registers.
void cap_dpa_randomize_csr(int chip_id, int inst_id);

void cap_dpa_bist_test(int chip_id, int inst_id);
void cap_dpa_bist_test_start(int chip_id, int inst_id);
void cap_dpa_bist_test_chk(int chip_id, int inst_id);
void cap_dpa_bist_test_clear(int chip_id, int inst_id);
void cap_dpa_pktin_ff_show(int chip_id, int inst_id);
void cap_dpa_csum_ff_show(int chip_id, int inst_id);

#endif // CAP_DPA_API_H
