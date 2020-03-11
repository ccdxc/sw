
//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_DHCP_RELAY_VPPCFG_H__
#define __VPP_DHCP_RELAY_VPPCFG_H__

#ifdef __cplusplus
extern "C" {
#endif

void pds_dhcp_relay_cfg_init(void);
int pds_dhcp4_relay_config_update(uint32_t server_ip,
                                  uint32_t agent_ip,
                                  bool del);

int pds_dhcp_relay_init_cb(bool external_server);

typedef struct dhcp_relay_cfg_main_s {
    uint32_t *svr_ip_list;
} dhcp_relay_cfg_main_t;

extern dhcp_relay_cfg_main_t dhcp_relay_cfg_main;

#ifdef __cplusplus
}
#endif

#endif      // __VPP_DHCP_RELAY_VPPCFG_H__
