/*
 *  {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
 */

#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include "includes.h"
#include "pdsa_vpp_cfg.h"
#include "vnic.h"

// *INDENT-OFF*
VLIB_PLUGIN_REGISTER () = {
    .description = "Pensando DHCP Relay Plugin",
};
// *INDENT-ON*

static CLIB_UNUSED(vlib_node_registration_t pds_dhcp_relay_uplink_clfy_node);
static CLIB_UNUSED(vlib_node_registration_t pds_dhcp_relay_host_clfy_node);
static vlib_node_registration_t pds_dhcp_relay_client_tx_node;
static vlib_node_registration_t dhcp_relay_to_server_node;
static vlib_node_registration_t dhcp_relay_to_client_node;

dhcp_relay_main_t dhcp_relay_main;

always_inline dhcp_relay_policy_t *
pds_dhcp_relay_policy_find (u16 subnet_id)
{
    dhcp_relay_main_t *dm = &dhcp_relay_main;
    dhcp_relay_policy_t *policy;
    u16 *pool_idx;

    pool_idx = vec_elt_at_index(dm->policy_pool_idx, subnet_id);
    if (0xffff == *pool_idx) {
        return NULL;
    }

    policy = pool_elt_at_index(dm->policy_pool, *pool_idx);
    return policy;
}

always_inline dhcp_relay_server_t *
pds_dhcp_relay_server_find (u16 subnet_id,
                            u32 relay_addr,
                            u32 server_addr,
                            u16 server_vpc,
                            u8 local)
{
    dhcp_relay_server_t *ret;
    dhcp_relay_main_t *dm = &dhcp_relay_main;
    dhcp_relay_policy_t *policy;
    u16 *pool_idx;
    u16 *server_idx;

    pool_idx = vec_elt_at_index(dm->policy_pool_idx, subnet_id);
    if (0xffff == *pool_idx) {
        return NULL;
    }

    policy = pool_elt_at_index(dm->policy_pool, *pool_idx);
    if (policy->local_server != local) {
        return NULL;
    }
    pool_foreach(server_idx, policy->servers, (({
        ret = pool_elt_at_index(dm->server_pool, *server_idx);
        if ((ret->server_addr.ip4.as_u32 == server_addr) &&
            (ret->relay_addr.ip4.as_u32 == relay_addr)) {
            return ret;
        }
    })));

    return NULL;
}
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

always_inline int
pds_dhcp_relay_clfy_buffer_advance_offset (vlib_buffer_t *b)
{
    return (VPP_P4_TO_ARM_HDR_SZ +
            vnet_buffer(b)->l4_hdr_offset - vnet_buffer (b)->l2_hdr_offset +
            sizeof(udp_header_t));
}

always_inline void
pds_dhcp_relay_clfy_fill_next (vlib_buffer_t *b, u16 *next,
                               u32 *counter, u8 host)
{
    if (host) {
        // packet coming from host, send towards DHCP server
        u16 vnic_id, subnet_hw_id;
        dhcp_relay_policy_t *policy = NULL;
        vnic_id = vnet_buffer(b)->pds_dhcp_data.vnic_id;
        if (0 != pds_vnic_subnet_get(vnic_id, &subnet_hw_id)) {
            counter[DHCP_RELAY_CLFY_COUNTER_NO_VNIC]++;
            *next = PDS_DHCP_RELAY_CLFY_NEXT_DROP;
            return;
        }
        policy = pds_dhcp_relay_policy_find(subnet_hw_id);
        if (PREDICT_FALSE(!policy)) {
            counter[DHCP_RELAY_CLFY_COUNTER_NO_DHCP]++;
            *next = PDS_DHCP_RELAY_CLFY_NEXT_DROP;
            return;
        }
        if (policy->local_server) {
            *next = PDS_DHCP_RELAY_CLFY_NEXT_TO_PROXY_SERVER;
            counter[DHCP_RELAY_CLFY_COUNTER_TO_PROXY_SERVER]++;
        } else {
            *next = PDS_DHCP_RELAY_CLFY_NEXT_TO_RELAY_SERVER;
            counter[DHCP_RELAY_CLFY_COUNTER_TO_RELAY_SERVER]++;
        }
    } else {
        // packet coming from uplink, send to relay host
        // from proxy server we don't expect packets to reach here
        // as it follows VPP's routing, not P4
        *next = PDS_DHCP_RELAY_CLFY_NEXT_TO_RELAY_CLIENT;
        counter[DHCP_RELAY_CLFY_COUNTER_TO_RELAY_CLIENT]++;
    }

    return;
}

