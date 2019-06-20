/*
 *  {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
 */

#include "flow_includes.h"

pds_flow_main_t pds_flow_main;

vlib_node_registration_t pds_fwd_flow_node,
                         pds_ip4_flow_prog_node,
                         pds_ip6_flow_prog_node,
                         pds_tun_ip4_flow_prog_node,
                         pds_tun_ip6_flow_prog_node,
                         pds_p4cpu_hdr_lookup_node;

u32
pen_flow_get_p4cpu_lk_node_index (void)
{
    return pds_p4cpu_hdr_lookup_node.index;
}

static u8 *
format_pds_fwd_flow_trace (u8 * s, va_list * args)
{
    CLIB_UNUSED (vlib_main_t * vm) = va_arg (*args, vlib_main_t *);
    CLIB_UNUSED (vlib_node_t * node) = va_arg (*args, vlib_node_t *);
    fwd_flow_trace_t *t = va_arg (*args, fwd_flow_trace_t *);

    s = format (s, "Out Interface %U",
        format_vnet_hw_if_index_name, vnet_get_main(), t->hw_index);
    return s;
}

void
pds_fwd_flow_trace_add (vlib_main_t * vm,
                        vlib_node_runtime_t * node,
                        vlib_frame_t * frame)
{
    u32 *from, n_left;

    n_left = frame->n_vectors;
    from = vlib_frame_vector_args (frame);

    while (n_left >= 4) {
        u32 bi0, bi1;
        vlib_buffer_t *b0, *b1;
        fwd_flow_trace_t *t0, *t1;

        /* Prefetch next iteration. */
        vlib_prefetch_buffer_with_index (vm, from[2], LOAD);
        vlib_prefetch_buffer_with_index (vm, from[3], LOAD);

        bi0 = from[0];
        bi1 = from[1];

        b0 = vlib_get_buffer (vm, bi0);
        b1 = vlib_get_buffer (vm, bi1);

        if (b0->flags & VLIB_BUFFER_IS_TRACED) {
            t0 = vlib_add_trace (vm, node, b0, sizeof (t0[0]));
            t0->hw_index = vnet_buffer (b0)->sw_if_index[VLIB_TX];
        }
        if (b1->flags & VLIB_BUFFER_IS_TRACED) {
            t1 = vlib_add_trace (vm, node, b1, sizeof (t1[0]));
            t1->hw_index = vnet_buffer (b1)->sw_if_index[VLIB_TX];
        }
        from += 2;
        n_left -= 2;
    }

    while (n_left >= 1) {
        u32 bi0;
        vlib_buffer_t *b0;
        fwd_flow_trace_t *t0;

        bi0 = from[0];

        b0 = vlib_get_buffer (vm, bi0);

        if (b0->flags & VLIB_BUFFER_IS_TRACED) {
            t0 = vlib_add_trace (vm, node, b0, sizeof (t0[0]));
            t0->hw_index = vnet_buffer (b0)->sw_if_index[VLIB_TX];
        }
        from += 1;
        n_left -= 1;
    }
}

u32 pds_fwd_flow_next = FWD_FLOW_NEXT_INTF_OUT;

