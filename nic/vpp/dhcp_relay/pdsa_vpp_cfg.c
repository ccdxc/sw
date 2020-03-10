//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#include <vnet/dhcp/dhcp_proxy.h>
#include <arpa/inet.h>
#include "pdsa_vpp_cfg.h"

int
pds_dhcp4_relay_config_update (uint32_t server_ip,
                               uint32_t agent_ip,
                               bool del) {
    ip46_address_t svr_addr;
    ip46_address_t src_addr;
    u32 *svr_ip = NULL;
    static bool dhcp_relay_initialized;

    if (!dhcp_relay_initialized) {
        /*
         * Initialize relay to relay to external server
         */
        pds_dhcp_relay_init_cb(true);
        dhcp_relay_initialized = true;
    }

    memset(&svr_addr, 0x0, sizeof(ip46_address_t));
    memset(&src_addr, 0x0, sizeof(ip46_address_t));
    svr_addr.ip4.as_u32 = clib_host_to_net_u32(server_ip);
    src_addr.ip4.as_u32 = clib_host_to_net_u32(agent_ip);

    // maintain server ip list for all the config objects
    if (!del) {
        pool_get(svr_ip_list, svr_ip);
        *svr_ip = svr_addr.ip4.as_u32;
    } else {
        //TODO do pool_put
        pool_foreach(svr_ip, svr_ip_list, ({
            if(*svr_ip == svr_addr.ip4.as_u32) {
                break;
            }
        }));

        if(svr_ip) {
            pool_put(svr_ip_list, svr_ip);
        }
    }

    return dhcp4_proxy_set_server(&svr_addr, &src_addr, 0, 0, del ? 1 : 0);
}
