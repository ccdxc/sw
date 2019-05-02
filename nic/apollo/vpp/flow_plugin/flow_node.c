/*
 *  {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
 */

#include "flow.h"
#include "p4_cpu_hdr.h"
#include "flow_prog_hw.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <vnet/ethernet/packet.h>
#include "flow_apollo.h"

//#include "pen_flow_test.h"

#define P4TBL_ID_FLOW_HASH      1

typedef struct pen_flow_main_t {
    ftl *table;
    volatile u32 *flow_prog_lock;
    pen_flow_params_t **ip4_flow_params;
    pen_flow_params_t **ip6_flow_params;
} pen_flow_main_t;

pen_flow_main_t pen_flow_main;

vlib_node_registration_t pen_fwd_flow_node,
                         pen_ip4_flow_prog_node,
                         pen_ip6_flow_prog_node,
                         pen_p4cpu_hdr_lookup_node;

u32
pen_flow_get_p4cpu_lk_node_index (void)
{
    return pen_p4cpu_hdr_lookup_node.index;
}

void
pen_flow_prog_lock (void)
{
    pen_flow_main_t *fm = &pen_flow_main;

    clib_atomic_test_and_set (fm->flow_prog_lock);
}

void
pen_flow_prog_unlock (void)
{
    pen_flow_main_t *fm = &pen_flow_main;

    clib_atomic_release (fm->flow_prog_lock);
}

ftl *
pen_flow_prog_get_table (void)
{
    pen_flow_main_t *fm = &pen_flow_main;

    return fm->table;
}

static u8 *
format_pen_fwd_flow_trace (u8 * s, va_list * args)
{
    CLIB_UNUSED (vlib_main_t * vm) = va_arg (*args, vlib_main_t *);
    CLIB_UNUSED (vlib_node_t * node) = va_arg (*args, vlib_node_t *);
    fwd_flow_trace_t *t = va_arg (*args, fwd_flow_trace_t *);

    s = format (s, "Out Interface %U",
        format_vnet_hw_if_index_name, vnet_get_main(), t->hw_index);
    return s;
}

