/*
 *  {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
 */

#include <vlib/vlib.h>
#include <vnet/ip/ip.h>
#include <vnet/plugin/plugin.h>
#include "node.h"
#include <pkt.h>
#include "nat_api.h"
#include "nat_utils.h"
#include "pdsa_uds_hdlr.h"

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
nat_trace_add (nat_trace_t *trace, u16 vpc_id, ip4_address_t pvt_ip,
               u16 pvt_port, ip4_address_t dip, u16 dport, u8 protocol,
               ip4_address_t public_ip, u16 public_port)
{
    trace->vpc_id = vpc_id;
    trace->pvt_ip = pvt_ip;
    trace->pvt_port = pvt_port;
    trace->dip = dip;
    trace->dport = dport;
    trace->protocol = protocol;
    trace->public_ip = public_ip;
    trace->public_port = public_port;
}

always_inline void
nat_internal (vlib_buffer_t *p0, u8 *next_idx, u16 *nexts, u32 *counter,
              vlib_node_runtime_t *node, vlib_main_t *vm)
{
    nat_trace_t *trace;
    ip4_header_t *ip40;
    udp_header_t *udp0;
    icmp46_header_t *icmp0;
    icmp_echo_header_t *echo0;
    ip4_address_t sip, pvt_ip, dip;
    u16 sport, pvt_port, dport;
    u16 vpc_id;
    u8 protocol;
    nat_hw_index_t xlate_idx, xlate_idx_rflow;
    nat_type_t nat_address_type = NAT_TYPE_INTERNET;

    vpc_id = vnet_buffer2(p0)->pds_nat_data.vpc_id;
    ip40 = vlib_buffer_get_current(p0);
    pvt_ip.as_u32 = clib_net_to_host_u32(ip40->src_address.as_u32);
    dip.as_u32 = clib_net_to_host_u32(ip40->dst_address.as_u32);
    protocol = ip40->protocol;
    if (protocol == IP_PROTOCOL_UDP || protocol == IP_PROTOCOL_TCP) {
        udp0 = (udp_header_t *) (((u8 *) ip40) +
               (vnet_buffer (p0)->l4_hdr_offset -
               vnet_buffer (p0)->l3_hdr_offset));
        pvt_port = clib_net_to_host_u16(udp0->src_port);
        dport = clib_net_to_host_u16(udp0->dst_port);
    } else if (protocol == IP_PROTOCOL_ICMP) {
        icmp0 = (icmp46_header_t *) (((u8 *) ip40) +
                (vnet_buffer (p0)->l4_hdr_offset -
                vnet_buffer (p0)->l3_hdr_offset));
        if (icmp4_is_query_message(icmp0)) {
            echo0 = (icmp_echo_header_t *)(icmp0 + 1);
            pvt_port = clib_net_to_host_u16(echo0->identifier);
            dport = 0;
        } else {
            // TODO : handle ICMP error
            goto error;
        }
    } else {
        goto error;
    }

    if (vnet_buffer(p0)->pds_flow_data.flags & VPP_CPU_FLAGS_NAPT_SVC_VALID) {
        nat_address_type = NAT_TYPE_INFRA;
    }
    if (nat_flow_alloc(vpc_id, dip, dport, protocol, pvt_ip, pvt_port,
                       nat_address_type, &sip, &sport, &xlate_idx,
                       &xlate_idx_rflow) !=
                       NAT_ERR_OK) {
        goto error;
    }
    vnet_buffer2(p0)->pds_nat_data.xlate_idx = xlate_idx;
    vnet_buffer2(p0)->pds_nat_data.xlate_idx_rflow = xlate_idx_rflow;
    vnet_buffer2(p0)->pds_nat_data.xlate_addr = sip.as_u32;
    vnet_buffer2(p0)->pds_nat_data.xlate_port = sport;
    if (PREDICT_FALSE(node->flags & VLIB_NODE_FLAG_TRACE &&
                      p0->flags & VLIB_BUFFER_IS_TRACED)) {
        trace = vlib_add_trace (vm, node, p0, sizeof (trace[0]));
        nat_trace_add(trace, vpc_id, pvt_ip, pvt_port, dip, dport, protocol,
                      sip, sport);
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
            nat_internal(_b[0], &next_idx, _nexts, counter, node, vm);

            nat_internal(_b[1], &next_idx, _nexts, counter, node, vm);

        } PDS_PACKET_DUAL_LOOP_END;

        PDS_PACKET_SINGLE_LOOP_START {
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
    return format(s, "Not Implemented");
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
    pds_nat_cfg_init();
    nat_init();
    pds_nat_dump_init();
    return 0;
}

VLIB_INIT_FUNCTION (vpp_nat_init) =
{
    .runs_after = VLIB_INITS("pds_infra_init"),
};
