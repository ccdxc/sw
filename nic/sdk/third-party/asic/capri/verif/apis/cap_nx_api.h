#ifndef CAP_NX_API_H
#define CAP_NX_API_H

#ifdef CAPRI_SW
#include "cap_base_api.h"
#include "cap_top_csr_defines.h"
#include "cap_pics_c_hdr.h"
#include "cap_hese_c_hdr.h"
#include "cap_mpse_c_hdr.h"
#include "cap_hens_c_hdr.h"
#include "cap_ms_c_hdr.h"
#include "ns_soc_ip.h"
#else
#include "cpu.h"
#include "cap_base_api.h"
#include "cap_top_csr_defines.h"
#include "cap_pics_c_hdr.h"
#include "cap_hese_c_hdr.h"
#include "cap_mpse_c_hdr.h"
#include "cap_hens_c_hdr.h"
#include "cap_ms_c_hdr.h"
#include "cap_blk_reg_model.h"
#include "sknobs.h"
#include "LogMsg.h"
#include "ns_soc_ip.h"
#endif

// soft reset sequence 
void cap_nx_soft_reset(int chip_id, int inst_id);
void cap_nx_enable_rbm_access(int chip_id, int inst_id) ;
void cap_nx_soft_reset_sw(int chip_id, int inst_id, int cache_enable, int pgm_coh, int cache_override, int cache_override_val, int clr_modify_on_masters, int setup_event_counters, int nwl_hash_mode);
void cap_nx_enable_1G_to_3G(int chip_id, int inst_id) ;
void cap_nx_ms_filter_db_sema(int chip_id, int inst_id) ;
void cap_nx_set_soft_reset(int chip_id, int inst_id, int value);
void cap_nx_set_axi_attrs(int chip_id, int inst_id ) ;
void cap_nx_set_md_axi_attrs(int chip_id, int inst_id ) ;
void cap_nx_sw_reset(int chip_id, int inst_id);
void cap_nx_init_start(int chip_id, int inst_id);
void cap_nx_init_done(int chip_id, int inst_id);

#ifndef CAPRI_SW
void cap_nx_set_pb_high_priority(uint32_t qos );
#endif
void cap_nx_set_pb_high_priority(uint32_t qos0, uint32_t qos1, uint32_t qos2, uint32_t qos3 );
void cap_nx_set_pt_high_priority(uint32_t qos0, uint32_t qos1, uint32_t qos2, uint32_t qos3 );
void cap_nx_set_md_high_priority(uint32_t qos0, uint32_t qos1, uint32_t qos2, uint32_t qos3 );
void cap_nx_cache_disable(void) ;
void cap_nx_cache_enable(void) ;
void cap_nx_global_alloc(uint32_t alloc_value);
void cap_nx_axcache_enable(void);
void cap_nx_clr_modifiable(void) ;
void cap_nx_cache_override(uint8_t cache_val) ;
void cap_nx_force_alloc_enable(uint32_t value);
void cap_nx_enable_interrupts(void) ;
void cap_nx_disable_interrupts(void) ;
void cap_nx_clear_interrupts(int chip_id) ;
void cap_nx_set_hash_func(uint64_t hash3_bmap, uint64_t hash2_bmap, uint64_t hash1_bmap, uint64_t hash0_bmap) ;
void cap_nx_get_hash(const char *master_name);
void cap_nx_flush_cache(void) ;
void cap_nx_wait_flush_done(void) ;
void cap_nx_invalidate_cache(void);
void cap_nx_wait_invalidate_done(void) ;

