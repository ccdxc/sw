
//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_DHCP_RELAY_VPPCFG_H__
#define __VPP_DHCP_RELAY_VPPCFG_H__

#ifdef __cplusplus
extern "C" {
#endif

void pds_dhcp_relay_cfg_init(void);

int
pds_dhcp4_cfg_add_del_all(uint16_t subnet);

int pds_dhcp4_relay_config_update(uint16_t subnet_id,
                                  uint8_t *id,
                                  uint32_t del);

int pds_dhcp4_proxy_config_update(uint16_t subnet_id,
                                  uint8_t *id,
                                  uint32_t del);

int
pds_dhcp4_server_add(uint8_t *id,
                     uint32_t server_ip,
                     uint32_t agent_ip,
                     uint16_t server_vpc);

int pds_dhcp4_server_del(uint8_t *id);

int pds_dhcp_relay_init_cb(bool external_server);

#ifdef __cplusplus
}
#endif

#endif      // __VPP_DHCP_RELAY_VPPCFG_H__
