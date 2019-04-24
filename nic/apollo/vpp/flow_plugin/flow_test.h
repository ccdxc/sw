//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_FLOW_PLUGIN_FLOW_TEST_H__
#define __VPP_FLOW_PLUGIN_FLOW_TEST_H__

#include "flow.h"
#include "flow_memhash.h"

/* Stub for test - start */
#define KEY_TYPE_NONE                   0
#define KEY_TYPE_IPV4                   1
#define KEY_TYPE_IPV6                   2
#define KEY_TYPE_MAC                    3

typedef struct __attribute__((__packed__)) flow_swkey {
    uint8_t key_metadata_ktype;
    uint16_t key_metadata_sport;/* Sourced from field union */
    uint8_t key_metadata_proto;/* Sourced from field union */
    uint8_t key_metadata_src[16];/* Sourced from field union */
    uint16_t vnic_metadata_local_vnic_tag;/* Sourced from field union */
    uint16_t key_metadata_dport;/* Sourced from field union */
    uint8_t key_metadata_dst[16];/* Sourced from field union */
} flow_swkey_t;

typedef struct __attribute__((__packed__)) __flow_appdata {
    uint32_t flow_index;
    uint8_t flow_role;
} flow_appdata_t;

/* Stub for test - end */

typedef struct pen_flow_params_t {
    flow_swkey_t swkey;
} pen_flow_params_t;

typedef struct pen_flow_hw_ctx_t {
    u8 dummy;
} pen_flow_hw_ctx_t;

pen_flow_hw_ctx_t *flow_index_pool = NULL;

always_inline void
pen_flow_extract_prog_args_x1 (vlib_buffer_t *p0,
                               pen_flow_params_t *params_arr,
                               int *size, u8 is_ip4)
{
    pen_flow_params_t   *local_params0 = params_arr + (*size),
                        *remote_params0 = local_params0 + 1;
    udp_header_t        *udp0;

    *size = *size + 2;
    if (is_ip4) {
        ip4_header_t *ip40;

        ip40 = vlib_buffer_get_current(p0);

        local_params0->swkey.key_metadata_ktype = KEY_TYPE_IPV4;
        clib_memcpy(local_params0->swkey.key_metadata_src,
                ip40->src_address.data, sizeof(u32));
        clib_memcpy(local_params0->swkey.key_metadata_dst,
                ip40->dst_address.data, sizeof(u32));
        local_params0->swkey.key_metadata_proto = ip40->protocol;
        local_params0->swkey.vnic_metadata_local_vnic_tag =
                vnet_buffer (p0)->sw_if_index[VLIB_TX];

        remote_params0->swkey.key_metadata_ktype = KEY_TYPE_IPV4;
        clib_memcpy(remote_params0->swkey.key_metadata_dst,
                ip40->src_address.data, sizeof(u32));
        clib_memcpy(remote_params0->swkey.key_metadata_src,
                ip40->dst_address.data, sizeof(u32));
        remote_params0->swkey.key_metadata_proto = ip40->protocol;
        remote_params0->swkey.vnic_metadata_local_vnic_tag =
                vnet_buffer (p0)->sw_if_index[VLIB_TX];
        if (PREDICT_TRUE(((ip40->protocol == IP_PROTOCOL_TCP)
                || (ip40->protocol == IP_PROTOCOL_UDP)))) {
            udp0 = (udp_header_t *) (((u8 *) ip40) +
                    (vnet_buffer (p0)->l4_hdr_offset -
                            vnet_buffer (p0)->l3_hdr_offset));
            local_params0->swkey.key_metadata_sport =
                    clib_net_to_host_u16(udp0->src_port);
            local_params0->swkey.key_metadata_dport =
                    clib_net_to_host_u16(udp0->dst_port);

            remote_params0->swkey.key_metadata_dport =
                    clib_net_to_host_u16(udp0->src_port);
            remote_params0->swkey.key_metadata_sport =
                    clib_net_to_host_u16(udp0->dst_port);
        } else {
            local_params0->swkey.key_metadata_sport =
                    local_params0->swkey.key_metadata_dport = 0;
            remote_params0->swkey.key_metadata_sport =
                    remote_params0->swkey.key_metadata_dport = 0;
        }
    } else {

        ip6_header_t *ip60;

        ip60 = vlib_buffer_get_current(p0);

        local_params0->swkey.key_metadata_ktype = KEY_TYPE_IPV6;
        clib_memcpy(local_params0->swkey.key_metadata_src,
                ip60->src_address.as_u8, sizeof(ip6_address_t));
        clib_memcpy(local_params0->swkey.key_metadata_dst,
                ip60->dst_address.as_u8, sizeof(ip6_address_t));
        local_params0->swkey.key_metadata_proto = ip60->protocol;
        local_params0->swkey.vnic_metadata_local_vnic_tag =
                vnet_buffer (p0)->sw_if_index[VLIB_TX];

        remote_params0->swkey.key_metadata_ktype = KEY_TYPE_IPV6;
        clib_memcpy(remote_params0->swkey.key_metadata_dst,
                ip60->src_address.as_u8, sizeof(ip6_address_t));
        clib_memcpy(remote_params0->swkey.key_metadata_src,
                ip60->dst_address.as_u8, sizeof(ip6_address_t));
        remote_params0->swkey.key_metadata_proto = ip60->protocol;
        remote_params0->swkey.vnic_metadata_local_vnic_tag =
                vnet_buffer (p0)->sw_if_index[VLIB_TX];

        if (PREDICT_TRUE(((ip60->protocol == IP_PROTOCOL_TCP)
                || (ip60->protocol == IP_PROTOCOL_UDP)))) {
            udp0 = (udp_header_t *) (((u8 *) ip60) +
                    (vnet_buffer (p0)->l4_hdr_offset -
                            vnet_buffer (p0)->l3_hdr_offset));
            local_params0->swkey.key_metadata_sport =
                    clib_net_to_host_u16(udp0->src_port);
            local_params0->swkey.key_metadata_dport =
                    clib_net_to_host_u16(udp0->dst_port);
            remote_params0->swkey.key_metadata_dport =
                    clib_net_to_host_u16(udp0->src_port);
            remote_params0->swkey.key_metadata_sport =
                    clib_net_to_host_u16(udp0->dst_port);
        } else {
            local_params0->swkey.key_metadata_sport =
                    local_params0->swkey.key_metadata_dport = 0;
            remote_params0->swkey.key_metadata_sport =
                    remote_params0->swkey.key_metadata_dport = 0;
        }
    }
    return;
}

always_inline void
pen_flow_program_hw (pen_flow_params_t *key,
                     int size, u32 *counter)
{
    int i;
    flow_appdata_t swappdata = {0};
    mem_hash *table = pen_flow_prog_get_table();
    //pen_flow_hw_ctx_t *ctx;
    static uint32_t count = 0;

    pen_flow_prog_lock();
    for (i = 0; i < size; i++) {
        //pool_get(flow_index_pool, ctx);
        //swappdata.flow_index = ctx - flow_index_pool;
        key[i].entry.flow_index = flow_index;
        if (PREDICT_TRUE(0 == mem_hash_insert(table, &key[i].entry))) {
            counter[FLOW_PROG_COUNTER_FLOW_SUCCESS]++;
        } else {
            counter[FLOW_PROG_COUNTER_FLOW_FAILED]++;
        }
    }
    pen_flow_prog_unlock();
}

#endif    // __VPP_FLOW_PLUGIN_FLOW_TEST_H__