void cap_nx_ecc_disable(void) ;
void cap_nx_field_print_am_err(int chip_id, int inst_id, uint64_t value) ;
void cap_nx_field_print_as_err(int chip_id, int inst_id, uint64_t value) ;
void cap_nx_display_ecc_count(void) ;
void display_ecc_info(int ccc_num, uint32_t val) ;
void cap_nx_setup_llc_counters(uint32_t event_mask) ;
void cap_nx_display_llc_counters(void) ;
void cap_nx_display_cache_enables(void) ;
void cap_nx_setup_ccc_counters(uint32_t event_mask) ;
void cap_nx_display_ccc_counters(void) ;
void cap_nx_setup_am_counters(uint32_t count_lat) ;
void cap_nx_setup_mbridge_counters(uint64_t caddr, uint64_t caddr_msk, uint64_t ccmd0, uint32_t ccmd0_msk, uint64_t ccmd1, uint32_t ccmd1_msk) ;
unsigned int cap_nx_read_pb_axi_cnt(int rd) ;
void cap_nx_display_aw_ar_cnt(void);
void cap_nx_display_master_err_addr(void);


void cap_nx_display_aranges(int chip_id, int inst_id) ;
void cap_nx_display_bg_range(int chip_id, int inst_id, const char *range_name, const char *master_name, const char *slave_name) ;
void cap_nx_display_fg_sec_range(int chip_id, int inst_id, const char *range_name, const char *master_name, const char *slave_name) ;
void cap_nx_display_fg_nc_range(int chip_id, int inst_id, const char *range_name, const char *master_name, const char *slave_name) ;
void cap_display_address_range_hdr(void) ;
void cap_display_address_range_val(const char *rname, const char *mname, const char *sname, uint64_t adbase, uint64_t admask) ;
void cap_display_address_range_entry(int chip_id, int inst_id, const char *range_name, const char *master_name, const char *slave_name, uint64_t adbase_addr, uint64_t admask_addr) ;

void cap_nx_add_secure_md_to_hbm(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask) ;
void cap_nx_add_secure_pt_to_hbm(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask) ;
void cap_nx_add_secure_pr_to_hbm(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask) ;
void cap_nx_add_secure_ar_to_hbm(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask) ;
void cap_nx_add_secure_pb_to_hbm(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask) ;
void cap_nx_add_secure_ss_to_hbm(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask) ;
void cap_nx_add_secure_ms_to_hbm(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask) ;
void cap_nx_add_secure_px_to_hbm(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask) ;
void cap_nx_add_noncoh_secure_md_to_hbm(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask);
void cap_nx_add_noncoh_secure_pt_to_hbm(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask);
void cap_nx_add_noncoh_secure_pr_to_hbm(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask);
void cap_nx_add_noncoh_secure_ar_to_hbm(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask);
void cap_nx_add_noncoh_secure_pb_to_hbm(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask);
void cap_nx_add_noncoh_secure_ss_to_hbm(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask);
void cap_nx_add_noncoh_secure_ms_to_hbm(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask);
void cap_nx_add_noncoh_secure_px_to_hbm(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask);

void cap_nx_add_secure_pt_to_ms(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask) ;
void cap_nx_add_secure_pr_to_ms(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask) ;
void cap_nx_add_secure_md_to_ms(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask) ;
void cap_nx_add_secure_ss_to_ms(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask) ;
void cap_nx_add_secure_ar_to_ms(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask) ;
void cap_nx_add_secure_pb_to_ms(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask) ;
void cap_nx_add_secure_ms_to_ms(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask) ;
void cap_nx_add_secure_px_to_ms(int chip_id, int inst_id, uint32_t range_num, uint64_t adbase, uint64_t admask) ;

// use sknobs base load cfg 
void cap_nx_load_from_cfg(int chip_id, int inst_id);
// eos check, this functions should call eos_cnt, eos_int, eos_sta 
void cap_nx_eos(int chip_id, int inst_id);
void cap_nx_eos_cnt(int chip_id, int inst_id);
void cap_nx_eos_int(int chip_id, int inst_id);
void cap_nx_eos_sta(int chip_id, int inst_id);

#ifndef SWIG
void nx_err_check_template (uint64_t rd_addr, const char *reg_name, uint32_t err_cmp_val);
#endif

#endif // CAP_NX_API_H
