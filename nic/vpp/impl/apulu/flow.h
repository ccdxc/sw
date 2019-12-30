//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_IMPL_APULU_FLOW_H__
#define __VPP_IMPL_APULU_FLOW_H__

#include <gen/p4gen/apulu/include/p4pd.h>
#include <api.h>
#include <nic/apollo/api/impl/apulu/nacl_data.h>

always_inline u32
pds_session_get_max (void)
{
    // session table is 2M - 1 as 0 is reserved.
    // so index is 0 to (2 *1024 * 1024) - 2
    return 2097150;
}

extern pds_flow_main_t pds_flow_main;

always_inline int
pds_session_get_advance_offset (void)
{
    return (APULU_P4_TO_ARM_HDR_SZ - APULU_ARM_TO_P4_HDR_SZ);
}

always_inline void
pds_session_prog_x2 (vlib_buffer_t *b0, vlib_buffer_t *b1,
                     u32 session_id0, u32 session_id1,
                     u16 *next0, u16 *next1, u32 *counter)
{
    p4_rx_cpu_hdr_t *ses_info0 = vlib_buffer_get_current(b0);
    p4_rx_cpu_hdr_t *ses_info1 = vlib_buffer_get_current(b1);
    static session_actiondata_t actiondata = {0};
    pds_flow_main_t *fm = &pds_flow_main;

    actiondata.action_id = SESSION_SESSION_INFO_ID;
    actiondata.action_u.session_session_info.tx_rewrite_flags =
        vec_elt(fm->nh_flags, (vnet_buffer(b0)->pds_data.nexthop) >> 16);
    actiondata.action_u.session_session_info.rx_rewrite_flags =
            ((RX_REWRITE_DMAC_FROM_MAPPING << RX_REWRITE_DMAC_START) |
                    (RX_REWRITE_ENCAP_VLAN << RX_REWRITE_ENCAP_START));

    if (PREDICT_FALSE(session_program(session_id0, (void *)&actiondata))) {
        *next0 = SESSION_PROG_NEXT_DROP;
    } else {
        *next0 = SESSION_PROG_NEXT_FWD_FLOW;
        /* Reuse flowhash field to store lif id for packet tx
         * as we have limited packet metadata fields
         */
        vnet_buffer(b0)->pds_data.flow_hash =
                clib_net_to_host_u16(ses_info0->lif);
    }
    actiondata.action_u.session_session_info.tx_rewrite_flags =
        vec_elt(fm->nh_flags, (vnet_buffer(b1)->pds_data.nexthop) >> 16);
    if (PREDICT_FALSE(session_program(session_id1, (void *)&actiondata))) {
        *next1 = SESSION_PROG_NEXT_DROP;
    } else {
        *next1 = SESSION_PROG_NEXT_FWD_FLOW;
        /* Reuse flowhash field to store lif id for packet tx
         * as we have limited packet metadata fields
         */
        vnet_buffer(b1)->pds_data.flow_hash =
                clib_net_to_host_u16(ses_info1->lif);
    }

    vlib_buffer_advance(b0, pds_session_get_advance_offset());
    vlib_buffer_advance(b1, pds_session_get_advance_offset());
}

always_inline void
pds_session_prog_x1 (vlib_buffer_t *b, u32 session_id,
                     u16 *next, u32 *counter)
{
    p4_rx_cpu_hdr_t *ses_info0 = vlib_buffer_get_current(b);
    static session_actiondata_t actiondata = {0};
    pds_flow_main_t *fm = &pds_flow_main;

    actiondata.action_id = SESSION_SESSION_INFO_ID;
    actiondata.action_u.session_session_info.tx_rewrite_flags =
        vec_elt(fm->nh_flags, (vnet_buffer(b)->pds_data.nexthop) >> 16);
    actiondata.action_u.session_session_info.rx_rewrite_flags =
            ((RX_REWRITE_DMAC_FROM_MAPPING << RX_REWRITE_DMAC_START) |
                    (RX_REWRITE_ENCAP_VLAN << RX_REWRITE_ENCAP_START));

    if (PREDICT_FALSE(session_program(session_id, (void *)&actiondata))) {
        next[0] = SESSION_PROG_NEXT_DROP;
    } else {
        next[0] = SESSION_PROG_NEXT_FWD_FLOW;
        /* Reuse flowhash field to store lif id for packet tx
         * as we have limited packet metadata fields
         */
        vnet_buffer(b)->pds_data.flow_hash =
                clib_net_to_host_u16(ses_info0->lif);
    }
    vlib_buffer_advance(b, pds_session_get_advance_offset());
}

