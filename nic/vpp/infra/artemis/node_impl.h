//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
// Contains PIPELINE specific implementation functions for infra plugin
//------------------------------------------------------------------------------

#ifndef __VPP_INFRA_ARTEMIS_IMPL_H__
#define __VPP_INFRA_ARTEMIS_IMPL_H__

always_inline int
pds_p4_cpu_node_get_advance_offset (vlib_buffer_t *b)
{
    return (VPP_P4_TO_ARM_HDR_SZ +
            (vnet_buffer(b)->l3_hdr_offset - vnet_buffer (b)->l2_hdr_offset));
}

always_inline void
pds_parse_p4cpu_hdr_x2 (vlib_buffer_t *p0, vlib_buffer_t *p1,
                        u16 *next0, u16 *next1, u32 *counter)
{
    p4_rx_cpu_hdr_t *hdr0 = vlib_buffer_get_current(p0);
    p4_rx_cpu_hdr_t *hdr1 = vlib_buffer_get_current(p1);
    u16 flag_orig0, flag_orig1;

    flag_orig0 = clib_net_to_host_u16(hdr0->flags);
    flag_orig1 = clib_net_to_host_u16(hdr1->flags);
    u16 flags0 = flag_orig0 &
        (VPP_CPU_FLAGS_IPV4_1_VALID | VPP_CPU_FLAGS_IPV6_1_VALID |
         VPP_CPU_FLAGS_IPV4_2_VALID | VPP_CPU_FLAGS_IPV6_2_VALID);
    u16 flags1 = flag_orig1 &
        (VPP_CPU_FLAGS_IPV4_1_VALID | VPP_CPU_FLAGS_IPV6_1_VALID |
         VPP_CPU_FLAGS_IPV4_2_VALID | VPP_CPU_FLAGS_IPV6_2_VALID);

    vnet_buffer (p0)->pds_data.flow_hash = clib_net_to_host_u32(hdr0->flow_hash);
    vnet_buffer (p0)->pds_data.flags = flag_orig0;
    vnet_buffer (p0)->l2_hdr_offset = hdr0->l2_offset;
    vnet_buffer (p0)->l3_hdr_offset =
            hdr0->l3_inner_offset ? hdr0->l3_inner_offset : hdr0->l3_offset;
    vnet_buffer (p0)->l4_hdr_offset =
            hdr0->l4_inner_offset ? hdr0->l4_inner_offset : hdr0->l4_offset;
    vnet_buffer (p0)->sw_if_index[VLIB_TX] = clib_net_to_host_u16(hdr0->local_vnic_tag);

    vnet_buffer (p1)->pds_data.flow_hash = clib_net_to_host_u32(hdr1->flow_hash);
    vnet_buffer (p1)->pds_data.flags = flag_orig1;
    vnet_buffer (p1)->l2_hdr_offset = hdr1->l2_offset;
    vnet_buffer (p1)->l3_hdr_offset =
            hdr1->l3_inner_offset ? hdr1->l3_inner_offset : hdr1->l3_offset;
    vnet_buffer (p1)->l4_hdr_offset =
            hdr1->l4_inner_offset ? hdr1->l4_inner_offset : hdr1->l4_offset;
    vnet_buffer (p1)->sw_if_index[VLIB_TX] = clib_net_to_host_u16(hdr1->local_vnic_tag);

    vlib_buffer_advance(p0, pds_p4_cpu_node_get_advance_offset(p0));
    vlib_buffer_advance(p1, pds_p4_cpu_node_get_advance_offset(p1));

    /* As of now only flow miss packets are punted to VPP for flow programming */
    if ((flags0 == flags1)) {
        if ((flags0 == VPP_CPU_FLAGS_IPV4_1_VALID)) {
            *next0 = *next1 = P4CPU_HDR_LOOKUP_NEXT_IP4_FLOW_PROG;
            counter[P4CPU_HDR_LOOKUP_COUNTER_IP4_FLOW] += 2;
        } else if (flags0 & VPP_CPU_FLAGS_IPV4_2_VALID) {
            *next0 = *next1 = P4CPU_HDR_LOOKUP_NEXT_IP4_TUN_FLOW_PROG;
            counter[P4CPU_HDR_LOOKUP_COUNTER_IP4_TUN_FLOW] += 2;
        } else if (flags0 == VPP_CPU_FLAGS_IPV6_1_VALID) {
            *next0 = *next1 = P4CPU_HDR_LOOKUP_NEXT_IP6_FLOW_PROG;
            counter[P4CPU_HDR_LOOKUP_COUNTER_IP6_FLOW] += 2;
        } else if (flags0 & VPP_CPU_FLAGS_IPV6_2_VALID) {
            *next0 = *next1 = P4CPU_HDR_LOOKUP_NEXT_IP6_TUN_FLOW_PROG;
            counter[P4CPU_HDR_LOOKUP_COUNTER_IP6_TUN_FLOW] += 2;
        } else if (flags0 & 0) { // TODO ARP_PROXY
            *next0 = *next1 = P4CPU_HDR_LOOKUP_NEXT_ARP_PROXY;
            counter[P4CPU_HDR_LOOKUP_COUNTER_ARP_PROXY] += 2;
        } else {
            *next0 = *next1 = P4CPU_HDR_LOOKUP_NEXT_DROP;
            counter[P4CPU_HDR_LOOKUP_COUNTER_UNKOWN] += 2;
        }
        return;
    }

    if ((flags0 == VPP_CPU_FLAGS_IPV4_1_VALID)) {
        *next0 = P4CPU_HDR_LOOKUP_NEXT_IP4_FLOW_PROG;
        counter[P4CPU_HDR_LOOKUP_COUNTER_IP4_FLOW] += 1;
    } else if (flags0 & VPP_CPU_FLAGS_IPV4_2_VALID) {
        *next0 = P4CPU_HDR_LOOKUP_NEXT_IP4_TUN_FLOW_PROG;
        counter[P4CPU_HDR_LOOKUP_COUNTER_IP4_TUN_FLOW] += 1;
    } else if (flags0 == VPP_CPU_FLAGS_IPV6_1_VALID) {
        *next0 = P4CPU_HDR_LOOKUP_NEXT_IP6_FLOW_PROG;
        counter[P4CPU_HDR_LOOKUP_COUNTER_IP6_FLOW] += 1;
    } else if (flags0 & VPP_CPU_FLAGS_IPV6_2_VALID) {
        *next0 = P4CPU_HDR_LOOKUP_NEXT_IP6_TUN_FLOW_PROG;
        counter[P4CPU_HDR_LOOKUP_COUNTER_IP6_TUN_FLOW] += 1;
    } else if (flags0 & 0) { // TODO ARP_PROXY
        *next0 = P4CPU_HDR_LOOKUP_NEXT_ARP_PROXY;
        counter[P4CPU_HDR_LOOKUP_COUNTER_ARP_PROXY] += 1;
    } else {
        *next0 = P4CPU_HDR_LOOKUP_NEXT_DROP;
        counter[P4CPU_HDR_LOOKUP_COUNTER_UNKOWN] += 1;
    }

    if ((flags1 == VPP_CPU_FLAGS_IPV4_1_VALID)) {
        *next1 = P4CPU_HDR_LOOKUP_NEXT_IP4_FLOW_PROG;
        counter[P4CPU_HDR_LOOKUP_COUNTER_IP4_FLOW] += 1;
    } else if (flags1 & VPP_CPU_FLAGS_IPV4_2_VALID) {
        *next1 = P4CPU_HDR_LOOKUP_NEXT_IP4_TUN_FLOW_PROG;
        counter[P4CPU_HDR_LOOKUP_COUNTER_IP4_TUN_FLOW] += 1;
    } else if (flags1 == VPP_CPU_FLAGS_IPV6_1_VALID) {
        *next1 = P4CPU_HDR_LOOKUP_NEXT_IP6_FLOW_PROG;
        counter[P4CPU_HDR_LOOKUP_COUNTER_IP6_FLOW] += 1;
    } else if (flags1 & VPP_CPU_FLAGS_IPV6_2_VALID) {
        *next1 = P4CPU_HDR_LOOKUP_NEXT_IP6_TUN_FLOW_PROG;
        counter[P4CPU_HDR_LOOKUP_COUNTER_IP6_TUN_FLOW] += 1;
    } else if (flags1 & 0) { // TODO ARP_PROXY
        *next1 = P4CPU_HDR_LOOKUP_NEXT_ARP_PROXY;
        counter[P4CPU_HDR_LOOKUP_COUNTER_ARP_PROXY] += 1;
    } else {
        *next1 = P4CPU_HDR_LOOKUP_NEXT_DROP;
        counter[P4CPU_HDR_LOOKUP_COUNTER_UNKOWN] += 1;
    }
}

