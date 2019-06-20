//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_FLOW_PLUGIN_FLOW_ARTEMIS_H__
#define __VPP_FLOW_PLUGIN_FLOW_ARTEMIS_H__

#include <gen/p4gen/artemis/include/p4pd.h>
#include <stddef.h>

#if 0
always_inline void
clib_memrev2 (u8 *dst1, u8* dst2, u8 *src, int size)
{
    for (src = src + (size - 1); size > 0; size--) {
        *dst1++ = *dst2++ = *src--;
    }
}
#endif

always_inline int
pds_session_get_advance_offset (void)
{
    return (ARTEMIS_P4_TO_ARM_HDR_SZ
            - offsetof(p4_rx_cpu_hdr_t, ses_info) - ARTEMIS_PREDICATE_HDR_SZ);
}

always_inline void
pds_session_prog_x2 (vlib_buffer_t **b, u32 session_id0,
                     u32 session_id1, u16 *next, u32 *counter)
{
    void *ses_info0 = vlib_buffer_get_current(b[0]);
    void *ses_info1 = vlib_buffer_get_current(b[1]);

    session_insert(session_id0, ses_info0);
    session_insert(session_id1, ses_info1);
    next[0] = next[1] = SESSION_PROG_NEXT_FWD_FLOW;

    vlib_buffer_advance(b[0], pds_session_get_advance_offset());
    vlib_buffer_advance(b[1], pds_session_get_advance_offset());
}

always_inline void
pds_session_prog_x1 (vlib_buffer_t *b, u32 session_id,
                     u16 *next, u32 *counter)
{
    void *ses_info = vlib_buffer_get_current(b);

    session_insert(session_id, ses_info);
    *next = SESSION_PROG_NEXT_FWD_FLOW;
    vlib_buffer_advance(b, pds_session_get_advance_offset());
}

always_inline int
pds_flow_prog_get_next_offset(vlib_buffer_t *p0)
{
    return (ARTEMIS_P4_TO_ARM_HDR_SZ - offsetof(p4_rx_cpu_hdr_t, ses_info) +
               (vnet_buffer(p0)->l3_hdr_offset - vnet_buffer(p0)->l2_hdr_offset));
}

always_inline void
pds_flow_extract_prog_args_x1 (vlib_buffer_t *p0,
                               pds_flow_params_t *params_arr,
                               int *size, u32 session_id,
                               u8 is_ip4)
{
    pds_flow_params_t   *local_params0 = params_arr + (*size),
                        *remote_params0 = local_params0 + 1;
    udp_header_t        *udp0;
    u32                 *ip4_local0, *ip4_remote0;

    *size = *size + 2;
    vnet_buffer(p0)->pds_data.ses_id = session_id;

    if (is_ip4) {
        ip4_header_t *ip40;

        local_params0->entry4.session_index =
            remote_params0->entry4.session_index = session_id;
        local_params0->entry4.epoch =
            remote_params0->entry4.epoch = 0xff;

        ip40 = vlib_buffer_get_current(p0);

        ip4_local0 = (u32 *) &(local_params0->entry4.src);
        ip4_remote0 = (u32 *) &(remote_params0->entry4.dst);
        *ip4_local0 = *ip4_remote0 =
            clib_net_to_host_u32(ip40->src_address.as_u32);
        ip4_local0 = (u32 *) &(local_params0->entry4.dst);
        ip4_remote0 = (u32 *) &(remote_params0->entry4.src);
        *ip4_local0 = *ip4_remote0 =
            clib_net_to_host_u32(ip40->dst_address.as_u32);
        local_params0->entry4.proto =
            remote_params0->entry4.proto = ip40->protocol;
        local_params0->entry4.vpc_id =
                remote_params0->entry4.vpc_id =
                (u8)vnet_buffer (p0)->sw_if_index[VLIB_TX];

        if (PREDICT_TRUE(((ip40->protocol == IP_PROTOCOL_TCP)
                || (ip40->protocol == IP_PROTOCOL_UDP)))) {
            udp0 = (udp_header_t *) (((u8 *) ip40) +
                    (vnet_buffer (p0)->l4_hdr_offset -
                            vnet_buffer (p0)->l3_hdr_offset));
            local_params0->entry4.sport =
                    remote_params0->entry4.dport =
                    clib_net_to_host_u16(udp0->src_port);
            local_params0->entry4.dport =
                    remote_params0->entry4.sport =
                    clib_net_to_host_u16(udp0->dst_port);
        } else {
            local_params0->entry4.sport =
                    local_params0->entry4.dport = 0;
            remote_params0->entry4.sport =
                    remote_params0->entry4.dport = 0;
        }
    } else {

        ip6_header_t *ip60;

        local_params0->entry6.session_index =
            remote_params0->entry6.session_index = session_id;
        local_params0->entry6.epoch =
            remote_params0->entry6.epoch = 0xff;
 
        ip60 = vlib_buffer_get_current(p0);

        clib_memcpy(local_params0->entry6.src,
                     ip60->src_address.as_u8, sizeof(ip6_address_t));
        clib_memcpy(remote_params0->entry6.dst,
                     ip60->src_address.as_u8, sizeof(ip6_address_t));
        clib_memcpy(local_params0->entry6.dst,
                     ip60->dst_address.as_u8, sizeof(ip6_address_t));
        clib_memcpy(remote_params0->entry6.src,
                     ip60->dst_address.as_u8, sizeof(ip6_address_t));
        local_params0->entry6.proto =
                     remote_params0->entry6.proto = ip60->protocol;
        local_params0->entry6.vpc_id =
                remote_params0->entry6.vpc_id =
                vnet_buffer (p0)->sw_if_index[VLIB_TX];

        if (PREDICT_TRUE(((ip60->protocol == IP_PROTOCOL_TCP)
                || (ip60->protocol == IP_PROTOCOL_UDP)))) {
            udp0 = (udp_header_t *) (((u8 *) ip60) +
                    (vnet_buffer (p0)->l4_hdr_offset -
                            vnet_buffer (p0)->l3_hdr_offset));
            local_params0->entry6.sport =
                    remote_params0->entry6.dport =
                    clib_net_to_host_u16(udp0->src_port);
            local_params0->entry6.dport =
                    remote_params0->entry6.sport =
                    clib_net_to_host_u16(udp0->dst_port);
        } else {
            local_params0->entry6.sport =
                    local_params0->entry6.dport = 0;
            remote_params0->entry6.sport =
                    remote_params0->entry6.dport = 0;
        }
    }
    local_params0->hash = vnet_buffer (p0)->pds_data.flow_hash;
    remote_params0->hash = 0;
    return;
}

