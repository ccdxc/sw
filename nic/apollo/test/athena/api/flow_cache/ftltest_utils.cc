//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "ftltest_utils.hpp"

void
fill_key (uint32_t index, pds_flow_key_t *key)
{
    uint8_t src_ip[IP6_ADDR8_LEN] = {0};
    uint8_t dst_ip[IP6_ADDR8_LEN] = {0};
    uint8_t smac[8] = {0};
    uint8_t dmac[8] = {0};

    memset(key, 0, sizeof(pds_flow_key_t));
    key->ip_proto = IP_PROTO_UDP;
    key->l4.tcp_udp.sport = index;
    key->l4.tcp_udp.dport = index;
    key->vnic_id = index;

    key->ip_addr_family = IP_AF_IPV6;
    src_ip[0] = 0x22;
    src_ip[12] = index >> 24 & 0xFF;
    src_ip[13] = index >> 16 & 0xFF;
    src_ip[14] = index >> 8 & 0xFF;
    src_ip[15] = index & 0xFF;
    memcpy(key->ip_saddr, src_ip, IP6_ADDR8_LEN);

    dst_ip[0] = 0x33;
    dst_ip[12] = index >> 24 & 0xFF;
    dst_ip[13] = index >> 16 & 0xFF;
    dst_ip[14] = index >> 8 & 0xFF;
    dst_ip[15] = index & 0xFF;
    memcpy(key->ip_daddr, dst_ip, IP6_ADDR8_LEN);

    smac[0] = 0xaa;
    smac[5] = index & 0xFF;
    memcpy(&key->smac, (uint64_t *)smac, ETH_ADDR_LEN);

    dmac[0] = 0xbb;
    dmac[5] = index & 0xFF;
    memcpy(&key->dmac, (uint64_t *)dmac, ETH_ADDR_LEN);
    return;
}

void
fill_data (uint32_t index, pds_flow_spec_index_type_t index_type,
           pds_flow_data_t *data)
{
    memset(data, 0, sizeof(pds_flow_data_t));
    data->index = index;
    data->index_type = index_type;
    return;
}
