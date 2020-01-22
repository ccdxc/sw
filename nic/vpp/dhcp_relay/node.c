/*
 *  {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
 */

#include "includes.h"
#include "pdsa_vpp_cfg.h"

// *INDENT-OFF* 
VLIB_PLUGIN_REGISTER () = {
    .description = "Pensando DHCP Relay Plugin",
};
// *INDENT-ON*

// used to fill dmac for traffic towards client
mac_addr_t vnic_mac[128];

vlib_node_registration_t pds_dhcp_relay_clfy_node;
vlib_node_registration_t pds_dhcp_relay_svr_tx_node;
vlib_node_registration_t pds_dhcp_relay_client_tx_node;

// clfy node

static u8 *
format_pds_dhcp_relay_clfy_trace (u8 * s, va_list * args)
{
    CLIB_UNUSED (vlib_main_t * vm) = va_arg (*args, vlib_main_t *);
    CLIB_UNUSED (vlib_node_t * node) = va_arg (*args, vlib_node_t *);
    dhcp_relay_clfy_trace_t *t = va_arg (*args, dhcp_relay_clfy_trace_t *);

    s = format (s, "lif  %d", t->lif);
    return s;
}

always_inline void
pds_dhcp_relay_clfy_trace_add (vlib_main_t *vm,
                               vlib_node_runtime_t *node,
                               vlib_frame_t *from_frame)
{

    PDS_PACKET_TRACE_LOOP_START {
        PDS_PACKET_TRACE_DUAL_LOOP_START {
            vlib_buffer_t *b0, *b1;
            dhcp_relay_clfy_trace_t *t0, *t1;
            u16 start_offset0, start_offset1;
            p4_rx_cpu_hdr_t *hdr0, *hdr1;

            b0 = PDS_PACKET_BUFFER(0);
            b1 = PDS_PACKET_BUFFER(1);

            start_offset0 = VPP_P4_TO_ARM_HDR_SZ +           \
                            vnet_buffer(b0)->l4_hdr_offset - \
                            vnet_buffer(b0)->l2_hdr_offset + \
                            sizeof(udp_header_t);

            start_offset1 = VPP_P4_TO_ARM_HDR_SZ +           \
                            vnet_buffer(b1)->l4_hdr_offset - \
                            vnet_buffer(b1)->l2_hdr_offset + \
                            sizeof(udp_header_t);

            hdr0 = vlib_buffer_get_current(b0);
            hdr1 = vlib_buffer_get_current(b1);

            //move pointers to start of rx header
            hdr0 = hdr0 - start_offset0;
            hdr1 = hdr1 - start_offset1;

            if (b0->flags & VLIB_BUFFER_IS_TRACED) {
                t0 = vlib_add_trace (vm, node, b0, sizeof (t0[0]));
                t0->lif = hdr0->lif;
            }
            if (b1->flags & VLIB_BUFFER_IS_TRACED) {
                t1 = vlib_add_trace (vm, node, b1, sizeof (t1[0]));
                t1->lif = hdr1->lif;
            }
        } PDS_PACKET_TRACE_DUAL_LOOP_END;
        PDS_PACKET_TRACE_SINGLE_LOOP_START {
            vlib_buffer_t *b0;
            dhcp_relay_clfy_trace_t *t0;
            p4_rx_cpu_hdr_t *hdr0;
            u16 start_offset0;

            b0 = PDS_PACKET_BUFFER(0);

            start_offset0 = VPP_P4_TO_ARM_HDR_SZ +           \
                            vnet_buffer(b0)->l4_hdr_offset - \
                            vnet_buffer(b0)->l2_hdr_offset + \
                            sizeof(udp_header_t);

            hdr0 = vlib_buffer_get_current(b0);

            //move pointers to start of rx header
            hdr0 = hdr0 - start_offset0;

            if (b0->flags & VLIB_BUFFER_IS_TRACED) {
                t0 = vlib_add_trace (vm, node, b0, sizeof (t0[0]));
                t0->lif = hdr0->lif;
            }
        } PDS_PACKET_TRACE_SINGLE_LOOP_END;
    } PDS_PACKET_TRACE_LOOP_END;
}

