
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

#ifdef __cplusplus
}
#endif

#endif      // __VPP_DHCP_RELAY_VPPCFG_H__
