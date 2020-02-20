/*
* Copyright (c) 2019, Pensando Systems Inc.
*/

#include <cstdio>
#include <cstring>
#include <arpa/inet.h>
#include <endian.h>
#include "ncsi_param_db.h"
#include "lib/logger/logger.hpp"

namespace sdk {
namespace platform {
namespace ncsi {

NcsiParamDb::NcsiParamDb()
{
    NcsiParamResp = (GetParamRespPkt*) calloc(1, sizeof(struct GetParamRespPkt));
    NcsiParamResp->vlan_cnt = NCSI_CAP_VLAN_FILTER_COUNT;
    NcsiParamResp->mac_cnt = NCSI_CAP_MIXED_MAC_FILTER_COUNT;
}

void NcsiParamDb::UpdateNcsiParam(struct VlanFilterMsg& msg)
{
    uint32_t filter_id = msg.filter_id;

    if (!msg.enable)
        NcsiParamResp->vlan_flags = htons(ntohs(NcsiParamResp->vlan_flags) & ((uint16_t) ~(1 << (filter_id - 1))));
    else
        NcsiParamResp->vlan_flags = htons(ntohs(NcsiParamResp->vlan_flags) | ((uint16_t) (1 << (filter_id - 1))));

    //Update the vlan id
    NcsiParamResp->vlan[filter_id - 1] = htons(msg.vlan_id);
    SDK_TRACE_INFO("Updating vlan for filter_id: 0x%x vlan_id: 0x%x", filter_id, msg.vlan_id);
}

void NcsiParamDb::UpdateNcsiParam(struct MacFilterMsg& msg)
{
    uint32_t filter_id = msg.filter_id;

    if (!msg.enable)
        NcsiParamResp->mac_addr_flags &= ((uint8_t) ~(1 << (filter_id - 1)));
    else
        NcsiParamResp->mac_addr_flags |= ((uint8_t) (1 << (filter_id - 1)));

    //Update the mac addr
    memcpy(NcsiParamResp->mac[filter_id - 1], msg.mac_addr, sizeof(msg.mac_addr));
    SDK_TRACE_INFO("Updating mac for filter_id: 0x%x mac_addr: %02x:%02x:%02x:%02x:%02x:%02x", filter_id, msg.mac_addr[0], msg.mac_addr[1], msg.mac_addr[2], msg.mac_addr[3], msg.mac_addr[4], msg.mac_addr[5]);
}

void NcsiParamDb::UpdateNcsiParam(struct VlanModeMsg& msg)
{
    NcsiParamResp->vlan_mode = msg.mode;
}

void NcsiParamDb::UpdateNcsiParam(struct EnableChanMsg& msg)
{
    if(!msg.enable)
        NcsiParamResp->config_flags = htonl(ntohl(NcsiParamResp->config_flags) & (uint32_t) ~(1 << 1));
    else
        NcsiParamResp->config_flags = htonl(ntohl(NcsiParamResp->config_flags) | (uint32_t) (1 << 1)); 
}

void NcsiParamDb::UpdateNcsiParam(struct ResetChanMsg& msg)
{
    if (msg.reset) { 
        memset(NcsiParamResp, 0, sizeof(struct GetParamRespPkt));
        NcsiParamResp->vlan_cnt = NCSI_CAP_VLAN_FILTER_COUNT;
        NcsiParamResp->mac_cnt = NCSI_CAP_MIXED_MAC_FILTER_COUNT;
    }
}

void NcsiParamDb::UpdateNcsiParam(struct EnableChanTxMsg& msg)
{
    if(!msg.enable)
        NcsiParamResp->config_flags = htonl(ntohl(NcsiParamResp->config_flags) & (uint32_t) ~(1 << 2));
    else
        NcsiParamResp->config_flags = htonl(ntohl(NcsiParamResp->config_flags) | (uint32_t) (1 << 2)); 
}

void NcsiParamDb::UpdateNcsiParam(struct SetLinkMsg& msg)
{
    NcsiParamResp->link_mode = msg.link_up;
}

void NcsiParamDb::UpdateNcsiParam(struct EnableBcastFilterMsg& msg)
{
    NcsiParamResp->bc_mode = 0;

    SDK_TRACE_INFO("enable_arp: %d, enable_dhcp_client: %d, enable_dhcp_server: %d, enable_netbios: %d", msg.enable_arp, msg.enable_dhcp_client, msg.enable_dhcp_server, msg.enable_netbios);

    NcsiParamResp->bc_mode = htonl(((msg.enable_arp ? 1:0) << 0)
     | ((msg.enable_dhcp_client ? 1:0) << 1)
     | ((msg.enable_dhcp_server ? 1:0) << 2)
     | ((msg.enable_netbios ? 1:0) << 3));

    if ((msg.enable_arp || msg.enable_dhcp_client || msg.enable_dhcp_server || msg.enable_netbios))
        NcsiParamResp->config_flags = htonl(ntohl(NcsiParamResp->config_flags) | 1);
    else
        NcsiParamResp->config_flags = htonl(ntohl(NcsiParamResp->config_flags) & (uint32_t)~(1));
}

void NcsiParamDb::UpdateNcsiParam(struct EnableGlobalMcastFilterMsg& msg)
{
    if (msg.enable_ipv6_neigh_adv || msg.enable_ipv6_router_adv || msg.enable_dhcpv6_relay || msg.enable_dhcpv6_mcast || msg.enable_ipv6_mld || msg.enable_ipv6_neigh_sol)
        NcsiParamResp->config_flags = htonl(ntohl(NcsiParamResp->config_flags) | (uint32_t)((1 << 3)));
    else
        NcsiParamResp->config_flags = htonl(ntohl(NcsiParamResp->config_flags) & (uint32_t)(~(1 << 3)));
}

} // namespace ncsi
} // namespace platform
} // namespace sdk

