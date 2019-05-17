//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_FLOW_PLUGIN_FLOW_APOLLO_H__
#define __VPP_FLOW_PLUGIN_FLOW_APOLLO_H__

#include <gen/p4gen/apollo/include/p4pd.h>
#include <nic/apollo/p4/include/defines.h>
#include <nic/utils/ftl/ftl_structs.hpp>
#include "flow.h"
#include "flow_prog_hw.h"

typedef struct pds_flow_params_s {
    ftentry_t entry;
    u32 hash;
} pds_flow_params_t;

typedef struct pds_flow_hw_ctx_s {
    u8 dummy;
} pds_flow_hw_ctx_t;

pds_flow_hw_ctx_t *session_index_pool = NULL;

#if 0
always_inline void
clib_memrev2 (u8 *dst1, u8* dst2, u8 *src, int size)
{
    for (src = src + (size - 1); size > 0; size--) {
        *dst1++ = *dst2++ = *src--;
    }
}
#endif

always_inline void
pds_flow_extract_prog_args_x1 (vlib_buffer_t *p0,
                               pds_flow_params_t *params_arr,
                               int *size, u8 is_ip4)
{
    pds_flow_params_t   *local_params0 = params_arr + (*size),
                        *remote_params0 = local_params0 + 1;
    udp_header_t        *udp0;
    u32                 *ip4_local0, *ip4_remote0;

    *size = *size + 2;
    if (is_ip4) {
        ip4_header_t *ip40;

        ip40 = vlib_buffer_get_current(p0);

        local_params0->entry.ktype = 
            remote_params0->entry.ktype = KEY_TYPE_IPV4;
        ip4_local0 = (u32 *) (local_params0->entry.src);
        ip4_remote0 = (u32 *) (remote_params0->entry.dst);
        *ip4_local0 = *ip4_remote0 =
            clib_net_to_host_u32(ip40->src_address.as_u32);
        ip4_local0 = (u32 *) (local_params0->entry.dst);
        ip4_remote0 = (u32 *) (remote_params0->entry.src);
        *ip4_local0 = *ip4_remote0 =
            clib_net_to_host_u32(ip40->dst_address.as_u32);
        local_params0->entry.proto =
            remote_params0->entry.proto = ip40->protocol;
        local_params0->entry.local_vnic_tag =
                remote_params0->entry.local_vnic_tag =
                vnet_buffer (p0)->sw_if_index[VLIB_TX];

        if (PREDICT_TRUE(((ip40->protocol == IP_PROTOCOL_TCP)
                || (ip40->protocol == IP_PROTOCOL_UDP)))) {
            udp0 = (udp_header_t *) (((u8 *) ip40) +
                    (vnet_buffer (p0)->l4_hdr_offset -
                            vnet_buffer (p0)->l3_hdr_offset));
            local_params0->entry.sport =
                    remote_params0->entry.dport =
                    clib_net_to_host_u16(udp0->src_port);
            local_params0->entry.dport =
                    remote_params0->entry.sport =
                    clib_net_to_host_u16(udp0->dst_port);
        } else {
            local_params0->entry.sport =
                    local_params0->entry.dport = 0;
            remote_params0->entry.sport =
                    remote_params0->entry.dport = 0;
        }
    } else {

        ip6_header_t *ip60;

        ip60 = vlib_buffer_get_current(p0);

        local_params0->entry.ktype =
            remote_params0->entry.ktype = KEY_TYPE_IPV6;
        clib_memcpy(local_params0->entry.src,
                     ip60->src_address.as_u8, sizeof(ip6_address_t));
        clib_memcpy(remote_params0->entry.dst,
                     ip60->src_address.as_u8, sizeof(ip6_address_t));
        clib_memcpy(local_params0->entry.dst,
                     ip60->dst_address.as_u8, sizeof(ip6_address_t));
        clib_memcpy(remote_params0->entry.src,
                     ip60->dst_address.as_u8, sizeof(ip6_address_t));
        local_params0->entry.proto =
                     remote_params0->entry.proto = ip60->protocol;
        local_params0->entry.local_vnic_tag =
                remote_params0->entry.local_vnic_tag =
                vnet_buffer (p0)->sw_if_index[VLIB_TX];

        if (PREDICT_TRUE(((ip60->protocol == IP_PROTOCOL_TCP)
                || (ip60->protocol == IP_PROTOCOL_UDP)))) {
            udp0 = (udp_header_t *) (((u8 *) ip60) +
                    (vnet_buffer (p0)->l4_hdr_offset -
                            vnet_buffer (p0)->l3_hdr_offset));
            local_params0->entry.sport =
                    remote_params0->entry.dport =
                    clib_net_to_host_u16(udp0->src_port);
            local_params0->entry.dport =
                    remote_params0->entry.sport =
                    clib_net_to_host_u16(udp0->dst_port);
        } else {
            local_params0->entry.sport =
                    local_params0->entry.dport = 0;
            remote_params0->entry.sport =
                    remote_params0->entry.dport = 0;
        }
    }
    local_params0->hash = vnet_buffer (p0)->pen_data.flow_hash;
    remote_params0->hash = 0;
    return;
}

always_inline void
pds_flow_program_hw (pds_flow_params_t *key,
                     int size, u16 *next, u32 *counter)
{
    int i;
    ftl *table = pds_flow_prog_get_table();
    pds_flow_hw_ctx_t *ctx;

    pds_flow_prog_lock();
    for (i = 0; i < size; i++) {
        pool_get(session_index_pool, ctx);
        key[i].entry.session_index = ctx - session_index_pool + 1;
        if (PREDICT_TRUE(0 == ftl_insert(table, &key[i].entry, key[i].hash))) {
            counter[FLOW_PROG_COUNTER_FLOW_SUCCESS]++;
            next[i/2] = FLOW_PROG_NEXT_FWD_FLOW;
        } else {
            counter[FLOW_PROG_COUNTER_FLOW_FAILED]++;
            next[i/2] = FLOW_PROG_NEXT_DROP;
            pool_put_index(session_index_pool, (key[i].entry.session_index - 1));
            if (i % 2) {
                /* Remove last entry as local flow succeeded
                 * but remote entry failed */
                pool_put_index(session_index_pool,
                               (key[i-1].entry.session_index - 1));
                /*TODO - Delete is resulting in crash, so comment for now */
#if 0
                if (PREDICT_FALSE(0 != ftl_remove(table,
                                                  &key[i-1].entry,
                                                  key[i-1].hash))) {
                    counter[FLOW_PROG_COUNTER_FLOW_DELETE_FAILED]++;
                }
#endif
            } else {
                /* Skip remote flow entry as local entry failed */
                i++;
            }
        }
    }
    pds_flow_prog_unlock();
}

#endif    // __VPP_FLOW_PLUGIN_FLOW_APOLLO_H__
