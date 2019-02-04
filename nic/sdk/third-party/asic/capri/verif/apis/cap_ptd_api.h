#ifndef CAP_PTD_API_H
#define CAP_PTD_API_H


#include "cpu.h"
#include "cap_base_api.h"
#include "cap_blk_reg_model.h"
#include "cap_pt_csr.h"
#include "cap_phv_intr_decoders.h"
#include "cap_ptd_decoders.h"

#define PTD_PRN_MSG(X) {\
        stringstream l_ss;\
        l_ss << X; \
        ptd_print_msg(l_ss.str()); \
}

void ptd_print_msg(string msg);
// soft reset sequence 
void cap_ptd_soft_reset(int chip_id, int inst_id);
// soft reset, value = 1 means apply reset, value = 0 release soft reset
void cap_ptd_set_soft_reset(int chip_id, int inst_id, int value);
// init start, no polling yet
void cap_ptd_init_start(int chip_id, int inst_id);
// poll for init done
void cap_ptd_init_done(int chip_id, int inst_id);
// use sknobs base load cfg 
void cap_ptd_load_from_cfg(int chip_id, int inst_id);
// eos check, this functions should call eos_cnt, eos_int, eos_sta 
void cap_ptd_eos(int chip_id, int inst_id);
void cap_ptd_eos_cnt(int chip_id, int inst_id);
void cap_ptd_eos_int(int chip_id, int inst_id);
void cap_ptd_eos_int_sim(int chip_id, int inst_id);
void cap_ptd_eos_sta(int chip_id, int inst_id);
void cap_ptd_bist_test(int chip_id, int inst_id);
void cap_ptd_bist_test_start(int chip_id, int inst_id);
void cap_ptd_bist_test_chk(int chip_id, int inst_id);
void cap_ptd_bist_test_clear(int chip_id, int inst_id);

void display_ptd_phv_mem();
void display_ptd_rdata_mem();
void display_ptd_wdata_mem();
void display_ptd_lat_mem();
void display_ptd_phv_mem_entry(unsigned addr, int & dma_ptr,  bool wr_mem_sel = true);
void display_ptd_rdata_mem_entry(unsigned addr, bool wr_mem_sel = true);
void display_ptd_wdata_mem_entry(unsigned addr, bool wr_mem_sel = true);
void display_ptd_lat_mem_entry(unsigned addr, bool wr_mem_sel = true);
void clear_ptd_phv_mem();
void clear_ptd_rdata_mem();
void clear_ptd_wdata_mem();
void clear_ptd_lat_mem();
void clear_ptd_phv_mem_entry(unsigned addr, bool wr_mem_sel = true);
void clear_ptd_rdata_mem_entry(unsigned addr, bool wr_mem_sel = true);
void clear_ptd_wdata_mem_entry(unsigned addr, bool wr_mem_sel = true);
void clear_ptd_lat_mem_entry(unsigned addr, bool wr_mem_sel = true);
void cap_ptd_dump_cntr();
void cap_ptd_clr_cntr();
#endif