always_inline void
pds_dhcp_relay_clfy_x2 (vlib_buffer_t *p0, vlib_buffer_t *p1,
                        u16 *next0, u16 *next1, u32 *counter, u8 host)
{
    p4_rx_cpu_hdr_t *hdr0 = vlib_buffer_get_current(p0);
    p4_rx_cpu_hdr_t *hdr1 = vlib_buffer_get_current(p1);

    vnet_buffer (p0)->l2_hdr_offset = hdr0->l2_offset;
    vnet_buffer (p0)->l3_hdr_offset =
         hdr0->l3_inner_offset ? hdr0->l3_inner_offset : hdr0->l3_offset;
    vnet_buffer (p0)->l4_hdr_offset =
        hdr0->l4_inner_offset ? hdr0->l4_inner_offset : hdr0->l4_offset;

    vnet_buffer (p1)->l2_hdr_offset = hdr1->l2_offset;
    vnet_buffer (p1)->l3_hdr_offset =
         hdr1->l3_inner_offset ? hdr1->l3_inner_offset : hdr1->l3_offset;
    vnet_buffer (p1)->l4_hdr_offset =
        hdr1->l4_inner_offset ? hdr1->l4_inner_offset : hdr1->l4_offset;

    pds_dhcp_relay_fill_data(p0, hdr0);
    pds_dhcp_relay_fill_data(p1, hdr1);

    vlib_buffer_advance(p0, pds_dhcp_relay_clfy_buffer_advance_offset(p0));
    vlib_buffer_advance(p1, pds_dhcp_relay_clfy_buffer_advance_offset(p1));

    pds_dhcp_relay_clfy_fill_next(p0, next0, counter, host);
    pds_dhcp_relay_clfy_fill_next(p1, next1, counter, host);

    return;
}

always_inline void
pds_dhcp_relay_clfy_x1 (vlib_buffer_t *p, u16 *next, u32 *counter, u8 host)
{
    p4_rx_cpu_hdr_t *hdr = vlib_buffer_get_current(p);

    vnet_buffer (p)->l2_hdr_offset = hdr->l2_offset;
    vnet_buffer (p)->l3_hdr_offset =
         hdr->l3_inner_offset ? hdr->l3_inner_offset : hdr->l3_offset;
    vnet_buffer (p)->l4_hdr_offset =
        hdr->l4_inner_offset ? hdr->l4_inner_offset : hdr->l4_offset;

    pds_dhcp_relay_fill_data(p, hdr);

    vlib_buffer_advance(p, pds_dhcp_relay_clfy_buffer_advance_offset(p));

    pds_dhcp_relay_clfy_fill_next(p, next, counter, host);

    return;
}

