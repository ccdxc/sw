/*
 *  {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
 */

#include <vlib/vlib.h>
#include <vnet/ip/ip.h>
#include <vnet/ethernet/arp.h>
#include <vnet/plugin/plugin.h>
#include <pkt.h>
#include "pi_impl.h"
#include "arp_proxy.h"

// *INDENT-OFF*
VLIB_PLUGIN_REGISTER () = {
    .description = "Arp-proxy Plugin",
};
// *INDENT-ON*

vlib_node_registration_t arp_proxy_node;

always_inline void
vr_mac_get (u8* mac, vnic_t vnic)
{
    memcpy(mac, vnic.vr_mac, 6);
}

always_inline word
arp_offset_get(vlib_buffer_t *p0, vnic_t vnic)
{
    if (vnic.vlan_id) {
        return (sizeof(ethernet_header_t) + sizeof(ethernet_vlan_header_t));
    } else {
        return (sizeof(ethernet_header_t));
        //return (sizeof(ethernet_header_t) + sizeof(ip4_header_t) +
        //        sizeof(udp_header_t) + 8 + sizeof(ethernet_header_t));
    }
}

always_inline void
arp_proxy_next_node_fill (u8 idx, u16 *next, u32 *counters, u16 node,
                          u32 cidx)
{
    next[idx] = node;
    counters[cidx]++;
}

always_inline void
arp_proxy_trace_add (arp_proxy_trace_t *trace, ethernet_arp_header_t *arp,
                     vnic_t *vnic, u32 vnic_id)
{
    clib_memcpy(&trace->src, &arp->ip4_over_ethernet[0].ip4,
                sizeof(ip4_address_t));
    clib_memcpy(&trace->dst, &arp->ip4_over_ethernet[1].ip4,
                sizeof(ip4_address_t));
    clib_memcpy(&trace->smac, &arp->ip4_over_ethernet[0].mac,
                6);
    clib_memcpy(trace->vr_mac, vnic->vr_mac, 6);
    trace->vnic = vnic_id;
}

always_inline void
arp_proxy_internal (vlib_buffer_t *p0, u8 *next_idx, u16 *nexts, u32 *counter,
                    vlib_node_runtime_t *node, vlib_main_t *vm)
{
    u32 vnic_id = 0;
    ethernet_header_t *e0;
    ethernet_arp_header_t *arp;
    u32 offset;
    vnic_t vnic = {};
    arp_proxy_trace_t *trace;

    //vnic_id = vnet_buffer (p0)->pds_data.vnic_id;
    if (PREDICT_FALSE(egress_vnic_read(vnic_id, &vnic)))
        goto error;
    if (PREDICT_FALSE(!(offset = arp_offset_get(p0, vnic))))
        goto error;
    arp = (ethernet_arp_header_t*) (vlib_buffer_get_current(p0) + offset);
    if (PREDICT_FALSE(node->flags & VLIB_NODE_FLAG_TRACE &&
                      p0->flags & VLIB_BUFFER_IS_TRACED)) {
        trace = vlib_add_trace (vm, node, p0, sizeof (trace[0]));
        arp_proxy_trace_add(trace, arp, &vnic, vnic_id);
    }
    if (PREDICT_TRUE(
            arp->opcode ==
            clib_host_to_net_u16 (ETHERNET_ARP_OPCODE_request))) {
        u8 vr_mac[6];
        vr_mac_get(vr_mac, vnic);
        // Ethernet
        e0 = vlib_buffer_get_current(p0);
        clib_memcpy(&e0->dst_address, &e0->src_address, 6);
        clib_memcpy(&e0->src_address, vr_mac, 6);
        // ARP Reply
        arp->opcode = clib_host_to_net_u16 (ETHERNET_ARP_OPCODE_reply);
        clib_memswap(&arp->ip4_over_ethernet[1].ip4.data_u32,
                     &arp->ip4_over_ethernet[0].ip4.data_u32,
                     sizeof(u32));
        clib_memcpy(&arp->ip4_over_ethernet[1].mac,
                    &arp->ip4_over_ethernet[0].mac, 6);
        clib_memcpy(&arp->ip4_over_ethernet[0].mac,
                    vr_mac, 6);
        arp_proxy_next_node_fill(*next_idx, nexts, counter,
                                 ARP_PROXY_NEXT_INTF_OUT,
                                 ARP_PROXY_COUNTER_REPLY_SUCCESS);
        (*next_idx)++;

    } else {
        // TODO
        goto error;
    }
    return;

error:
    arp_proxy_next_node_fill(*next_idx, nexts, counter,
                             ARP_PROXY_NEXT_DROP,
                             ARP_PROXY_COUNTER_REPLY_FAILED);
    (*next_idx)++;
    return;
}

static uword
arp_proxy (vlib_main_t * vm,
           vlib_node_runtime_t * node,
           vlib_frame_t * from_frame)
{
    u32 counter[ARP_PROXY_COUNTER_LAST] = {0};
    u8 next_idx = 0;

    PDS_PACKET_LOOP_START {

        PDS_PACKET_DUAL_LOOP_START (WRITE, READ) {
            vnet_buffer (_b[0])->sw_if_index[VLIB_TX] = vnet_buffer (_b[0])->sw_if_index[VLIB_RX];
            vnet_buffer (_b[1])->sw_if_index[VLIB_TX] = vnet_buffer (_b[1])->sw_if_index[VLIB_RX];

            arp_proxy_internal(_b[0], &next_idx, _nexts, counter, node, vm);
            // TODO : advance based on the new header that will be added
            //vlib_buffer_advance(_b[0], -16);

            arp_proxy_internal(_b[1], &next_idx, _nexts, counter, node, vm);
            //vlib_buffer_advance(_b[1], -16);

        } PDS_PACKET_DUAL_LOOP_END;

        PDS_PACKET_SINGLE_LOOP_START {

            vnet_buffer (_b[0])->sw_if_index[VLIB_TX] = vnet_buffer (_b[0])->sw_if_index[VLIB_RX];

            arp_proxy_internal(_b[0], &next_idx, _nexts, counter, node, vm);
            //vlib_buffer_advance(_b[0], -16);

        } PDS_PACKET_SINGLE_LOOP_END;

    } PDS_PACKET_LOOP_END;

#define _(n, s) \
    vlib_node_increment_counter (vm, node->node_index,           \
            ARP_PROXY_COUNTER_##n,                               \
            counter[ARP_PROXY_COUNTER_##n]);
    foreach_arp_proxy_counter
#undef _

    return from_frame->n_vectors;
}

static u8 *
arp_proxy_trace (u8 * s, va_list * args)
{
    return format(0, "Not Implemented");
}

static char * arp_proxy_error_strings[] = {
#define _(n,s) s,
    foreach_arp_proxy_counter
#undef _
};

VLIB_REGISTER_NODE (arp_proxy_node) = {
    .function = arp_proxy,
    .name = "pds-arp-proxy",
    /* Takes a vector of packets. */
    .vector_size = sizeof (u32),

    .n_errors = ARP_PROXY_COUNTER_LAST,
    .error_strings = arp_proxy_error_strings,

    .n_next_nodes = ARP_PROXY_N_NEXT,
    .next_nodes = {
#define _(s,n) [ARP_PROXY_NEXT_##s] = n,
    foreach_arp_proxy_next
#undef _
    },

    .format_trace = arp_proxy_trace,
};

static clib_error_t *
arp_proxy_init (vlib_main_t * vm)
{
    return 0;
}

VLIB_INIT_FUNCTION (arp_proxy_init);
