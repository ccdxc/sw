#ifndef CAP_PB_API_H
#define CAP_PB_API_H

#define CHK_SKNOB_EXISTS(x) \
   if (!sknobs_exists((char *) x.c_str())) { PLOG_ERR ("sknob "<<x<<" not found"<<endl); } \

#define PB_PRN_MSG(X) {\
        stringstream l_ss;\
        l_ss << X; \
        pb_print_msg(l_ss.str()); \
}


#include "cpu.h"
#include "cap_base_api.h"
#include "cap_blk_reg_model.h"
#include "sknobs.h"
#include "LogMsg.h"
#include "cap_pbc_csr.h"
#include "cap_pbm_csr.h"
#include "pknobs_reader.h"
#include "cap_csr_util_api.h"

void pb_print_msg(string msg); 
// soft reset sequence 
void cap_pb_soft_reset(int chip_id, int inst_id);
// soft reset, value = 1 means apply reset, value = 0 release soft reset
void cap_pb_set_soft_reset(int chip_id, int inst_id, int value);
// init start, no polling yet
void cap_pb_init_start(int chip_id, int inst_id);
// poll for init done
void cap_pb_init_done(int chip_id, int inst_id);
// use sknobs base load cfg 
void cap_pb_load_from_cfg(int chip_id, int inst_id);
void cap_pb_hbm_thr_from_api(int chip_id, int inst_id);
bool cap_pb_adjust_island_max(int chip_id, int inst_id);
void cap_pb_check_island_split(int chip_id, int inst_id);
void cap_pb_oq_ctl(int chip_id, int inst_id, uint32_t enable);
void cap_pb_hbm_force_xon(int chip_id, int inst_id, int pri, uint32_t enable);
void cap_pb_bump_up_min_resv(int chip_id, int inst_id);
void cap_pb_enable_write_ports(int chip_id, int inst_id) ;
// eos check, this functions should call eos_cnt, eos_int, eos_sta 
void cap_pb_eos(int chip_id, int inst_id);
void cap_pb_eos_cnt(int chip_id, int inst_id);
void cap_pb_eos_int(int chip_id, int inst_id);
void cap_pb_eos_sta(int chip_id, int inst_id);
void cap_pb_eos_fc_check(int chip_id, int inst_id);
//void cap_pb_eos_dhs_write(int chip_id, int inst_id);

bool cap_pb_is_empty(int chip_id, int inst_id);
void cap_pbc_csr_set_hdl_path(int chip_id, int inst_id, string path);
void cap_pbm_csr_set_hdl_path(int chip_id, int inst_id, string path);
void cap_pf_csr_set_hdl_path(int chip_id, int inst_id, string path);

void cap_pb_cfg_sched(int chip_id, int inst_id, int is_random=0);
void cap_pb_cfg_sched_node(int chip_id, int inst_id, int level, int oport, int queue_node, int quota);
void cap_pb_read_sched_node(int chip_id, int inst_id, int level, int oport, int queue_node, int & quota, int & current_credit);

void cap_pb_read_port_mon_in (int chip_id, int inst_id, int addr, cpp_int & rval);
void cap_pb_read_port_mon_out (int chip_id, int inst_id, int addr, cpp_int & rval);
void cap_pb_write_port_mon_in (int chip_id, int inst_id, int addr, cpp_int wval);
void cap_pb_write_port_mon_out (int chip_id, int inst_id, int addr, cpp_int wval);
void cap_pb_read_flits_cnt(int chip_id, int inst_id, int hbm, int sel, int port, cpp_int & rval); 
void cap_pb_write_flits_cnt(int chip_id, int inst_id, int hbm, int sel, int port, cpp_int wval); 

