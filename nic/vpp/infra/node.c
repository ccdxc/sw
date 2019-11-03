/*
 *  {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
 */

#include "node.h"
#include <vnet/plugin/plugin.h>
#include <node_impl.h>
#include "hw_program.h"

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

/* Don't change this function name as all other plugin
 * inits have to run after this init. So all other plugins
 * Refer to this function name 
 */
static clib_error_t *
pds_infra_init (vlib_main_t * vm) 
{
    if (0 != initialize_pds()) {
        ASSERT(0);
    }   

    return 0;
}

VLIB_INIT_FUNCTION (pds_infra_init);

