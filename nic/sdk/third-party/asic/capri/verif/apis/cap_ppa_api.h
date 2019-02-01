#ifndef CAP_PPA_API_H
#define CAP_PPA_API_H

#include "cpu.h"
#include "cap_base_api.h"
#include "cap_ppa_decoders.h"
#include "cap_blk_reg_model.h"
#include "cap_csr_util_api.h"
#include "sknobs.h"
#include "LogMsg.h"
#include "cap_ppa_csr.h"

// soft reset sequence 
void cap_ppa_soft_reset(int chip_id, int inst_id);
// soft reset, value = 1 means apply reset, value = 0 release soft reset
void cap_ppa_set_soft_reset(int chip_id, int inst_id, int value);
// init start, no polling yet
void cap_ppa_init_start(int chip_id, int inst_id);
// poll for init done
void cap_ppa_init_done(int chip_id, int inst_id);
// use sknobs base load cfg 
void cap_ppa_load_from_cfg(int chip_id, int inst_id);
// eos check, this functions should call eos_cnt, eos_int, eos_sta 
void cap_ppa_eos(int chip_id, int inst_id);
void cap_ppa_eos_cnt(int chip_id, int inst_id);
void cap_ppa_eos_int(int chip_id, int inst_id);
void cap_ppa_eos_sta(int chip_id, int inst_id);

void cap_ppa_add_bndl0_tcam_entry(int chip_id, int inst_id, uint32_t idx, uint32_t state, uint32_t ctrl, uint32_t lkp_val0, uint32_t lkp_val1, uint32_t lkp_val2) ;
void cap_ppa_add_bndl1_tcam_entry(int chip_id, int inst_id, uint32_t idx, uint32_t state, uint32_t ctrl, uint32_t lkp_val0, uint32_t lkp_val1, uint32_t lkp_val2) ;
void cap_ppa_program_tcam_sram_pair(int chip_id, int inst_id, uint32_t idx, cap_ppa_decoders_ppa_lkp_tcam_entry_t _tcam,
           cap_ppa_decoders_ppa_lkp_sram_entry_t _sram);
void cap_ppa_read_and_show_tcam_sram_pair(int chip_id, int inst_id, uint32_t idx);

void cap_ppa_check_progr_sanity(int chip_id, int inst_id);
void cap_ppa_disable_backdoor_paths(int chip_id, int inst_id);
void cap_ppa_csr_set_hdl_path(int chip_id, int inst_id, string path);

//This api writes random values to CFG & CNT type registers.
void cap_ppa_randomize_csr(int chip_id, int inst_id);

void cap_ppa_sw_prof_cfg(int profile_id, cpp_int *sw_phv_data, int num_flits, int cnt_rep_en, int localtime_en, int frm_sz_en, uint64_t insert_period, int counter_max);
void cap_ppa_sw_phv_start_enable(int profile_id, int enable);

void cap_ppa_bist_test(int chip_id, int inst_id);
void cap_ppa_bist_test_start(int chip_id, int inst_id);
void cap_ppa_bist_test_chk(int chip_id, int inst_id);
void cap_ppa_bist_test_clear(int chip_id, int inst_id);
void cap_pa_sw_phv_start_enable(int chip_id, int inst_id, int profile_id, int enable);

int  cap_run_ppa_tcam_rdwr_test(int chip_id, int inst_id, int bndl, int fast_mode = 1, int verbosity=0);

#endif // CAP_PPA_API_H
