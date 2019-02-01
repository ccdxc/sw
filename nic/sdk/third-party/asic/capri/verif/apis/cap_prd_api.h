#ifndef CAP_PRD_API_H
#define CAP_PRD_API_H

#include "cpu.h"
#include "cap_base_api.h"
#include "cap_blk_reg_model.h"
#include "cap_pr_csr.h"
#include "cap_phv_intr_decoders.h"
#include "cap_ptd_decoders.h"

#define PRD_PRN_MSG(X) {\
        stringstream l_ss;\
        l_ss << X; \
        prd_print_msg(l_ss.str()); \
}
void prd_print_msg(string msg);

// soft reset sequence 
void cap_prd_soft_reset(int chip_id, int inst_id);
// soft reset, value = 1 means apply reset, value = 0 release soft reset
void cap_prd_set_soft_reset(int chip_id, int inst_id, int value);
// init start, no polling yet
void cap_prd_init_start(int chip_id, int inst_id);
// poll for init done
void cap_prd_init_done(int chip_id, int inst_id);
// use sknobs base load cfg 
void cap_prd_load_from_cfg(int chip_id, int inst_id);
// eos check, this functions should call eos_cnt, eos_int, eos_sta 
void cap_prd_eos(int chip_id, int inst_id);
void cap_prd_eos_cnt(int chip_id, int inst_id);
void cap_prd_eos_int(int chip_id, int inst_id);
void cap_prd_eos_int_sim(int chip_id, int inst_id);
void cap_prd_eos_sta(int chip_id, int inst_id);
void cap_prd_bist_test(int chip_id, int inst_id);
void cap_prd_bist_test_start(int chip_id, int inst_id);
void cap_prd_bist_test_chk(int chip_id, int inst_id);
void cap_prd_bist_test_clear(int chip_id, int inst_id);

void display_prd_phv_mem_entry(unsigned addr, int & dma_ptr, bool wr_mem_sel);
void display_prd_rdata_mem_entry(unsigned addr, bool wr_mem_sel);
void display_prd_pkt_mem_entry(unsigned addr, bool wr_mem_sel);
void display_prd_lat_mem_entry(unsigned addr, bool wr_mem_sel);
void display_prd_phv_mem();
void display_prd_rdata_mem();
void display_prd_pkt_mem();
void display_prd_lat_mem();
void clear_prd_phv_mem_entry(unsigned addr, bool wr_mem_sel);
void clear_prd_rdata_mem_entry(unsigned addr, bool wr_mem_sel);
void clear_prd_pkt_mem_entry(unsigned addr, bool wr_mem_sel);
void clear_prd_lat_mem_entry(unsigned addr, bool wr_mem_sel);
void clear_prd_phv_mem();
void clear_prd_rdata_mem();
void clear_prd_pkt_mem();
void clear_prd_lat_mem();

void cap_prd_dump_cntr();
void cap_prd_clr_cntr();
#endif