static uword
pds_fwd_flow (vlib_main_t * vm,
              vlib_node_runtime_t * node,
              vlib_frame_t * from_frame)
{

    u32 n_left_from, *from = NULL, *to_next = NULL;
    vnet_main_t *vnm = vnet_get_main ();
    vnet_interface_main_t *im = &vnm->interface_main;
    u32 thread_index = vm->thread_index;
    u64 pkt_bytes = 0, pkt_count = 0;
    u32 sw_if_index = ~0;
    vlib_frame_t *to_frame = 0;
    vnet_hw_interface_t *hw = 0;

    from = vlib_frame_vector_args (from_frame);
    n_left_from = from_frame->n_vectors;

   // next = pds_fwd_flow_next; //FWD_FLOW_NEXT_INTF_OUT; //node->cached_next_index;

    while (n_left_from > 0) {

        while (n_left_from >= 4) {
            vlib_buffer_t *p0, *p1;
            u32 pi0, pi1;
            p4_tx_cpu_hdr_t *tx0, *tx1;
            u16 flag0, flag1;

            /* Prefetch next iteration. */
            {
                vlib_buffer_t *p2, *p3;

                p2 = vlib_get_buffer (vm, from[2]);
                p3 = vlib_get_buffer (vm, from[3]);

                vlib_prefetch_buffer_header (p2, WRITE);
                vlib_prefetch_buffer_header (p3, WRITE);

                CLIB_PREFETCH (p2->data, VPP_PREDICATE_HDR_SZ, WRITE);
                CLIB_PREFETCH (p3->data, VPP_PREDICATE_HDR_SZ, WRITE);
            }

            pi0 = from[0];
            pi1 = from[1];

            p0 = vlib_get_buffer (vm, pi0);
            p1 = vlib_get_buffer (vm, pi1);
            vnet_buffer (p0)->sw_if_index[VLIB_TX] = vnet_buffer (p0)->sw_if_index[VLIB_RX];
            vnet_buffer (p1)->sw_if_index[VLIB_TX] = vnet_buffer (p1)->sw_if_index[VLIB_RX];
 
            tx0 = vlib_buffer_get_current(p0);
            tx1 = vlib_buffer_get_current(p1);
            tx0->flags_octet = 0;
            tx1->flags_octet = 0;
            flag0 = vnet_buffer (p0)->pds_data.flags;
            flag1 = vnet_buffer (p1)->pds_data.flags;
            if (PREDICT_TRUE(flag0 & VPP_CPU_FLAGS_DIRECTION)) {
                tx0->direction = 1;
            }
            if (PREDICT_TRUE(flag1 & VPP_CPU_FLAGS_DIRECTION)) {
                tx1->direction = 1;
            }
            if (PREDICT_FALSE((sw_if_index != vnet_buffer (p0)->sw_if_index[VLIB_TX]) || !to_frame)) {
                if (to_frame) {
                    hw = vnet_get_sup_hw_interface (vnm, sw_if_index);
                    vlib_put_frame_to_node (vm, hw->tx_node_index, to_frame);
                    vlib_increment_combined_counter(im->combined_sw_if_counters
                                                    + VNET_INTERFACE_COUNTER_TX,
                                                    thread_index,
                                                    sw_if_index,
                                                    pkt_count, pkt_bytes);
                    pkt_bytes = 0;
                    pkt_count = 0;
                }
                sw_if_index = vnet_buffer (p0)->sw_if_index[VLIB_TX];
                hw = vnet_get_sup_hw_interface(vnm, sw_if_index);
                to_frame = vlib_get_frame_to_node(vm, hw->tx_node_index);
                to_next = vlib_frame_vector_args(to_frame);
            }
            pkt_bytes += vlib_buffer_length_in_chain(vm, p0);
            pkt_count++;
            to_frame->n_vectors++;

            if (PREDICT_FALSE((sw_if_index != vnet_buffer (p1)->sw_if_index[VLIB_TX]) || !to_frame)) {
                if (to_frame) {
                    hw = vnet_get_sup_hw_interface (vnm, sw_if_index);
                    vlib_put_frame_to_node (vm, hw->tx_node_index, to_frame);
                    vlib_increment_combined_counter(im->combined_sw_if_counters
                                                    + VNET_INTERFACE_COUNTER_TX,
                                                    thread_index,
                                                    sw_if_index,
                                                    pkt_count, pkt_bytes);
                    pkt_bytes = 0;
                    pkt_count = 0;
                }
                sw_if_index = vnet_buffer (p1)->sw_if_index[VLIB_TX];
                hw = vnet_get_sup_hw_interface(vnm, sw_if_index);
                to_frame = vlib_get_frame_to_node(vm, hw->tx_node_index);
                to_next = vlib_frame_vector_args(to_frame);
            }
            pkt_bytes += vlib_buffer_length_in_chain(vm, p1);
            pkt_count++;
            to_frame->n_vectors++;
            
            to_next[0] = from[0];
            to_next[1] = from[1];

            from += 2;
            to_next += 2;
            n_left_from -= 2;
        }

        while (n_left_from > 0) {
            vlib_buffer_t *p0;
            u32 pi0;
            p4_tx_cpu_hdr_t *tx0;
            u16 flag0;

            pi0 = from[0];

            p0 = vlib_get_buffer (vm, pi0);
            vnet_buffer (p0)->sw_if_index[VLIB_TX] = vnet_buffer (p0)->sw_if_index[VLIB_RX];

            tx0 = vlib_buffer_get_current(p0);
            tx0->flags_octet = 0;
            flag0 = vnet_buffer (p0)->pds_data.flags;
            if (PREDICT_TRUE(flag0 & VPP_CPU_FLAGS_DIRECTION)) {
                tx0->direction = 1;
            }
            if (PREDICT_FALSE((sw_if_index != vnet_buffer (p0)->sw_if_index[VLIB_TX]) || !to_frame)) {
                if (to_frame) {
                    hw = vnet_get_sup_hw_interface (vnm, sw_if_index);
                    vlib_put_frame_to_node (vm, hw->tx_node_index, to_frame);
                    vlib_increment_combined_counter(im->combined_sw_if_counters
                                                    + VNET_INTERFACE_COUNTER_TX,
                                                    thread_index,
                                                    sw_if_index,
                                                    pkt_count, pkt_bytes);
                    pkt_bytes = 0;
                    pkt_count = 0;
                }
                sw_if_index = vnet_buffer (p0)->sw_if_index[VLIB_TX];
                hw = vnet_get_sup_hw_interface(vnm, sw_if_index);
                to_frame = vlib_get_frame_to_node(vm, hw->tx_node_index);
                to_next = vlib_frame_vector_args(to_frame);
            }
            pkt_bytes += vlib_buffer_length_in_chain(vm, p0);
            pkt_count++;
            to_frame->n_vectors++;

            to_next[0] = pi0;

            from += 1;
            to_next += 1;
            n_left_from -= 1;
        }
        vlib_put_frame_to_node (vm, hw->tx_node_index, to_frame);
    }

    vlib_increment_combined_counter(im->combined_sw_if_counters
                                    + VNET_INTERFACE_COUNTER_TX,
                                    thread_index,
                                    sw_if_index,
                                    pkt_count, pkt_bytes);

    if (node->flags & VLIB_NODE_FLAG_TRACE) {
        pds_fwd_flow_trace_add (vm, node, from_frame);
    }

    return from_frame->n_vectors;
}

