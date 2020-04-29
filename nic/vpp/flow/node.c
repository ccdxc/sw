/*
 *  {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
 */

#include <vnet/buffer.h>
#include <pkt.h>
#include <session.h>
#include "node.h"
#include <flow.h>
#include "log.h"
#include "pdsa_hdlr.h"
#include "pdsa_uds_hdlr.h"
#include <nic/vpp/infra/utils.h>
#include <nic/vpp/impl/nh.h>

extern void pds_flow_monitor_init(void);

pds_flow_main_t pds_flow_main;

vlib_node_registration_t pds_fwd_flow_node,
                         pds_ip4_flow_prog_node,
                         pds_ip6_flow_prog_node,
                         pds_tun_ip4_flow_prog_node,
                         pds_tun_ip6_flow_prog_node,
                         pds_flow_classify_node;

u8 g_dis_reinject = 0;

static u8 *
format_pds_fwd_flow_trace (u8 * s, va_list * args)
{
    CLIB_UNUSED (vlib_main_t * vm) = va_arg(*args, vlib_main_t *);
    CLIB_UNUSED (vlib_node_t * node) = va_arg(*args, vlib_node_t *);
    fwd_flow_trace_t *t = va_arg(*args, fwd_flow_trace_t *);

    s = format(s, "Out Interface %U",
               format_vnet_hw_if_index_name, vnet_get_main(), t->hw_index);
    return s;
}

void
pds_fwd_flow_trace_add (vlib_main_t *vm,
                        vlib_node_runtime_t *node,
                        vlib_frame_t *from_frame)
{
    PDS_PACKET_TRACE_LOOP_START {
        PDS_PACKET_TRACE_DUAL_LOOP_START {
            vlib_buffer_t *b0, *b1;
            fwd_flow_trace_t *t0, *t1;

            b0 = PDS_PACKET_BUFFER(0);
            b1 = PDS_PACKET_BUFFER(1);

            if (b0->flags & VLIB_BUFFER_IS_TRACED) {
                t0 = vlib_add_trace(vm, node, b0, sizeof(t0[0]));
                t0->hw_index = vnet_buffer(b0)->sw_if_index[VLIB_TX];
            }
            if (b1->flags & VLIB_BUFFER_IS_TRACED) {
                t1 = vlib_add_trace(vm, node, b1, sizeof(t1[0]));
                t1->hw_index = vnet_buffer(b1)->sw_if_index[VLIB_TX];
            }
        } PDS_PACKET_TRACE_DUAL_LOOP_END;
        PDS_PACKET_TRACE_SINGLE_LOOP_START {
            vlib_buffer_t *b0;
            fwd_flow_trace_t *t0;

            b0 = PDS_PACKET_BUFFER(0);

            if (b0->flags & VLIB_BUFFER_IS_TRACED) {
                t0 = vlib_add_trace(vm, node, b0, sizeof(t0[0]));
                t0->hw_index = vnet_buffer(b0)->sw_if_index[VLIB_TX];
            }
        } PDS_PACKET_TRACE_SINGLE_LOOP_END;
    } PDS_PACKET_TRACE_LOOP_END;
}