void cap_pb_wait_port_mon_init_done(int chip_id, int inst_id); 
void cap_pb_get_write_error_cnt(int chip_id, int inst_id, int port, int sel, cpp_int & rval); 
void cap_pb_read_hbm_ctx_mem(int chip_id, int inst_id, int port, int addr, cpp_int & rval); 
void cap_pb_write_hbm_ctx_mem(int chip_id, int inst_id, int port, int addr, cpp_int wval); 
void cap_pb_read_hbm_ctx_stat(int chip_id, int inst_id, int port, int addr, cpp_int & good_cnt, cpp_int & error_cnt);
void cap_pb_init_hbm_ctx_mem(int chip_id, int inst_id); 

void cap_pb_port_enable(int chip_id, int inst_id, int port, int value);
void cap_pb_port_enable_toggle(int chip_id, int inst_id, int port, int down_time_ns, int up_time_ns);

void cap_pb_port_flush(int chip_id, int inst_id, int port, int value);
void cap_pb_pcos_flush(int chip_id, int inst_id, int port, int cos, int value);

void cap_pb_read_emergency_stop_drop_count(int chip_id, int inst_id, int hbm, cpp_int & rval) ;
void cap_pb_read_write_ack_filling_up_drop_count(int chip_id, int inst_id, int hbm, cpp_int & rval) ;
void cap_pb_read_write_ack_full_drop_count(int chip_id, int inst_id, int hbm, cpp_int & rval) ;
void cap_pb_read_truncate_drop_count(int chip_id, int inst_id, int hbm, cpp_int & rval) ;
void cap_pb_read_ctrl_full_drop_count(int chip_id, int inst_id, int hbm, cpp_int & rval) ;

void cap_pb_dump_cntrs(int chip_id, int inst_id, int print_ctxt=0, int print_qdepth=0, int is_hex=0);
void cap_pb_clr_cntrs(int chip_id, int inst_id, int clr_ctxt=0); 

void cap_pb_dump_qdepth(int chip_id, int inst_id, int is_hex=0);
void cap_pb_rd_flush_cnt(int chip_id, int inst_id, int port, cpp_int & rval);
void cap_pb_rand_qisable(int chip_id, int inst_id, int loop_cnt) ;
void cap_pb_dump_iq_stats(int chip_id, int inst_id, int is_hex=0);
void cap_pb_dump_oq_stats(int chip_id, int inst_id, int is_hex=0);

void cap_pb_bist(int chip_id, int inst_id);
void cap_pb_bist_hbm_rd_wr_launch(int chip_id, int inst_id);
void cap_pb_bist_hbm_rb_launch(int chip_id, int inst_id);
void cap_pb_bist_hbm_wb_launch(int chip_id, int inst_id);
void cap_pb_bist_hbm_ctx_launch(int chip_id, int inst_id);
void cap_pb_bist_launch(int chip_id, int inst_id);
void cap_pb_bist_hbm_rd_wr_poll(int chip_id, int inst_id);
void cap_pb_bist_hbm_rb_poll(int chip_id, int inst_id);
void cap_pb_bist_hbm_wb_poll(int chip_id, int inst_id);
void cap_pb_bist_hbm_ctx_poll(int chip_id, int inst_id);
void cap_pb_bist_poll(int chip_id, int inst_id);
void cap_pm_bist_launch(int chip_id, int inst_id) ;
void cap_pm_bist_poll(int chip_id, int inst_id) ;
uint64_t cap_pb_dump_drop_and_err_cntrs(int chip_id, int inst_id, int no_print=0, int is_hex=0);

void cap_pb_bist_hbm_rd_wr_clear(int chip_id, int inst_id);
void cap_pb_bist_hbm_rb_clear(int chip_id, int inst_id);
void cap_pb_bist_hbm_wb_clear(int chip_id, int inst_id);
void cap_pb_bist_hbm_ctx_clear(int chip_id, int inst_id);
void cap_pb_bist_clear(int chip_id, int inst_id);
void cap_pm_bist_clear(int chip_id, int inst_id) ;

#endif // CAP_PB_API_H