static char * fwd_flow_error_strings[] = {
#define _(n,s) s,
    foreach_fwd_flow_counter
#undef _
};

VLIB_REGISTER_NODE (pds_fwd_flow_node) = {
    .function = pds_fwd_flow,
    .name = "pds-fwd-flow",
    /* Takes a vector of packets. */
    .vector_size = sizeof (u32),

    .n_errors = FWD_FLOW_COUNTER_LAST,
    .error_strings = fwd_flow_error_strings,

    .n_next_nodes = FWD_FLOW_N_NEXT,
    .next_nodes = {
#define _(s,n) [FWD_FLOW_NEXT_##s] = n,
    foreach_fwd_flow_next
#undef _
    },

    .format_trace = format_pds_fwd_flow_trace,
};

static u8 *
format_pds_session_prog_trace (u8 * s, va_list * args)
{
    CLIB_UNUSED (vlib_main_t * vm) = va_arg (*args, vlib_main_t *);
    CLIB_UNUSED (vlib_node_t * node) = va_arg (*args, vlib_node_t *);
    session_prog_trace_t *t = va_arg (*args, session_prog_trace_t *);

    s = format (s, "Session ID %u,\nData: %U", t->session_id,
                format_hex_bytes, t->data, sizeof(t->data));
    return s;
}

void
pds_session_prog_trace_add (vlib_main_t * vm,
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
        session_prog_trace_t *t0, *t1;
        udp_header_t *udp0, *udp1;

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
            t0->session_id = vnet_buffer (b0)->pds_data.ses_id;
            clib_memcpy(t0->data,
                        (vlib_buffer_get_current(b0) - pds_session_get_advance_offset()),
                        sizeof(t0->data));
        }
        if (b1->flags & VLIB_BUFFER_IS_TRACED)
        {
            t1 = vlib_add_trace (vm, node, b1, sizeof (t1[0]));
            t1->session_id = vnet_buffer (b1)->pds_data.ses_id;
            clib_memcpy(t1->data,
                        (vlib_buffer_get_current(b1) - pds_session_get_advance_offset()),
                        sizeof(t0->data));
        }
        from += 2;
        n_left -= 2;
    }

    while (n_left >= 1)
    {
        u32 bi0;
        vlib_buffer_t *b0;
        session_prog_trace_t *t0;

        bi0 = from[0];

        b0 = vlib_get_buffer (vm, bi0);

        if (b0->flags & VLIB_BUFFER_IS_TRACED)
        {
            t0 = vlib_add_trace (vm, node, b0, sizeof (t0[0]));
            t0->session_id = vnet_buffer (b0)->pds_data.ses_id;
            clib_memcpy(t0->data,
                        (vlib_buffer_get_current(b0) - pds_session_get_advance_offset()),
                        sizeof(t0->data));
        }
        from += 1;
        n_left -= 1;
    }
}