always_inline void
pds_parse_p4cpu_hdr_x1 (vlib_buffer_t *p, u16 *next, u32 *counter)
{
    p4_rx_cpu_hdr_t *hdr = vlib_buffer_get_current(p);
    u16 flag_orig;

    flag_orig = clib_net_to_host_u16(hdr->flags);
    u16 flags = flag_orig &
        (VPP_CPU_FLAGS_IPV4_1_VALID | VPP_CPU_FLAGS_IPV6_1_VALID |
         VPP_CPU_FLAGS_IPV4_2_VALID | VPP_CPU_FLAGS_IPV6_2_VALID);

    vnet_buffer (p)->pds_data.flow_hash = clib_net_to_host_u32(hdr->flow_hash);
    vnet_buffer (p)->pds_data.flags = flag_orig;
    vnet_buffer (p)->l2_hdr_offset = hdr->l2_offset;
    vnet_buffer (p)->l3_hdr_offset =
            hdr->l3_inner_offset ? hdr->l3_inner_offset : hdr->l3_offset;
    vnet_buffer (p)->l4_hdr_offset =
            hdr->l4_inner_offset ? hdr->l4_inner_offset : hdr->l4_offset;

    vnet_buffer (p)->sw_if_index[VLIB_TX] = clib_net_to_host_u16(hdr->local_vnic_tag);

    vlib_buffer_advance(p, pds_p4_cpu_node_get_advance_offset(p));

    /* As of now only flow miss packets are punted to VPP for flow programming */
    if ((flags == VPP_CPU_FLAGS_IPV4_1_VALID)) {
        *next = P4CPU_HDR_LOOKUP_NEXT_IP4_FLOW_PROG;
        counter[P4CPU_HDR_LOOKUP_COUNTER_IP4_FLOW] += 1;
    } else if (flags & VPP_CPU_FLAGS_IPV4_2_VALID) {
        *next = P4CPU_HDR_LOOKUP_NEXT_IP4_TUN_FLOW_PROG;
        counter[P4CPU_HDR_LOOKUP_COUNTER_IP4_TUN_FLOW] += 1;
    } else if (flags == VPP_CPU_FLAGS_IPV6_1_VALID) {
        *next = P4CPU_HDR_LOOKUP_NEXT_IP6_FLOW_PROG;
        counter[P4CPU_HDR_LOOKUP_COUNTER_IP6_FLOW] += 1;
    } else if (flags & VPP_CPU_FLAGS_IPV6_2_VALID) {
        *next = P4CPU_HDR_LOOKUP_NEXT_IP6_TUN_FLOW_PROG;
        counter[P4CPU_HDR_LOOKUP_COUNTER_IP6_TUN_FLOW] += 1;
    } else if (0) { // TODO ARP_PROXY
        *next = P4CPU_HDR_LOOKUP_NEXT_ARP_PROXY;
        counter[P4CPU_HDR_LOOKUP_COUNTER_ARP_PROXY] += 1;
    } else {
        *next = P4CPU_HDR_LOOKUP_NEXT_DROP;
        counter[P4CPU_HDR_LOOKUP_COUNTER_UNKOWN] += 1;
    }
}

#endif    // __VPP_INFRA_ARTEMIS_IMPL_H__

