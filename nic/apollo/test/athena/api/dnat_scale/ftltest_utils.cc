//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "ftltest_utils.hpp"

#define UPDATE_DELTA 1

void
fill_key (uint32_t index, pds_dnat_mapping_key_t *key)
{
    uint8_t key_ip[IP6_ADDR8_LEN];

    memset(key, 0, sizeof(pds_dnat_mapping_key_t));
    memset(key_ip, 0xFF, IP6_ADDR8_LEN);
    key->vnic_id = (index % 511) + 1;
    key->key_type = IP_AF_IPV6;
#if 0
    if (index < 256)
        key->key_type = IP_AF_IPV4 + (index % 2);
    else
        key->key_type = IP_AF_IPV4 + !(index % 2);
#endif
    key_ip[0] = 0x22;
    key_ip[1] = index >> 1 & 0xFF;
    key_ip[2] = (index >> 8 & 0xF0) | 0x0F;
    key_ip[3] = (index & 0x0F) | 0xF0;
    if (key->key_type == IP_AF_IPV6) {
        key_ip[4] = index & 0xFF;
        key_ip[5] = index >> 8 & 0xFF;
        key_ip[6] = (index >> 8 & 0x0F) | 0xF0;
        key_ip[7] = (index & 0xF0) | 0x0F;
        key_ip[8] = index >> 2 & 0xFF;
        key_ip[9] = index >> 4 & 0xFF;
        key_ip[10] = index >> 10 & 0xFF;
        key_ip[11] = index >> 12 & 0xFF;
        key_ip[12] = (index >> 8 & 0x0F) | 0xF0;
        key_ip[13] = index << 2 & 0xFF;
        key_ip[14] = index << 4 & 0xFF;
        key_ip[15] = (index >> 12 & 0xF0) | 0xF0;
        memcpy(key->addr, key_ip, IP6_ADDR8_LEN);
    } else {
        memcpy(key->addr, key_ip, IP4_ADDR8_LEN);
    }
    return;
}

void
fill_data (uint32_t index, pds_dnat_mapping_data_t *data)
{
    uint8_t data_ip[IP6_ADDR8_LEN];

    memset(data, 0, sizeof(pds_dnat_mapping_data_t));
    memset(data_ip, 0xFF, IP6_ADDR8_LEN);
    data->epoch = index;
    //data->addr_type = IP_AF_IPV4 + (index % 2);
    data->addr_type = IP_AF_IPV6;
    data_ip[0] = 0x33;
    data_ip[2] = index >> 8 & 0xFF;
    data_ip[3] = index & 0xFF;
    if (data->addr_type == IP_AF_IPV6) {
        data_ip[4] = index & 0xFF;
        data_ip[5] = index >> 8 & 0xFF;
        data_ip[14] = index >> 8 & 0xFF;
        data_ip[15] = index & 0xFF;
        memcpy(data->addr, data_ip, IP6_ADDR8_LEN);
    } else {
        memcpy(data->addr, data_ip, IP4_ADDR8_LEN);
    }
    return;
}

void
update_data (uint32_t index, pds_dnat_mapping_data_t *data)
{
    uint8_t data_ip[IP6_ADDR8_LEN];

    memset(data, 0, sizeof(pds_dnat_mapping_data_t));
    memset(data_ip, 0xFF, IP6_ADDR8_LEN);
    data->epoch = index + UPDATE_DELTA;
    //data->addr_type = IP_AF_IPV4 + (index % 2);
    data->addr_type = IP_AF_IPV6;
    data_ip[0] = 0x32;
    data_ip[2] = index >> 8 & 0xFF;
    data_ip[3] = index & 0xFF;
    if (data->addr_type == IP_AF_IPV6) {
        data_ip[4] = index & 0xFF;
        data_ip[5] = index >> 8 & 0xFF;
        data_ip[6] = index >> 16 & 0xFF;
        data_ip[7] = index >> 24 & 0xFF;
        data_ip[12] = index >> 24 & 0xFF;
        data_ip[13] = index >> 16 & 0xFF;
        data_ip[14] = index & 0xFF;
        data_ip[15] = index >> 8 & 0xFF;
        memcpy(data->addr, data_ip, IP6_ADDR8_LEN);
    } else {
        memcpy(data->addr, data_ip, IP4_ADDR8_LEN);
    }
    return;
}