always_inline uword
pds_dhcp_relay_clfy (vlib_main_t * vm,
                     vlib_node_runtime_t * node,
                     vlib_frame_t * from_frame,
                     u8 host)
{
    /*check lif in this node and give it to host or server accordingly*/
    u32 counter[DHCP_RELAY_CLFY_COUNTER_LAST] = {0};

    PDS_PACKET_LOOP_START {
        PDS_PACKET_DUAL_LOOP_START(WRITE, READ) {
            pds_dhcp_relay_clfy_x2(PDS_PACKET_BUFFER(0),
                                   PDS_PACKET_BUFFER(1),
                                   PDS_PACKET_NEXT_NODE_PTR(0),
                                   PDS_PACKET_NEXT_NODE_PTR(1),
                                   counter, host);
        } PDS_PACKET_DUAL_LOOP_END;
        PDS_PACKET_SINGLE_LOOP_START {
            pds_dhcp_relay_clfy_x1(PDS_PACKET_BUFFER(0),
                                   PDS_PACKET_NEXT_NODE_PTR(0),
                                   counter, host);
        } PDS_PACKET_SINGLE_LOOP_END;
    } PDS_PACKET_LOOP_END;

#define _(n, s) \
    vlib_node_increment_counter (vm, node->node_index,                  \
                                 DHCP_RELAY_CLFY_COUNTER_##n,           \
                                 counter[DHCP_RELAY_CLFY_COUNTER_##n]);
    foreach_dhcp_relay_clfy_counter
#undef _

    if (node->flags & VLIB_NODE_FLAG_TRACE) {
        pds_dhcp_relay_clfy_trace_add(vm, node, from_frame);
    }

    return from_frame->n_vectors;
}

static uword
pds_dhcp_relay_uplink_clfy (vlib_main_t *vm,
                            vlib_node_runtime_t *node,
                            vlib_frame_t *from_frame)
{
    return pds_dhcp_relay_clfy(vm, node, from_frame, 0);
}

static uword
pds_dhcp_relay_host_clfy (vlib_main_t *vm,
                          vlib_node_runtime_t *node,
                          vlib_frame_t *from_frame)
{
    return pds_dhcp_relay_clfy(vm, node, from_frame, 1);
}

static char * pds_dhcp_relay_clfy_error_strings[] = {
#define _(n,s) s,
    foreach_dhcp_relay_clfy_counter
#undef _
};

VLIB_REGISTER_NODE(pds_dhcp_relay_host_clfy_node, static) = {
    .function = pds_dhcp_relay_host_clfy,
    .name = "pds-dhcp-relay-host-classify",
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

VLIB_REGISTER_NODE(pds_dhcp_relay_uplink_clfy_node, static) = {
    .function = pds_dhcp_relay_uplink_clfy,
    .name = "pds-dhcp-relay-uplink-classify",
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

// client header node
static u8 *
format_pds_dhcp_relay_client_tx_trace (u8 * s, va_list * args)
{
    CLIB_UNUSED (vlib_main_t * vm) = va_arg (*args, vlib_main_t *);
    CLIB_UNUSED (vlib_node_t * node) = va_arg (*args, vlib_node_t *);
    dhcp_relay_client_tx_trace_t *t = va_arg (*args, dhcp_relay_client_tx_trace_t *);

    s = format (s, "Client mac  %U", format_ethernet_address, t->client_mac);
    return s;
}

always_inline void
pds_dhcp_relay_client_tx_trace_add (vlib_main_t *vm,
                                    vlib_node_runtime_t *node,
                                    vlib_frame_t *from_frame)
{
    PDS_PACKET_TRACE_LOOP_START {
        PDS_PACKET_TRACE_DUAL_LOOP_START {
            vlib_buffer_t *b0, *b1;
            dhcp_relay_client_tx_trace_t *t0, *t1;

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
            dhcp_relay_client_tx_trace_t *t0;

            b0 = PDS_PACKET_BUFFER(0);

            if (b0->flags & VLIB_BUFFER_IS_TRACED) {
                t0 = vlib_add_trace(vm, node, b0, sizeof (t0[0]));
                //TODO fill client mac
            }
        } PDS_PACKET_TRACE_SINGLE_LOOP_END;
    } PDS_PACKET_TRACE_LOOP_END;
}

always_inline int
pds_dhcp_relay_client_tx_buffer_advance_offset (vlib_buffer_t *b)
{
    return (VPP_ARM_TO_P4_HDR_SZ);
}

always_inline void
pds_dhcp_relay_client_tx_fill_next (u16 *next, u32 *counter, bool error)
{
    if (!error) {
        *next = PDS_DHCP_RELAY_CLIENT_TX_NEXT_INTF_OUT;
        counter[DHCP_RELAY_CLIENT_TX_COUNTER_TX]++;
    } else {
        *next = PDS_DHCP_RELAY_CLIENT_TX_NEXT_DROP;
    }

    return;
}

always_inline void
pds_dhcp_relay_client_tx_x2 (vlib_buffer_t *p0, vlib_buffer_t *p1,
                             u16 *next0, u16 *next1, u32 *counter)
{
    bool error0, error1;
    vlib_buffer_advance(p0, -(pds_dhcp_relay_client_tx_buffer_advance_offset(p0)));
    vlib_buffer_advance(p1, -(pds_dhcp_relay_client_tx_buffer_advance_offset(p1)));

    // TODO need to fill all the p4 tx header fileds

    pds_dhcp_relay_client_fill_tx_hdr_x2(p0, p1, &error0, &error1);

    pds_dhcp_relay_client_tx_fill_next(next0, counter, error0);
    pds_dhcp_relay_client_tx_fill_next(next1, counter, error1);

    return;
}

always_inline void
pds_dhcp_relay_client_tx_x1 (vlib_buffer_t *p, u16 *next, u32 *counter)
{
    bool error;
    vlib_buffer_advance(p, -(pds_dhcp_relay_client_tx_buffer_advance_offset(p)));

    // TODO need to fill all the p4 tx header fileds
    pds_dhcp_relay_client_fill_tx_hdr_x1(p, &error);

    pds_dhcp_relay_client_tx_fill_next(next, counter, error);

    return;
}

static uword
pds_dhcp_relay_client_tx (vlib_main_t * vm,
                           vlib_node_runtime_t * node,
                           vlib_frame_t * from_frame)
{
    /*add corresponding p4 hdr details for packet towards client*/
    u32 counter[DHCP_RELAY_CLIENT_TX_COUNTER_LAST] = {0};

    PDS_PACKET_LOOP_START {
        PDS_PACKET_DUAL_LOOP_START(WRITE, WRITE) {
            pds_dhcp_relay_client_tx_x2(PDS_PACKET_BUFFER(0),
                                         PDS_PACKET_BUFFER(1),
                                         PDS_PACKET_NEXT_NODE_PTR(0),
                                         PDS_PACKET_NEXT_NODE_PTR(1),
                                         counter);
        } PDS_PACKET_DUAL_LOOP_END;
        PDS_PACKET_SINGLE_LOOP_START {
            pds_dhcp_relay_client_tx_x1(PDS_PACKET_BUFFER(0),
                                         PDS_PACKET_NEXT_NODE_PTR(0),
                                         counter);
        } PDS_PACKET_SINGLE_LOOP_END;
    } PDS_PACKET_LOOP_END;

#define _(n, s) \
    vlib_node_increment_counter (vm, pds_dhcp_relay_client_tx_node.index,   \
                                 DHCP_RELAY_CLIENT_TX_COUNTER_##n,          \
                                 counter[DHCP_RELAY_CLIENT_TX_COUNTER_##n]);
    foreach_dhcp_relay_client_tx_counter
#undef _

    if (node->flags & VLIB_NODE_FLAG_TRACE) {
        pds_dhcp_relay_client_tx_trace_add(vm, node, from_frame);
    }

    return from_frame->n_vectors;
}

static char * pds_dhcp_relay_client_tx_error_strings[] = {
#define _(n,s) s,
    foreach_dhcp_relay_client_tx_counter
#undef _
};

VLIB_REGISTER_NODE (pds_dhcp_relay_client_tx_node, static) = {
    .function = pds_dhcp_relay_client_tx,
    .name = "pds-dhcp-relay-client-tx",
    /* Takes a vector of packets. */
    .vector_size = sizeof (u32),

    .n_errors = DHCP_RELAY_CLIENT_TX_COUNTER_LAST,
    .error_strings = pds_dhcp_relay_client_tx_error_strings,
    .n_next_nodes = PDS_DHCP_RELAY_CLIENT_TX_N_NEXT,
    .next_nodes = {
#define _(s,n) [PDS_DHCP_RELAY_CLIENT_TX_NEXT_##s] = n,
        foreach_dhcp_relay_client_tx_next
#undef _
    },

    .format_trace = format_pds_dhcp_relay_client_tx_trace,
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
        if (o->option == 82) {
            int len = 0;

            // suboption 1
            // circuit id option will be used for BD VNID and vnic_id
            o->data[len++] = PDS_DHCP_OPT_82_CIRC_ID;
            // BD VNID takes 3 bytes and vnicid takes 2 bytes
            o->data[len++] = PDS_DHCP_OPT_82_CIRC_ID_LEN;
            clib_memcpy(&o->data[len], &l2_vni, 3);  // l2 vnid
            pds_host_to_net_u24((u8 *) &o->data[len]);
            len += 3;
            u16 *vnic_id = (u16 *) & o->data[len];
            *vnic_id = vnet_buffer(b)->pds_dhcp_data.vnic_id;  // vnic id
            *vnic_id = clib_host_to_net_u16(*vnic_id);
            len += PDS_VNIC_ID_LEN;

            // suboption 5
            // suboption which stores subnet prefix
            o->data[len++] = PDS_DHCP_OPT_82_LINK_SEL;
            // prefix lenght is 4 bytes
            o->data[len++] = PDS_DHCP_OPT_82_LINK_SEL_LEN;
            u32 *subnet_prefix = (u32 *) & o->data[len];
            clib_memset(subnet_prefix, 0x0, PDS_IP4_PREFIX_LEN);
            len += PDS_IP4_PREFIX_LEN;

            // suboption 11
            // suboption which stores subnet ip
            o->data[len++] = PDS_DHCP_OPT_82_SVR_IDENT;
            // prefix lenght is 4 bytes
            o->data[len++] = PDS_DHCP_OPT_82_SVR_IDENT_LEN;
            u32 *subnet_ip = (u32 *) & o->data[len];
            clib_memset(subnet_ip, 0x0, PDS_IP4_PREFIX_LEN);
            len += PDS_IP4_PREFIX_LEN;

            u32 vnic = vnet_buffer(b)->pds_dhcp_data.vnic_id;
            pds_dhcp_relay_fill_subnet_info(vnic, subnet_prefix, subnet_ip);

            // suboption 151
            // suboption which stores vss type and vrf to choose subnet from
            o->data[len++] = PDS_DHCP_OPT_82_VSS;
            // 1 byte for vss type, 3 bytes for l3 vni and 4 bytes padded 0x0
            o->data[len++] = PDS_DHCP_OPT_82_VSS_LEN;
            o->data[len++] = 0x01;    // vss type 1
            clib_memcpy(&o->data[len], &l3_vni, 3);  // l3 vnid or vrf
            pds_host_to_net_u24((u8 *) &o->data[len]);
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

int
pds_dhcp_client_tx_callback (vlib_main_t * vm, vlib_buffer_t *b)
{
    udp_header_t *u0;
    ip4_header_t *ip0;
    u32 vrip=0, old0, new0;
    ip_csum_t sum0;
    u16 vnic_id, subnet_hw_id;
    u8 *vrmac;

    ethernet_header_t *eth0;

    ip0 = vlib_buffer_get_current(b);
    u0 = (udp_header_t *)(ip0 + 1);

    vlib_buffer_advance(b, - sizeof (ethernet_header_t));
    eth0 = vlib_buffer_get_current(b);

    // extract vnic and subnet info
    vnic_id = vnet_buffer(b)->pds_dhcp_data.vnic_id;
    if (0 != pds_vnic_subnet_get(vnic_id, &subnet_hw_id)) {
        return -1;
    }
    if (0 != pds_impl_db_vr_ip_mac_get(subnet_hw_id, &vrip, &vrmac)) {
        return -2;
    }

    // set dest port
    u0->checksum = 0;
    u0->dst_port = clib_net_to_host_u16(UDP_DST_PORT_dhcp_to_client);

    // ip header filling
    sum0 = ip0->checksum;
    old0 = ip0->dst_address.as_u32;
    new0 = 0xFFFFFFFF;
    ip0->dst_address.as_u32 = new0;
    sum0 = ip_csum_update(sum0, old0, new0, ip4_header_t /* structure */ ,
                          dst_address /* offset of changed member */ );
    ip0->checksum = ip_csum_fold (sum0);

    sum0 = ip0->checksum;
    old0 = ip0->src_address.as_u32;
    new0 = vrip;
    new0 = clib_host_to_net_u32(new0);
    ip0->src_address.as_u32 = new0;
    sum0 = ip_csum_update(sum0, old0, new0, ip4_header_t /* structure */ ,
                          src_address /* offset of changed member */ );
    ip0->checksum = ip_csum_fold(sum0);

    // eth header filling
    eth0->type =  clib_net_to_host_u16(ETHERNET_TYPE_IP4);
    clib_memcpy(eth0->src_address, vrmac, ETH_ADDR_LEN);
    clib_memset(eth0->dst_address, 0xff, ETH_ADDR_LEN);
    return 0;
}

int
pds_dhcp_extract_circ_id_callback (vlib_main_t * vm, vlib_buffer_t *b,
                                   dhcp_option_t *o)
{
    u16 vnic_id;

    if (o->option == PDS_DHCP_OPT_82_CIRC_ID) {
        if (o->length == PDS_DHCP_OPT_82_CIRC_ID_LEN) {
            clib_memcpy(&vnic_id, &o->data[3], PDS_VNIC_ID_LEN);
            vnet_buffer(b)->pds_dhcp_data.vnic_id =
                clib_net_to_host_u16(vnic_id);
        } else {
            return 1;
        }
    }

    return 0;
}

always_inline void
pds_dhcp_relay_to_server_x1 (vlib_main_t *vm,
                             vlib_node_runtime_t *node,
                             vlib_buffer_t *b0,
                             u16 *next0, u32 *counter)
{
    udp_header_t *u0;
    dhcp_header_t *h0;
    ip4_header_t *ip0;
    u32 old0, new0;
    ip_csum_t sum0;
    dhcp_relay_policy_t *policy;
    dhcp_relay_server_t *server = NULL;
    dhcp_option_t *o, *end;
    u32 len = 0;
    u8 is_discover = 0;
    pds_impl_db_vnic_entry_t *vnic_info = NULL;
    u16 vnic_id;
    dhcp_relay_main_t *dm = &dhcp_relay_main;
    u16 *svr_idx = NULL;
    u32 server_ip = 0;

    h0 = vlib_buffer_get_current(b0);

    vlib_buffer_advance(b0, -(sizeof (*u0)));
    u0 = vlib_buffer_get_current(b0);

    vnic_id = vnet_buffer(b0)->pds_dhcp_data.vnic_id;
    vnic_info = pds_impl_db_vnic_get(vnic_id);
    if(vnic_info == NULL) {
        counter[DHCP_RELAY_TO_SVR_COUNTER_NO_VNIC]++;
        *next0 = PDS_DHCP_RELAY_TO_SVR_NEXT_DROP;
        goto trace;
    }
    policy = pds_dhcp_relay_policy_find(vnic_info->subnet_hw_id);
    if (PREDICT_FALSE (NULL == policy)) {
        counter[DHCP_RELAY_TO_SVR_COUNTER_NO_DHCP]++;
        *next0 = PDS_DHCP_RELAY_TO_SVR_NEXT_DROP;
        goto trace;
    }

    if (!vlib_buffer_chain_linearize(vm, b0)) {
        counter[DHCP_RELAY_TO_SVR_COUNTER_LINEARIZE_FAILED]++;
        *next0 = PDS_DHCP_RELAY_TO_SVR_NEXT_DROP;
        goto trace;
    }
    o = h0->options;
    end = (void *) vlib_buffer_get_tail(b0);

    // TLVs are not performance-friendly
    while (o->option != DHCP_PACKET_OPTION_END && o < end) {
        if (DHCP_PACKET_OPTION_MSG_TYPE == o->option) {
            if (DHCP_PACKET_DISCOVER == o->data[0]) {
                is_discover = 1;
            }
        } else if (54 == o->option) {
            // get dhcp server IP
            server_ip = o->data_as_u32[0];
        }
        o = (dhcp_option_t *) (o->data + o->length);
    }
    // if discover then pick first server and do packet manupulation.
    // packet has to be replicated to all servers.
    // if not discover msg, then find actual dhcp server to which packet
    // has to be unicast and forward
    if (is_discover) {
        server = pool_elt_at_index(dm->server_pool, policy->servers[0]);
    } else {
        pool_foreach(svr_idx, policy->servers, (({
            server = pool_elt_at_index(dm->server_pool, *svr_idx);
            if (server->server_addr.ip4.as_u32 == server_ip) {
                goto found_server;
            }
        })));
        // server not found
        counter[DHCP_RELAY_TO_SVR_COUNTER_NO_DHCP]++;
        *next0 = PDS_DHCP_RELAY_TO_SVR_NEXT_DROP;
        goto trace;
    }

found_server:
    vlib_buffer_advance(b0, -(sizeof(*ip0)));
    ip0 = vlib_buffer_get_current(b0);

    // disable UDP checksum
    u0->checksum = 0;
    sum0 = ip0->checksum;
    old0 = ip0->dst_address.as_u32;
    new0 = server->server_addr.ip4.as_u32;
    ip0->dst_address.as_u32 = server->server_addr.ip4.as_u32;
    sum0 = ip_csum_update(sum0, old0, new0,
                          ip4_header_t, dst_address);
    ip0->checksum = ip_csum_fold(sum0);

    sum0 = ip0->checksum;
    old0 = ip0->src_address.as_u32;
    new0 = server->relay_addr.ip4.as_u32;
    ip0->src_address.as_u32 = new0;
    sum0 = ip_csum_update(sum0, old0, new0,
                          ip4_header_t, src_address);
    ip0->checksum = ip_csum_fold(sum0);

    h0->gateway_ip_address = server->relay_addr.ip4;


    if (o->option == DHCP_PACKET_OPTION_END && o <= end) {
        u16 old_l0, new_l0;

        o->option = 82;  /* option 82 */
        pds_dhcp_options_callback(vm, b0);

        len = o->length + 3;
        b0->current_length += len;

        // fix IP header length and checksum
        old_l0 = ip0->length;
        new_l0 = clib_net_to_host_u16 (old_l0);
        new_l0 += len;
        new_l0 = clib_host_to_net_u16 (new_l0);
        ip0->length = new_l0;
        sum0 = ip0->checksum;
        sum0 = ip_csum_update(sum0, old_l0, new_l0, ip4_header_t,
                           length);
        ip0->checksum = ip_csum_fold(sum0);

        // fix UDP length
        new_l0 = clib_net_to_host_u16(u0->length);
        new_l0 += len;
        u0->length = clib_host_to_net_u16(new_l0);
    } else {
        counter[DHCP_RELAY_TO_SVR_COUNTER_PACKET_TOO_BIG]++;
        *next0 = PDS_DHCP_RELAY_TO_SVR_NEXT_DROP;
        goto trace;
    }

    counter[DHCP_RELAY_TO_SVR_COUNTER_TX]++;
    *next0 = PDS_DHCP_RELAY_TO_SVR_NEXT_LINUX_INJECT;

    // if we have multiple servers configured and this is the
    // client's discover message, then send copies to each of
    // those servers
    if (is_discover && vec_len(policy->servers) > 1) {
        u32 ii;
        for (ii = 1; ii < vec_len(policy->servers); ii++) {
            vlib_buffer_t *c0;
            u32 ci0;
            u16 next = PDS_DHCP_RELAY_TO_SVR_NEXT_LINUX_INJECT;
            dhcp_relay_server_t *server0;

            c0 = vlib_buffer_copy(vm, b0);
            VLIB_BUFFER_TRACE_TRAJECTORY_INIT(c0);
            ci0 = vlib_get_buffer_index(vm, c0);
            server0 = pool_elt_at_index(dm->server_pool, policy->servers[ii]);

            ip0 = vlib_buffer_get_current(c0);
            h0 = (dhcp_header_t *) (((u8 *) (ip0 + 1)) + sizeof(udp_header_t));
            h0->gateway_ip_address = server0->relay_addr.ip4;;

            sum0 = ip0->checksum;
            old0 = ip0->dst_address.as_u32;
            new0 = server0->server_addr.ip4.as_u32;
            ip0->dst_address.as_u32 = server0->server_addr.ip4.as_u32;
            sum0 = ip_csum_update(sum0, old0, new0,
                                  ip4_header_t,
                                  dst_address);
            ip0->checksum = ip_csum_fold(sum0);

            sum0 = ip0->checksum;
            old0 = ip0->src_address.as_u32;
            new0 = server0->relay_addr.ip4.as_u32;
            ip0->src_address.as_u32 = new0;
            sum0 = ip_csum_update(sum0, old0, new0,
                                  ip4_header_t, src_address);
            ip0->checksum = ip_csum_fold(sum0);

            vlib_buffer_enqueue_to_next(vm, node, &ci0,
                                        &next, 1);

            if (PREDICT_FALSE (b0->flags & VLIB_BUFFER_IS_TRACED)) {
                // trace packet
                dhcp_relay_to_svr_client_trace_t *t0 =
                        vlib_add_trace(vm, node, b0, sizeof(t0[0]));
                ip46_address_set_ip4(&t0->relay_addr,
                                     &server0->relay_addr.ip4);
                ip46_address_set_ip4(&t0->svr_addr,
                                     &server0->server_addr.ip4);
                t0->server_found = 1;
            }
        }
    }

trace:
    if (PREDICT_FALSE (b0->flags & VLIB_BUFFER_IS_TRACED)) {
        // trace packet
        dhcp_relay_to_svr_client_trace_t *t0 =
                vlib_add_trace(vm, node, b0, sizeof(t0[0]));
        if (server) {
            ip46_address_set_ip4(&t0->relay_addr,
                                 &server->relay_addr.ip4);
            ip46_address_set_ip4(&t0->svr_addr,
                                 &server->server_addr.ip4);
            t0->server_found = 1;
        } else {
            clib_memset(t0, 0, sizeof(t0[0]));
        }
    }
}

static uword
dhcp_relay_to_server_input (vlib_main_t *vm,
                            vlib_node_runtime_t *node,
                            vlib_frame_t *from_frame)
{
    u32 counter[DHCP_RELAY_TO_CLIENT_COUNTER_LAST] = {0};

    PDS_PACKET_LOOP_START {
        PDS_PACKET_SINGLE_LOOP_START {
            pds_dhcp_relay_to_server_x1(vm, node,
                                        PDS_PACKET_BUFFER(0),
                                        PDS_PACKET_NEXT_NODE_PTR(0),
                                        counter);
        } PDS_PACKET_SINGLE_LOOP_END;
    } PDS_PACKET_LOOP_END;

#define _(n, s) \
    vlib_node_increment_counter (vm, dhcp_relay_to_server_node.index,   \
                                 DHCP_RELAY_TO_SVR_COUNTER_##n,          \
                                 counter[DHCP_RELAY_TO_SVR_COUNTER_##n]);
    foreach_dhcp_relay_to_server_counter
#undef _

    return from_frame->n_vectors;
}

static u8 *
format_dhcp_relay_to_svr_client_trace (u8 * s, va_list * args)
{
    CLIB_UNUSED (vlib_main_t * vm) = va_arg(*args, vlib_main_t *);
    CLIB_UNUSED (vlib_node_t * node) = va_arg(*args, vlib_node_t *);
    dhcp_relay_to_svr_client_trace_t *t =
                    va_arg(*args, dhcp_relay_to_svr_client_trace_t *);

    s = format(s, "Server addr %U, Relay addr %U, server found - %s",
               format_ip46_address, &t->svr_addr, IP46_TYPE_ANY,
               format_ip46_address, &t->relay_addr, IP46_TYPE_ANY,
               t->server_found ? "True" : "False");
    return s;
}

static char * dhcp_relay_to_server_error_str[] = {
#define _(n,s) s,
    foreach_dhcp_relay_to_server_counter
#undef _
};

VLIB_REGISTER_NODE(dhcp_relay_to_server_node, static) = {
    .function = dhcp_relay_to_server_input,
    .name = "dhcp-relay-to-server",
    /* Takes a vector of packets. */
    .vector_size = sizeof (u32),
    .n_errors = DHCP_RELAY_TO_SVR_COUNTER_LAST,
    .error_strings = dhcp_relay_to_server_error_str,
    .n_next_nodes = PDS_DHCP_RELAY_TO_SVR_N_NEXT,
    .next_nodes = {
#define _(s,n) [PDS_DHCP_RELAY_TO_SVR_NEXT_##s] = n,
    foreach_dhcp_relay_to_server_next
#undef _
    },
    .format_trace = format_dhcp_relay_to_svr_client_trace,
};

always_inline void
pds_dhcp_relay_to_client_x1 (vlib_main_t *vm,
                             vlib_node_runtime_t *node,
                             vlib_buffer_t *b0,
                             u16 *next, u32 *counter)
{
    dhcp_header_t *h0;
    ip4_header_t *ip0 = 0;
    u16 vnic_id, subnet_hw_id;
    u8 *vrmac;
    u32 vrip = 0;
    dhcp_relay_server_t *server = NULL;

    h0 = vlib_buffer_get_current(b0);

    vlib_buffer_advance(b0, -(sizeof(udp_header_t) + sizeof(*ip0)));
    ip0 = vlib_buffer_get_current(b0);

    // linearize needed to "unclone" and scan options
    int rv = vlib_buffer_chain_linearize(vm, b0);
    if ((b0->flags & VLIB_BUFFER_NEXT_PRESENT) != 0 || !rv) {
        counter[DHCP_RELAY_TO_CLIENT_COUNTER_LINEARIZE_FAILED]++;
        *next = PDS_DHCP_RELAY_TO_CLIENT_NEXT_DROP;
        goto trace;
    }

    dhcp_option_t *o = h0->options, *end =
            (void *) vlib_buffer_get_tail(b0);

    while (o->option != DHCP_PACKET_OPTION_END && o < end) {
        if (o->option == 82) {
            dhcp_option_t *sub = (dhcp_option_t *) & o->data[0];
            dhcp_option_t *subend =
                    (dhcp_option_t *) (o->data + o->length);
            while (sub->option != DHCP_PACKET_OPTION_END
                    && sub < subend) {
                if (sub->option == 1) {
                    pds_dhcp_extract_circ_id_callback(vm, b0, sub);
                }
                sub = (dhcp_option_t *) (sub->data + sub->length);
            }
        }
        o = (dhcp_option_t *) (o->data + o->length);
    }

    vnic_id = vnet_buffer(b0)->pds_dhcp_data.vnic_id;
    if (0 != pds_vnic_subnet_get(vnic_id, &subnet_hw_id)) {
        counter[DHCP_RELAY_TO_CLIENT_COUNTER_NO_VNIC]++;
        *next = PDS_DHCP_RELAY_TO_CLIENT_NEXT_DROP;
        goto trace;
    }
    if (0 != pds_impl_db_vr_ip_mac_get(subnet_hw_id, &vrip, &vrmac)) {
        counter[DHCP_RELAY_TO_CLIENT_COUNTER_NO_SUBNET]++;
        *next = PDS_DHCP_RELAY_TO_CLIENT_NEXT_DROP;
        goto trace;
    }
    server = pds_dhcp_relay_server_find(subnet_hw_id,
                                        ip0->dst_address.as_u32,
                                        ip0->src_address.as_u32, 0, 0);
    if (!server) {
        counter[DHCP_RELAY_TO_CLIENT_COUNTER_INVALID_SERVER]++;
        *next = PDS_DHCP_RELAY_TO_CLIENT_NEXT_DROP;
        goto trace;
    }

    //ip and ethernet header filling callback
    (void)pds_dhcp_client_tx_callback(vm, b0);
    counter[DHCP_RELAY_TO_CLIENT_COUNTER_TX]++;
    *next = PDS_DHCP_RELAY_TO_CLIENT_NEXT_CLIENT_TX;

trace:
    if (PREDICT_FALSE (b0->flags & VLIB_BUFFER_IS_TRACED)) {
        // trace packet
        dhcp_relay_to_svr_client_trace_t *t0 =
                vlib_add_trace(vm, node, b0, sizeof (t0[0]));
        ip46_address_set_ip4(&t0->relay_addr, &ip0->dst_address);
        ip46_address_set_ip4(&t0->svr_addr, &ip0->src_address);
        t0->server_found = server ? 1 : 0;
    }
}

static uword
dhcp_relay_to_client_input (vlib_main_t *vm,
                            vlib_node_runtime_t *node,
                            vlib_frame_t *from_frame)
{
    u32 counter[DHCP_RELAY_TO_CLIENT_COUNTER_LAST] = {0};

    PDS_PACKET_LOOP_START {
        PDS_PACKET_SINGLE_LOOP_START {
            pds_dhcp_relay_to_client_x1(vm, node,
                                        PDS_PACKET_BUFFER(0),
                                        PDS_PACKET_NEXT_NODE_PTR(0),
                                        counter);
        } PDS_PACKET_SINGLE_LOOP_END;
    } PDS_PACKET_LOOP_END;

#define _(n, s) \
    vlib_node_increment_counter (vm, dhcp_relay_to_client_node.index,   \
                                 DHCP_RELAY_TO_CLIENT_COUNTER_##n,          \
                                 counter[DHCP_RELAY_TO_CLIENT_COUNTER_##n]);
    foreach_dhcp_relay_to_client_counter
#undef _

    return from_frame->n_vectors;
}

static char * dhcp_relay_to_client_error_str[] = {
#define _(n,s) s,
    foreach_dhcp_relay_to_client_counter
#undef _
};

VLIB_REGISTER_NODE(dhcp_relay_to_client_node, static) = {
    .function = dhcp_relay_to_client_input,
    .name = "dhcp-relay-to-client",
    /* Takes a vector of packets. */
    .vector_size = sizeof (u32),

    .n_errors = DHCP_RELAY_TO_CLIENT_COUNTER_LAST,
    .error_strings = dhcp_relay_to_client_error_str,
    .n_next_nodes = PDS_DHCP_RELAY_TO_CLIENT_N_NEXT,
    .next_nodes = {
#define _(s,n) [PDS_DHCP_RELAY_TO_CLIENT_NEXT_##s] = n,
    foreach_dhcp_relay_to_client_next
#undef _
    },
    .format_trace = format_dhcp_relay_to_svr_client_trace,
};

int
pds_dhcp_set_internal_proxy_server(void)
{
    ip46_address_t svr_addr = { 0 };
    ip46_address_t src_addr = { 0 };

    svr_addr.ip4.as_u32 = clib_host_to_net_u32(PDS_LOCAL_DHCP_SERVER_ADDR);
    src_addr.ip4.as_u32 = clib_host_to_net_u32(PDS_LOCAL_DHCP_AGENT_ADDR);

    return dhcp4_proxy_set_server(&svr_addr, &src_addr, 0, 0, 0);
}

int
pds_dhcp_relay_init_cb (bool external_server)
{
    vlib_main_t *vm;
    static bool inited;

    ASSERT(inited == false);

    inited = true;

    vm = vlib_get_main();

    if (external_server) {
        dhcp_register_server_next_node_tx(vm, (u8 *) "pds-ip4-linux-inject");
    } else {
        pds_dhcp_set_internal_proxy_server();
    }

    dhcp_register_client_next_node_tx(vm, (u8 *) "pds-dhcp-relay-client-tx");
    dhcp_register_custom_options_cb(&pds_dhcp_options_callback);
    dhcp_register_custom_client_hdr_cb(&pds_dhcp_client_tx_callback);
    dhcp_register_extract_circ_id_cb(&pds_dhcp_extract_circ_id_callback);
    return 0;
}

static clib_error_t *
pds_dhcp_relay_init (vlib_main_t *vm)
{
    clib_memset(&dhcp_relay_main, 0, sizeof(dhcp_relay_main_t));
    vec_validate_init_empty(dhcp_relay_main.policy_pool_idx,
                            (2048 - 1), 0xFFFF);

    pds_dhcp_relay_pipeline_init();

    pds_dhcp_relay_cfg_init();

    return 0;
}

VLIB_INIT_FUNCTION (pds_dhcp_relay_init) =
{
    .runs_after = VLIB_INITS("pds_infra_init", "dhcp4_proxy_init"),
};
