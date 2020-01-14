//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#include <vnet/dhcp/dhcp_proxy.h>
#include "pdsa_vpp_cfg.h"

int
pds_dhcp4_relay_config_update (uint32_t server_ip,
                               uint32_t agent_ip,
                               bool del) {
    ip46_address_t svr_addr;
    ip46_address_t src_addr;

    memset(&svr_addr, 0x0, sizeof(ip46_address_t));
    memset(&src_addr, 0x0, sizeof(ip46_address_t));
    svr_addr.ip4.as_u32 = server_ip;
    src_addr.ip4.as_u32 = agent_ip;

    return dhcp4_proxy_set_server(&svr_addr, &src_addr, 0, 0, del ? 1 : 0);
}