void
pen_fwd_flow_trace_add (vlib_main_t * vm,
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

static uword
pen_fwd_flow (vlib_main_t * vm,
              vlib_node_runtime_t * node,
              vlib_frame_t * from_frame)
{

    u32 n_left_from, next, n_left_to_next, * from, * to_next;
    //u32 thread_index = vlib_get_thread_index();
    vnet_main_t *vnm = vnet_get_main ();

    from = vlib_frame_vector_args (from_frame);
    n_left_from = from_frame->n_vectors;

    next = FWD_FLOW_NEXT_INTF_OUT; //node->cached_next_index;

    while (n_left_from > 0) {

        vlib_get_next_frame (vm, node, next, to_next, n_left_to_next);

        while (n_left_from >= 4 && n_left_to_next >= 2) {
            vlib_buffer_t *p0, *p1;
            u32 pi0, pi1;
            p4_tx_cpu_hdr_t *tx0, *tx1;
            u16 *flag0, *flag1;

            /* Prefetch next iteration. */
            {
                vlib_buffer_t *p2, *p3;

                p2 = vlib_get_buffer (vm, from[2]);
                p3 = vlib_get_buffer (vm, from[3]);

                vlib_prefetch_buffer_header (p2, LOAD);
                vlib_prefetch_buffer_header (p3, LOAD);

                /* Fetch Max ethernet+vlan headers 22 bytes and predicate header */
                CLIB_PREFETCH ((((u8 *) p2->data) - 22 - APOLLO_PREDICATE_HDR_SZ),
                               64, WRITE);
                CLIB_PREFETCH ((((u8 *) p3->data) - 22 - APOLLO_PREDICATE_HDR_SZ),
                               64, WRITE);
            }

            pi0 = to_next[0] = from[0];
            pi1 = to_next[1] = from[1];

            from += 2;
            to_next += 2;
            n_left_to_next -= 2;
            n_left_from -= 2;

            p0 = vlib_get_buffer (vm, pi0);
            p1 = vlib_get_buffer (vm, pi1);
            vnet_buffer (p0)->sw_if_index[VLIB_TX] = vnet_buffer (p0)->sw_if_index[VLIB_RX];
            vnet_buffer (p1)->sw_if_index[VLIB_TX] = vnet_buffer (p1)->sw_if_index[VLIB_RX];

            vlib_buffer_advance(p0, -(APOLLO_PREDICATE_HDR_SZ +
                        (vnet_buffer (p0)->l3_hdr_offset - vnet_buffer (p0)->l2_hdr_offset)));
            vlib_buffer_advance(p1, -(APOLLO_PREDICATE_HDR_SZ +
                        (vnet_buffer (p1)->l3_hdr_offset - vnet_buffer (p1)->l2_hdr_offset)));
       
            tx0 = vlib_buffer_get_current(p0);
            tx1 = vlib_buffer_get_current(p1);
            tx0->flags_octet = 0;
            tx1->flags_octet = 0;
            flag0 = (u16 *) &(vnet_buffer (p0)->pen_data.flags);
            flag1 = (u16 *) &(vnet_buffer (p1)->pen_data.flags);
            if (PREDICT_TRUE((*flag0) & APOLLO_CPU_FLAGS_DIRECTION)) {
                tx0->direction = 1;
            }
            if (PREDICT_TRUE((*flag1) & APOLLO_CPU_FLAGS_DIRECTION)) {
                tx1->direction = 1;
            }
        }

        while (n_left_from > 0 && n_left_to_next > 0) {
            vlib_buffer_t *p0;
            u32 pi0;
            p4_tx_cpu_hdr_t *tx0;
            u16 *flag0;

            pi0 = from[0];
            to_next[0] = pi0;

            from += 1;
            to_next += 1;
            n_left_to_next -= 1;
            n_left_from -= 1;

            p0 = vlib_get_buffer (vm, pi0);
            vnet_buffer (p0)->sw_if_index[VLIB_TX] = vnet_buffer (p0)->sw_if_index[VLIB_RX];
            vlib_buffer_advance(p0, -(APOLLO_PREDICATE_HDR_SZ +
                        (vnet_buffer (p0)->l3_hdr_offset - vnet_buffer (p0)->l2_hdr_offset)));

            tx0 = vlib_buffer_get_current(p0);
            tx0->flags_octet = 0;
            flag0 = (u16 *) &(vnet_buffer (p0)->pen_data.flags);
            if (PREDICT_TRUE((*flag0) & APOLLO_CPU_FLAGS_DIRECTION)) {
                tx0->direction = 1;
            }
        }

        vlib_put_next_frame (vm, node, next, n_left_to_next);
    }

    if (node->flags & VLIB_NODE_FLAG_TRACE) {
        pen_fwd_flow_trace_add (vm, node, from_frame);
    }

    return from_frame->n_vectors;
}

static char * fwd_flow_error_strings[] = {
#define _(n,s) s,
    foreach_fwd_flow_counter
#undef _
};

VLIB_REGISTER_NODE (pen_fwd_flow_node) = {
    .function = pen_fwd_flow,
    .name = "pen-fwd-flow",
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

    .format_trace = format_pen_fwd_flow_trace,
};

static u8 *
format_pen_flow_prog_trace (u8 * s, va_list * args)
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
pen_flow_prog_trace_add (vlib_main_t * vm,
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
            if (is_ip4) {
                ip4_header_t *ip40;

                ip40 = vlib_buffer_get_current(b0);
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

                ip60 = vlib_buffer_get_current(b0);

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

                ip41 = vlib_buffer_get_current(b1);
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

                ip61 = vlib_buffer_get_current(b1);
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

        bi0 = from[0];

        b0 = vlib_get_buffer (vm, bi0);

        if (b0->flags & VLIB_BUFFER_IS_TRACED)
        {
            t0 = vlib_add_trace (vm, node, b0, sizeof (t0[0]));
            if (is_ip4) {
                ip4_header_t *ip40;

                ip40 = vlib_buffer_get_current(b0);
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

                ip60 = vlib_buffer_get_current(b0);
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
pen_flow_prog (vlib_main_t * vm,
               vlib_node_runtime_t * node,
               vlib_frame_t * from_frame, u8 is_ip4)
{
    u32 n_left_from, next = FLOW_PROG_NEXT_FWD_FLOW,
            n_left_to_next, * from, * to_next;
    pen_flow_main_t *fm = &pen_flow_main;
    int thread_id = node->thread_index;
    int size = 0;
    pen_flow_params_t *params =
            is_ip4 ? fm->ip4_flow_params[thread_id] :
                    fm->ip6_flow_params[thread_id];
    u32 counter[FLOW_PROG_COUNTER_LAST] = {0};

    from = vlib_frame_vector_args (from_frame);
    n_left_from = from_frame->n_vectors;

    while (n_left_from > 0) {

        vlib_get_next_frame (vm, node, next, to_next, n_left_to_next);

        while (n_left_from >= 4 && n_left_to_next >= 2) {
            vlib_buffer_t *p0, *p1;
            u32 pi0, pi1;
                //next0 = FLOW_PROG_NEXT_INTF_OUT,
                //next1 = FLOW_PROG_NEXT_INTF_OUT;

            /* Prefetch next iteration. */
            {
                vlib_buffer_t *p2, *p3;

                p2 = vlib_get_buffer (vm, from[2]);
                p3 = vlib_get_buffer (vm, from[3]);

                vlib_prefetch_buffer_header (p2, LOAD);
                vlib_prefetch_buffer_header (p3, LOAD);

                CLIB_PREFETCH (p2->data, CLIB_CACHE_LINE_BYTES, LOAD);
                CLIB_PREFETCH (p3->data, CLIB_CACHE_LINE_BYTES, LOAD);
            }

            pi0 = to_next[0] = from[0];
            pi1 = to_next[1] = from[1];

            from += 2;
            to_next += 2;
            n_left_to_next -= 2;
            n_left_from -= 2;

            p0 = vlib_get_buffer (vm, pi0);
            p1 = vlib_get_buffer (vm, pi1);

            pen_flow_extract_prog_args_x1(p0, params, &size, is_ip4);
            pen_flow_extract_prog_args_x1(p1, params, &size, is_ip4);

        }

        while (n_left_from > 0 && n_left_to_next > 0) {
            vlib_buffer_t *p0;
            u32 pi0;// next0 = FLOW_PROG_NEXT_INTF_OUT;

            pi0 = from[0];
            to_next[0] = pi0;

            from += 1;
            to_next += 1;
            n_left_to_next -= 1;
            n_left_from -= 1;

            p0 = vlib_get_buffer (vm, pi0);
            pen_flow_extract_prog_args_x1(p0, params, &size, is_ip4);
        }

        vlib_put_next_frame (vm, node, next, n_left_to_next);
    }

    pen_flow_program_hw(params, size, counter);

#define _(n, s) \
    vlib_node_increment_counter (vm, node->node_index,           \
            FLOW_PROG_COUNTER_##n,                               \
            counter[FLOW_PROG_COUNTER_##n]);
    foreach_flow_prog_counter
#undef _

    if (node->flags & VLIB_NODE_FLAG_TRACE) {
        pen_flow_prog_trace_add (vm, node, from_frame, is_ip4);
    }

    return from_frame->n_vectors;
}

static uword
pen_ip6_flow_prog (vlib_main_t * vm,
                   vlib_node_runtime_t * node,
                   vlib_frame_t * from_frame)
{
    return pen_flow_prog(vm, node, from_frame, 1);
}

static char * flow_prog_error_strings[] = {
#define _(n,s) s,
    foreach_flow_prog_counter
#undef _
};

VLIB_REGISTER_NODE (pen_ip6_flow_prog_node) = {
    .function = pen_ip6_flow_prog,
    .name = "pen-ip6-flow-program",
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

    .format_trace = format_pen_flow_prog_trace,
};

static uword
pen_ip4_flow_prog (vlib_main_t * vm,
                   vlib_node_runtime_t * node,
                   vlib_frame_t * from_frame)
{
    return pen_flow_prog(vm, node, from_frame, 1);
}

VLIB_REGISTER_NODE (pen_ip4_flow_prog_node) = {
    .function = pen_ip4_flow_prog,
    .name = "pen-ip4-flow-program",
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

    .format_trace = format_pen_flow_prog_trace,
};

static u8 *
format_pen_p4cpu_hdr_lookup_trace (u8 * s, va_list * args)
{
    CLIB_UNUSED (vlib_main_t * vm) = va_arg (*args, vlib_main_t *);
    CLIB_UNUSED (vlib_node_t * node) = va_arg (*args, vlib_node_t *);
    p4cpu_hdr_lookup_trace_t *t = va_arg (*args, p4cpu_hdr_lookup_trace_t *);

    s = format (s, "Flags[%d], flow_hash[0x%x], l2_offset[%d], "
            "l3_offset[%d] l4_offset[%d], vrf[%d]",
            t->flags, t->flow_hash, t->l2_offset, t->l3_offset,
            t->l4_offset, t->vrf);
    return s;
}

void
pen_p4cpu_hdr_lookup_trace_add (vlib_main_t * vm,
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
        u16 *flag0, *flag1;

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
            t0->flow_hash = vnet_buffer (b0)->pen_data.flow_hash;
            flag0 = (u16 *) &(vnet_buffer (b0)->pen_data.flags);
            t0->flags = *flag0;
            t0->l2_offset = vnet_buffer (b0)->l2_hdr_offset;
            t0->l3_offset = vnet_buffer (b0)->l3_hdr_offset;
            t0->l4_offset = vnet_buffer (b0)->l4_hdr_offset;
            t0->vrf = vnet_buffer (b0)->sw_if_index[VLIB_TX];
        }
        if (b1->flags & VLIB_BUFFER_IS_TRACED)
        {
            t1 = vlib_add_trace (vm, node, b1, sizeof (t1[0]));
            t1->flow_hash = vnet_buffer (b1)->pen_data.flow_hash;
            flag1 = (u16 *) &(vnet_buffer (b1)->pen_data.flags);
            t1->flags = *flag0;
            t1->l2_offset = vnet_buffer (b1)->l2_hdr_offset;
            t1->l3_offset = vnet_buffer (b1)->l3_hdr_offset;
            t1->l4_offset = vnet_buffer (b1)->l4_hdr_offset;
            t1->vrf = vnet_buffer (b1)->sw_if_index[VLIB_TX];
        }
        from += 2;
        n_left -= 2;
    }

    while (n_left >= 1)
    {
        u32 bi0;
        vlib_buffer_t *b0;
        p4cpu_hdr_lookup_trace_t *t0;
        u16 *flag0;

        bi0 = from[0];

        b0 = vlib_get_buffer (vm, bi0);

        if (b0->flags & VLIB_BUFFER_IS_TRACED)
        {
            t0 = vlib_add_trace (vm, node, b0, sizeof (t0[0]));
            t0->flow_hash = vnet_buffer (b0)->pen_data.flow_hash;
            flag0 = (u16 *) &(vnet_buffer (b0)->pen_data.flags);
            t0->flags = *flag0;
            t0->l2_offset = vnet_buffer (b0)->l2_hdr_offset;
            t0->l3_offset = vnet_buffer (b0)->l3_hdr_offset;
            t0->l4_offset = vnet_buffer (b0)->l4_hdr_offset;
            t0->vrf = vnet_buffer (b0)->sw_if_index[VLIB_TX];
        }
        from += 1;
        n_left -= 1;
    }
}

always_inline void
pen_parse_p4cpu_hdr_x2 (vlib_buffer_t *p0, vlib_buffer_t *p1,
                        u32 *next0, u32 *next1, u32 *counter)
{
    p4_rx_cpu_hdr_t *hdr0 = vlib_buffer_get_current(p0);
    p4_rx_cpu_hdr_t *hdr1 = vlib_buffer_get_current(p1);
    u16 flag_orig0, flag_orig1;
    u16 *pen_flag0, *pen_flag1;

    //hdr0--;
    //hdr1--;
    //hdr0->flags = APOLLO_CPU_FLAGS_IPV4_1_VALID;
    //hdr1->flags = APOLLO_CPU_FLAGS_IPV4_1_VALID;
    flag_orig0 = clib_net_to_host_u16(hdr0->flags);
    flag_orig1 = clib_net_to_host_u16(hdr1->flags);
    u16 flags0 = flag_orig0 &
        (APOLLO_CPU_FLAGS_IPV4_1_VALID | APOLLO_CPU_FLAGS_IPV6_1_VALID);
    u16 flags1 = flag_orig1 &
        (APOLLO_CPU_FLAGS_IPV4_1_VALID | APOLLO_CPU_FLAGS_IPV6_1_VALID);

    vnet_buffer (p0)->pen_data.flow_hash = clib_net_to_host_u32(hdr0->flow_hash);
    pen_flag0 = (u16 *) &(vnet_buffer (p0)->pen_data.flags);
    *pen_flag0 = flag_orig0;
    vnet_buffer (p0)->l2_hdr_offset = hdr0->l2_offset;
    vnet_buffer (p0)->l3_hdr_offset = hdr0->l3_offset;
    vnet_buffer (p0)->l4_hdr_offset = hdr0->l4_offset;
    vnet_buffer (p0)->sw_if_index[VLIB_TX] = clib_net_to_host_u16(hdr0->local_vnic_tag);

    vnet_buffer (p1)->pen_data.flow_hash = clib_net_to_host_u32(hdr1->flow_hash);
    pen_flag1 = (u16 *) &(vnet_buffer (p1)->pen_data.flags);
    *pen_flag1 = flag_orig1;
    vnet_buffer (p1)->l2_hdr_offset = hdr1->l2_offset;
    vnet_buffer (p1)->l3_hdr_offset = hdr1->l3_offset;
    vnet_buffer (p1)->l4_hdr_offset = hdr1->l4_offset;
    vnet_buffer (p1)->sw_if_index[VLIB_TX] = clib_net_to_host_u16(hdr1->local_vnic_tag);

    /* Move to IPv4/IPv6 header */
    vlib_buffer_advance(p0, (APOLLO_P4_TO_ARM_HDR_SZ +
        (vnet_buffer(p0)->l3_hdr_offset - vnet_buffer (p0)->l2_hdr_offset)));
    vlib_buffer_advance(p1, (APOLLO_P4_TO_ARM_HDR_SZ +
        (vnet_buffer(p1)->l3_hdr_offset - vnet_buffer (p1)->l2_hdr_offset)));

    /* As of now only flow miss packets are punted to VPP for flow programming */
    if (flags0 == flags1) {
        if (flags0 == APOLLO_CPU_FLAGS_IPV4_1_VALID) {
            *next0 = *next1 = P4CPU_HDR_LOOKUP_NEXT_IP4_FLOW_PROG;
            counter[P4CPU_HDR_LOOKUP_COUNTER_IP4_FLOW] += 2;
        } else if (flags0 == APOLLO_CPU_FLAGS_IPV6_1_VALID) {
            *next0 = *next1 = P4CPU_HDR_LOOKUP_NEXT_IP6_FLOW_PROG;
            counter[P4CPU_HDR_LOOKUP_COUNTER_IP6_FLOW] += 2;
        } else {
            *next0 = *next1 = P4CPU_HDR_LOOKUP_NEXT_DROP;
            counter[P4CPU_HDR_LOOKUP_COUNTER_UNKOWN] += 2;
        }
        return;
    }
    if (flags0 == APOLLO_CPU_FLAGS_IPV4_1_VALID) {
        *next0 = P4CPU_HDR_LOOKUP_NEXT_IP4_FLOW_PROG;
        counter[P4CPU_HDR_LOOKUP_COUNTER_IP4_FLOW]++;
        if (flags1 == APOLLO_CPU_FLAGS_IPV6_1_VALID) {
            *next1 = P4CPU_HDR_LOOKUP_NEXT_IP6_FLOW_PROG;
            counter[P4CPU_HDR_LOOKUP_COUNTER_IP6_FLOW]++;
        } else {
            *next1 = P4CPU_HDR_LOOKUP_NEXT_DROP;
            counter[P4CPU_HDR_LOOKUP_COUNTER_UNKOWN]++;
        }
    } else if (flags0 == APOLLO_CPU_FLAGS_IPV6_1_VALID) {
        *next0 = P4CPU_HDR_LOOKUP_NEXT_IP6_FLOW_PROG;
        counter[P4CPU_HDR_LOOKUP_COUNTER_IP6_FLOW]++;
        if (flags1 == APOLLO_CPU_FLAGS_IPV4_1_VALID) {
            *next1 = P4CPU_HDR_LOOKUP_NEXT_IP4_FLOW_PROG;
            counter[P4CPU_HDR_LOOKUP_COUNTER_IP4_FLOW]++;
        } else {
            *next1 = P4CPU_HDR_LOOKUP_NEXT_DROP;
            counter[P4CPU_HDR_LOOKUP_COUNTER_UNKOWN]++;
        }
    } else {
        *next0 = P4CPU_HDR_LOOKUP_NEXT_DROP;
        counter[P4CPU_HDR_LOOKUP_COUNTER_UNKOWN]++;
        if (flags1 == APOLLO_CPU_FLAGS_IPV4_1_VALID) {
            *next1 = P4CPU_HDR_LOOKUP_NEXT_IP4_FLOW_PROG;
            counter[P4CPU_HDR_LOOKUP_COUNTER_IP4_FLOW]++;
        } else if (flags1 == APOLLO_CPU_FLAGS_IPV6_1_VALID) {
            *next1 = P4CPU_HDR_LOOKUP_NEXT_IP6_FLOW_PROG;
            counter[P4CPU_HDR_LOOKUP_COUNTER_IP6_FLOW]++;
        }
    }
}

always_inline void
pen_parse_p4cpu_hdr_x1 (vlib_buffer_t *p, u32 *next, u32 *counter)
{
    p4_rx_cpu_hdr_t *hdr = vlib_buffer_get_current(p);
    //hdr--;
    u16 flag_orig;
    u16 *pen_flag;
    //hdr->flags = APOLLO_CPU_FLAGS_IPV4_1_VALID;

    flag_orig = clib_net_to_host_u16(hdr->flags);
    u16 flags = flag_orig & 
        (APOLLO_CPU_FLAGS_IPV4_1_VALID | APOLLO_CPU_FLAGS_IPV6_1_VALID);

    vnet_buffer (p)->pen_data.flow_hash = clib_net_to_host_u32(hdr->flow_hash);
    pen_flag = (u16 *) &(vnet_buffer (p)->pen_data.flags);
    *pen_flag = flag_orig;
    vnet_buffer (p)->l2_hdr_offset = hdr->l2_offset;
    vnet_buffer (p)->l3_hdr_offset = hdr->l3_offset;
    vnet_buffer (p)->l4_hdr_offset = hdr->l4_offset;

    vnet_buffer (p)->sw_if_index[VLIB_TX] = clib_net_to_host_u16(hdr->local_vnic_tag);

    /* Move to IPv4/IPv6 header */
    vlib_buffer_advance(p, (APOLLO_P4_TO_ARM_HDR_SZ + 
        (vnet_buffer (p)->l3_hdr_offset - vnet_buffer (p)->l2_hdr_offset)));

    /* As of now only flow miss packets are punted to VPP for flow programming */
    if (flags == APOLLO_CPU_FLAGS_IPV4_1_VALID) {
        *next = P4CPU_HDR_LOOKUP_NEXT_IP4_FLOW_PROG;
        counter[P4CPU_HDR_LOOKUP_COUNTER_IP4_FLOW]++;
    } else if (flags == APOLLO_CPU_FLAGS_IPV6_1_VALID) {
        *next = P4CPU_HDR_LOOKUP_NEXT_IP6_FLOW_PROG;
        counter[P4CPU_HDR_LOOKUP_COUNTER_IP6_FLOW]++;
    } else {
        *next = P4CPU_HDR_LOOKUP_NEXT_DROP;
        counter[P4CPU_HDR_LOOKUP_COUNTER_UNKOWN]++;
    }

}

static uword
pen_p4cpu_hdr_lookup (vlib_main_t * vm,
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

                vlib_prefetch_buffer_header (p2, LOAD);
                vlib_prefetch_buffer_header (p3, LOAD);

                CLIB_PREFETCH ((p2->data - sizeof(p4_rx_cpu_hdr_t)), sizeof(p4_rx_cpu_hdr_t), LOAD);
                CLIB_PREFETCH ((p3->data - sizeof(p4_rx_cpu_hdr_t)), sizeof(p4_rx_cpu_hdr_t), LOAD);
            }

            pi0 = to_next[0] = from[0];
            pi1 = to_next[1] = from[1];

            from += 2;
            to_next += 2;
            n_left_to_next -= 2;
            n_left_from -= 2;

            p0 = vlib_get_buffer (vm, pi0);
            p1 = vlib_get_buffer (vm, pi1);

            pen_parse_p4cpu_hdr_x2(p0, p1, &next0, &next1, counter);

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
            pen_parse_p4cpu_hdr_x1(p0, &next0, counter);

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
    vlib_node_increment_counter (vm, pen_p4cpu_hdr_lookup_node.index,   \
            P4CPU_HDR_LOOKUP_COUNTER_##n,                               \
            counter[P4CPU_HDR_LOOKUP_COUNTER_##n]);
    foreach_p4cpu_hdr_lookup_counter
#undef _

    if (node->flags & VLIB_NODE_FLAG_TRACE) {
        pen_p4cpu_hdr_lookup_trace_add (vm, node, from_frame);
    }

    return from_frame->n_vectors;
}

static char * p4cpu_error_strings[] = {
#define _(n,s) s,
    foreach_p4cpu_hdr_lookup_counter
#undef _
};

VLIB_REGISTER_NODE (pen_p4cpu_hdr_lookup_node) = {
    .function = pen_p4cpu_hdr_lookup,
    .name = "pen-p4cpu-hdr-lookup",
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

    .format_trace = format_pen_p4cpu_hdr_lookup_trace,
};

static char *
pen_flow_key2str (void *key)
{
    static char str[256];
    flow_swkey_t *k = (flow_swkey_t *)key;
    char srcstr[INET_ADDRSTRLEN];
    char dststr[INET_ADDRSTRLEN];

    inet_ntop(AF_INET, k->key_metadata_src, srcstr, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, k->key_metadata_dst, dststr, INET_ADDRSTRLEN);
    sprintf(str, "Type:%d Src:%s Dst:%s Dport:%d Sport:%d Proto:%d Tag:%d",
            k->key_metadata_ktype, srcstr, dststr,
            k->key_metadata_dport, k->key_metadata_sport,
            k->key_metadata_proto, k->vnic_metadata_local_vnic_tag);
    return str;
}

static char *
pen_flow_appdata2str (void *appdata)
{
    static char str[512];
    flow_appdata_t *d = (flow_appdata_t *)appdata;
    sprintf(str, "session_index:%d flow_role:%d",
            d->session_index, d->flow_role);
    return str;
}

static clib_error_t *
pen_flow_init (vlib_main_t * vm)
{
    pen_flow_main_t *fm = &pen_flow_main;
    int no_of_threads = vec_len (vlib_worker_threads);
    int i;

    vec_validate_aligned (fm->ip4_flow_params, no_of_threads - 1,
                            CLIB_CACHE_LINE_BYTES);
    vec_validate_aligned (fm->ip6_flow_params, no_of_threads - 1,
                                CLIB_CACHE_LINE_BYTES);

    if (0 != initialize_pds()) {
        ASSERT(0);
    }

    fm->table = ftl_create(P4TBL_ID_FLOW, 5, 8,
            (void *) pen_flow_key2str, (void *)pen_flow_appdata2str);

    fm->flow_prog_lock = clib_mem_alloc_aligned (CLIB_CACHE_LINE_BYTES,
                              CLIB_CACHE_LINE_BYTES);
    clib_atomic_release (fm->flow_prog_lock);

    for (i = 0; i < no_of_threads; i++) {
        vec_validate (fm->ip4_flow_params[i], (MAX_FLOWS_PER_FRAME - 1));
        vec_validate (fm->ip6_flow_params[i], (MAX_FLOWS_PER_FRAME - 1));
    }

    return 0;
}

VLIB_INIT_FUNCTION (pen_flow_init);