always_inline int
pds_flow_prog_get_next_offset (vlib_buffer_t *p0)
{
    return -(APULU_P4_TO_ARM_HDR_SZ +
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

    tx0 = vlib_buffer_get_current(b0);
    tx1 = vlib_buffer_get_current(b1);

    tx0->pad = 0;
    tx1->pad = 0;
    tx0->nexthop_valid = 0;
    tx1->nexthop_valid = 0;
    tx0->lif_sbit0_ebit7 = vnet_buffer(b0)->pds_data.flow_hash & 0xff;
    tx1->lif_sbit0_ebit7 = vnet_buffer(b1)->pds_data.flow_hash & 0xff;
    tx0->lif_sbit8_ebit10 = vnet_buffer(b0)->pds_data.flow_hash >> 0x8;
    tx1->lif_sbit8_ebit10 = vnet_buffer(b1)->pds_data.flow_hash >> 0x8;

    tx0->lif_flags = clib_host_to_net_u16(tx0->lif_flags);
    tx1->lif_flags = clib_host_to_net_u16(tx1->lif_flags);
    //Dont care about nexthoptype/id as we don't set nexthop_valid.
}

always_inline void
pds_flow_add_tx_hdrs_x1 (vlib_buffer_t *b0)
{
    p4_tx_cpu_hdr_t *tx0;

    tx0 = vlib_buffer_get_current(b0);
    tx0->pad = 0;
    tx0->nexthop_valid = 0;
    tx0->lif_sbit0_ebit7 = vnet_buffer(b0)->pds_data.flow_hash & 0xff;
    tx0->lif_sbit8_ebit10 = vnet_buffer(b0)->pds_data.flow_hash >> 0x8;

    tx0->lif_flags = clib_host_to_net_u16(tx0->lif_flags);
    //Dont care about nexthoptype/id as we don't set nexthop_valid.
}

static char *
pds_flow4_key2str (void *key)
{
    static char str[256] = {0};
    flow_swkey_t *k = (flow_swkey_t *)key;
    char srcstr[INET_ADDRSTRLEN + 1];
    char dststr[INET_ADDRSTRLEN + 1];

    inet_ntop(AF_INET, k->key_metadata_src, srcstr, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, k->key_metadata_dst, dststr, INET_ADDRSTRLEN);
    sprintf(str, "Src:%s Dst:%s Dport:%u Sport:%u Proto:%u VNIC:%u",
            srcstr, dststr,
            k->key_metadata_dport, k->key_metadata_sport,
            k->key_metadata_proto, k->vnic_metadata_bd_id);
    return str;
}

static char *
pds_flow6_key2str (void *key)
{
    static char str[256] = {0};
    flow_swkey_t *k = (flow_swkey_t *)key;
    char srcstr[INET6_ADDRSTRLEN + 1];
    char dststr[INET6_ADDRSTRLEN + 1];

    inet_ntop(AF_INET6, k->key_metadata_src, srcstr, INET6_ADDRSTRLEN);
    inet_ntop(AF_INET6, k->key_metadata_dst, dststr, INET6_ADDRSTRLEN);
    sprintf(str, "Src:%s Dst:%s Dport:%u Sport:%u Proto:%u VNIC:%u",
            srcstr, dststr,
            k->key_metadata_dport, k->key_metadata_sport,
            k->key_metadata_proto, k->vnic_metadata_bd_id);
    return str;
}

static char *
pds_flow_appdata2str (void *appdata)
{
    static char str[512] = {0};
    flow_appdata_t *d = (flow_appdata_t *)appdata;
    sprintf(str, "session_index:%d flow_role:%d",
            d->session_index, d->flow_role);
    return str;
}

always_inline void
pds_flow_extract_nexthop_info(vlib_buffer_t *p0,
                              u8 is_ip4)
{
    u32 nexthop = vnet_buffer(p0)->pds_data.nexthop;

    //TODO: Rflow nexthop
    if (is_ip4) {
        if (nexthop) {
            ftlv4_cache_set_nexthop(nexthop & 0xff, ((nexthop >> 16) & 0x3), 1);
        } else {
            ftlv4_cache_set_nexthop(0, 0, 0);
        }
    } else {
        if (nexthop) {
            ftlv6_cache_set_nexthop(nexthop & 0xff, ((nexthop >> 16) & 0x3), 1);
        } else {
            ftlv6_cache_set_nexthop(0, 0, 0);
        }
    }
}

always_inline void
pds_flow_nh_flags_add (void)
{
    pds_flow_main_t *fm = &pds_flow_main;
    u16 encap, mapping, nexthop, tunnel;

    encap = (TX_REWRITE_ENCAP_VXLAN << TX_REWRITE_ENCAP_START);
    nexthop = (TX_REWRITE_DMAC_FROM_NEXTHOP << TX_REWRITE_DMAC_START);
    mapping = (TX_REWRITE_DMAC_FROM_MAPPING << TX_REWRITE_DMAC_START);
    tunnel = (TX_REWRITE_DMAC_FROM_TUNNEL << TX_REWRITE_DMAC_START);
    fm->nh_flags = vec_new(u16, NEXTHOP_TYPE_MAX);
    vec_elt(fm->nh_flags, NEXTHOP_TYPE_VPC) = mapping | encap;
    vec_elt(fm->nh_flags, NEXTHOP_TYPE_ECMP) = nexthop | encap;
    vec_elt(fm->nh_flags, NEXTHOP_TYPE_TUNNEL) = tunnel | encap;
    vec_elt(fm->nh_flags, NEXTHOP_TYPE_NEXTHOP) = nexthop | encap;
    vec_elt(fm->nh_flags, NEXTHOP_TYPE_NAT) = mapping | encap;
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
    u32 nexthop;

    flag_orig0 = hdr0->flags;
    flag_orig1 = hdr1->flags;
    //flag_orig0 = flag_orig1 = VPP_CPU_FLAGS_IPV4_1_VALID;

    u8 flags0 = flag_orig0 &
        (VPP_CPU_FLAGS_IPV4_1_VALID | VPP_CPU_FLAGS_IPV6_1_VALID |
         VPP_CPU_FLAGS_IPV4_2_VALID | VPP_CPU_FLAGS_IPV6_2_VALID);
    u8 flags1 = flag_orig1 &
        (VPP_CPU_FLAGS_IPV4_1_VALID | VPP_CPU_FLAGS_IPV6_1_VALID |
         VPP_CPU_FLAGS_IPV4_2_VALID | VPP_CPU_FLAGS_IPV6_2_VALID);

    vnet_buffer (p0)->pds_data.flow_hash = clib_net_to_host_u32(hdr0->flow_hash);
    vnet_buffer (p0)->pds_data.flags = flag_orig0;
    nexthop = clib_net_to_host_u16(hdr0->nexthop_id);
    if (NEXTHOP_TYPE_VPC != hdr0->nexthop_type) {
        vnet_buffer (p0)->pds_data.nexthop = nexthop | (hdr0->nexthop_type << 16);
    } else {
        vnet_buffer (p0)->pds_data.nexthop = 0;
    }
    vnet_buffer (p0)->l2_hdr_offset = hdr0->l2_offset;
    vnet_buffer (p0)->l3_hdr_offset =
            hdr0->l3_inner_offset ? hdr0->l3_inner_offset : hdr0->l3_offset;
    vnet_buffer (p0)->l4_hdr_offset =
            hdr0->l4_inner_offset ? hdr0->l4_inner_offset : hdr0->l4_offset;
    vnet_buffer (p0)->sw_if_index[VLIB_TX] = clib_net_to_host_u16(hdr0->ingress_bd_id);

    vnet_buffer (p1)->pds_data.flow_hash = clib_net_to_host_u32(hdr1->flow_hash);
    vnet_buffer (p1)->pds_data.flags = flag_orig1;
    nexthop = clib_net_to_host_u16(hdr1->nexthop_id);
    if (NEXTHOP_TYPE_VPC != hdr0->nexthop_type) {
        vnet_buffer (p1)->pds_data.nexthop = nexthop | (hdr1->nexthop_type << 16);
    } else {
        vnet_buffer (p1)->pds_data.nexthop = 0;
    }
    vnet_buffer (p1)->l2_hdr_offset = hdr1->l2_offset;
    vnet_buffer (p1)->l3_hdr_offset =
            hdr1->l3_inner_offset ? hdr1->l3_inner_offset : hdr1->l3_offset;
    vnet_buffer (p1)->l4_hdr_offset =
            hdr1->l4_inner_offset ? hdr1->l4_inner_offset : hdr1->l4_offset;
    vnet_buffer (p1)->sw_if_index[VLIB_TX] = clib_net_to_host_u16(hdr1->ingress_bd_id);

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
    u32 nexthop;

    flag_orig = hdr->flags;
    //flag_orig = VPP_CPU_FLAGS_IPV4_1_VALID;
    u8 flags = flag_orig &
        (VPP_CPU_FLAGS_IPV4_1_VALID | VPP_CPU_FLAGS_IPV6_1_VALID |
         VPP_CPU_FLAGS_IPV4_2_VALID | VPP_CPU_FLAGS_IPV6_2_VALID);

    vnet_buffer (p)->pds_data.flow_hash = clib_net_to_host_u32(hdr->flow_hash);
    vnet_buffer (p)->pds_data.flags = flag_orig;
    nexthop = clib_net_to_host_u16(hdr->nexthop_id);
    if (NEXTHOP_TYPE_VPC != hdr->nexthop_type) {
        vnet_buffer (p)->pds_data.nexthop = nexthop | (hdr->nexthop_type << 16);
    } else {
        vnet_buffer (p)->pds_data.nexthop = 0;
    }
    vnet_buffer (p)->l2_hdr_offset = hdr->l2_offset;
    vnet_buffer (p)->l3_hdr_offset =
            hdr->l3_inner_offset ? hdr->l3_inner_offset : hdr->l3_offset;
    vnet_buffer (p)->l4_hdr_offset =
            hdr->l4_inner_offset ? hdr->l4_inner_offset : hdr->l4_offset;

    vnet_buffer (p)->sw_if_index[VLIB_TX] = clib_net_to_host_u16(hdr->ingress_bd_id);

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
pds_flow_pipeline_init (void)
{
    pds_infra_api_reg_t params = {0}; 
    //vlib_node_t *flow;

    params.nacl_id = NACL_DATA_ID_FLOW_MISS_IP4_IP6;
    params.node = format(0, "pds-flow-classify");
    //flow = vlib_get_node_by_name(vlib_get_main(), (u8 *) "pds-flow-classify");
    //params.frame_queue_index = vlib_frame_queue_main_init (flow->index, 0);
    //params.handoff_thread = 0;
    params.frame_queue_index = ~0;
    params.handoff_thread = ~0;
    params.offset = 0;
    params.unreg = 0;
    
    if (0 != pds_register_nacl_id_to_node(&params)) {
        ASSERT(0);
    }
    return;
}

#endif    // __VPP_IMPL_APULU_FLOW_H__