static uword
pds_dhcp_relay_clfy (vlib_main_t * vm,
                     vlib_node_runtime_t * node,
                     vlib_frame_t * from_frame)
{
    /*check lif in this node and give it to host or server accordingly*/
    u32 counter[DHCP_RELAY_CLFY_COUNTER_LAST] = {0};

    PDS_PACKET_LOOP_START {
        PDS_PACKET_DUAL_LOOP_START(WRITE, READ) {
            pds_dhcp_relay_clfy_x2(PDS_PACKET_BUFFER(0),
                                   PDS_PACKET_BUFFER(1),
                                   PDS_PACKET_NEXT_NODE_PTR(0),
                                   PDS_PACKET_NEXT_NODE_PTR(1),
                                   counter);
        } PDS_PACKET_DUAL_LOOP_END;
        PDS_PACKET_SINGLE_LOOP_START {
            pds_dhcp_relay_clfy_x1(PDS_PACKET_BUFFER(0),
                                   PDS_PACKET_NEXT_NODE_PTR(0),
                                   counter);
        } PDS_PACKET_SINGLE_LOOP_END;
    } PDS_PACKET_LOOP_END;

#define _(n, s) \
    vlib_node_increment_counter (vm, pds_dhcp_relay_clfy_node.index,   \
                                 DHCP_RELAY_CLFY_COUNTER_##n,          \
                                 counter[DHCP_RELAY_CLFY_COUNTER_##n]);
    foreach_dhcp_relay_clfy_counter
#undef _

    if (node->flags & VLIB_NODE_FLAG_TRACE) {
        pds_dhcp_relay_clfy_trace_add(vm, node, from_frame);
    }

    return from_frame->n_vectors;
}

static char * pds_dhcp_relay_clfy_error_strings[] = {
#define _(n,s) s,
    foreach_dhcp_relay_clfy_counter
#undef _
};

VLIB_REGISTER_NODE (pds_dhcp_relay_clfy_node) = {
    .function = pds_dhcp_relay_clfy,
    .name = "pds-dhcp-relay-classify",
    /* Takes a vector of packets. */
    .vector_size = sizeof (u32),

    .n_errors = DHCP_RELAY_CLFY_COUNTER_LAST,
    .error_strings = pds_dhcp_relay_clfy_error_strings,
    .n_next_nodes = PDS_DHCP_RELAY_CLFY_N_NEXT,
    .next_nodes = {
#define _(s,n) [PDS_DHCP_RELAY_CLFY_NEXT_##s] = n,
        foreach_dhcp_relay_clfy_next
#undef _
    },

    .format_trace = format_pds_dhcp_relay_clfy_trace,
};

// server header node

static u8 *
format_pds_dhcp_relay_svr_hdr_trace (u8 * s, va_list * args)
{
    CLIB_UNUSED (vlib_main_t * vm) = va_arg (*args, vlib_main_t *);
    CLIB_UNUSED (vlib_node_t * node) = va_arg (*args, vlib_node_t *);
    dhcp_relay_svr_hdr_trace_t *t = va_arg (*args, dhcp_relay_svr_hdr_trace_t *);

    s = format (s, "lif  %d", t->next_hop);
    return s;
}

always_inline void
pds_dhcp_relay_svr_hdr_trace_add (vlib_main_t *vm,
                                  vlib_node_runtime_t *node,
                                  vlib_frame_t *from_frame)
{
    PDS_PACKET_TRACE_LOOP_START {
        PDS_PACKET_TRACE_DUAL_LOOP_START {
            vlib_buffer_t *b0, *b1;
            dhcp_relay_svr_hdr_trace_t *t0, *t1;

            b0 = PDS_PACKET_BUFFER(0);
            b1 = PDS_PACKET_BUFFER(1);

            if (b0->flags & VLIB_BUFFER_IS_TRACED) {
                t0 = vlib_add_trace(vm, node, b0, sizeof (t0[0]));
                //TODO fill nexthop
            }
            if (b1->flags & VLIB_BUFFER_IS_TRACED) {
                t1 = vlib_add_trace(vm, node, b1, sizeof (t1[0]));
                //TODO fill nexthop
            }
        } PDS_PACKET_TRACE_DUAL_LOOP_END;
        PDS_PACKET_TRACE_SINGLE_LOOP_START {
            vlib_buffer_t *b0;
            dhcp_relay_svr_hdr_trace_t *t0;
            b0 = PDS_PACKET_BUFFER(0);

            if (b0->flags & VLIB_BUFFER_IS_TRACED) {
                t0 = vlib_add_trace(vm, node, b0, sizeof (t0[0]));
                //TODO fill nexthop
            }
        } PDS_PACKET_TRACE_SINGLE_LOOP_END;
    } PDS_PACKET_TRACE_LOOP_END;
}

always_inline int
pds_dhcp_relay_svr_hdr_buffer_advance_offset (vlib_buffer_t *b)
{
    return (VPP_ARM_TO_P4_HDR_SZ +
            vnet_buffer(b)->l3_hdr_offset - vnet_buffer (b)->l2_hdr_offset);
}

always_inline void
pds_dhcp_relay_svr_hdr_fill_next (u16 *next, u32 *counter)
{
    *next = PDS_DHCP_RELAY_SVR_HDR_NEXT_INTF_OUT;
    counter[DHCP_RELAY_SVR_HDR_COUNTER_TX]++;

    return;
}

always_inline void
pds_dhcp_relay_svr_hdr_x2 (vlib_buffer_t *p0, vlib_buffer_t *p1,
                           u16 *next0, u16 *next1, u32 *counter)
{
    vlib_buffer_advance(p0, -(pds_dhcp_relay_svr_hdr_buffer_advance_offset(p0)));
    vlib_buffer_advance(p1, -(pds_dhcp_relay_svr_hdr_buffer_advance_offset(p1)));

    // TODO need to fill all the p4 tx header fileds 

    pds_dhcp_relay_svr_hdr_fill_next(next0, counter);
    pds_dhcp_relay_svr_hdr_fill_next(next1, counter);

    return;
}
    
always_inline void
pds_dhcp_relay_svr_hdr_x1 (vlib_buffer_t *p, u16 *next, u32 *counter)
{
    vlib_buffer_advance(p, -(pds_dhcp_relay_svr_hdr_buffer_advance_offset(p)));

    // TODO need to fill all the p4 tx header fileds 

    pds_dhcp_relay_svr_hdr_fill_next(next, counter);

    return;
}
    
static uword
pds_dhcp_relay_svr_hdr (vlib_main_t * vm,
                        vlib_node_runtime_t * node,
                        vlib_frame_t * from_frame)
{
    /*add corresponding p4 hdr details for packet towards server*/
    u32 counter[DHCP_RELAY_SVR_HDR_COUNTER_LAST] = {0};

    PDS_PACKET_LOOP_START {
        PDS_PACKET_DUAL_LOOP_START(WRITE, WRITE) {
            pds_dhcp_relay_svr_hdr_x2(PDS_PACKET_BUFFER(0),
                                      PDS_PACKET_BUFFER(1),
                                      PDS_PACKET_NEXT_NODE_PTR(0),
                                      PDS_PACKET_NEXT_NODE_PTR(1),
                                      counter);
        } PDS_PACKET_DUAL_LOOP_END;
        PDS_PACKET_SINGLE_LOOP_START {
            pds_dhcp_relay_svr_hdr_x1(PDS_PACKET_BUFFER(0),
                                      PDS_PACKET_NEXT_NODE_PTR(0),
                                      counter);
        } PDS_PACKET_SINGLE_LOOP_END;
    } PDS_PACKET_LOOP_END;

#define _(n, s) \
    vlib_node_increment_counter (vm, pds_dhcp_relay_svr_tx_node.index,   \
                                 DHCP_RELAY_SVR_HDR_COUNTER_##n,          \
                                 counter[DHCP_RELAY_SVR_HDR_COUNTER_##n]);
    foreach_dhcp_relay_svr_hdr_counter
#undef _

    if (node->flags & VLIB_NODE_FLAG_TRACE) {
        pds_dhcp_relay_svr_hdr_trace_add(vm, node, from_frame);
    }

    return from_frame->n_vectors;
}

static char * pds_dhcp_relay_svr_hdr_error_strings[] = {
#define _(n,s) s,
    foreach_dhcp_relay_svr_hdr_counter
#undef _
};

VLIB_REGISTER_NODE (pds_dhcp_relay_svr_tx_node) = {
    .function = pds_dhcp_relay_svr_hdr,
    .name = "pds-dhcp-relay-svr-tx",
    /* Takes a vector of packets. */
    .vector_size = sizeof (u32),

    .n_errors = DHCP_RELAY_SVR_HDR_COUNTER_LAST,
    .error_strings = pds_dhcp_relay_svr_hdr_error_strings,
    .n_next_nodes = PDS_DHCP_RELAY_SVR_HDR_N_NEXT,
    .next_nodes = {
#define _(s,n) [PDS_DHCP_RELAY_SVR_HDR_NEXT_##s] = n,
        foreach_dhcp_relay_svr_hdr_next
#undef _
    },

    .format_trace = format_pds_dhcp_relay_svr_hdr_trace,
};

// client header node

static u8 *
format_pds_dhcp_relay_client_hdr_trace (u8 * s, va_list * args)
{
    CLIB_UNUSED (vlib_main_t * vm) = va_arg (*args, vlib_main_t *);
    CLIB_UNUSED (vlib_node_t * node) = va_arg (*args, vlib_node_t *);
    dhcp_relay_client_hdr_trace_t *t = va_arg (*args, dhcp_relay_client_hdr_trace_t *);

    s = format (s, "Client mac  %U", format_ethernet_address, t->client_mac);
    return s;
}

always_inline void
pds_dhcp_relay_client_hdr_trace_add (vlib_main_t *vm,
                                     vlib_node_runtime_t *node,
                                     vlib_frame_t *from_frame)
{
    PDS_PACKET_TRACE_LOOP_START {
        PDS_PACKET_TRACE_DUAL_LOOP_START {
            vlib_buffer_t *b0, *b1;
            dhcp_relay_client_hdr_trace_t *t0, *t1;

            b0 = PDS_PACKET_BUFFER(0);
            b1 = PDS_PACKET_BUFFER(1);

            if (b0->flags & VLIB_BUFFER_IS_TRACED) {
                t0 = vlib_add_trace(vm, node, b0, sizeof (t0[0]));
                //TODO fill client mac
            }
            if (b1->flags & VLIB_BUFFER_IS_TRACED) {
                t1 = vlib_add_trace(vm, node, b1, sizeof (t1[0]));
                //TODO fill client mac
            }
        } PDS_PACKET_TRACE_DUAL_LOOP_END;
        PDS_PACKET_TRACE_SINGLE_LOOP_START {
            vlib_buffer_t *b0;
            dhcp_relay_client_hdr_trace_t *t0;

            if (b0->flags & VLIB_BUFFER_IS_TRACED) {
                t0 = vlib_add_trace(vm, node, b0, sizeof (t0[0]));
                //TODO fill client mac
            }
        } PDS_PACKET_TRACE_SINGLE_LOOP_END;
    } PDS_PACKET_TRACE_LOOP_END;
}

always_inline int
pds_dhcp_relay_client_hdr_buffer_advance_offset (vlib_buffer_t *b)
{
    return (VPP_ARM_TO_P4_HDR_SZ +
            vnet_buffer(b)->l3_hdr_offset - vnet_buffer (b)->l2_hdr_offset);
}

always_inline void
pds_dhcp_relay_client_hdr_fill_next (u16 *next, u32 *counter)
{
    *next = PDS_DHCP_RELAY_CLIENT_HDR_NEXT_INTF_OUT;
    counter[DHCP_RELAY_CLIENT_HDR_COUNTER_TX]++;

    return;
}

always_inline void
pds_dhcp_relay_client_hdr_x2 (vlib_buffer_t *p0, vlib_buffer_t *p1,
                              u16 *next0, u16 *next1, u32 *counter)
{
    vlib_buffer_advance(p0, -(pds_dhcp_relay_client_hdr_buffer_advance_offset(p0)));
    vlib_buffer_advance(p1, -(pds_dhcp_relay_client_hdr_buffer_advance_offset(p1)));

    // TODO need to fill all the p4 tx header fileds 

    pds_dhcp_relay_client_hdr_fill_next(next0, counter);
    pds_dhcp_relay_client_hdr_fill_next(next1, counter);

    return;
}
    
always_inline void
pds_dhcp_relay_client_hdr_x1 (vlib_buffer_t *p, u16 *next, u32 *counter)
{
    vlib_buffer_advance(p, -(pds_dhcp_relay_client_hdr_buffer_advance_offset(p)));

    // TODO need to fill all the p4 tx header fileds 

    pds_dhcp_relay_client_hdr_fill_next(next, counter);

    return;
}
    
static uword
pds_dhcp_relay_client_hdr (vlib_main_t * vm,
                           vlib_node_runtime_t * node,
                           vlib_frame_t * from_frame)
{
    /*add corresponding p4 hdr details for packet towards client*/
    u32 counter[DHCP_RELAY_CLIENT_HDR_COUNTER_LAST] = {0};

    PDS_PACKET_LOOP_START {
        PDS_PACKET_DUAL_LOOP_START(WRITE, WRITE) {
            pds_dhcp_relay_client_hdr_x2(PDS_PACKET_BUFFER(0),
                                         PDS_PACKET_BUFFER(1),
                                         PDS_PACKET_NEXT_NODE_PTR(0),
                                         PDS_PACKET_NEXT_NODE_PTR(1),
                                         counter);
        } PDS_PACKET_DUAL_LOOP_END;
        PDS_PACKET_SINGLE_LOOP_START {
            pds_dhcp_relay_client_hdr_x1(PDS_PACKET_BUFFER(0),
                                         PDS_PACKET_NEXT_NODE_PTR(0),
                                         counter);
        } PDS_PACKET_SINGLE_LOOP_END;
    } PDS_PACKET_LOOP_END;

#define _(n, s) \
    vlib_node_increment_counter (vm, pds_dhcp_relay_client_tx_node.index,   \
                                 DHCP_RELAY_CLIENT_HDR_COUNTER_##n,          \
                                 counter[DHCP_RELAY_CLIENT_HDR_COUNTER_##n]);
    foreach_dhcp_relay_client_hdr_counter
#undef _

    if (node->flags & VLIB_NODE_FLAG_TRACE) {
        pds_dhcp_relay_svr_hdr_trace_add(vm, node, from_frame);
    }

    return from_frame->n_vectors;
}

static char * pds_dhcp_relay_client_hdr_error_strings[] = {
#define _(n,s) s,
    foreach_dhcp_relay_client_hdr_counter
#undef _
};

VLIB_REGISTER_NODE (pds_dhcp_relay_client_tx_node) = {
    .function = pds_dhcp_relay_client_hdr,
    .name = "pds-dhcp-relay-client-tx",
    /* Takes a vector of packets. */
    .vector_size = sizeof (u32),

    .n_errors = DHCP_RELAY_CLIENT_HDR_COUNTER_LAST,
    .error_strings = pds_dhcp_relay_client_hdr_error_strings,
    .n_next_nodes = PDS_DHCP_RELAY_CLIENT_HDR_N_NEXT,
    .next_nodes = {
#define _(s,n) [PDS_DHCP_RELAY_CLIENT_HDR_NEXT_##s] = n,
        foreach_dhcp_relay_client_hdr_next
#undef _
    },

    .format_trace = format_pds_dhcp_relay_client_hdr_trace,
};

int
pds_dhcp_options_callback (vlib_main_t * vm, vlib_buffer_t *b)
{
    // current buffer is positioned at IP header.
    // modify DHCP option 82 suboption values as per pensando req
    udp_header_t *uh;
    dhcp_header_t *dh;
    ip4_header_t *ip;
    dhcp_option_t *o, *end;
    u32 l2_vni;
    u32 l3_vni;

    ip = vlib_buffer_get_current(b);
    uh = (udp_header_t *) (ip + 1);
    dh = (dhcp_header_t *) (uh + 1);

    pds_l2vnid_get(vnet_buffer(b)->pds_dhcp_data.bd_id, &l2_vni);
    pds_l3vnid_get(vnet_buffer(b)->pds_dhcp_data.vpc_id, &l3_vni);

    o = dh->options;
    end = (void *) vlib_buffer_get_tail (b);

    while (o->option != DHCP_PACKET_OPTION_END && o < end) {
        if(o->option == 82) {
            int len = 0;

            // suboption 1
            // circuit id option will be used for BD VNID and vnic_id
            o->data[len++] = PDS_DHCP_OPT_82_CIRC_ID;
            // BD VNID takes 3 bytes and vnicid takes 4 bytes
            o->data[len++] = PDS_DHCP_OPT_82_CIRC_ID_LEN;
            clib_memcpy(&o->data[len], &l2_vni, 3);  // l2 vnid
            len += 3;
            u32 *vnic_id = (u32 *) & o->data[len];
            *vnic_id = vnet_buffer(b)->pds_dhcp_data.vnic_id;  // vnic id
            len += PDS_VNIC_ID_LEN;

            // suboption 5
            // suboption which stores subnet prefix
            o->data[len++] = PDS_DHCP_OPT_82_LINK_SEL;
            // prefix lenght is 4 bytes
            o->data[len++] = PDS_DHCP_OPT_82_LINK_SEL_LEN;
            u32 *subnet_prefix = (u32 *) & o->data[len];
            clib_memset(subnet_prefix, 0x0, PDS_IP4_PREFIX_LEN);
            //TODO o->data[11-14] = fill subnet prefix
            len += PDS_IP4_PREFIX_LEN;

            // suboption 11
            // suboption which stores subnet ip
            o->data[len++] = PDS_DHCP_OPT_82_SVR_IDENT;
            // prefix lenght is 4 bytes
            o->data[len++] = PDS_DHCP_OPT_82_SVR_IDENT_LEN;
            u32 *subnet_ip = (u32 *) & o->data[len];
            clib_memset(subnet_ip, 0x0, PDS_IP4_PREFIX_LEN);
            //TODO o->data[17-20] = fill subnet prefix
            len += PDS_IP4_PREFIX_LEN;

            // suboption 151
            // suboption which stores vss type and vrf to choose subnet from
            o->data[len++] = PDS_DHCP_OPT_82_VSS;
            // 1 byte for vss type, 3 bytes for l3 vni and 4 bytes padded 0x0
            o->data[len++] = PDS_DHCP_OPT_82_VSS_LEN;
            o->data[len++] = 0x01;    // vss type 1
            clib_memcpy(&o->data[len], &l3_vni, 3);  // l3 vnid or vrf
            len += 3;
            clib_memset(&o->data[len], 0x0, 4);
            len+= sizeof(u32);

            // suboption 152
            // suboption for vss control
            o->data[len++] = PDS_DHCP_OPT_82_VSS_CONTROL;
            // no data to store
            o->data[len++] = PDS_DHCP_OPT_82_VSS_CONTROL_LEN;

            o->length = len;    // length of all suboptions

            // suboption marking end
            o->data[len] = DHCP_PACKET_OPTION_END;     // suboption fto identify end

        }

        o = (dhcp_option_t *) (o->data + o->length);
    }
    return 0;
}

static clib_error_t *
pds_dhcp_relay_init (vlib_main_t * vm)
{
    pds_dhcp_relay_pipeline_init();

    dhcp_register_server_next_node_tx(vm, (u8 *) "pds-dhcp-relay-svr-tx");

    dhcp_register_client_next_node_tx(vm, (u8 *) "pds-dhcp-relay-client-tx");

    dhcp_register_custom_options_cb(&pds_dhcp_options_callback);

    pds_dhcp_relay_cfg_init();

    return 0;
}

VLIB_INIT_FUNCTION (pds_dhcp_relay_init) =
{
    .runs_after = VLIB_INITS("pds_infra_init", "dhcp4_proxy_init"),
};
