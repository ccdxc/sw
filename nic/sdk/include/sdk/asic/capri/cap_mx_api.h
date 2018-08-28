// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __SDK_LINKMGR_CAP_MX_API_H__
#define __SDK_LINKMGR_CAP_MX_API_H__

#define MAX_CHANNEL 4
#define MAX_PORT    4
#define MAX_MAC     2

struct mac_profile_t {
   mac_mode_t mac_mode;
   int low_power_mode;
   int enable_pause;
   int enable_legacy_pause;
   int tx_pad_disable;
   int enable_mx_sd_init;
   int serdes_speed;
   int ld_sd_rom_bd;
   int for_simulation;
   int glbl_mode;
   int port_enable[MAX_PORT];
   int ch_mode[MAX_CHANNEL];
   int speed[MAX_CHANNEL];
   int enable[MAX_CHANNEL];
   int serdes_lp;
};

extern mac_profile_t mx[MAX_MAC];
extern mac_profile_t bx[MAX_MAC];

void cap_mx_set_soft_reset(int chip_id, int inst_id, int value);
void cap_mx_load_from_cfg(int chip_id, int inst_id, int rst);
void cap_mx_load_from_cfg_glbl1(int chip_id, int inst_id, int *ch_enable_vec);
void cap_mx_load_from_cfg_glbl2(int chip_id, int inst_id, int ch_enable_vec);
void cap_mx_cfg_ch(int chip_id, int inst_id, int ch);
void cap_mx_cfg_ch_en(int chip_id, int inst_id, int ch, int ch_enable_vec);
int  cap_mx_check_ch_sync(int chip_id, int inst_id, int ch);
void cap_mx_tx_pad_disable(int chip_id, int inst_id);
void cap_mx_eos_cnt(int chip_id, int inst_id);
void cap_mx_eos_int(int chip_id, int inst_id);
void cap_mx_eos_sta(int chip_id, int inst_id);
void cap_mx_sd_clr_error_cnt(int chip_id, int inst_id, int start_ch, int end_ch,
                             int poll_sleep_time, int wait_external_sd);
void cap_mx_set_ch_enable(int chip_id, int inst_id, int ch, int value);
void cap_mx_set_soft_reset(int chip_id, int inst_id, int ch, int value);
sdk_ret_t linkmgr_csr_init(void);
void cap_mx_mac_stat(int chip_id, int inst_id, int ch, int short_report);
void cap_mx_set_fec(int chip_id, int inst_id, int ch, int value);
void cap_mx_set_tx_padding(int chip_id, int inst_id, int enable);
void cap_mx_set_rx_padding(int chip_id, int inst_id, int ch, bool enable);
void cap_mx_set_mtu(int chip_id, int inst_id, int ch, int speed, int max_value);

void cap_bx_set_glbl_mode(int chip_id, int inst_id, int value);
void cap_bx_apb_write(int chip_id, int inst_id, int addr, int data);
void cap_bx_set_tx_rx_enable(int chip_id, int inst_id, int value);
void cap_bx_set_mtu(int chip_id , int inst_id, int max_value, int jabber_value);
void cap_bx_set_ch_enable(int chip_id, int inst_id, int value);
void cap_bx_set_soft_reset(int chip_id, int inst_id, int value);
int cap_bx_check_sync(int chip_id, int inst_id);

void cap_pp_sbus_write(int chip_id, int rcvr_addr, int data_addr, int data);
int  cap_pp_sbus_read(int chip_id, int rcvr_addr, int data_addr);
void cap_ms_sbus_write(int chip_id, int rcvr_addr, int data_addr, int data);
int  cap_ms_sbus_read(int chip_id, int rcvr_addr, int data_addr);

#endif // __SDK_LINKMGR_CAP_MX_API_H__
