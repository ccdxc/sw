/*
 *  {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
 */

#include <vlib/vlib.h>
#include <vnet/ip/ip.h>
#include <vnet/plugin/plugin.h>
#include "node.h"
#include <pkt.h>
#include "nat_api.h"

// *INDENT-OFF*
VLIB_PLUGIN_REGISTER () = {
    .description = "NAT Plugin",
};
// *INDENT-ON*

vlib_node_registration_t nat_node;

always_inline void
nat_next_node_fill (u8 idx, u16 *next, u32 *counters, u16 node,
                          u32 cidx)
{
    next[idx] = node;
    counters[cidx]++;
}

always_inline void
nat_trace_add (nat_trace_t *trace)
{
    // TODO
}

always_inline void
nat_internal (vlib_buffer_t *p0, u8 *next_idx, u16 *nexts, u32 *counter,
              vlib_node_runtime_t *node, vlib_main_t *vm)
{
    nat_trace_t *trace;
    ip4_header_t *ip40;
    udp_header_t *udp0;
    ip4_address_t sip, pvt_ip, dip;
    u16 sport, pvt_port, dport;
    u16 vpc_id;
    u8 protocol;

    if (PREDICT_FALSE(node->flags & VLIB_NODE_FLAG_TRACE &&
                      p0->flags & VLIB_BUFFER_IS_TRACED)) {
        trace = vlib_add_trace (vm, node, p0, sizeof (trace[0]));
        nat_trace_add(trace);
    }
    vpc_id = vnet_buffer (p0)->sw_if_index[VLIB_TX];
    ip40 = vlib_buffer_get_current(p0);
    pvt_ip.as_u32 = ip40->src_address.as_u32;
    dip.as_u32 = ip40->dst_address.as_u32;
    protocol = ip40->protocol;
    // TODO : ICMP
    if (protocol != IP_PROTOCOL_UDP && protocol != IP_PROTOCOL_TCP) {
        goto error;
    }
#if 1
    udp0 = (udp_header_t *)(ip40 + 1);
#else
    udp0 = (udp_header_t *) (((u8 *) ip40) +
            (vnet_buffer (p0)->l4_hdr_offset -
             vnet_buffer (p0)->l3_hdr_offset));
#endif
    pvt_port = clib_net_to_host_u16(udp0->src_port);
    dport = clib_net_to_host_u16(udp0->dst_port);

    // TODO : get nat_type
    if (nat_flow_alloc(vpc_id, dip, dport, protocol, pvt_ip,
                       pvt_port, NAT_TYPE_INTERNET, &sip, &sport) !=
                       NAT_ERR_OK) {
        goto error;
    }

    nat_next_node_fill(*next_idx, nexts, counter,
                       NAT_NEXT_IP4_FLOW_PROG,
                       NAT_COUNTER_SUCCESS);
    (*next_idx)++;
    return;

error:
    nat_next_node_fill(*next_idx, nexts, counter,
                             NAT_NEXT_DROP,
                             NAT_COUNTER_FAILED);
    (*next_idx)++;
    return;
}

static uword
nat (vlib_main_t * vm,
     vlib_node_runtime_t * node,
     vlib_frame_t * from_frame)
{
    u32 counter[NAT_COUNTER_LAST] = {0};
    u8 next_idx = 0;

    PDS_PACKET_LOOP_START {

        PDS_PACKET_DUAL_LOOP_START (WRITE, READ) {
            vnet_buffer (_b[0])->sw_if_index[VLIB_TX] = vnet_buffer (_b[0])->sw_if_index[VLIB_RX];
            vnet_buffer (_b[1])->sw_if_index[VLIB_TX] = vnet_buffer (_b[1])->sw_if_index[VLIB_RX];

            nat_internal(_b[0], &next_idx, _nexts, counter, node, vm);

            nat_internal(_b[1], &next_idx, _nexts, counter, node, vm);

        } PDS_PACKET_DUAL_LOOP_END;

        PDS_PACKET_SINGLE_LOOP_START {

            vnet_buffer (_b[0])->sw_if_index[VLIB_TX] = vnet_buffer (_b[0])->sw_if_index[VLIB_RX];

            nat_internal(_b[0], &next_idx, _nexts, counter, node, vm);

        } PDS_PACKET_SINGLE_LOOP_END;

    } PDS_PACKET_LOOP_END;

#define _(n, s) \
    vlib_node_increment_counter (vm, node->node_index,           \
            NAT_COUNTER_##n,                               \
            counter[NAT_COUNTER_##n]);
    foreach_nat_counter
#undef _

    return from_frame->n_vectors;
}

static u8 *
nat_trace (u8 * s, va_list * args)
{
    return format(0, "Not Implemented");
}

static char * nat_error_strings[] = {
#define _(n,s) s,
    foreach_nat_counter
#undef _
};

VLIB_REGISTER_NODE (nat_node) = {
    .function = nat,
    .name = "pds-nat44",
    /* Takes a vector of packets. */
    .vector_size = sizeof (u32),

    .n_errors = NAT_COUNTER_LAST,
    .error_strings = nat_error_strings,

    .n_next_nodes = NAT_N_NEXT,
    .next_nodes = {
#define _(s,n) [NAT_NEXT_##s] = n,
    foreach_nat_next
#undef _
    },

    .format_trace = nat_trace,
};

static clib_error_t *
vpp_nat_init (vlib_main_t * vm)
{
    nat_init();
    return 0;
}

VLIB_INIT_FUNCTION (vpp_nat_init);
