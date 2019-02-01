#ifndef CAP_PSP_API_H
#define CAP_PSP_API_H


#include "cpu.h"
#include "cap_base_api.h"
#include "cap_blk_reg_model.h"
#include "cap_pr_csr.h"
#include "cap_lif_qstate_decoders.h"

#define PSP_PRN_MSG(X) {\
        stringstream l_ss;\
        l_ss << X; \
        psp_print_msg(l_ss.str()); \
}

void psp_print_msg(string msg);
// soft reset sequence 
void cap_psp_soft_reset(int chip_id, int inst_id);
// soft reset, value = 1 means apply reset, value = 0 release soft reset
void cap_psp_set_soft_reset(int chip_id, int inst_id, int value);
// init start, no polling yet
void cap_psp_init_start(int chip_id, int inst_id);
// poll for init done
void cap_psp_init_done(int chip_id, int inst_id);
// use sknobs base load cfg 
void cap_psp_load_from_cfg(int chip_id, int inst_id);
// eos check, this functions should call eos_cnt, eos_int, eos_sta 
void cap_psp_eos(int chip_id, int inst_id);
void cap_psp_eos_cnt(int chip_id, int inst_id);
void cap_psp_eos_int_sim(int chip_id, int inst_id);
void cap_psp_eos_int(int chip_id, int inst_id);
void cap_psp_eos_sta(int chip_id, int inst_id);


#ifndef SWIG
void cap_psp_config(int chip_id, int inst_id); 
void cap_psp_lif_qstate_map(int lif, cap_lif_qstate_data_bundle_t lif_qstate_entry);
void cap_psp_sw_phv_mem(int index, cap_psp_csr_dhs_sw_phv_mem_entry_t sw_phv_mem_entry);
void cap_psp_cfg_sw_phv_control(int profile_id, int data);
void cap_psp_cfg_sw_phv_config(int profile_id, cap_psp_csr_cfg_sw_phv_config_t sw_phv_config);
void cap_psp_sw_prof_cfg(int profile_id, cpp_int *sw_phv_data, int num_flits, int cnt_rep_en, int localtime_en, int frm_sz_en, uint64_t insert_period, int counter_max);
#endif
void cap_psp_sw_phv_start_enable(int profile_id, int enable);
void cap_psp_sw_phv_status(int profile_id, uint64_t &data);
void cap_psp_sw_phv_display();
void cap_psp_clr_cntr();
void cap_psp_dump_cntr();
#endif
