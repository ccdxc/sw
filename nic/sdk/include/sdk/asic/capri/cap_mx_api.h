// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __SDK_LINKMGR_CAP_MX_API_H__
#define __SDK_LINKMGR_CAP_MX_API_H__

#define MAX_CHANNEL 4
#define MAX_PORT    4
#define MAX_MAC     2

enum mac_mode_t {
   MAC_MODE_1x100g,
   MAC_MODE_1x40g,
   MAC_MODE_1x50g,
   MAC_MODE_2x40g,
   MAC_MODE_2x50g,
   MAC_MODE_4x25g,
   MAC_MODE_4x10g,
   MAC_MODE_4x1g,
   MAC_MODE_1x50g_2x25g,
   MAC_MODE_2x25g_1x50g
};

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

void cap_mx_set_soft_reset(int chip_id, int inst_id, int value);
void cap_mx_load_from_cfg(int chip_id, int inst_id, int rst);
void cap_mx_load_from_cfg_glbl1(int chip_id, int inst_id, int *ch_enable_vec);
void cap_mx_load_from_cfg_glbl2(int chip_id, int inst_id, int ch_enable_vec);
void cap_mx_load_from_cfg_channel(int chip_id, int inst_id, int ch,
                                  int ch_enable_vec);
int  cap_mx_check_ch_sync(int chip_id, int inst_id, int ch);
void cap_mx_tx_pad_disable(int chip_id, int inst_id);
void cap_mx_eos_cnt(int chip_id, int inst_id);
void cap_mx_eos_int(int chip_id, int inst_id);
void cap_mx_eos_sta(int chip_id, int inst_id);
void cap_mx_sd_clr_error_cnt(int chip_id, int inst_id, int start_ch, int end_ch,
                             int poll_sleep_time, int wait_external_sd);
void cap_mx_set_ch_enable(int chip_id, int inst_id, int ch, int value);
void cap_mx_set_soft_reset(int chip_id, int inst_id, int ch, int value);

#endif // __SDK_LINKMGR_CAP_MX_API_H__
