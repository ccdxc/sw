//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_IMPL_ARTEMIS_FLOW_H__
#define __VPP_IMPL_ARTEMIS_FLOW_H__

#include <gen/p4gen/artemis/include/p4pd.h>
#include <nic/apollo/p4/include/artemis_defines.h>
#include <stddef.h>

always_inline u32
pds_session_get_max (void)
{
    //Session ID is 23 bits and ID 0 is reserved, so Max sessions are (8 * 1024 * 1024) - 2.
    return 8388606;
}

always_inline int
pds_session_get_advance_offset (void)
{
    return (ARTEMIS_P4_TO_ARM_HDR_SZ -
            offsetof(p4_rx_cpu_hdr_t, ses_info) - ARTEMIS_PREDICATE_HDR_SZ);
}

always_inline void
pds_session_prog_x2 (vlib_buffer_t *b0, vlib_buffer_t *b1,
                     u32 session_id0, u32 session_id1,
                     u16 *next0, u16 *next1, u32 *counter)
{
    void *ses_info0 = vlib_buffer_get_current(b0);
    void *ses_info1 = vlib_buffer_get_current(b1);

    session_insert(session_id0, ses_info0);
    session_insert(session_id1, ses_info1);
    *next0 = *next1 = SESSION_PROG_NEXT_FWD_FLOW;

    vlib_buffer_advance(b0, pds_session_get_advance_offset());
    vlib_buffer_advance(b1, pds_session_get_advance_offset());
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
pds_flow_prog_get_next_offset (vlib_buffer_t *p0)
{
    return -(ARTEMIS_P4_TO_ARM_HDR_SZ - offsetof(p4_rx_cpu_hdr_t, ses_info) +
               (vnet_buffer(p0)->l3_hdr_offset - vnet_buffer(p0)->l2_hdr_offset));
}

always_inline int
pds_flow_prog_get_next_node (void)
{
    return FLOW_PROG_NEXT_SESSION_PROG;
}

always_inline void
pds_flow_add_tx_hdrs_x2 (vlib_buffer_t *b0, vlib_buffer_t *b1)
{
    p4_tx_cpu_hdr_t *tx0, *tx1;
    u8 flag0, flag1;

    tx0 = vlib_buffer_get_current(b0);
    tx1 = vlib_buffer_get_current(b1);
    tx0->flags_octet = 0;
    tx1->flags_octet = 0;
    flag0 = vnet_buffer (b0)->pds_flow_data.flags;
    flag1 = vnet_buffer (b1)->pds_flow_data.flags;
    if (PREDICT_TRUE(flag0 & VPP_CPU_FLAGS_DIRECTION)) {
        tx0->direction = 1;
    }
    if (PREDICT_TRUE(flag1 & VPP_CPU_FLAGS_DIRECTION)) {
        tx1->direction = 1;
    }
}

always_inline void
pds_flow_add_tx_hdrs_x1 (vlib_buffer_t *b0)
{
    p4_tx_cpu_hdr_t *tx0;
    u8 flag0;

    tx0 = vlib_buffer_get_current(b0);
    tx0->flags_octet = 0;
    flag0 = vnet_buffer (b0)->pds_flow_data.flags;
    if (PREDICT_TRUE(flag0 & VPP_CPU_FLAGS_DIRECTION)) {
        tx0->direction = 1;
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

always_inline void
pds_flow_extract_nexthop_info(vlib_buffer_t *p0, u8 is_ip4, u8 iflow)
{
    return;
}

always_inline void
pds_flow_nh_flags_add (void)
{
    return;
}

always_inline int
pds_flow_classify_get_advance_offset (vlib_buffer_t *b)
{
    return (VPP_P4_TO_ARM_HDR_SZ +
            (vnet_buffer(b)->l3_hdr_offset - vnet_buffer (b)->l2_hdr_offset));
}

always_inline void
pds_flow_classify_x2 (vlib_buffer_t *p0, vlib_buffer_t *p1,
                        u16 *next0, u16 *next1, u32 *counter)
{
    p4_rx_cpu_hdr_t *hdr0 = vlib_buffer_get_current(p0);
    p4_rx_cpu_hdr_t *hdr1 = vlib_buffer_get_current(p1);
    u8 flag_orig0, flag_orig1;

    flag_orig0 = clib_net_to_host_u16(hdr0->flags);
    flag_orig1 = clib_net_to_host_u16(hdr1->flags);
    //flag_orig0 = flag_orig1 = VPP_CPU_FLAGS_IPV4_1_VALID;

    u8 flags0 = flag_orig0 &
        (VPP_CPU_FLAGS_IPV4_1_VALID | VPP_CPU_FLAGS_IPV6_1_VALID |
         VPP_CPU_FLAGS_IPV4_2_VALID | VPP_CPU_FLAGS_IPV6_2_VALID);
    u8 flags1 = flag_orig1 &
        (VPP_CPU_FLAGS_IPV4_1_VALID | VPP_CPU_FLAGS_IPV6_1_VALID |
         VPP_CPU_FLAGS_IPV4_2_VALID | VPP_CPU_FLAGS_IPV6_2_VALID);

    vnet_buffer (p0)->pds_flow_data.flow_hash = clib_net_to_host_u32(hdr0->flow_hash);
    vnet_buffer (p0)->pds_flow_data.flags = flag_orig0;
    vnet_buffer (p0)->l2_hdr_offset = hdr0->l2_offset;
    vnet_buffer (p0)->l3_hdr_offset =
            hdr0->l3_inner_offset ? hdr0->l3_inner_offset : hdr0->l3_offset;
    vnet_buffer (p0)->l4_hdr_offset =
            hdr0->l4_inner_offset ? hdr0->l4_inner_offset : hdr0->l4_offset;
    vnet_buffer (p0)->sw_if_index[VLIB_TX] = clib_net_to_host_u16(hdr0->local_vnic_tag);

    vnet_buffer (p1)->pds_flow_data.flow_hash = clib_net_to_host_u32(hdr1->flow_hash);
    vnet_buffer (p1)->pds_flow_data.flags = flag_orig1;
    vnet_buffer (p1)->l2_hdr_offset = hdr1->l2_offset;
    vnet_buffer (p1)->l3_hdr_offset =
            hdr1->l3_inner_offset ? hdr1->l3_inner_offset : hdr1->l3_offset;
    vnet_buffer (p1)->l4_hdr_offset =
            hdr1->l4_inner_offset ? hdr1->l4_inner_offset : hdr1->l4_offset;
    vnet_buffer (p1)->sw_if_index[VLIB_TX] = clib_net_to_host_u16(hdr1->local_vnic_tag);

    vlib_buffer_advance(p0, pds_flow_classify_get_advance_offset(p0));
    vlib_buffer_advance(p1, pds_flow_classify_get_advance_offset(p1));

    /* As of now only flow miss packets are punted to VPP for flow programming */
    if ((flags0 == flags1)) {
        if ((flags0 == VPP_CPU_FLAGS_IPV4_1_VALID)) {
            *next0 = *next1 = FLOW_CLASSIFY_NEXT_IP4_FLOW_PROG;
            counter[FLOW_CLASSIFY_COUNTER_IP4_FLOW] += 2;
        } else if (flags0 & VPP_CPU_FLAGS_IPV4_2_VALID) {
            *next0 = *next1 = FLOW_CLASSIFY_NEXT_IP4_TUN_FLOW_PROG;
            counter[FLOW_CLASSIFY_COUNTER_IP4_TUN_FLOW] += 2;
        } else if (flags0 == VPP_CPU_FLAGS_IPV6_1_VALID) {
            *next0 = *next1 = FLOW_CLASSIFY_NEXT_IP6_FLOW_PROG;
            counter[FLOW_CLASSIFY_COUNTER_IP6_FLOW] += 2;
        } else if (flags0 & VPP_CPU_FLAGS_IPV6_2_VALID) {
            *next0 = *next1 = FLOW_CLASSIFY_NEXT_IP6_TUN_FLOW_PROG;
            counter[FLOW_CLASSIFY_COUNTER_IP6_TUN_FLOW] += 2;
        } else {
            *next0 = *next1 = FLOW_CLASSIFY_NEXT_DROP;
            counter[FLOW_CLASSIFY_COUNTER_UNKOWN] += 2;
        }
        return;
    }

    if ((flags0 == VPP_CPU_FLAGS_IPV4_1_VALID)) {
        *next0 = FLOW_CLASSIFY_NEXT_IP4_FLOW_PROG;
        counter[FLOW_CLASSIFY_COUNTER_IP4_FLOW] += 1;
    } else if (flags0 & VPP_CPU_FLAGS_IPV4_2_VALID) {
        *next0 = FLOW_CLASSIFY_NEXT_IP4_TUN_FLOW_PROG;
        counter[FLOW_CLASSIFY_COUNTER_IP4_TUN_FLOW] += 1;
    } else if (flags0 == VPP_CPU_FLAGS_IPV6_1_VALID) {
        *next0 = FLOW_CLASSIFY_NEXT_IP6_FLOW_PROG;
        counter[FLOW_CLASSIFY_COUNTER_IP6_FLOW] += 1;
    } else if (flags0 & VPP_CPU_FLAGS_IPV6_2_VALID) {
        *next0 = FLOW_CLASSIFY_NEXT_IP6_TUN_FLOW_PROG;
        counter[FLOW_CLASSIFY_COUNTER_IP6_TUN_FLOW] += 1;
    } else {
        *next0 = FLOW_CLASSIFY_NEXT_DROP;
        counter[FLOW_CLASSIFY_COUNTER_UNKOWN] += 1;
    }

    if ((flags1 == VPP_CPU_FLAGS_IPV4_1_VALID)) {
        *next1 = FLOW_CLASSIFY_NEXT_IP4_FLOW_PROG;
        counter[FLOW_CLASSIFY_COUNTER_IP4_FLOW] += 1;
    } else if (flags1 & VPP_CPU_FLAGS_IPV4_2_VALID) {
        *next1 = FLOW_CLASSIFY_NEXT_IP4_TUN_FLOW_PROG;
        counter[FLOW_CLASSIFY_COUNTER_IP4_TUN_FLOW] += 1;
    } else if (flags1 == VPP_CPU_FLAGS_IPV6_1_VALID) {
        *next1 = FLOW_CLASSIFY_NEXT_IP6_FLOW_PROG;
        counter[FLOW_CLASSIFY_COUNTER_IP6_FLOW] += 1;
    } else if (flags1 & VPP_CPU_FLAGS_IPV6_2_VALID) {
        *next1 = FLOW_CLASSIFY_NEXT_IP6_TUN_FLOW_PROG;
        counter[FLOW_CLASSIFY_COUNTER_IP6_TUN_FLOW] += 1;
    } else {
        *next1 = FLOW_CLASSIFY_NEXT_DROP;
        counter[FLOW_CLASSIFY_COUNTER_UNKOWN] += 1;
    }
}

always_inline void
pds_flow_classify_x1 (vlib_buffer_t *p, u16 *next, u32 *counter)
{
    p4_rx_cpu_hdr_t *hdr = vlib_buffer_get_current(p);
    u8 flag_orig;

    flag_orig = clib_net_to_host_u16(hdr->flags);
    //flag_orig = VPP_CPU_FLAGS_IPV4_1_VALID;
    u8 flags = flag_orig &
        (VPP_CPU_FLAGS_IPV4_1_VALID | VPP_CPU_FLAGS_IPV6_1_VALID |
         VPP_CPU_FLAGS_IPV4_2_VALID | VPP_CPU_FLAGS_IPV6_2_VALID);

    vnet_buffer (p)->pds_flow_data.flow_hash = clib_net_to_host_u32(hdr->flow_hash);
    vnet_buffer (p)->pds_flow_data.flags = flag_orig;
    vnet_buffer (p)->l2_hdr_offset = hdr->l2_offset;
    vnet_buffer (p)->l3_hdr_offset =
            hdr->l3_inner_offset ? hdr->l3_inner_offset : hdr->l3_offset;
    vnet_buffer (p)->l4_hdr_offset =
            hdr->l4_inner_offset ? hdr->l4_inner_offset : hdr->l4_offset;

    vnet_buffer (p)->sw_if_index[VLIB_TX] = clib_net_to_host_u16(hdr->local_vnic_tag);

    vlib_buffer_advance(p, pds_flow_classify_get_advance_offset(p));

    /* As of now only flow miss packets are punted to VPP for flow programming */
    if ((flags == VPP_CPU_FLAGS_IPV4_1_VALID)) {
        *next = FLOW_CLASSIFY_NEXT_IP4_FLOW_PROG;
        counter[FLOW_CLASSIFY_COUNTER_IP4_FLOW] += 1;
    } else if (flags & VPP_CPU_FLAGS_IPV4_2_VALID) {
        *next = FLOW_CLASSIFY_NEXT_IP4_TUN_FLOW_PROG;
        counter[FLOW_CLASSIFY_COUNTER_IP4_TUN_FLOW] += 1;
    } else if (flags == VPP_CPU_FLAGS_IPV6_1_VALID) {
        *next = FLOW_CLASSIFY_NEXT_IP6_FLOW_PROG;
        counter[FLOW_CLASSIFY_COUNTER_IP6_FLOW] += 1;
    } else if (flags & VPP_CPU_FLAGS_IPV6_2_VALID) {
        *next = FLOW_CLASSIFY_NEXT_IP6_TUN_FLOW_PROG;
        counter[FLOW_CLASSIFY_COUNTER_IP6_TUN_FLOW] += 1;
    } else {
        *next = FLOW_CLASSIFY_NEXT_DROP;
        counter[FLOW_CLASSIFY_COUNTER_UNKOWN] += 1;
    }
}

always_inline void
pds_flow_handle_l2l (vlib_buffer_t *p0, u8 flow_exists,
                     u8 *miss_hit, u32 ses_id)
{
    return;
}

always_inline void
pds_flow_pipeline_init (void)
{
    return;
}

#endif    // __VPP_IMPL_ARTEMIS_FLOW_H__
