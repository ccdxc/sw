//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#include <vnet/dhcp/dhcp_proxy.h>
#include <arpa/inet.h>
#include "pdsa_vpp_cfg.h"
#include "node.h"

#define MAX_SUBNET_HW_ID 2048

int
pds_dhcp4_cfg_add_del_all (uint16_t subnet)
{
    dhcp_relay_main_t *dmain = &dhcp_relay_main;
    u16 pool_idx;
    dhcp_relay_policy_t *policy;

    if (subnet > MAX_SUBNET_HW_ID) {
        return -1;
    }
    pool_idx = dmain->policy_pool_idx[subnet];

    if (0xFFFF != pool_idx) {
        policy = pool_elt_at_index(dmain->policy_pool, pool_idx);
        pool_free(policy->servers);
        pool_put(dmain->policy_pool, policy);
        dmain->policy_pool_idx[subnet] = 0xFFFF;
        return 0;
    }
    return -1;
}

int
pds_dhcp4_cfg_add_del (uint16_t subnet,
                       u8 *id,
                       u8 local_server,
                       uint32_t del)
{
    dhcp_relay_main_t *dmain = &dhcp_relay_main;
    u16 pool_idx;
    dhcp_relay_policy_t *policy;
    dhcp_relay_server_t *server = NULL;
    u16 *server_idx;
    u8 existing_svr = 0;

    if (subnet > MAX_SUBNET_HW_ID) {
        return -1;
    }
    pool_idx = dmain->policy_pool_idx[subnet];

    pool_foreach(server, dmain->server_pool, (({
        if (0 == memcmp(server->obj_id, id, PDS_OBJ_ID_LEN)) {
            existing_svr = 1;
            goto done;
        }
    })));

done:
    if (!existing_svr) {
        return -1;
    }

    if (0xFFFF != pool_idx) {
        // update case
        policy = pool_elt_at_index(dmain->policy_pool, pool_idx);
    } else if (!del) {
        pool_get(dmain->policy_pool, policy);
        dmain->policy_pool_idx[subnet] = policy - dmain->policy_pool;
        policy->subnet_hw_id= subnet;
        policy->local_server = local_server;
        policy->servers = NULL;
    } else {
        // delete case
        // we don't have dhcp policy for this as pool_idx if 0xffff
        return -1;
    }

    if (!del) {
        pool_get(policy->servers, server_idx);
        *server_idx = server - dmain->server_pool;
        return 0;
    }
    // delete case
    pool_foreach(server_idx, policy->servers, (({
        if (*server_idx == (server - dmain->server_pool)) {
            goto found;
        }
    })));

    return -1;

found:
    pool_put(policy->servers, server_idx);
    if (0 == pool_elts(policy->servers)) {
        pool_free(policy->servers);
        dmain->policy_pool_idx[subnet] = 0xFFFF;
        pool_put(dmain->policy_pool, policy);
    }
    return 0;
}

int
pds_dhcp4_relay_config_update (uint16_t subnet_id,
                               uint8_t *id,
                               uint32_t del)
{
    return pds_dhcp4_cfg_add_del(subnet_id, id, 0, del);
}

int
pds_dhcp4_proxy_config_update (uint16_t subnet_id,
                               uint8_t *id,
                               uint32_t del)
{
    return pds_dhcp4_cfg_add_del(subnet_id, id, 1, del);
}

int
pds_dhcp4_server_add (uint8_t *id,
                      uint32_t server_ip,
                      uint32_t agent_ip,
                      uint16_t server_vpc)
{
    dhcp_relay_main_t *dmain = &dhcp_relay_main;
    dhcp_relay_server_t *server;
    ip46_address_t svr_addr = ip46_address_initializer;
    ip46_address_t src_addr = ip46_address_initializer;

    svr_addr.ip4.as_u32 = clib_host_to_net_u32(server_ip);
    src_addr.ip4.as_u32 = clib_host_to_net_u32(agent_ip);

    pool_foreach(server, dmain->server_pool, (({
        if (0 == memcmp(server->obj_id, id, PDS_OBJ_ID_LEN)) {
            // existing server update
            goto found;
        }
    })));
    // new server add
    pool_get(dmain->server_pool, server);
    clib_memcpy(server->obj_id, id, PDS_OBJ_ID_LEN);

found:
    server->relay_addr = src_addr;
    server->server_addr = svr_addr;
    server->server_vpc = server_vpc;
    return 0;
}

int
pds_dhcp4_server_del (uint8_t *id)
{
    dhcp_relay_main_t *dmain = &dhcp_relay_main;
    dhcp_relay_server_t *server;

    pool_foreach(server, dmain->server_pool, (({
        if (0 == memcmp(server->obj_id, id, PDS_OBJ_ID_LEN)) {
            goto found;
        }
    })));

    return -1;
found:
    pool_put(dmain->server_pool, server);
    return 0;
}
