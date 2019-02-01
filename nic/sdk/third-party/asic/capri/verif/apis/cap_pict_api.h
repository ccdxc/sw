#ifndef CAP_PICT_API_H
#define CAP_PICT_API_H

#include "cpu.h"
#include "cap_base_api.h"
#include "dpi_registry.h"
#include "cap_blk_reg_model.h"
#include "cap_pict_csr.h"
#include "cap_pic_defs.h"

#define TCAM_TEST_LOG 0

void cap_pict_soft_reset(int chip_id, int inst_id);
void cap_pict_set_soft_reset(int chip_id, int inst_id, int value);
#ifndef SWIG
void cap_pict_sw_reset(int chip_id, int inst_id);
#endif
void cap_pict_init_start(int chip_id, int inst_id);
void cap_pict_init_done(int chip_id, int inst_id);
#ifndef SWIG
void cap_pict_load_from_cfg(int chip_id, int inst_id, block_type type = SI);
#endif
void cap_pict_eos(int chip_id, int inst_id);
void cap_pict_eos_cnt(int chip_id, int inst_id);
void cap_pict_eos_int(int chip_id, int inst_id);
void cap_pict_eos_sta(int chip_id, int inst_id);

#ifndef SWIG
void cap_pict_load_tcam_entry(int chip_id, int inst_id, 
                              uint32_t table_start, 
                              uint32_t entry_size,
                              uint32_t entries_per_line,
                              uint32_t entry_idx,
                              cpp_int key,
                              cpp_int mask,
                              block_type type = SI);

void cap_pict_load_logical_tcam_entry(int chip_id, int inst_id, 
                                      uint32_t table_id, 
                                      uint32_t entry_idx,
                                      cpp_int key,
                                      cpp_int mask,
                                      bool read_from_hw,
                                      block_type type = SI);

void cap_pict_add_logical_tcam(int chip_id, int inst_id, 
                               uint32_t tcam_id,
                               uint32_t upper_x,
                               uint32_t upper_y,
                               uint32_t lower_x,
                               uint32_t lower_y,
                               uint32_t entry_size_bytes,
                               uint32_t entries_per_line,
                               uint32_t en_tbid,
                               uint32_t tbid,
                               uint32_t keyshift,
                               block_type type = SI);

void cap_pict_load_tcam_def_from_cfg(int chip_id, int inst_id, block_type type = SI);

void cap_pict_check_progr_sanity(int chip_id, int inst_id, block_type type = SI);

void cap_pict_zero_init_tcam(int chip_id, int inst_id, int num_tcams);

#endif

void cap_pict_csr_set_hdl_path(int chip_id, int inst_id, string path);

void cap_pict_bist_run(int chip_id, int inst_id, int enable);
void cap_pict_bist_chk(int chip_id, int inst_id);

// pict tcam test
int cap_run_pict_tcam_rdwr_test(int chip_id, int pict, int fast_mode = 1, int max_err_cnt = 3, int verbosity = 0);
void cap_pict_tcam_reset(int chip_id, int pict, int bank);
#ifndef SWIG
void cap_pict_tcam_write(int chip_id, int pict, cpp_int key, cpp_int mask, int bank, cpp_int line, cpp_int valid);
void cap_pict_tcam_read(int chip_id, int pict, int bank, cpp_int line);
int  cap_pict_tcam_read_chk(int chip_id, int pict, cpp_int key, cpp_int mask, int bank, cpp_int line, int verbosity);
int  cap_pict_tcam_addr( int bank, cpp_int line);
cpp_int cap_pict_tcam_search(int chip_id, int pict, cpp_int key, cpp_int mask, int bank);
cpp_int cap_pict_key_xy(cpp_int key, cpp_int mask);
cpp_int cap_pict_mask_xy(cpp_int key, cpp_int mask);
#endif


#endif // CAP_PIC_API_H
