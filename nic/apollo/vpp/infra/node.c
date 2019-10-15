/*
 *  {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
 */

#include "node.h"
#include <vnet/plugin/plugin.h>

// *INDENT-OFF*
VLIB_PLUGIN_REGISTER () = {
    .description = "Pensando Infra Plugin",
};
// *INDENT-ON*

vlib_node_registration_t pds_p4cpu_hdr_lookup_node;

static u8 *
format_pds_p4cpu_hdr_lookup_trace (u8 * s, va_list * args)
{
    CLIB_UNUSED (vlib_main_t * vm) = va_arg (*args, vlib_main_t *);
    CLIB_UNUSED (vlib_node_t * node) = va_arg (*args, vlib_node_t *);
    p4cpu_hdr_lookup_trace_t *t = va_arg (*args, p4cpu_hdr_lookup_trace_t *);

    s = format (s, "Flags[0x%x], flow_hash[0x%x], l2_offset[%d], "
            "l3_offset[%d] l4_offset[%d], vnic[%d]",
            t->flags, t->flow_hash, t->l2_offset, t->l3_offset,
            t->l4_offset, t->vnic);
    return s;
}

void
pds_p4cpu_hdr_lookup_trace_add (vlib_main_t * vm,
                                vlib_node_runtime_t * node,
                                vlib_frame_t * frame)
{
    u32 *from, n_left;

    n_left = frame->n_vectors;
    from = vlib_frame_vector_args (frame);

    while (n_left >= 4)
    {
        u32 bi0, bi1;
        vlib_buffer_t *b0, *b1;
        p4cpu_hdr_lookup_trace_t *t0, *t1;
        u16 flag0, flag1;

        /* Prefetch next iteration. */
        vlib_prefetch_buffer_with_index (vm, from[2], LOAD);
        vlib_prefetch_buffer_with_index (vm, from[3], LOAD);

        bi0 = from[0];
        bi1 = from[1];

        b0 = vlib_get_buffer (vm, bi0);
        b1 = vlib_get_buffer (vm, bi1);

        if (b0->flags & VLIB_BUFFER_IS_TRACED)
        {
            t0 = vlib_add_trace (vm, node, b0, sizeof (t0[0]));
            t0->flow_hash = vnet_buffer (b0)->pds_data.flow_hash;
            flag0 = vnet_buffer (b0)->pds_data.flags;
            t0->flags = flag0;
            t0->l2_offset = vnet_buffer (b0)->l2_hdr_offset;
            t0->l3_offset = vnet_buffer (b0)->l3_hdr_offset;
            t0->l4_offset = vnet_buffer (b0)->l4_hdr_offset;
            t0->vnic = vnet_buffer (b0)->sw_if_index[VLIB_TX];
        }
        if (b1->flags & VLIB_BUFFER_IS_TRACED)
        {
            t1 = vlib_add_trace (vm, node, b1, sizeof (t1[0]));
            t1->flow_hash = vnet_buffer (b1)->pds_data.flow_hash;
            flag1 = vnet_buffer (b1)->pds_data.flags;
            t1->flags = flag1;
            t1->l2_offset = vnet_buffer (b1)->l2_hdr_offset;
            t1->l3_offset = vnet_buffer (b1)->l3_hdr_offset;
            t1->l4_offset = vnet_buffer (b1)->l4_hdr_offset;
            t1->vnic = vnet_buffer (b1)->sw_if_index[VLIB_TX];
        }
        from += 2;
        n_left -= 2;
    }

    while (n_left >= 1)
    {
        u32 bi0;
        vlib_buffer_t *b0;
        p4cpu_hdr_lookup_trace_t *t0;
        u16 flag0;

        bi0 = from[0];

        b0 = vlib_get_buffer (vm, bi0);

        if (b0->flags & VLIB_BUFFER_IS_TRACED)
        {
            t0 = vlib_add_trace (vm, node, b0, sizeof (t0[0]));
            t0->flow_hash = vnet_buffer (b0)->pds_data.flow_hash;
            flag0 = vnet_buffer (b0)->pds_data.flags;
            t0->flags = flag0;
            t0->l2_offset = vnet_buffer (b0)->l2_hdr_offset;
            t0->l3_offset = vnet_buffer (b0)->l3_hdr_offset;
            t0->l4_offset = vnet_buffer (b0)->l4_hdr_offset;
            t0->vnic = vnet_buffer (b0)->sw_if_index[VLIB_TX];
        }
        from += 1;
        n_left -= 1;
    }
}

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

static uword
pds_p4cpu_hdr_lookup (vlib_main_t * vm,
                      vlib_node_runtime_t * node,
                      vlib_frame_t * from_frame)
{
    u32 counter[P4CPU_HDR_LOOKUP_COUNTER_LAST] = {0};

    PDS_PACKET_LOOP_START {
        PDS_PACKET_DUAL_LOOP_START(WRITE, WRITE) {
            pds_parse_p4cpu_hdr_x2(PDS_PACKET_BUFFER(0),
                                   PDS_PACKET_BUFFER(1),
                                   PDS_PACKET_NEXT_NODE_PTR(0),
                                   PDS_PACKET_NEXT_NODE_PTR(1),
                                   counter);
        } PDS_PACKET_DUAL_LOOP_END;
        PDS_PACKET_SINGLE_LOOP_START {
            pds_parse_p4cpu_hdr_x1(PDS_PACKET_BUFFER(0),
                                   PDS_PACKET_NEXT_NODE_PTR(0),
                                   counter);
        } PDS_PACKET_SINGLE_LOOP_END;
    } PDS_PACKET_LOOP_END;

#define _(n, s) \
    vlib_node_increment_counter (vm, pds_p4cpu_hdr_lookup_node.index,   \
            P4CPU_HDR_LOOKUP_COUNTER_##n,                               \
            counter[P4CPU_HDR_LOOKUP_COUNTER_##n]);
    foreach_p4cpu_hdr_lookup_counter
#undef _

    if (node->flags & VLIB_NODE_FLAG_TRACE) {
        pds_p4cpu_hdr_lookup_trace_add (vm, node, from_frame);
    }

    return from_frame->n_vectors;
}

static char * p4cpu_error_strings[] = {
#define _(n,s) s,
    foreach_p4cpu_hdr_lookup_counter
#undef _
};

VLIB_REGISTER_NODE (pds_p4cpu_hdr_lookup_node) = {
    .function = pds_p4cpu_hdr_lookup,
    .name = "pds-p4cpu-hdr-lookup",
    /* Takes a vector of packets. */
    .vector_size = sizeof (u32),

    .n_errors = P4CPU_HDR_LOOKUP_COUNTER_LAST,
    .error_strings = p4cpu_error_strings,

    .n_next_nodes = P4CPU_HDR_LOOKUP_N_NEXT,
    .next_nodes = {
#define _(s,n) [P4CPU_HDR_LOOKUP_NEXT_##s] = n,
    foreach_p4cpu_hdr_lookup_next
#undef _
    },

    .format_trace = format_pds_p4cpu_hdr_lookup_trace,
};


static clib_error_t *
pds_infra_init (vlib_main_t * vm)
{
    if (0 != initialize_pds()) {
        ASSERT(0);
    }

    return 0;
}

VLIB_INIT_FUNCTION (pds_infra_init);
