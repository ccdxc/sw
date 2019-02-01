#ifndef CAP_PICS_API_H
#define CAP_PICS_API_H

#include "cpu.h"
#include "cap_base_api.h"
#include "dpi_registry.h"
#include "cap_blk_reg_model.h"
#include "cap_pics_csr.h"
#include "cap_pic_defs.h"

void cap_pics_soft_reset(int chip_id, int inst_id);
void cap_pics_set_soft_reset(int chip_id, int inst_id, int value);
////void cap_pics_sw_reset(int chip_id, int inst_id);
void cap_pics_init_start(int chip_id, int inst_id);
void cap_pics_init_done(int chip_id, int inst_id);
#ifndef SWIG
void cap_pics_load_from_cfg(int chip_id, int inst_id, block_type type = SI);
#endif
void cap_pics_set_axi_attrs(int chip_id, int inst_id);
void cap_pics_eos(int chip_id, int inst_id);
void cap_pics_eos_cnt(int chip_id, int inst_id);
void cap_pics_eos_int(int chip_id, int inst_id);
void cap_pics_eos_sta(int chip_id, int inst_id);

#ifndef SWIG
cpp_int cap_pics_read_table_entry(int chip_id, int inst_id, 
                                  uint32_t table_start, 
                                  uint32_t entry_size,
                                  uint32_t entries_per_line,
                                  uint32_t entry_idx);

cpp_int cap_pics_read_logical_table_entry(int chip_id, int inst_id, 
                                          uint32_t table_id, 
                                          uint32_t entry_idx,
                                          bool read_from_hw = false);

void cap_pics_load_table_entry(int chip_id, int inst_id, 
                               uint32_t table_start, 
                               uint32_t entry_size,
                               uint32_t entries_per_line,
                               uint32_t entry_idx,
                               cpp_int entry,
                               block_type type = SI);

void cap_pics_load_logical_table_entry(int chip_id, int inst_id, 
                                       uint32_t table_id, 
                                       uint32_t entry_idx,
                                       cpp_int entry,
                                       bool read_from_hw = false,
                                       block_type type = SI);

void cap_pics_add_logical_table(int chip_id, int inst_id, 
                                uint32_t table_id,
                                uint32_t upper_x,
                                uint32_t upper_y,
                                uint32_t lower_x,
                                uint32_t lower_y,
                                uint32_t entry_size_bytes,
                                uint32_t hash_table,
                                uint32_t opcode,
                                block_type type = SI);

void cap_pics_load_sram_def_from_cfg(int chip_id, int inst_id, block_type type = SI);

void cap_pics_check_progr_sanity(int chip_id, int inst_id, block_type type = SI);
#endif

void cap_pics_check_mem_values(int chip_id, int inst_id, int num_srams = 8);

void cap_pics_zero_init_sram(int chip_id, int inst_id, int num_srams = 8);

void cap_pics_csr_set_hdl_path(int chip_id, int inst_id, string path, bool is_pc = false);
void cap_picc_csr_set_hdl_path(int chip_id, int inst_id, string path, bool is_pc = false);

//void cap_pics_bgnd_prof_on_off(int chip_id, int inst_id, uint32_t bg_idx, uint32_t on_off);
void cap_pics_bgnd_prof_on_off(int chip_id, int inst_id, uint32_t vector);

void cap_pics_enable_cache(int chip_id, int inst_id);

void cap_pics_print_cache_cfg_info(int chip_id, int inst_id);
void cap_pics_cache_cnt(int chip_id, int inst_id);
void cap_pics_cache_tag_tbl_report(int chip_id, int inst_id, uint32_t addr_lo = 0, uint32_t addr_hi = ~0, bool show_addr = false);
int cap_pics_cache_get_bit(int s, int sel);
int cap_pics_cache_set_bit(int s, int sel, int val);
int cap_pics_cache_hash_func1(int chip_id, int inst_id, int key);
int cap_pics_cache_hash_func2(int chip_id, int inst_id, int hash, int key);
void cap_pics_invalidate_cache(int chip_id, int inst_id);

void cap_pics_bist_run(int chip_id, int inst_id, int enable);
void cap_pics_bist_chk(int chip_id, int inst_id);

#endif // CAP_PIC_API_H
