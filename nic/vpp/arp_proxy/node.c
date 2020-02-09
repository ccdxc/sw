//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#include "includes.h"

// *INDENT-OFF*
VLIB_PLUGIN_REGISTER () = {
    .description = "Arp-proxy Plugin",
};
// *INDENT-ON*

vlib_node_registration_t arp_proxy_node;

always_inline void
arp_proxy_trace_fill (arp_proxy_trace_t *trace, ethernet_arp_header_t *arp,
                      mac_addr_t mac, u32 id)
{
    clib_memcpy(&trace->src, &arp->ip4_over_ethernet[0].ip4,
                sizeof(ip4_address_t));
    clib_memcpy(&trace->dst, &arp->ip4_over_ethernet[1].ip4,
                sizeof(ip4_address_t));
    clib_memcpy(trace->smac, &arp->ip4_over_ethernet[1].mac,
                ETH_ADDR_LEN);
    clib_memcpy(trace->vr_mac, mac, ETH_ADDR_LEN);
    trace->bd = id;
}

always_inline void
arp_proxy_internal (vlib_buffer_t *p0, u16 *next0, u32 *counter,
                    vlib_node_runtime_t *node, vlib_main_t *vm)
{
    ethernet_header_t *e0;
    ethernet_arp_header_t *arp = NULL;
    arp_proxy_trace_t *trace;
    void *p4_rx_meta = NULL;
    mac_addr_t vr_mac;
    u32 bd_id = 0;
    bool remote = FALSE; // knob based on config for arp-proxy
                         // set based on device cfg
    u32 dst;
    u32 offset = 0;
    u16 vnic_nh_hw_id;

    p4_rx_meta = (void*) (vlib_buffer_get_current(p0));
    bd_id = pds_ingress_bd_id_get(p4_rx_meta);
    if (PREDICT_FALSE(!pds_vnic_data_fill(p4_rx_meta, &vnic_nh_hw_id,
                                          &offset))) {
        counter[ARP_PROXY_COUNTER_VNIC_MISSING]++;
        goto error;
    }
    vnet_buffer(p0)->pds_arp_data.vnic_nh_hw_id = vnic_nh_hw_id;

    arp = (ethernet_arp_header_t*) (vlib_buffer_get_current(p0) + offset);
    dst = clib_net_to_host_u32(arp->ip4_over_ethernet[1].ip4.data_u32);
    if (PREDICT_FALSE(!pds_subnet_check(bd_id, dst))) {
        counter[ARP_PROXY_COUNTER_SUBNET_CHECK_FAIL]++;
        goto error;
    }

    if (PREDICT_TRUE(
            arp->opcode ==
            clib_host_to_net_u16 (ETHERNET_ARP_OPCODE_request))) {
        // TODO take vr mac from vnic
        // if vnic is null, drop it
        // make sure the dst addr is in the same subnet
        // remote flag - reset based on device config
        pds_dst_mac_get(p4_rx_meta, vr_mac, remote, dst);

        // Ethernet
        e0 = vlib_buffer_get_current(p0) + VPP_P4_TO_ARM_HDR_SZ;
        clib_memcpy(&e0->dst_address, &e0->src_address, ETH_ADDR_LEN);
        clib_memcpy(&e0->src_address, vr_mac, ETH_ADDR_LEN);

        // ARP Reply
        arp->opcode = clib_host_to_net_u16 (ETHERNET_ARP_OPCODE_reply);
        clib_memswap(&arp->ip4_over_ethernet[1].ip4.data_u32,
                     &arp->ip4_over_ethernet[0].ip4.data_u32,
                     sizeof(u32));
        clib_memcpy(&arp->ip4_over_ethernet[1].mac,
                    &arp->ip4_over_ethernet[0].mac, ETH_ADDR_LEN);
        clib_memcpy(&arp->ip4_over_ethernet[0].mac,
                    vr_mac, ETH_ADDR_LEN);
        *next0 = ARP_PROXY_NEXT_EXIT;
        counter[ARP_PROXY_COUNTER_REPLY_SUCCESS]++;
        if (PREDICT_FALSE(node->flags & VLIB_NODE_FLAG_TRACE &&
                          p0->flags & VLIB_BUFFER_IS_TRACED)) {
            trace = vlib_add_trace (vm, node, p0, sizeof (trace[0]));
            arp_proxy_trace_fill(trace, arp, vr_mac, bd_id);
        }
    } else {
        // TODO
        counter[ARP_PROXY_COUNTER_NOT_ARP_REQUEST]++;
        goto error;
    }
    return;

error:
    *next0 = ARP_PROXY_NEXT_DROP;
    if (PREDICT_FALSE(node->flags & VLIB_NODE_FLAG_TRACE &&
                      p0->flags & VLIB_BUFFER_IS_TRACED)) {
        trace = vlib_add_trace (vm, node, p0, sizeof (trace[0]));
        arp_proxy_trace_fill(trace, arp, vr_mac, bd_id);
    }
    return;
}

