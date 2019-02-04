#ifndef CAP_BX_API_H
#define CAP_BX_API_H

#include "cpu.h"
#include "cap_base_api.h"
#include "cap_blk_reg_model.h"
#include "sknobs.h"
#include "LogMsg.h"
#include "cap_bx_csr.h"

#define PRN_MSG(X) {\
        stringstream l_ss;\
        l_ss << X; \
        print_msg(l_ss.str()); \
}

#ifdef _CSV_INCLUDED_
extern "C" void top_bx_ld_spico_rom(const char * rom_file);
extern "C" void ext_bx_ld_spico_rom(const char * rom_file);
extern "C" void ext_bx_get_core_int_in_progress(int & val);
#endif    

void cap_bx_soft_reset(int chip_id, int inst_id);
void cap_bx_set_soft_reset(int chip_id, int inst_id, int value); 
void cap_bx_init_start(int chip_id, int inst_id);
void cap_bx_init_done(int chip_id, int inst_id);
void cap_bx_load_from_cfg(int chip_id, int inst_id);
void cap_bx_eos(int chip_id, int inst_id);
void cap_bx_mac_stat(int chip_id, int inst_id, int port, int short_report=1);
void cap_bx_eos_cnt(int chip_id, int inst_id);
void cap_bx_eos_int(int chip_id, int inst_id);
void cap_bx_eos_sta(int chip_id, int inst_id);
void cap_bx_set_debug_ctl(int chip_id, int inst_id, int enable, int select);
void print_msg(string msg); 

int cap_bx_apb_read(int chip_id, int inst_id, int addr);
void cap_bx_apb_write(int chip_id, int inst_id, int addr, int data);
void cap_bx_set_ch_enable(int chip_id, int inst_id, int value);
void cap_bx_set_glbl_mode(int chip_id, int inst_id, int value);
void cap_bx_set_tx_rx_enable(int chip_id, int inst_id, int value);
void cap_bx_set_mtu(int chip_id , int inst_id, int max_value, int jabber_value);
void cap_bx_set_pause(int chip_id , int inst_id, int pri_vec, int legacy);
void cap_bx_show_mac_mode(int chip_id, int inst_id);
void cap_bx_set_mac_mode(int chip_id, int inst_id, string new_mode);
int cap_bx_get_port_enable_state(int chip_id, int inst_id, int port);
int cap_bx_get_mac_detail(int chip_id, int inst_id, string field);

void cap_bx_set_an_ability(int chip_id, int inst_id, int value); 
int cap_bx_rd_rx_an_ability(int chip_id, int inst_id);
void cap_bx_start_an(int chip_id, int inst_id); 
void cap_bx_stop_an(int chip_id, int inst_id); 
void cap_bx_wait_an_done(int chip_id, int inst_id); 
void cap_bx_set_an_link_timer(int chip_id, int inst_id, int value);

int cap_bx_check_tx_idle(int chip_id, int inst_id);
int cap_bx_check_sync(int chip_id, int inst_id);
void cap_bx_set_tx_rx_enable(int chip_id, int inst_id, int tx_enable, int rx_enable);

void cap_bx_sd_core_interrupt(int chip_id, int inst_id, int int_code, int int_data, int poll_sleep_time=10000, int wait_external_sd=0);  
void cap_bx_sd_config_speed(int chip_id, int inst_id, string speed, int poll_sleep_time=10000, int wait_external_sd=0); 
void cap_bx_sd_ld_firmware(int chip_id, int inst_id,  const char * rom_file, int backdoor=0);
void cap_ext_bx_sd_ld_firmware(int chip_id, int inst_id, const char * rom_file, int backdoor=0);

void cap_bx_sd_start_prbs(int chip_id, int inst_id, int prbs_select, int poll_sleep_time=10000, int wait_external_sd=0); 
void cap_bx_sd_stop_prbs_check(int chip_id, int inst_id, int poll_sleep_time=10000, int wait_external_sd=0); 
void cap_bx_sd_stop_prbs(int chip_id, int inst_id, int poll_sleep_time=10000, int wait_external_sd=0); 
void cap_bx_sd_stop_tx_prbs(int chip_id, int inst_id, int poll_sleep_time=10000, int wait_external_sd=0); 
void cap_bx_sd_stop_rx_prbs(int chip_id, int inst_id, int poll_sleep_time=10000, int wait_external_sd=0); 
int cap_bx_sd_read_error_cnt(int chip_id, int inst_id, int poll_sleep_time=10000, int wait_external_sd=0); 
void cap_bx_sd_clr_error_cnt(int chip_id, int inst_id, int poll_sleep_time=10000, int wait_external_sd=0);
void cap_bx_set_core_to_cntl(int chip_id, int inst_id, int wval);
//zebu compile void cap_bx_set_mx_sd_loopback(int chip_id, int inst_id, int lpbk_mode, int poll_sleep_time=10000, int wait_external_sd=0);
//zebu compile void cap_bx_clr_mx_sd_loopback(int chip_id, int inst_id, int lpbk_mode, int poll_sleep_time=10000, int wait_external_sd=0);
void cap_bx_sd_sbus_interrupt(int chip_id, int inst_id, int int_code, int int_data, int wait_timeout=100);
void cap_bx_sd_sbus_config_speed(int chip_id, int inst_id, string speed, int wait_timeout=100); 
void cap_bx_sd_sbus_dis_core_interrupt(int chip_id, int inst_id);

void cap_bx_bist_run(int chip_id, int inst_id, int enable);
void cap_bx_bist_chk(int chip_id, int inst_id);

#endif // CAP_PB_API_H