static uword
pds_fwd_flow (vlib_main_t * vm, vlib_node_runtime_t * node,
              vlib_frame_t * from_frame)
{
    vnet_main_t *vnm = vnet_get_main();
    vnet_interface_main_t *im = &vnm->interface_main;
    u32 thread_index = vm->thread_index;
    u64 pkt_bytes = 0, pkt_count = 0;
    u32 sw_if_index = ~0;
    vlib_frame_t *to_frame = 0;
    vnet_hw_interface_t *hw = 0;
    u32 *to_next = NULL;
    u16 pak_count = 0;

    PDS_PACKET_LOOP_START {
        if (PREDICT_FALSE(g_dis_reinject)) {
            vlib_node_t *node = vlib_get_node_by_name(vm,
                                                      (u8 *) "pds-error-drop");
            to_frame = vlib_get_frame_to_node(vm, node->index);
            to_frame->n_vectors = from_frame->n_vectors;
            vlib_put_frame_to_node(vm, node->index, to_frame);
            break;
        }
        PDS_PACKET_DUAL_LOOP_START(WRITE, WRITE) {
            vlib_buffer_t *p0, *p1;

            p0 = PDS_PACKET_BUFFER(0);
            p1 = PDS_PACKET_BUFFER(1);
            pds_flow_add_tx_hdrs_x2(p0, p1);
            vnet_buffer(p0)->sw_if_index[VLIB_TX] =
                    vnet_buffer(p0)->sw_if_index[VLIB_RX];
            vnet_buffer(p1)->sw_if_index[VLIB_TX] =
                    vnet_buffer(p1)->sw_if_index[VLIB_RX];


            if (PREDICT_FALSE((sw_if_index !=
                               vnet_buffer(p0)->sw_if_index[VLIB_TX]) ||
                              !to_frame)) {
                if (to_frame) {
                    hw = vnet_get_sup_hw_interface(vnm, sw_if_index);
                    vlib_put_frame_to_node(vm, hw->tx_node_index, to_frame);
                    vlib_increment_combined_counter(
                        im->combined_sw_if_counters + VNET_INTERFACE_COUNTER_TX,
                        thread_index, sw_if_index, pkt_count, pkt_bytes);
                    pkt_bytes = 0;
                    pkt_count = 0;
                }
                sw_if_index = vnet_buffer(p0)->sw_if_index[VLIB_TX];
                hw = vnet_get_sup_hw_interface(vnm, sw_if_index);
                to_frame = vlib_get_frame_to_node(vm, hw->tx_node_index);
                to_next = vlib_frame_vector_args(to_frame);
            }
            pkt_bytes += vlib_buffer_length_in_chain(vm, p0);
            pkt_count++;
            to_frame->n_vectors++;

            if (PREDICT_FALSE((sw_if_index !=
                               vnet_buffer(p1)->sw_if_index[VLIB_TX]) ||
                              !to_frame)) {
                if (to_frame) {
                    hw = vnet_get_sup_hw_interface(vnm, sw_if_index);
                    vlib_put_frame_to_node(vm, hw->tx_node_index, to_frame);
                    vlib_increment_combined_counter(
                        im->combined_sw_if_counters + VNET_INTERFACE_COUNTER_TX,
                        thread_index, sw_if_index, pkt_count, pkt_bytes);
                    pkt_bytes = 0;
                    pkt_count = 0;
                }
                sw_if_index = vnet_buffer(p1)->sw_if_index[VLIB_TX];
                hw = vnet_get_sup_hw_interface(vnm, sw_if_index);
                to_frame = vlib_get_frame_to_node(vm, hw->tx_node_index);
                to_next = vlib_frame_vector_args(to_frame);
            }
            pkt_bytes += vlib_buffer_length_in_chain(vm, p1);
            pkt_count++;
            to_frame->n_vectors++;
            to_next[0] = *PDS_PACKET_BUFFER_INDEX_PTR(pak_count);
            to_next[1] = *PDS_PACKET_BUFFER_INDEX_PTR(pak_count+1);
            pak_count += 2;
            to_next += 2;
        } PDS_PACKET_DUAL_LOOP_END;
        PDS_PACKET_SINGLE_LOOP_START {
            vlib_buffer_t *p0;

            p0 = PDS_PACKET_BUFFER(0);
            pds_flow_add_tx_hdrs_x1(p0);
            vnet_buffer(p0)->sw_if_index[VLIB_TX] =
                            vnet_buffer(p0)->sw_if_index[VLIB_RX];
            if (PREDICT_FALSE((sw_if_index !=
                               vnet_buffer(p0)->sw_if_index[VLIB_TX]) ||
                              !to_frame)) {
                if (to_frame) {
                    hw = vnet_get_sup_hw_interface(vnm, sw_if_index);
                    vlib_put_frame_to_node(vm, hw->tx_node_index, to_frame);
                    vlib_increment_combined_counter(
                        im->combined_sw_if_counters + VNET_INTERFACE_COUNTER_TX,
                        thread_index, sw_if_index, pkt_count, pkt_bytes);
                    pkt_bytes = 0;
                    pkt_count = 0;
                }
                sw_if_index = vnet_buffer(p0)->sw_if_index[VLIB_TX];
                hw = vnet_get_sup_hw_interface(vnm, sw_if_index);
                to_frame = vlib_get_frame_to_node(vm, hw->tx_node_index);
                to_next = vlib_frame_vector_args(to_frame);
            }
            pkt_bytes += vlib_buffer_length_in_chain(vm, p0);
            pkt_count++;
            to_frame->n_vectors++;
            to_next[0] = *PDS_PACKET_BUFFER_INDEX_PTR(pak_count);
            to_next++;
            pak_count++;
        } PDS_PACKET_SINGLE_LOOP_END;
        vlib_put_frame_to_node(vm, hw->tx_node_index, to_frame);
        vlib_increment_combined_counter(
            im->combined_sw_if_counters + VNET_INTERFACE_COUNTER_TX,
            thread_index, sw_if_index, pkt_count, pkt_bytes);
    } PDS_PACKET_LOOP_END_NO_ENQUEUE;

    if (node->flags & VLIB_NODE_FLAG_TRACE) {
        pds_fwd_flow_trace_add(vm, node, from_frame);
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
    .vector_size = sizeof(u32),  // takes a vector of packets

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
    CLIB_UNUSED(vlib_main_t * vm) = va_arg(*args, vlib_main_t *);
    CLIB_UNUSED(vlib_node_t * node) = va_arg(*args, vlib_node_t *);
    session_prog_trace_t *t = va_arg(*args, session_prog_trace_t *);
    u8 *session_data = NULL;
    u32 size;

    session_get_addr(t->session_id, &session_data, &size);
    s = format(s, "Session ID %u,\nData: %U", t->session_id,
               format_hex_bytes, session_data, size);
    return s;
}

void
pds_session_prog_trace_add (vlib_main_t *vm,
                            vlib_node_runtime_t *node,
                            vlib_frame_t *from_frame)
{
    PDS_PACKET_TRACE_LOOP_START {
        PDS_PACKET_TRACE_DUAL_LOOP_START {
            vlib_buffer_t *b0, *b1;
            session_prog_trace_t *t0, *t1;

            b0 = PDS_PACKET_BUFFER(0);
            b1 = PDS_PACKET_BUFFER(1);

            if (b0->flags & VLIB_BUFFER_IS_TRACED) {
                t0 = vlib_add_trace(vm, node, b0, sizeof(t0[0]));
                t0->session_id = vnet_buffer(b0)->pds_flow_data.ses_id;
            }

            if (b1->flags & VLIB_BUFFER_IS_TRACED) {
                t1 = vlib_add_trace(vm, node, b1, sizeof(t1[0]));
                t1->session_id = vnet_buffer(b1)->pds_flow_data.ses_id;
            }

        } PDS_PACKET_TRACE_DUAL_LOOP_END;

        PDS_PACKET_TRACE_SINGLE_LOOP_START {
            vlib_buffer_t *b0;
            session_prog_trace_t *t0;

            b0 = PDS_PACKET_BUFFER(0);

            if (b0->flags & VLIB_BUFFER_IS_TRACED) {
                t0 = vlib_add_trace(vm, node, b0, sizeof(t0[0]));
                t0->session_id = vnet_buffer(b0)->pds_flow_data.ses_id;
            }
        } PDS_PACKET_TRACE_SINGLE_LOOP_END;
    } PDS_PACKET_TRACE_LOOP_END;
}

static uword
pds_session_prog (vlib_main_t * vm,
                  vlib_node_runtime_t * node,
                  vlib_frame_t * from_frame)
{
    u32 counter[SESSION_PROG_COUNTER_LAST] = {0};

    PDS_PACKET_LOOP_START {
        PDS_PACKET_DUAL_LOOP_START(LOAD, LOAD) {
            vlib_buffer_t *b0, *b1;
            u32 session_id0, session_id1;

            b0 = PDS_PACKET_BUFFER(0);
            b1 = PDS_PACKET_BUFFER(1);

            session_id0 = vnet_buffer(b0)->pds_flow_data.ses_id;
            session_id1 = vnet_buffer(b1)->pds_flow_data.ses_id;
            pds_session_prog_x2(b0, b1,
                                session_id0, session_id1,
                                PDS_PACKET_NEXT_NODE_PTR(0),
                                PDS_PACKET_NEXT_NODE_PTR(1),
                                counter);
        } PDS_PACKET_DUAL_LOOP_END;

        PDS_PACKET_SINGLE_LOOP_START {
            u32 session_id0;
            vlib_buffer_t *b;

            b = PDS_PACKET_BUFFER(0);

            session_id0 = vnet_buffer(b)->pds_flow_data.ses_id;
            pds_session_prog_x1(b, session_id0,
                                PDS_PACKET_NEXT_NODE_PTR(0),
                                counter);
        } PDS_PACKET_SINGLE_LOOP_END;
    } PDS_PACKET_LOOP_END;

#define _(n, s)                                                         \
    vlib_node_increment_counter (vm, node->node_index,                  \
                                 SESSION_PROG_COUNTER_##n,              \
                                 counter[SESSION_PROG_COUNTER_##n]);
    foreach_session_prog_counter
#undef _

    if (node->flags & VLIB_NODE_FLAG_TRACE) {
        pds_session_prog_trace_add(vm, node, from_frame);
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
    .vector_size = sizeof(u32),

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
    CLIB_UNUSED (vlib_main_t * vm) = va_arg(*args, vlib_main_t *);
    CLIB_UNUSED (vlib_node_t * node) = va_arg(*args, vlib_node_t *);
    flow_prog_trace_t *t = va_arg(*args, flow_prog_trace_t *);

    s = format(s, "iFlow: src[%U], dst[%U], IP_Proto[%U], src_port[%d], "
               "dst_port[%d]\n", format_ip46_address, &t->isrc, IP46_TYPE_ANY,
               format_ip46_address, &t->idst, IP46_TYPE_ANY,
               format_ip_protocol, t->iprotocol,
               clib_net_to_host_u16(t->isrc_port),
               clib_net_to_host_u16(t->idst_port));
    s = format(s, "rFlow: src[%U], dst[%U], IP_Proto[%U], src_port[%d], "
               "dst_port[%d]", format_ip46_address, &t->rsrc, IP46_TYPE_ANY,
               format_ip46_address, &t->rdst, IP46_TYPE_ANY,
               format_ip_protocol, t->rprotocol,
               clib_net_to_host_u16(t->rsrc_port),
               clib_net_to_host_u16(t->rdst_port));
    return s;
}

void
pds_flow_prog_populate_trace (vlib_buffer_t *CLIB_UNUSED(b0),
                              flow_prog_trace_t *t0,
                              void *data, u8 is_ip4)
{
    if (is_ip4) {
        ip4_header_t *ip40 = data;

        ip46_address_set_ip4(&t0->isrc, &ip40->src_address);
        ip46_address_set_ip4(&t0->rdst, &ip40->src_address);

        ip46_address_set_ip4(&t0->idst, &ip40->dst_address);
        ip46_address_set_ip4(&t0->rsrc, &ip40->dst_address);

        t0->iprotocol = t0->rprotocol = ip40->protocol;
    } else {
        ip6_header_t *ip60 = data;

        ip46_address_set_ip6(&t0->isrc, &ip60->src_address);
        ip46_address_set_ip6(&t0->rdst, &ip60->src_address);

        ip46_address_set_ip6(&t0->idst, &ip60->dst_address);
        ip46_address_set_ip6(&t0->rsrc, &ip60->dst_address);

        t0->iprotocol = t0->rprotocol = ip60->protocol;
    }

    if (PREDICT_TRUE(((t0->iprotocol == IP_PROTOCOL_TCP) ||
                      (t0->iprotocol == IP_PROTOCOL_UDP)))) {
        tcp_header_t *tcp0 =
            (tcp_header_t *)(((u8 *) data) +
                             (vnet_buffer(b0)->l4_hdr_offset -
                              vnet_buffer(b0)->l3_hdr_offset));
        t0->isrc_port = t0->rdst_port = tcp0->src_port;
        t0->idst_port = t0->rsrc_port = tcp0->dst_port;
    }
    return;
}

void
pds_flow_prog_trace_add (vlib_main_t *vm,
                         vlib_node_runtime_t *node,
                         vlib_frame_t *from_frame, u8 is_ip4, u8 is_l2)
{
    PDS_PACKET_TRACE_LOOP_START {
        PDS_PACKET_TRACE_DUAL_LOOP_START {
            vlib_buffer_t *b0, *b1;
            flow_prog_trace_t *t0, *t1;
            int offset0, offset1;
            void *data0, *data1;

            b0 = PDS_PACKET_BUFFER(0);
            b1 = PDS_PACKET_BUFFER(1);

            offset0 = pds_flow_prog_get_next_offset(b0, is_l2);
            offset1 = pds_flow_prog_get_next_offset(b1, is_l2);
            data0 = vlib_buffer_get_current(b0) - offset0;
            data1 = vlib_buffer_get_current(b1) - offset1;

            if (b0->flags & VLIB_BUFFER_IS_TRACED) {
                t0 = vlib_add_trace(vm, node, b0, sizeof(t0[0]));
                pds_flow_prog_populate_trace(b0, t0, data0, is_ip4);
            }
            if (b1->flags & VLIB_BUFFER_IS_TRACED) {
                t1 = vlib_add_trace(vm, node, b1, sizeof(t1[0]));
                pds_flow_prog_populate_trace(b1, t1, data1, is_ip4);
            }
        } PDS_PACKET_TRACE_DUAL_LOOP_END;

        PDS_PACKET_TRACE_SINGLE_LOOP_START {
            vlib_buffer_t *b0;
            flow_prog_trace_t *t0;
            int offset0;
            void *data0;

            b0 = PDS_PACKET_BUFFER(0);

            offset0 = pds_flow_prog_get_next_offset(b0, is_l2);
            data0 = vlib_buffer_get_current(b0) - offset0;

            if (b0->flags & VLIB_BUFFER_IS_TRACED) {
                t0 = vlib_add_trace(vm, node, b0, sizeof(t0[0]));
                pds_flow_prog_populate_trace(b0, t0, data0, is_ip4);
            }
        } PDS_PACKET_TRACE_SINGLE_LOOP_END;
    } PDS_PACKET_TRACE_LOOP_END;
}

always_inline void
pds_flow_extract_prog_args_x1 (vlib_buffer_t *p0,
                               u32 session_id,
                               u8 is_ip4, u8 is_l2,
                               u8 flow_exists)
{
    tcp_header_t        *tcp0 = NULL;
    icmp46_header_t     *icmp0;
    u8                  miss_hit = 0;
    u8                  napt = 0;
    pds_flow_hw_ctx_t   *ses = pds_flow_get_hw_ctx(session_id);
    pds_flow_main_t     *fm = &pds_flow_main;

    vnet_buffer(p0)->pds_flow_data.ses_id = session_id;

    if (is_ip4) {
        ip4_header_t *ip40;
        u32 src_ip, dst_ip, r_src_ip, r_dst_ip;
        u16 sport, dport, r_sport, r_dport;
        u8 protocol;
        u16 lkp_id;

        ip40 = vlib_buffer_get_current(p0);

        r_dst_ip = src_ip = clib_net_to_host_u32(ip40->src_address.as_u32);
        r_src_ip = dst_ip = clib_net_to_host_u32(ip40->dst_address.as_u32);
        protocol = ip40->protocol;
        lkp_id = vnet_buffer(p0)->sw_if_index[VLIB_TX];
        if (PREDICT_FALSE(flow_exists && (ses->ingress_bd != 0))) {
            lkp_id = ses->ingress_bd;
        }

        if (PREDICT_TRUE(((ip40->protocol == IP_PROTOCOL_TCP) ||
                          (ip40->protocol == IP_PROTOCOL_UDP)))) {
            if (ip40->protocol == IP_PROTOCOL_TCP) {
                ftlv4_cache_set_counter_index(FLOW_TYPE_COUNTER_TCPV4);
            } else {
                ftlv4_cache_set_counter_index(FLOW_TYPE_COUNTER_UDPV4);
            }
            tcp0 = (tcp_header_t *)(((u8 *) ip40) +
                   (vnet_buffer(p0)->l4_hdr_offset - vnet_buffer(p0)->l3_hdr_offset));
            r_dport = sport = clib_net_to_host_u16(tcp0->src_port);
            r_sport = dport = clib_net_to_host_u16(tcp0->dst_port);
        } else if (ip40->protocol == IP_PROTOCOL_ICMP) {
            icmp0 = (icmp46_header_t *) (((u8 *) ip40) +
                    (vnet_buffer (p0)->l4_hdr_offset -
                    vnet_buffer (p0)->l3_hdr_offset));
            sport = *((u16 *) (icmp0 + 1));
            sport = clib_net_to_host_u16(sport);
            r_sport = sport;
            dport = ((u16) (icmp0->type << 8)) | icmp0->code;
            if (PREDICT_TRUE(icmp0->type == ICMP4_echo_request)) {
                r_dport = (ICMP4_echo_reply << 8) | icmp0->code;
            } else if (icmp0->type == ICMP4_echo_reply) {
                r_dport = (ICMP4_echo_request << 8) | icmp0->code;
            } else {
                r_dport = dport;
            }
            ftlv4_cache_set_counter_index(FLOW_TYPE_COUNTER_ICMPV4);
        } else {
            sport = dport = r_sport = r_dport = 0;
            ftlv4_cache_set_counter_index(FLOW_TYPE_COUNTER_OTHERV4);
        }
        ftlv4_cache_set_key(src_ip, dst_ip,
                            protocol, sport, dport, lkp_id);
        ftlv4_cache_set_session_index(session_id);
        ftlv4_cache_set_flow_role(TCP_FLOW_INITIATOR);
        ftlv4_cache_set_epoch(pds_get_flow_epoch(p0));
        if (PREDICT_FALSE(pds_is_flow_l2l(p0))) {
            pds_flow_handle_l2l(p0, flow_exists, &miss_hit, session_id);
        }
        ftlv4_cache_set_flow_miss_hit(miss_hit);
        ftlv4_cache_set_update_flag(flow_exists);
        pds_flow_extract_nexthop_info(p0, 1, 1);
        ftlv4_cache_set_hash_log(vnet_buffer(p0)->pds_flow_data.flow_hash,
                                 pds_get_flow_log_en(p0));
        ftlv4_cache_set_host_origin(pds_is_rx_pkt(p0));

        if (pds_is_flow_napt_en(p0)) {
            // NAPT - both port and ip are changed
            napt = 1;
            r_dst_ip = vnet_buffer2(p0)->pds_nat_data.xlate_addr;
            if (ip40->protocol != IP_PROTOCOL_ICMP) {
                r_dport = vnet_buffer2(p0)->pds_nat_data.xlate_port;
            }
        } else if (vnet_buffer2(p0)->pds_nat_data.xlate_idx &&
                   !pds_is_flow_svc_map_en(p0)) {
            // static NAT
            if (pds_is_rx_pkt(p0)) {
                r_src_ip = vnet_buffer2(p0)->pds_nat_data.xlate_addr;
            } else {
                r_dst_ip = vnet_buffer2(p0)->pds_nat_data.xlate_addr;
            }
        }
        if (vnet_buffer2(p0)->pds_nat_data.xlate_idx2) {
            // twice NAT
            r_src_ip = vnet_buffer2(p0)->pds_nat_data.xlate_addr2;
        }
        lkp_id = vnet_buffer(p0)->pds_flow_data.egress_lkp_id;
        ftlv4_cache_set_napt_flag(napt);
        ftlv4_cache_advance_count(1);
        ftlv4_cache_set_key(r_src_ip, r_dst_ip,
                            protocol, r_sport, r_dport, lkp_id);
        ftlv4_cache_set_session_index(session_id);
        ftlv4_cache_set_flow_role(TCP_FLOW_RESPONDER);
        ftlv4_cache_set_epoch(pds_get_flow_epoch(p0));
        ftlv4_cache_set_flow_miss_hit(miss_hit);
        ftlv4_cache_set_update_flag(flow_exists);
        pds_flow_extract_nexthop_info(p0, 1, 0);
        ftlv4_cache_set_hash_log(0, pds_get_flow_log_en(p0));
        ftlv4_cache_set_napt_flag(napt);
        ftlv4_cache_advance_count(1);

        if (fm->con_track_en && tcp0) {
            vnet_buffer(p0)->pds_flow_data.tcp_seq_no = clib_net_to_host_u32(tcp0->seq_number);
            vnet_buffer(p0)->pds_flow_data.tcp_win_sz = clib_net_to_host_u16(tcp0->window);
        }
    } else {
        ip6_header_t *ip60;
        ethernet_header_t *e0;
        static u8 src[16], dst[16];
        u16 sport, dport, r_sport, r_dport;
        u8 protocol;
        u16 lkp_id, ether_type;

        if (!flow_exists) {
            lkp_id = vnet_buffer(p0)->sw_if_index[VLIB_TX];
        } else {
            lkp_id = ses->ingress_bd;
        }

        if (is_l2) {
            e0 = vlib_buffer_get_current(p0);

            // Copy the MAC addresses in reverse
            int j = 0;
            for (int i = ETH_ADDR_LEN - 1; i >= 0; i--) {
                src[j] = e0->src_address[i];
                dst[j] = e0->dst_address[i];
                j++;
            }

            ether_type = clib_net_to_host_u16(e0->type);
            lkp_id = vnet_buffer(p0)->sw_if_index[VLIB_TX];
            ftll2_cache_set_key(src, dst, ether_type, lkp_id);
            ftll2_cache_set_counter_index(FLOW_TYPE_COUNTER_L2);
        } else {
            ip60 = vlib_buffer_get_current(p0);

            clib_memcpy(src, ip60->src_address.as_u8, 16);
            clib_memcpy(dst, ip60->dst_address.as_u8, 16);
            protocol = ip60->protocol;
            lkp_id = vnet_buffer(p0)->sw_if_index[VLIB_TX];

            if (PREDICT_TRUE(((ip60->protocol == IP_PROTOCOL_TCP) ||
                             (ip60->protocol == IP_PROTOCOL_UDP)))) {
                tcp0 = (tcp_header_t *)(((u8 *) ip60) +
                                        (vnet_buffer(p0)->l4_hdr_offset -
                                         vnet_buffer(p0)->l3_hdr_offset));
                if (ip60->protocol == IP_PROTOCOL_TCP) {
                    ftlv6_cache_set_counter_index(FLOW_TYPE_COUNTER_TCPV6);
                } else {
                    ftlv6_cache_set_counter_index(FLOW_TYPE_COUNTER_UDPV6);
                }
                r_dport = sport = clib_net_to_host_u16(tcp0->src_port);
                r_sport = dport = clib_net_to_host_u16(tcp0->dst_port);
            } else if (ip60->protocol == IP_PROTOCOL_ICMP6) {
                icmp0 = (icmp46_header_t *) (((u8 *) ip60) +
                        (vnet_buffer (p0)->l4_hdr_offset -
                         vnet_buffer (p0)->l3_hdr_offset));
                sport = *((u16 *) (icmp0 + 1));
                sport = clib_net_to_host_u16(sport);
                r_sport = sport;
                dport = ((u16) (icmp0->type << 8)) | icmp0->code;
                if (PREDICT_TRUE(icmp0->type == ICMP4_echo_request)) {
                    r_dport = (ICMP4_echo_reply << 8) | icmp0->code;
                } else if (icmp0->type == ICMP4_echo_reply) {
                    r_dport = (ICMP4_echo_request << 8) | icmp0->code;
                } else {
                    r_dport = dport;
                }
                ftlv6_cache_set_counter_index(FLOW_TYPE_COUNTER_ICMPV6);
            } else {
                sport = dport = r_sport = r_dport = 0;
                ftlv6_cache_set_counter_index(FLOW_TYPE_COUNTER_OTHERV6);
            }
            ftlv6_cache_set_key(src, dst, protocol, sport, dport, lkp_id);
            pds_flow_extract_nexthop_info(p0, 0, 1);
        }
        ftlv6_cache_set_session_index(session_id);
        ftlv6_cache_set_flow_role(TCP_FLOW_INITIATOR);
        ftlv6_cache_set_epoch(pds_get_flow_epoch(p0));
        if (PREDICT_FALSE(pds_is_flow_l2l(p0))) {
            pds_flow_handle_l2l(p0, flow_exists, &miss_hit, session_id);
        }
        ftlv6_cache_set_flow_miss_hit(miss_hit);
        ftlv6_cache_set_update_flag(flow_exists);
        ftlv6_cache_set_hash_log(vnet_buffer(p0)->pds_flow_data.flow_hash,
                                 pds_get_flow_log_en(p0));
        ftlv6_cache_advance_count(1);
        lkp_id = vnet_buffer(p0)->pds_flow_data.egress_lkp_id;
        if (is_l2) {
            ftll2_cache_set_key(dst, src, ether_type, lkp_id);
        } else {
            ftlv6_cache_set_key(dst, src, protocol, r_sport, r_dport, lkp_id);
            pds_flow_extract_nexthop_info(p0, 0, 0);
        }
        ftlv6_cache_set_session_index(session_id);
        ftlv6_cache_set_flow_role(TCP_FLOW_RESPONDER);
        ftlv6_cache_set_epoch(pds_get_flow_epoch(p0));
        ftlv6_cache_set_flow_miss_hit(miss_hit);
        ftlv6_cache_set_update_flag(flow_exists);
        ftlv6_cache_set_hash_log(0, pds_get_flow_log_en(p0));
        ftlv6_cache_advance_count(1);
    }
    return;
}

always_inline void
pds_flow_program_hw_ip4 (vlib_buffer_t **b, u16 *next, u32 *counter)
{
    int i, ret, size = ftlv4_cache_get_count();
    u32 i_pindex, i_sindex, r_pindex, r_sindex;
    ftlv4 *table = pds_flow_prog_get_table4();
    vlib_buffer_t *p0;
    pds_flow_main_t *fm = &pds_flow_main;
    uint8_t ctr_idx;
    uint64_t ses_ctr[FLOW_TYPE_COUNTER_LAST] = {0};

    for (i = 0; i < size; i+=2) {
        uint32_t session_id = ftlv4_cache_get_session_index(i);

        ret = ftlv4_cache_program_index(table, i, &i_pindex, &i_sindex);
        if (PREDICT_FALSE(ret != 0)) {
            goto err_iflow;
        }
        ret = ftlv4_cache_program_index(table, i+1, &r_pindex, &r_sindex);
        if (PREDICT_FALSE(ret != 0)) {
            goto err_rflow;
        }
        ses_ctr[ftlv4_cache_get_counter_index(i)]++;
        p0 = b[i/2];
        pds_session_set_data(session_id, i_pindex,
                             i_sindex, r_pindex,
                             r_sindex,
                             pds_flow_trans_proto(ftlv4_cache_get_proto(i)),
                             vnet_buffer2(p0)->pds_nat_data.vnic_id,
                             true, pds_is_rx_pkt(p0));
        counter[FLOW_PROG_COUNTER_FLOW_SUCCESS]++;
        next[i/2] = pds_flow_prog_get_next_node();
        ftlv4_cache_log_session(i, i+1, FLOW_EXPORT_REASON_ADD);
        continue;

    err_rflow:
        // Remove last entry as local flow succeeded
        // but remote entry failed
        ret = ftlv4_cache_delete_index(table, i);
        if (PREDICT_FALSE(0 != ret)) {
            counter[FLOW_PROG_COUNTER_FLOW_DELETE_FAILED]++;
        }
    err_iflow:
        counter[FLOW_PROG_COUNTER_FLOW_FAILED]++;
        if (ftlv4_cache_get_napt_flag(i)) {
            p0 = b[i/2];
            int offset0 = pds_flow_prog_get_nat_drop_next_offset(p0);
            vlib_buffer_advance(p0, offset0);
            next[i/2] = FLOW_PROG_NEXT_NAT_DROP;
        } else {
            next[i/2] = FLOW_PROG_NEXT_DROP;
        }
        if (session_id) {
            vlib_buffer_t *p0 = b[i/2];
            if (pds_is_flow_session_present(p0)) {
                // flow update failed - so clear the flow
                pds_flow_delete_session(session_id);
            } else {
                pds_session_id_dealloc(session_id);
            }
        }
    }
    // accumulate all v4 counters atomically
    for (ctr_idx = FLOW_TYPE_COUNTER_TCPV4;
         ctr_idx <= FLOW_TYPE_COUNTER_OTHERV4; ctr_idx++) {
        if (ses_ctr[ctr_idx]) {
            clib_atomic_fetch_add(&fm->stats.counter[ctr_idx],
                                  ses_ctr[ctr_idx]);
        }
    }
}

always_inline void
pds_flow_program_hw_ip6_or_l2 (vlib_buffer_t **b, u16 *next, u32 *counter)
{
    int i, ret, size = ftlv6_cache_get_count();
    ftlv6 *table = pds_flow_prog_get_table6_or_l2();
    u32 i_pindex, i_sindex, r_pindex, r_sindex;
    vlib_buffer_t *p0;
    pds_flow_main_t *fm = &pds_flow_main;
    uint8_t ctr_idx;
    uint64_t ses_ctr[FLOW_TYPE_COUNTER_LAST] = {0};

    for (i = 0; i < size; i+=2) {
        uint32_t session_id = ftlv6_cache_get_session_index(i);

        ret = ftlv6_cache_program_index(table, i, &i_pindex, &i_sindex);
        if (PREDICT_FALSE(ret != 0)) {
            goto err_iflow;
        }
        ret = ftlv6_cache_program_index(table, i+1, &r_pindex, &r_sindex);
        if (PREDICT_FALSE(ret != 0)) {
            goto err_rflow;
        }
        ses_ctr[ftlv4_cache_get_counter_index(i)]++;
        p0 = b[i/2];
        pds_session_set_data(session_id, i_pindex,
                             i_sindex, r_pindex,
                             r_sindex,
                             pds_flow_trans_proto(ftlv6_cache_get_proto(i)),
                             vnet_buffer2(p0)->pds_nat_data.vnic_id,
                             false, pds_is_rx_pkt(p0));
        counter[FLOW_PROG_COUNTER_FLOW_SUCCESS]++;
        next[i/2] = pds_flow_prog_get_next_node();
        ftlv6_cache_log_session(i, i+1, FLOW_EXPORT_REASON_ADD);
        continue;

    err_rflow:
        // Remove last entry as local flow succeeded
        // but remote entry failed
        ret = ftlv6_cache_delete_index(table, i);
        if (PREDICT_FALSE(0 != ret)) {
            counter[FLOW_PROG_COUNTER_FLOW_DELETE_FAILED]++;
        }
    err_iflow:
        counter[FLOW_PROG_COUNTER_FLOW_FAILED]++;
        next[i/2] = FLOW_PROG_NEXT_DROP;
        if (session_id) {
            vlib_buffer_t *p0 = b[i/2];
            if (pds_is_flow_session_present(p0)) {
                // flow update failed - so clear the flow
                pds_flow_delete_session(session_id);
            } else {
                pds_session_id_dealloc(session_id);
            }
        }
    }
    // accumulate all v6 and l2 counters atomically
    for (ctr_idx = FLOW_TYPE_COUNTER_TCPV6;
         ctr_idx <= FLOW_TYPE_COUNTER_L2; ctr_idx++) {
        if (ses_ctr[ctr_idx]) {
            clib_atomic_fetch_add(&fm->stats.counter[ctr_idx],
                                  ses_ctr[ctr_idx]);
        }
    }
}

always_inline uword
pds_flow_prog (vlib_main_t *vm,
               vlib_node_runtime_t *node,
               vlib_frame_t *from_frame, u8 is_ip4, u8 is_l2)
{
    u32 counter[FLOW_PROG_COUNTER_LAST] = {0};
    pds_flow_main_t *fm = &pds_flow_main;

    if (is_ip4) {
        ftlv4_cache_batch_init();
    } else {
        ftlv6_cache_batch_init();
    }
    PDS_PACKET_LOOP_START {
        PDS_PACKET_DUAL_LOOP_START(LOAD, LOAD) {
            u32 session_id0 = 0, session_id1 = 0;
            int offset0, offset1;
            vlib_buffer_t *b0, *b1;
            u8 flow_exists0 = 1, flow_exists1 = 1;

            b0 = PDS_PACKET_BUFFER(0);
            b1 = PDS_PACKET_BUFFER(1);

            session_id0 = vnet_buffer(b0)->pds_flow_data.ses_id;
            session_id1 = vnet_buffer(b1)->pds_flow_data.ses_id;
            if (PREDICT_TRUE((0 == session_id0) && (0 == session_id1))) {
                pds_session_id_alloc2(&session_id0, &session_id1);
                flow_exists0 = flow_exists1 = 0;
            } else {
                if (0 == session_id0) {
                    session_id0 = pds_session_id_alloc();
                    flow_exists0 = 0;
                }
                if (0 == session_id1) {
                    session_id1 = pds_session_id_alloc();
                    flow_exists1 = 0;
                }
            }
            pds_flow_extract_prog_args_x1(b0, session_id0, is_ip4, is_l2,
                                          flow_exists0);
            pds_flow_extract_prog_args_x1(b1, session_id1, is_ip4, is_l2,
                                          flow_exists1);
            offset0 = pds_flow_prog_get_next_offset(b0, is_l2);
            offset1 = pds_flow_prog_get_next_offset(b1, is_l2);
            vlib_buffer_advance(b0, offset0);
            vlib_buffer_advance(b1, offset1);
        } PDS_PACKET_DUAL_LOOP_END;

        PDS_PACKET_SINGLE_LOOP_START {
            u32 session_id0;
            int offset0;
            vlib_buffer_t *b0;
            u8 flow_exists0;

            b0 = PDS_PACKET_BUFFER(0);

            session_id0 = vnet_buffer(b0)->pds_flow_data.ses_id ?
                          vnet_buffer(b0)->pds_flow_data.ses_id :
                          pds_session_id_alloc();
            flow_exists0 = vnet_buffer(b0)->pds_flow_data.ses_id ? 1 : 0;
            pds_flow_extract_prog_args_x1(b0, session_id0, is_ip4, is_l2,
                                          flow_exists0);
            offset0 = pds_flow_prog_get_next_offset(b0, is_l2);
            vlib_buffer_advance(b0, offset0);
        } PDS_PACKET_SINGLE_LOOP_END;
    } PDS_PACKET_LOOP_END_NO_ENQUEUE;

    if (is_ip4) {
        pds_flow_program_hw_ip4(PDS_PACKET_BUFFER_ARR, PDS_PACKET_NEXT_NODE_ARR,
                                counter);
    } else {
        pds_flow_program_hw_ip6_or_l2(PDS_PACKET_BUFFER_ARR,
                                      PDS_PACKET_NEXT_NODE_ARR, counter);
    }
    vlib_buffer_enqueue_to_next(vm, node,
                                PDS_PACKET_BUFFER_INDEX_PTR(0),
                                PDS_PACKET_NEXT_NODE_ARR,
                                from_frame->n_vectors);
    clib_atomic_fetch_add(&fm->stats.counter[FLOW_TYPE_COUNTER_ERROR],
                          counter[FLOW_PROG_COUNTER_FLOW_FAILED]);

#define _(n, s)                                                 \
    vlib_node_increment_counter (vm, node->node_index,          \
            FLOW_PROG_COUNTER_##n,                              \
            counter[FLOW_PROG_COUNTER_##n]);
    foreach_flow_prog_counter
#undef _

    if (node->flags & VLIB_NODE_FLAG_TRACE) {
        pds_flow_prog_trace_add(vm, node, from_frame, is_ip4, is_l2);
    }

    return from_frame->n_vectors;
}

static uword
pds_ip6_flow_prog (vlib_main_t * vm,
                   vlib_node_runtime_t * node,
                   vlib_frame_t * from_frame)
{
    return pds_flow_prog(vm, node, from_frame, 0, 0);
}

static char * flow_prog_error_strings[] = {
#define _(n,s) s,
    foreach_flow_prog_counter
#undef _
};

VLIB_REGISTER_NODE (pds_ip6_flow_prog_node) = {
    .function = pds_ip6_flow_prog,
    .name = "pds-ip6-flow-program",

    .vector_size = sizeof(u32),  // takes a vector of packets

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
    return pds_flow_prog(vm, node, from_frame, 1, 0);
}

VLIB_REGISTER_NODE (pds_ip4_flow_prog_node) = {
    .function = pds_ip4_flow_prog,
    .name = "pds-ip4-flow-program",

    .vector_size = sizeof(u32),  // takes a vector of packets

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
pds_l2_flow_prog (vlib_main_t * vm,
                  vlib_node_runtime_t * node,
                  vlib_frame_t * from_frame)
{
    return pds_flow_prog(vm, node, from_frame, 0, 1);
}

VLIB_REGISTER_NODE (pds_l2_flow_prog_node) = {
    .function = pds_l2_flow_prog,
    .name = "pds-l2-flow-program",

    .vector_size = sizeof(u32),  // takes a vector of packets

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
    return pds_flow_prog(vm, node, from_frame, 0, 0);
}

VLIB_REGISTER_NODE (pds_tun_ip6_flow_prog_node) = {
    .function = pds_tunnel_ip6_flow_prog,
    .name = "pds-tunnel-ip6-flow-program",

    .vector_size = sizeof(u32),  // takes a vector of packets

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
    return pds_flow_prog(vm, node, from_frame, 1, 0);
}

VLIB_REGISTER_NODE (pds_tun_ip4_flow_prog_node) = {
    .function = pds_tunnel_ip4_flow_prog,
    .name = "pds-tunnel-ip4-flow-program",

    .vector_size = sizeof(u32),  // takes a vector of packets

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
format_pds_flow_classify_trace (u8 * s, va_list * args)
{
    CLIB_UNUSED (vlib_main_t * vm) = va_arg(*args, vlib_main_t *);
    CLIB_UNUSED (vlib_node_t * node) = va_arg(*args, vlib_node_t *);
    flow_classify_trace_t *t = va_arg(*args, flow_classify_trace_t *);

    s = format(s, "Flags[0x%x], flow_hash[0x%x], l2_offset[%d], "
               "l3_offset[%d] l4_offset[%d], vnic[%d]",
               t->flags, t->flow_hash, t->l2_offset, t->l3_offset,
               t->l4_offset, t->vnic);
    return s;
}

void
pds_flow_classify_trace_add (vlib_main_t *vm,
                             vlib_node_runtime_t *node,
                             vlib_frame_t *from_frame)
{
    PDS_PACKET_TRACE_LOOP_START {
        PDS_PACKET_TRACE_DUAL_LOOP_START {
            flow_classify_trace_t *t0, *t1;
            u16 flag0, flag1;
            vlib_buffer_t *b0, *b1;

            b0 = PDS_PACKET_BUFFER(0);
            b1 = PDS_PACKET_BUFFER(1);

            if (b0->flags & VLIB_BUFFER_IS_TRACED) {
                t0 = vlib_add_trace(vm, node, b0, sizeof(t0[0]));
                t0->flow_hash = vnet_buffer(b0)->pds_flow_data.flow_hash;
                flag0 = vnet_buffer(b0)->pds_flow_data.flags;
                t0->flags = flag0;
                t0->l2_offset = vnet_buffer(b0)->l2_hdr_offset;
                t0->l3_offset = vnet_buffer(b0)->l3_hdr_offset;
                t0->l4_offset = vnet_buffer(b0)->l4_hdr_offset;
                t0->vnic = vnet_buffer(b0)->sw_if_index[VLIB_TX];
            }
            if (b1->flags & VLIB_BUFFER_IS_TRACED) {
                t1 = vlib_add_trace(vm, node, b1, sizeof(t1[0]));
                t1->flow_hash = vnet_buffer(b1)->pds_flow_data.flow_hash;
                flag1 = vnet_buffer(b1)->pds_flow_data.flags;
                t1->flags = flag1;
                t1->l2_offset = vnet_buffer(b1)->l2_hdr_offset;
                t1->l3_offset = vnet_buffer(b1)->l3_hdr_offset;
                t1->l4_offset = vnet_buffer(b1)->l4_hdr_offset;
                t1->vnic = vnet_buffer(b1)->sw_if_index[VLIB_TX];
            }
        } PDS_PACKET_TRACE_DUAL_LOOP_END;

        PDS_PACKET_TRACE_SINGLE_LOOP_START {
            vlib_buffer_t *b0;
            flow_classify_trace_t *t0;
            u16 flag0;

            b0 = PDS_PACKET_BUFFER(0);

            if (b0->flags & VLIB_BUFFER_IS_TRACED) {
                t0 = vlib_add_trace(vm, node, b0, sizeof(t0[0]));
                t0->flow_hash = vnet_buffer(b0)->pds_flow_data.flow_hash;
                flag0 = vnet_buffer(b0)->pds_flow_data.flags;
                t0->flags = flag0;
                t0->l2_offset = vnet_buffer(b0)->l2_hdr_offset;
                t0->l3_offset = vnet_buffer(b0)->l3_hdr_offset;
                t0->l4_offset = vnet_buffer(b0)->l4_hdr_offset;
                t0->vnic = vnet_buffer(b0)->sw_if_index[VLIB_TX];
            }
        } PDS_PACKET_TRACE_SINGLE_LOOP_END;
    } PDS_PACKET_TRACE_LOOP_END;
}

static uword
pds_flow_classify (vlib_main_t *vm,
                   vlib_node_runtime_t *node,
                   vlib_frame_t *from_frame)
{
    u32 counter[FLOW_CLASSIFY_COUNTER_LAST] = {0};
    static __thread u8 ftl_init_done = 0;

    if (PREDICT_FALSE(!ftl_init_done)) {
        ftlv4_set_thread_id(pds_flow_prog_get_table4(), node->thread_index);
        ftlv6_set_thread_id(pds_flow_prog_get_table6_or_l2(),
                            node->thread_index);
        ftl_init_done = 1;
    }

    PDS_PACKET_LOOP_START {
        PDS_PACKET_DUAL_LOOP_START(WRITE, WRITE) {
            pds_flow_classify_x2(PDS_PACKET_BUFFER(0),
                                 PDS_PACKET_BUFFER(1),
                                 PDS_PACKET_NEXT_NODE_PTR(0),
                                 PDS_PACKET_NEXT_NODE_PTR(1),
                                 counter);
        } PDS_PACKET_DUAL_LOOP_END;
        PDS_PACKET_SINGLE_LOOP_START {
            pds_flow_classify_x1(PDS_PACKET_BUFFER(0),
                                 PDS_PACKET_NEXT_NODE_PTR(0),
                                 counter);
        } PDS_PACKET_SINGLE_LOOP_END;
    } PDS_PACKET_LOOP_END;

#define _(n, s) \
    vlib_node_increment_counter (vm, pds_flow_classify_node.index,   \
            FLOW_CLASSIFY_COUNTER_##n,                               \
            counter[FLOW_CLASSIFY_COUNTER_##n]);
    foreach_flow_classify_counter
#undef _

    if (node->flags & VLIB_NODE_FLAG_TRACE) {
        pds_flow_classify_trace_add(vm, node, from_frame);
    }

    return from_frame->n_vectors;
}

static char * flow_classify_error_strings[] = {
#define _(n,s) s,
    foreach_flow_classify_counter
#undef _
};

VLIB_REGISTER_NODE (pds_flow_classify_node) = {
    .function = pds_flow_classify,
    .name = "pds-flow-classify",

    .vector_size = sizeof(u32),  // takes a vector of packets

    .n_errors = FLOW_CLASSIFY_COUNTER_LAST,
    .error_strings = flow_classify_error_strings,

    .n_next_nodes = FLOW_CLASSIFY_N_NEXT,
    .next_nodes = {
#define _(s,n) [FLOW_CLASSIFY_NEXT_##s] = n,
    foreach_flow_classify_next
#undef _
    },

    .format_trace = format_pds_flow_classify_trace,
};

static void
pds_session_update_data(u32 ses_id, u32 pindex, u32 sindex,
                        bool iflow, bool move_complete)
{
    pds_flow_hw_ctx_t *session;
    pds_flow_index_t *index;

    if (move_complete) {
        session = pds_flow_get_hw_ctx(ses_id);
        pds_flow_hw_ctx_unlock(session);
        return;
    }

    session = pds_flow_get_hw_ctx_lock(ses_id);
    index = iflow ? &session->iflow : &session->rflow;
    if (pindex != ((u32) (~0L))) {
        index->table_id = pindex;
        index->primary = 1;
    } else {
        index->table_id = sindex;
        index->primary = 0;
    }
}

void
pds_packet_type_flags_build (void)
{
    pds_flow_main_t *fm = &pds_flow_main;

    fm->packet_types = vec_new(u16, PDS_FLOW_PKT_TYPE_MAX);
    vec_elt(fm->packet_types, PDS_FLOW_L2L_INTER_SUBNET) = PDS_PKT_TYPE_L2L;
    vec_elt(fm->packet_types, PDS_FLOW_L2L_INTRA_SUBNET) = PDS_PKT_TYPE_L2L;
    vec_elt(fm->packet_types, PDS_FLOW_L2R_INTER_SUBNET) = PDS_PKT_TYPE_L2R;
    vec_elt(fm->packet_types, PDS_FLOW_L2R_INTRA_SUBNET) = PDS_PKT_TYPE_L2R;
    vec_elt(fm->packet_types, PDS_FLOW_L2N_OVERLAY_ROUTE_EN) = PDS_PKT_TYPE_N;
    vec_elt(fm->packet_types, PDS_FLOW_L2N_OVERLAY_ROUTE_EN_NAPT) =
                                                        PDS_PKT_TYPE_N;
    vec_elt(fm->packet_types, PDS_FLOW_L2N_OVERLAY_ROUTE_EN_NAT) =
                                                        PDS_PKT_TYPE_N;
    vec_elt(fm->packet_types, PDS_FLOW_L2N_OVERLAY_ROUTE_DIS) = PDS_PKT_TYPE_N;
    vec_elt(fm->packet_types, PDS_FLOW_L2N_OVERLAY_ROUTE_DIS_NAPT) =
                                                        PDS_PKT_TYPE_N;
    vec_elt(fm->packet_types, PDS_FLOW_L2N_OVERLAY_ROUTE_DIS_NAT) =
                                                        PDS_PKT_TYPE_N;
    vec_elt(fm->packet_types, PDS_FLOW_L2N_INTRA_VCN_ROUTE) = PDS_PKT_TYPE_N;
    vec_elt(fm->packet_types, PDS_FLOW_R2L_INTRA_SUBNET) = PDS_PKT_TYPE_R2L;
    vec_elt(fm->packet_types, PDS_FLOW_R2L_INTER_SUBNET) = PDS_PKT_TYPE_R2L;
    vec_elt(fm->packet_types, PDS_FLOW_N2L_OVERLAY_ROUTE_EN) = PDS_PKT_TYPE_N;
    vec_elt(fm->packet_types, PDS_FLOW_N2L_OVERLAY_ROUTE_EN_NAT) =
                                                            PDS_PKT_TYPE_N;
    vec_elt(fm->packet_types, PDS_FLOW_N2L_OVERLAY_ROUTE_DIS) = PDS_PKT_TYPE_N;
    vec_elt(fm->packet_types, PDS_FLOW_N2L_OVERLAY_ROUTE_DIS_NAT) =
                                                            PDS_PKT_TYPE_N;
    vec_elt(fm->packet_types, PDS_FLOW_N2L_INTRA_VCN_ROUTE) = PDS_PKT_TYPE_N;
    return;
}

static clib_error_t *
pds_flow_init (vlib_main_t * vm)
{
    pds_flow_main_t *fm = &pds_flow_main;
    int no_of_threads = fm->no_threads = vec_len(vlib_worker_threads);
    int i;

    pds_vpp_flow_log_init();
    fm->max_sessions = pds_session_get_max();
    pool_init_fixed(fm->session_index_pool, fm->max_sessions);
    clib_memset(fm->session_index_pool, 0,
                fm->max_sessions * sizeof(pds_flow_hw_ctx_t));

    vec_validate(fm->session_id_thr_local_pool,
                 no_of_threads - 1);
    clib_memset(fm->session_id_thr_local_pool, 0,
                (sizeof(pds_flow_session_id_thr_local_pool_t) * no_of_threads));
    pds_flow_rewrite_flags_init();
    if (0 != initialize_session()) {
        ASSERT(0);
    }

    fm->flow_prog_lock = clib_mem_alloc_aligned(CLIB_CACHE_LINE_BYTES,
                                                CLIB_CACHE_LINE_BYTES);
    clib_atomic_release(fm->flow_prog_lock);

    fm->table4 = ftlv4_create((void *) pds_flow4_key2str,
                              (void *) pds_flow_appdata2str);
    fm->table6_or_l2 = ftl_create((void *) pds_flow_key2str,
                                  (void *) pds_flow_appdata2str);
    vec_validate_init_empty(fm->stats_buf, DISPLAY_BUF_SIZE, 0);

    for (i = 0; i < no_of_threads; i++) {
        fm->session_id_thr_local_pool[i].pool_count = -1;
    }

    fm->drop_nexthop = pds_nh_drop_id_get();

    pdsa_flow_hdlr_init();
    pds_flow_dump_init();
    pds_flow_monitor_init();

    pds_flow_pipeline_init(vm);
    pds_packet_type_flags_build();

    fm->flow_metrics_hdl = pdsa_flow_stats_init();
    
    /* Create the TCP keep alive packet template */
    {
        ip4_and_tcp_header_t h;

        clib_memset (&h, 0, sizeof (h));

        /* 
         * Fill in the constant fields. Data should be empty and rest will 
         * be filled when sending the packet 
         * */
        h.ip4_hdr.ip_version_and_header_length = 0x45;
        // Length = length of IP heaer + length of TCP header because TCP
        // keepalive doesn't carry any data
        h.ip4_hdr.length = clib_host_to_net_u16(0x28);
        h.ip4_hdr.protocol = IP_PROTOCOL_TCP;
        h.ip4_hdr.ttl = 255;        
        h.tcp_hdr.flags = 1 << TCP_FLAG_BIT_ACK;
        // TCP header is 5 32-bit words as there are no options
        h.tcp_hdr.data_offset_and_reserved = 0x5 << 4;

        vlib_packet_template_init
            (vm, &fm->tcp_keepalive_packet_template,
            /* data */ &h,
            sizeof (h),
            /* alloc chunk size */ 8,
            "tcp-keepalive");
    }

    ftl_reg_session_update_cb(pds_session_update_data);
    return 0;
}

VLIB_INIT_FUNCTION (pds_flow_init) =
{
    .runs_after = VLIB_INITS("pds_infra_init"),
};