static uword
arp_proxy (vlib_main_t *vm,
           vlib_node_runtime_t *node,
           vlib_frame_t *from_frame)
{
    u32 counter[ARP_PROXY_COUNTER_LAST] = {0};

    PDS_PACKET_LOOP_START {

        PDS_PACKET_DUAL_LOOP_START (WRITE, READ) {
            vlib_buffer_t *p0, *p1;

            p0 = PDS_PACKET_BUFFER(0);
            p1 = PDS_PACKET_BUFFER(1);
            vnet_buffer(p0)->sw_if_index[VLIB_TX] = vnet_buffer(p0)->sw_if_index[VLIB_RX];
            vnet_buffer(p1)->sw_if_index[VLIB_TX] = vnet_buffer(p1)->sw_if_index[VLIB_RX];

            arp_proxy_internal(p0, PDS_PACKET_NEXT_NODE_PTR(0), counter, node, vm);
            arp_proxy_internal(p1, PDS_PACKET_NEXT_NODE_PTR(1), counter, node, vm);
            vlib_buffer_advance(p0,
                                VPP_P4_TO_ARM_HDR_SZ - VPP_ARM_TO_P4_HDR_SZ );
            vlib_buffer_advance(p1,
                                VPP_P4_TO_ARM_HDR_SZ - VPP_ARM_TO_P4_HDR_SZ );

        } PDS_PACKET_DUAL_LOOP_END;

        PDS_PACKET_SINGLE_LOOP_START {
            vlib_buffer_t *p0;

            p0 = PDS_PACKET_BUFFER(0);

            vnet_buffer(p0)->sw_if_index[VLIB_TX] = vnet_buffer(p0)->sw_if_index[VLIB_RX];

            arp_proxy_internal(p0, PDS_PACKET_NEXT_NODE_PTR(0), counter, node, vm);
            vlib_buffer_advance(p0,
                                VPP_P4_TO_ARM_HDR_SZ - VPP_ARM_TO_P4_HDR_SZ );

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
    CLIB_UNUSED (vlib_main_t * vm) = va_arg (*args, vlib_main_t *);
    CLIB_UNUSED (vlib_node_t * node) = va_arg (*args, vlib_node_t *);
    arp_proxy_trace_t *t = va_arg (*args, arp_proxy_trace_t *);

    s = format(s, "%U %U -> %U %U %d",
               format_ip4_address, &t->src,
               format_mac_address_t, &t->smac,
               format_ip4_address, &t->dst,
               format_mac_address_t, &t->vr_mac, t->bd);
    return s;
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
    pds_arp_proxy_pipeline_init();
    pds_mapping_table_init();
    return 0;
}

VLIB_INIT_FUNCTION (arp_proxy_init) =
{
    .runs_after = VLIB_INITS("pds_infra_init"),
};

vlib_node_registration_t exit_node;

always_inline void
arp_proxy_exit_internal (vlib_buffer_t *p, u16 *next, u32 *counter)
{
    u16 vnic_nh_hw_id;

    vnic_nh_hw_id = vnet_buffer(p)->pds_arp_data.vnic_nh_hw_id;
    pds_arp_proxy_add_tx_hdrs_x1(p, vnic_nh_hw_id);
    *next = ARP_PROXY_EXIT_NEXT_INTF_OUT;
    counter[ARP_PROXY_EXIT_COUNTER_BUILD_P4_HDR]++;
}

static uword
arp_proxy_exit (vlib_main_t *vm,
                vlib_node_runtime_t *node,
                vlib_frame_t *from_frame)
{
    u32 counter[ARP_PROXY_COUNTER_LAST] = {0};

    PDS_PACKET_LOOP_START {

        PDS_PACKET_DUAL_LOOP_START (WRITE, WRITE) {
            arp_proxy_exit_internal(PDS_PACKET_BUFFER(0),
                                    PDS_PACKET_NEXT_NODE_PTR(0), counter);
            arp_proxy_exit_internal(PDS_PACKET_BUFFER(1),
                                    PDS_PACKET_NEXT_NODE_PTR(1), counter);
        } PDS_PACKET_DUAL_LOOP_END;

        PDS_PACKET_SINGLE_LOOP_START {
            arp_proxy_exit_internal(PDS_PACKET_BUFFER(0),
                                    PDS_PACKET_NEXT_NODE_PTR(0), counter);
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
arp_proxy_exit_trace (u8 * s, va_list * args)
{
    s = format(s, "exit trace");
    return s;
}

static char * arp_proxy_exit_error_strings[] = {
#define _(n,s) s,
    foreach_arp_proxy_exit_counter
#undef _
};

VLIB_REGISTER_NODE (exit_node) = {
    .function = arp_proxy_exit,
    .name = "pds-arp-proxy-exit",
    /* Takes a vector of packets. */
    .vector_size = sizeof (u32),

    .n_errors = ARP_PROXY_COUNTER_LAST,
    .error_strings = arp_proxy_exit_error_strings,

    .n_next_nodes = ARP_PROXY_EXIT_N_NEXT,
    .next_nodes = {
#define _(s,n) [ARP_PROXY_EXIT_NEXT_##s] = n,
    foreach_arp_proxy_exit_next
#undef _
    },

    .format_trace = arp_proxy_exit_trace,
};