always_inline void
pds_flow_program_hw_ip4 (pds_flow_params_t *key,
                         int size, u16 *next, u32 *counter)
{
    int i;
    ftlv4 *table = pds_flow_prog_get_table4();
    pds_flow_hw_ctx_t *ctx;
    pds_flow_main_t *fm = &pds_flow_main;

    for (i = 0; i < size; i++) {

        if (PREDICT_TRUE(key[i].entry4.session_index > 0) &&
                PREDICT_TRUE(0 == ftlv4_insert(table, &key[i].entry4, key[i].hash))) {
            counter[FLOW_PROG_COUNTER_FLOW_SUCCESS]++;
            next[i/2] = FLOW_PROG_NEXT_SESSION_PROG;
        } else {
            counter[FLOW_PROG_COUNTER_FLOW_FAILED]++;
            next[i/2] = FLOW_PROG_NEXT_DROP;
            if (key[i].entry4.session_index) {
                pds_session_id_dealloc(key[i].entry4.session_index);
            }
            if (i % 2) {
#if 0
                /*TODO - Delete is resulting in crash, so comment for now */
                /* Remove last entry as local flow succeeded
                 * but remote entry failed */
                pds_session_id_dealloc(key[i].entry4.session_index);
                /*TODO - Delete is resulting in crash, so comment for now */
                if (PREDICT_FALSE(0 != ftlv4_remove(table,
                                                    &key[i-1].entry4,
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
}
always_inline void
pds_flow_program_hw_ip6 (pds_flow_params_t *key,
                         int size, u16 *next, u32 *counter)
{
    int i;
    ftlv6 *table = pds_flow_prog_get_table6();
    pds_flow_hw_ctx_t *ctx;
    pds_flow_main_t *fm = &pds_flow_main;

    for (i = 0; i < size; i++) {

        if (PREDICT_TRUE(key[i].entry6.session_index > 0) &&
                PREDICT_TRUE(0 == ftlv6_insert(table, &key[i].entry6, key[i].hash))) {
            counter[FLOW_PROG_COUNTER_FLOW_SUCCESS]++;
            next[i/2] = FLOW_PROG_NEXT_SESSION_PROG;
        } else {
            counter[FLOW_PROG_COUNTER_FLOW_FAILED]++;
            next[i/2] = FLOW_PROG_NEXT_DROP;
            if (key[i].entry6.session_index) {
                pds_session_id_dealloc(key[i].entry6.session_index);
            }
            if (i % 2) {
#if 0
                /*TODO - Delete is resulting in crash, so comment for now */
                /* Remove last entry as local flow succeeded
                 * but remote entry failed */
                pds_session_id_dealloc(key[i].entry6.session_index);
                /*TODO - Delete is resulting in crash, so comment for now */
                if (PREDICT_FALSE(0 != ftlv6_remove(table,
                                                    &key[i-1].entry6,
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
}

static char *
pds_flow4_key2str (void *key)
{
    static char str[256];
    flow_swkey_t *k = (flow_swkey_t *)key;
    char srcstr[INET_ADDRSTRLEN + 1];
    char dststr[INET_ADDRSTRLEN + 1];

    inet_ntop(AF_INET, k->key_metadata_src, srcstr, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, k->key_metadata_dst, dststr, INET_ADDRSTRLEN);
    sprintf(str, "Src:%s Dst:%s Dport:%u Sport:%u Proto:%u VPC:%u",
            srcstr, dststr,
            k->key_metadata_dport, k->key_metadata_sport,
            k->key_metadata_proto, k->vnic_metadata_vpc_id);
    return str;
}

static char *
pds_flow6_key2str (void *key)
{
    static char str[256];
    flow_swkey_t *k = (flow_swkey_t *)key;
    char srcstr[INET6_ADDRSTRLEN + 1];
    char dststr[INET6_ADDRSTRLEN + 1];

    inet_ntop(AF_INET6, k->key_metadata_src, srcstr, INET6_ADDRSTRLEN);
    inet_ntop(AF_INET6, k->key_metadata_dst, dststr, INET6_ADDRSTRLEN);
    sprintf(str, "Src:%s Dst:%s Dport:%u Sport:%u Proto:%u VPC:%u",
            srcstr, dststr,
            k->key_metadata_dport, k->key_metadata_sport,
            k->key_metadata_proto, k->vnic_metadata_vpc_id);
    return str;
}

static char *
pds_flow_appdata2str (void *appdata)
{
    static char str[512];
    flow_appdata_t *d = (flow_appdata_t *)appdata;
    sprintf(str, "session_index:%d flow_role:%d",
            d->session_index, d->flow_role);
    return str;
}
#endif    // __VPP_FLOW_PLUGIN_FLOW_ARTEMIS_H__