static uword
pds_session_prog (vlib_main_t * vm,
                  vlib_node_runtime_t * node,
                  vlib_frame_t * from_frame)
{
    u32 n_left_from, * from;
    pds_flow_main_t *fm = &pds_flow_main;
    vlib_buffer_t *bufs[VLIB_FRAME_SIZE], **b;
    int thread_id = node->thread_index;
    u32 counter[SESSION_PROG_COUNTER_LAST] = {0};
    u16 nexts[VLIB_FRAME_SIZE], *next;
    int session_id;

    from = vlib_frame_vector_args (from_frame);
    n_left_from = from_frame->n_vectors;

    vlib_get_buffers (vm, from, bufs, n_left_from);
    b = bufs;
    next = nexts;

    while (n_left_from > 0) {

        while (n_left_from >= 4) {
            u32 session_id0, session_id1;

            /* Prefetch next iteration. */
            {
                vlib_prefetch_buffer_header (b[2], LOAD);
                vlib_prefetch_buffer_header (b[3], LOAD);

                vlib_prefetch_buffer_data(b[2], LOAD);
                vlib_prefetch_buffer_data(b[3], LOAD);
            }
            session_id0 = vnet_buffer (b[0])->pds_data.ses_id;
            session_id1 = vnet_buffer (b[1])->pds_data.ses_id;
            pds_session_prog_x2(b, session_id0, session_id1, next, counter);

            b += 2;
            next += 2;
            n_left_from -= 2;
        }

        while (n_left_from > 0) {
            u32 session_id0;

            session_id0 = vnet_buffer (b[0])->pds_data.ses_id;
            pds_session_prog_x1(b[0], session_id0, next, counter);

            b += 1;
            next += 1;
            n_left_from -= 1;
        }
    }

    vlib_buffer_enqueue_to_next (vm, node, from, nexts, from_frame->n_vectors);

#define _(n, s) \
    vlib_node_increment_counter (vm, node->node_index,              \
            SESSION_PROG_COUNTER_##n,                               \
            counter[SESSION_PROG_COUNTER_##n]);
    foreach_session_prog_counter
#undef _

    if (node->flags & VLIB_NODE_FLAG_TRACE) {
        pds_session_prog_trace_add (vm, node, from_frame);
    }

    return from_frame->n_vectors;
}

static char * session_prog_error_strings[] = {
#define _(n,s) s,
    foreach_session_prog_counter
#undef _
};

VLIB_REGISTER_NODE (pds_session_prog_node) = {
    .function = pds_session_prog,
    .name = "pds-session-program",
    /* Takes a vector of packets. */
    .vector_size = sizeof (u32),

    .n_errors = SESSION_PROG_COUNTER_LAST,
    .error_strings = session_prog_error_strings,

    .n_next_nodes = SESSION_PROG_N_NEXT,
    .next_nodes = {
#define _(s,n) [SESSION_PROG_NEXT_##s] = n,
    foreach_session_prog_next
#undef _
    },

    .format_trace = format_pds_session_prog_trace,
};

static u8 *
format_pds_flow_prog_trace (u8 * s, va_list * args)
{
    CLIB_UNUSED (vlib_main_t * vm) = va_arg (*args, vlib_main_t *);
    CLIB_UNUSED (vlib_node_t * node) = va_arg (*args, vlib_node_t *);
    flow_prog_trace_t *t = va_arg (*args, flow_prog_trace_t *);

    s = format (s, "src[%U], dst[%U], IP_Proto[%U], src_port[%d], "
            "dst_port[%d]", format_ip46_address, &t->src, IP46_TYPE_ANY,
            format_ip46_address, &t->dst, IP46_TYPE_ANY,
            format_ip_protocol, t->protocol,
            clib_net_to_host_u16(t->src_port),
            clib_net_to_host_u16(t->dst_port));
    return s;
}

void
pds_flow_prog_trace_add (vlib_main_t * vm,
                         vlib_node_runtime_t * node,
                         vlib_frame_t * frame, u8 is_ip4)
{
    u32 *from, n_left;

    n_left = frame->n_vectors;
    from = vlib_frame_vector_args (frame);

    while (n_left >= 4)
    {
        u32 bi0, bi1;
        vlib_buffer_t *b0, *b1;
        flow_prog_trace_t *t0, *t1;
        udp_header_t *udp0, *udp1;
        int offset0, offset1;

        /* Prefetch next iteration. */
        vlib_prefetch_buffer_with_index (vm, from[2], LOAD);
        vlib_prefetch_buffer_with_index (vm, from[3], LOAD);

        bi0 = from[0];
        bi1 = from[1];

        b0 = vlib_get_buffer (vm, bi0);
        b1 = vlib_get_buffer (vm, bi1);

        offset0 = pds_flow_prog_get_next_offset(b0);
        offset1 = pds_flow_prog_get_next_offset(b1);

        if (b0->flags & VLIB_BUFFER_IS_TRACED)
        {
            t0 = vlib_add_trace (vm, node, b0, sizeof (t0[0]));
            if (is_ip4) {
                ip4_header_t *ip40;

                ip40 = vlib_buffer_get_current(b0);
                ip40 = (ip4_header_t *) (((u8 *)vlib_buffer_get_current(b0)) +
                        offset0);
                ip46_address_set_ip4(&t0->src, &ip40->src_address);
                ip46_address_set_ip4(&t0->dst, &ip40->dst_address);
                t0->protocol = ip40->protocol;
                if (PREDICT_TRUE(((t0->protocol == IP_PROTOCOL_TCP)
                        || (t0->protocol == IP_PROTOCOL_UDP)))) {
                    udp0 = (udp_header_t *) (((u8 *) ip40) +
                            (vnet_buffer (b0)->l4_hdr_offset -
                                    vnet_buffer (b0)->l3_hdr_offset));
                    t0->src_port = udp0->src_port;
                    t0->dst_port = udp0->dst_port;
                }
            } else {

                ip6_header_t *ip60;

                ip60 = (ip6_header_t *) (((u8 *)vlib_buffer_get_current(b0)) +
                        offset0);
                ip46_address_set_ip6(&t0->src, &ip60->src_address);
                ip46_address_set_ip6(&t0->dst, &ip60->dst_address);
                t0->protocol = ip60->protocol;
                if (PREDICT_TRUE(((t0->protocol == IP_PROTOCOL_TCP)
                        || (t0->protocol == IP_PROTOCOL_UDP)))) {
                    udp0 = (udp_header_t *) (((u8 *) ip60) +
                            (vnet_buffer (b0)->l4_hdr_offset -
                                    vnet_buffer (b0)->l3_hdr_offset));
                    t0->src_port = udp0->src_port;
                    t0->dst_port = udp0->dst_port;
                }
            }
        }
        if (b1->flags & VLIB_BUFFER_IS_TRACED)
        {
            t1 = vlib_add_trace (vm, node, b1, sizeof (t1[0]));
            if (is_ip4) {
                ip4_header_t *ip41;

                ip41 = (ip4_header_t *) (((u8 *)vlib_buffer_get_current(b1)) +
                        offset1);
                ip46_address_set_ip4(&t1->src, &ip41->src_address);
                ip46_address_set_ip4(&t1->dst, &ip41->dst_address);
                t1->protocol = ip41->protocol;
                if (PREDICT_TRUE(((t1->protocol == IP_PROTOCOL_TCP)
                        || (t1->protocol == IP_PROTOCOL_UDP)))) {
                    udp1 = (udp_header_t *) (((u8 *) ip41) +
                            (vnet_buffer (b1)->l4_hdr_offset -
                                    vnet_buffer (b1)->l3_hdr_offset));
                    t1->src_port = udp1->src_port;
                    t1->dst_port = udp1->dst_port;
                }
            } else {
                ip6_header_t *ip61;

                ip61 = (ip6_header_t *) (((u8 *)vlib_buffer_get_current(b1)) +
                        offset1);
                ip46_address_set_ip6(&t1->src, &ip61->src_address);
                ip46_address_set_ip6(&t1->dst, &ip61->dst_address);
                t1->protocol = ip61->protocol;
                if (PREDICT_TRUE(((t1->protocol == IP_PROTOCOL_TCP)
                        || (t1->protocol == IP_PROTOCOL_UDP)))) {
                    udp1 = (udp_header_t *) (((u8 *) ip61) +
                            (vnet_buffer (b1)->l4_hdr_offset -
                                    vnet_buffer (b1)->l3_hdr_offset));
                    t1->src_port = udp1->src_port;
                    t1->dst_port = udp1->dst_port;
                }
            }
        }
        from += 2;
        n_left -= 2;
    }

    while (n_left >= 1)
    {
        u32 bi0;
        vlib_buffer_t *b0;
        flow_prog_trace_t *t0;
        udp_header_t *udp0;
        int offset0;

        bi0 = from[0];

        b0 = vlib_get_buffer (vm, bi0);
        offset0 = pds_flow_prog_get_next_offset(b0);

        if (b0->flags & VLIB_BUFFER_IS_TRACED)
        {
            t0 = vlib_add_trace (vm, node, b0, sizeof (t0[0]));
            if (is_ip4) {
                ip4_header_t *ip40;

                ip40 = (ip4_header_t *) (((u8 *)vlib_buffer_get_current(b0)) +
                        offset0);
                ip46_address_set_ip4(&t0->src, &ip40->src_address);
                ip46_address_set_ip4(&t0->dst, &ip40->dst_address);
                t0->protocol = ip40->protocol;
                if (PREDICT_TRUE(((t0->protocol == IP_PROTOCOL_TCP)
                        || (t0->protocol == IP_PROTOCOL_UDP)))) {
                    udp0 = (udp_header_t *) (((u8 *) ip40) +
                            (vnet_buffer (b0)->l4_hdr_offset -
                                    vnet_buffer (b0)->l3_hdr_offset));
                    t0->src_port = udp0->src_port;
                    t0->dst_port = udp0->dst_port;
                }
            } else {
                ip6_header_t *ip60;

                ip60 = (ip6_header_t *) (((u8 *)vlib_buffer_get_current(b0)) +
                        offset0);
                ip46_address_set_ip6(&t0->src, &ip60->src_address);
                ip46_address_set_ip6(&t0->dst, &ip60->dst_address);
                t0->protocol = ip60->protocol;
                if (PREDICT_TRUE(((t0->protocol == IP_PROTOCOL_TCP)
                        || (t0->protocol == IP_PROTOCOL_UDP)))) {
                    udp0 = (udp_header_t *) (((u8 *) ip60) +
                            (vnet_buffer (b0)->l4_hdr_offset -
                                    vnet_buffer (b0)->l3_hdr_offset));
                    t0->src_port = udp0->src_port;
                    t0->dst_port = udp0->dst_port;
                }
            }
        }
        from += 1;
        n_left -= 1;
    }
}

always_inline uword
pds_flow_prog (vlib_main_t * vm,
               vlib_node_runtime_t * node,
               vlib_frame_t * from_frame, u8 is_ip4)
{
    u32 n_left_from, * from;
    pds_flow_main_t *fm = &pds_flow_main;
    vlib_buffer_t *bufs[VLIB_FRAME_SIZE], **b;
    int thread_id = node->thread_index;
    int size = 0, i = 0;
    pds_flow_params_t *params =
            is_ip4 ? fm->ip4_flow_params[thread_id] :
                    fm->ip6_flow_params[thread_id];
    u32 counter[FLOW_PROG_COUNTER_LAST] = {0};
    u16 nexts[VLIB_FRAME_SIZE], *next;
    int session_id;

    from = vlib_frame_vector_args (from_frame);
    n_left_from = from_frame->n_vectors;

    vlib_get_buffers (vm, from, bufs, n_left_from);
    b = bufs;
    next = nexts;

    while (n_left_from > 0) {

        while (n_left_from >= 4) {
            u32 session_id0 = 0, session_id1 = 0;
            int offset0, offset1;

            /* Prefetch next iteration. */
            {
                vlib_prefetch_buffer_header (b[2], LOAD);
                vlib_prefetch_buffer_header (b[3], LOAD);

                vlib_prefetch_buffer_data(b[2], LOAD);
                vlib_prefetch_buffer_data(b[3], LOAD);
                CLIB_PREFETCH (params + size,
                               (2 * sizeof(pds_flow_params_t)), WRITE);
            }

            pds_session_id_alloc2(&session_id0, &session_id1);
            pds_flow_extract_prog_args_x1(b[0], params, &size, session_id0, is_ip4);
            pds_flow_extract_prog_args_x1(b[1], params, &size, session_id1, is_ip4);
            offset0 = pds_flow_prog_get_next_offset(b[0]);
            offset1 = pds_flow_prog_get_next_offset(b[1]);
            vlib_buffer_advance(b[0], (-offset0));
            vlib_buffer_advance(b[1], (-offset1));

            b += 2;
            n_left_from -= 2;
        }

        while (n_left_from > 0) {
            u32 session_id0;
            int offset0;

            session_id0 = pds_session_id_alloc();
            pds_flow_extract_prog_args_x1(b[0], params, &size, session_id0, is_ip4);
            offset0 = pds_flow_prog_get_next_offset(b[0]);
            vlib_buffer_advance(b[0], (-offset0));

            b += 1;
            n_left_from -= 1;
        }
    }

    if (is_ip4) {
        pds_flow_program_hw_ip4(params, size, nexts, counter);
    } else {
        pds_flow_program_hw_ip6(params, size, nexts, counter);
    }
    vlib_buffer_enqueue_to_next (vm, node, from, nexts, from_frame->n_vectors);

#define _(n, s) \
    vlib_node_increment_counter (vm, node->node_index,           \
            FLOW_PROG_COUNTER_##n,                               \
            counter[FLOW_PROG_COUNTER_##n]);
    foreach_flow_prog_counter
#undef _

    if (node->flags & VLIB_NODE_FLAG_TRACE) {
        pds_flow_prog_trace_add (vm, node, from_frame, is_ip4);
    }

    return from_frame->n_vectors;
}

static uword
pds_ip6_flow_prog (vlib_main_t * vm,
                   vlib_node_runtime_t * node,
                   vlib_frame_t * from_frame)
{
    return pds_flow_prog(vm, node, from_frame, 1);
}

static char * flow_prog_error_strings[] = {
#define _(n,s) s,
    foreach_flow_prog_counter
#undef _
};

VLIB_REGISTER_NODE (pds_ip6_flow_prog_node) = {
    .function = pds_ip6_flow_prog,
    .name = "pds-ip6-flow-program",
    /* Takes a vector of packets. */
    .vector_size = sizeof (u32),

    .n_errors = FLOW_PROG_COUNTER_LAST,
    .error_strings = flow_prog_error_strings,

    .n_next_nodes = FLOW_PROG_N_NEXT,
    .next_nodes = {
#define _(s,n) [FLOW_PROG_NEXT_##s] = n,
    foreach_flow_prog_next
#undef _
    },

    .format_trace = format_pds_flow_prog_trace,
};

static uword
pds_ip4_flow_prog (vlib_main_t * vm,
                   vlib_node_runtime_t * node,
                   vlib_frame_t * from_frame)
{
    return pds_flow_prog(vm, node, from_frame, 1);
}

VLIB_REGISTER_NODE (pds_ip4_flow_prog_node) = {
    .function = pds_ip4_flow_prog,
    .name = "pds-ip4-flow-program",
    /* Takes a vector of packets. */
    .vector_size = sizeof (u32),

    .n_errors = FLOW_PROG_COUNTER_LAST,
    .error_strings = flow_prog_error_strings,

    .n_next_nodes = FLOW_PROG_N_NEXT,
    .next_nodes = {
#define _(s,n) [FLOW_PROG_NEXT_##s] = n,
    foreach_flow_prog_next
#undef _
    },

    .format_trace = format_pds_flow_prog_trace,
};

static uword
pds_tunnel_ip6_flow_prog (vlib_main_t * vm,
                          vlib_node_runtime_t * node,
                          vlib_frame_t * from_frame)
{
    return pds_flow_prog(vm, node, from_frame, 1);
}

VLIB_REGISTER_NODE (pds_tun_ip6_flow_prog_node) = {
    .function = pds_tunnel_ip6_flow_prog,
    .name = "pds-tunnel-ip6-flow-program",
    /* Takes a vector of packets. */
    .vector_size = sizeof (u32),

    .n_errors = FLOW_PROG_COUNTER_LAST,
    .error_strings = flow_prog_error_strings,

    .n_next_nodes = FLOW_PROG_N_NEXT,
    .next_nodes = {
#define _(s,n) [FLOW_PROG_NEXT_##s] = n,
    foreach_flow_prog_next
#undef _
    },

    .format_trace = format_pds_flow_prog_trace,
};

static uword
pds_tunnel_ip4_flow_prog (vlib_main_t * vm,
                          vlib_node_runtime_t * node,
                          vlib_frame_t * from_frame)
{
    return pds_flow_prog(vm, node, from_frame, 1);
}

VLIB_REGISTER_NODE (pds_tun_ip4_flow_prog_node) = {
    .function = pds_tunnel_ip4_flow_prog,
    .name = "pds-tunnel-ip4-flow-program",
    /* Takes a vector of packets. */
    .vector_size = sizeof (u32),

    .n_errors = FLOW_PROG_COUNTER_LAST,
    .error_strings = flow_prog_error_strings,

    .n_next_nodes = FLOW_PROG_N_NEXT,
    .next_nodes = {
#define _(s,n) [FLOW_PROG_NEXT_##s] = n,
    foreach_flow_prog_next
#undef _
    },

    .format_trace = format_pds_flow_prog_trace,
};

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

always_inline void
pds_parse_p4cpu_hdr_x2 (vlib_buffer_t *p0, vlib_buffer_t *p1,
                        u32 *next0, u32 *next1, u32 *counter)
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

    /* Move to IPv4/IPv6 header */
    vlib_buffer_advance(p0, (VPP_P4_TO_ARM_HDR_SZ +
        (vnet_buffer(p0)->l3_hdr_offset - vnet_buffer (p0)->l2_hdr_offset)));
    vlib_buffer_advance(p1, (VPP_P4_TO_ARM_HDR_SZ +
        (vnet_buffer(p1)->l3_hdr_offset - vnet_buffer (p1)->l2_hdr_offset)));

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
    } else {
        *next1 = P4CPU_HDR_LOOKUP_NEXT_DROP;
        counter[P4CPU_HDR_LOOKUP_COUNTER_UNKOWN] += 1;
    }
}

always_inline void
pds_parse_p4cpu_hdr_x1 (vlib_buffer_t *p, u32 *next, u32 *counter)
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

    /* Move to IPv4/IPv6 header */
    vlib_buffer_advance(p, (VPP_P4_TO_ARM_HDR_SZ + 
        (vnet_buffer (p)->l3_hdr_offset - vnet_buffer (p)->l2_hdr_offset)));

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
    u32 n_left_from, n_left_to_next, * from, * to_next;
    u32 counter[P4CPU_HDR_LOOKUP_COUNTER_LAST] = {0};
    u32 next = ~0;

    from = vlib_frame_vector_args (from_frame);
    n_left_from = from_frame->n_vectors;

    next = node->cached_next_index;

    while (n_left_from > 0) {

        vlib_get_next_frame (vm, node, next, to_next, n_left_to_next);

        while (n_left_from >= 4 && n_left_to_next >= 2) {
            vlib_buffer_t *p0, *p1;
            u32 pi0, pi1, next0, next1 = 0;

            /* Prefetch next iteration. */
            {
                vlib_buffer_t *p2, *p3;

                p2 = vlib_get_buffer (vm, from[2]);
                p3 = vlib_get_buffer (vm, from[3]);

                vlib_prefetch_buffer_header (p2, WRITE);
                vlib_prefetch_buffer_header (p3, WRITE);

                CLIB_PREFETCH (p2->data, sizeof(p4_rx_cpu_hdr_t), LOAD);
                CLIB_PREFETCH (p3->data, sizeof(p4_rx_cpu_hdr_t), LOAD);
            }

            pi0 = to_next[0] = from[0];
            pi1 = to_next[1] = from[1];

            from += 2;
            to_next += 2;
            n_left_to_next -= 2;
            n_left_from -= 2;

            p0 = vlib_get_buffer (vm, pi0);
            p1 = vlib_get_buffer (vm, pi1);

            pds_parse_p4cpu_hdr_x2(p0, p1, &next0, &next1, counter);

            vlib_validate_buffer_enqueue_x2 (vm, node, next,
                    to_next, n_left_to_next,
                    pi0, pi1, next0, next1);
        }
        while (n_left_from > 0 && n_left_to_next > 0) {
            vlib_buffer_t *p0;
            u32 pi0, next0;

            pi0 = from[0];
            to_next[0] = pi0;

            from += 1;
            to_next += 1;
            n_left_to_next -= 1;
            n_left_from -= 1;

            p0 = vlib_get_buffer (vm, pi0);
            pds_parse_p4cpu_hdr_x1(p0, &next0, counter);

            if (PREDICT_FALSE (next0 != next)) {
                n_left_to_next += 1;
                vlib_put_next_frame (vm, node, next, n_left_to_next);
                next = next0;
                vlib_get_next_frame (vm, node, next, to_next, n_left_to_next);
                to_next[0] = pi0;
                to_next += 1;
                n_left_to_next -= 1;
            }
        }

        vlib_put_next_frame (vm, node, next, n_left_to_next);
    }

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
pds_flow_init (vlib_main_t * vm)
{
    pds_flow_main_t *fm = &pds_flow_main;
    int no_of_threads = fm->no_threads = vec_len (vlib_worker_threads);
    int i;

    pool_init_fixed(fm->session_index_pool, MAX_SESSION_INDEX);
    vec_validate_aligned(fm->ip4_flow_params, no_of_threads - 1,
                         CLIB_CACHE_LINE_BYTES);
    clib_memset(fm->ip4_flow_params, 0,
                (sizeof(pds_flow_params_t *) * no_of_threads));
    vec_validate_aligned(fm->ip6_flow_params, no_of_threads - 1,
                         CLIB_CACHE_LINE_BYTES);
    clib_memset(fm->ip6_flow_params, 0,
                (sizeof(pds_flow_params_t *) * no_of_threads));
    vec_validate_aligned(fm->table4, no_of_threads - 1,
                         CLIB_CACHE_LINE_BYTES);
    vec_validate_aligned(fm->table6, no_of_threads - 1,
                         CLIB_CACHE_LINE_BYTES);

    vec_validate(fm->session_id_thr_local_pool,
                 no_of_threads - 1);
    clib_memset(fm->session_id_thr_local_pool, 0,
                (sizeof(pds_flow_session_id_thr_local_pool_t) * no_of_threads));

    if (0 != initialize_pds()) {
        ASSERT(0);
    }

    fm->flow_prog_lock = clib_mem_alloc_aligned(CLIB_CACHE_LINE_BYTES,
                                                CLIB_CACHE_LINE_BYTES);
    clib_atomic_release (fm->flow_prog_lock);

    for (i = 0; i < no_of_threads; i++) {
        vec_validate(fm->ip4_flow_params[i], (MAX_FLOWS_PER_FRAME - 1));
        vec_validate(fm->ip6_flow_params[i], (MAX_FLOWS_PER_FRAME - 1));

        clib_memset(fm->ip4_flow_params[i], 0, sizeof(pds_flow_params_t));
        clib_memset(fm->ip6_flow_params[i], 0, sizeof(pds_flow_params_t));

        fm->table4[i] = ftlv4_create((void *) pds_flow4_key2str, 
                                     (void *) pds_flow_appdata2str,
                                     i);
        fm->table6[i] = ftlv6_create((void *) pds_flow6_key2str, 
                                     (void *) pds_flow_appdata2str,
                                     i);
        fm->session_id_thr_local_pool[i].pool_count = -1;
    }

    return 0;
}

VLIB_INIT_FUNCTION (pds_flow_init);
