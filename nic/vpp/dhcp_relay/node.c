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

// *INDENT-OFF* 
VLIB_PLUGIN_REGISTER () = {
    .description = "Pensando DHCP Relay Plugin",
};
// *INDENT-ON*

vlib_node_registration_t pds_dhcp_relay_clfy_node;
vlib_node_registration_t pds_dhcp_relay_svr_tx_node;
vlib_node_registration_t pds_dhcp_relay_client_tx_node;
vlib_node_registration_t pds_dhcp_relay_linux_inject_node;

dhcp_relay_main_t dhcp_relay_main;

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

always_inline bool
pds_dhcp_relay_server_found(u32 server_ip)
{
    bool svr_found = false;
    u32 *svr_ip;

    pool_foreach(svr_ip, svr_ip_list, ({
        if(*svr_ip == server_ip) {
            svr_found = true;
            return svr_found;
        }
    }));

    return svr_found;
}
always_inline void
pds_dhcp_relay_clfy_fill_next (u16 *next, p4_rx_cpu_hdr_t *hdr,
                               ip4_header_t *ip, u32 *counter)
{
    // check vpp db if sip is matching server ip
    bool svr_found = pds_dhcp_relay_server_found(ip->src_address.as_u32);

    if(!svr_found) {
        *next = PDS_DHCP_RELAY_CLFY_NEXT_TO_SERVER;
        counter[DHCP_RELAY_CLFY_COUNTER_TO_SERVER]++;

    } else {
        *next = PDS_DHCP_RELAY_CLFY_NEXT_TO_CLIENT;
        counter[DHCP_RELAY_CLFY_COUNTER_TO_CLIENT]++;
    }

    return;
}

always_inline void
pds_dhcp_relay_clfy_x2 (vlib_buffer_t *p0, vlib_buffer_t *p1,
                        u16 *next0, u16 *next1, u32 *counter)
{
    p4_rx_cpu_hdr_t *hdr0 = vlib_buffer_get_current(p0);
    p4_rx_cpu_hdr_t *hdr1 = vlib_buffer_get_current(p1);
    ip4_header_t *ip0, *ip1;

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

    ip0 = vlib_buffer_get_current(p0) + VPP_P4_TO_ARM_HDR_SZ +
          vnet_buffer (p0)->l3_hdr_offset - vnet_buffer (p0)->l2_hdr_offset;
    ip1 = vlib_buffer_get_current(p1) + VPP_P4_TO_ARM_HDR_SZ +
           vnet_buffer (p1)->l3_hdr_offset - vnet_buffer (p1)->l2_hdr_offset;

    vlib_buffer_advance(p0, pds_dhcp_relay_clfy_buffer_advance_offset(p0));
    vlib_buffer_advance(p1, pds_dhcp_relay_clfy_buffer_advance_offset(p1));

    pds_dhcp_relay_clfy_fill_next(next0, hdr0, ip0, counter);
    pds_dhcp_relay_clfy_fill_next(next1, hdr1, ip1, counter);

    return;
}

always_inline void
pds_dhcp_relay_clfy_x1 (vlib_buffer_t *p, u16 *next, u32 *counter)
{
    p4_rx_cpu_hdr_t *hdr = vlib_buffer_get_current(p);
    ip4_header_t *ip;

    vnet_buffer (p)->l2_hdr_offset = hdr->l2_offset;
    vnet_buffer (p)->l3_hdr_offset =
         hdr->l3_inner_offset ? hdr->l3_inner_offset : hdr->l3_offset;
    vnet_buffer (p)->l4_hdr_offset =
        hdr->l4_inner_offset ? hdr->l4_inner_offset : hdr->l4_offset;

    pds_dhcp_relay_fill_data(p, hdr);

    ip = vlib_buffer_get_current(p) + VPP_P4_TO_ARM_HDR_SZ +
         vnet_buffer (p)->l3_hdr_offset - vnet_buffer (p)->l2_hdr_offset;

    vlib_buffer_advance(p, pds_dhcp_relay_clfy_buffer_advance_offset(p));

    pds_dhcp_relay_clfy_fill_next(next, hdr, ip, counter);

    return;
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

always_inline int
pds_dhcp_relay_linux_inject_fd_get (void)
{
    dhcp_relay_main_t *dm = &dhcp_relay_main;
    int thread_id = vlib_get_thread_index();
    int fd;
    const int sock_opt = 1;

    if (PREDICT_TRUE(dm->inject_fds[thread_id] != -1)) {
        return dm->inject_fds[thread_id];
    }

    fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (fd < 0) {
        return -1;
    }
    // set flag so that socket expects application to frame IP4 header
    if (setsockopt(fd, IPPROTO_IP, IP_HDRINCL, &sock_opt, sizeof(sock_opt)) < 0) {
        return -1;
    }
    dm->inject_fds[thread_id] = fd;
    return fd;
}

always_inline void
pds_dhcp_relay_linux_inject_x1 (vlib_main_t *vm,
                                vlib_node_runtime_t *node,
                                vlib_buffer_t *p, u32 *counter)
{
    static struct sockaddr_in sin;
    ip4_header_t *ip4 = vlib_buffer_get_current(p);
    int fd, ret, err;

    fd = pds_dhcp_relay_linux_inject_fd_get();
    if (PREDICT_FALSE(-1 == fd)) {
        counter[DHCP_RELAY_LINUX_INJECT_COUNTER_SOCK_ERR]++;
        err = DHCP_RELAY_LINUX_INJECT_COUNTER_SOCK_ERR;
        goto end;
    }
    // set dest addr as hint to Linux kernel to route the packet
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = ip4->dst_address.as_u32;

    ret = sendto(fd, ip4, p->current_length, 0, (struct sockaddr *) &sin,
                 sizeof(struct sockaddr));
    if (ret < 0) {
        counter[DHCP_RELAY_LINUX_INJECT_COUNTER_SEND_ERR]++;
        err = DHCP_RELAY_LINUX_INJECT_COUNTER_SEND_ERR;
        goto end;
    }
    counter[DHCP_RELAY_LINUX_INJECT_COUNTER_TX]++;
    err = DHCP_RELAY_LINUX_INJECT_COUNTER_TX;

end:
    if (p->flags & VLIB_BUFFER_IS_TRACED) {
        dhcp_relay_linux_inject_trace_t *t = vlib_add_trace(vm, node, p, sizeof (t[0]));
        t->error = err;
        t->sys_errno = errno;
    }
    return;
}

always_inline void
pds_dhcp_relay_linux_inject_x2 (vlib_main_t *vm,
                                vlib_node_runtime_t *node,
                                vlib_buffer_t *p0, vlib_buffer_t *p1,
                                u32 *counter)
{
    static struct sockaddr_in sin0, sin1;
    ip4_header_t *ip40 = vlib_buffer_get_current(p0);
    ip4_header_t *ip41 = vlib_buffer_get_current(p1);
    int fd, ret0, ret1, err1, err2, errno1 = 0, errno2 = 0;

    fd = pds_dhcp_relay_linux_inject_fd_get();
    if (PREDICT_FALSE(-1 == fd)) {
        counter[DHCP_RELAY_LINUX_INJECT_COUNTER_SOCK_ERR] += 2;
        err1 = err2 = DHCP_RELAY_LINUX_INJECT_COUNTER_SOCK_ERR;
        goto end;
    }
    // set dest addr as hint to Linux kernel to route the packet
    sin0.sin_family = AF_INET;
    sin1.sin_family = AF_INET;
    sin0.sin_addr.s_addr = ip40->dst_address.as_u32;
    sin1.sin_addr.s_addr = ip41->dst_address.as_u32;

    ret0 = sendto(fd, ip40, p0->current_length, 0, (struct sockaddr *) &sin0,
                  sizeof(struct sockaddr));
    errno1 = errno;
    ret1 = sendto(fd, ip41, p1->current_length, 0, (struct sockaddr *) &sin1,
                  sizeof(struct sockaddr));
    errno2 = errno;
    if (ret0 < 0 && ret1 < 0) {
        counter[DHCP_RELAY_LINUX_INJECT_COUNTER_SEND_ERR] += 2;
        err1 = err2 = DHCP_RELAY_LINUX_INJECT_COUNTER_SEND_ERR;
        goto end;
    }

    if (ret0 > 0) {
        counter[DHCP_RELAY_LINUX_INJECT_COUNTER_TX]++;
        err1 = DHCP_RELAY_LINUX_INJECT_COUNTER_TX;
    } else {
        counter[DHCP_RELAY_LINUX_INJECT_COUNTER_SEND_ERR]++;
        err2 = DHCP_RELAY_LINUX_INJECT_COUNTER_SEND_ERR;
    }
    if (ret1 > 0) {
        counter[DHCP_RELAY_LINUX_INJECT_COUNTER_TX]++;
        err2 = DHCP_RELAY_LINUX_INJECT_COUNTER_TX;
    } else {
        counter[DHCP_RELAY_LINUX_INJECT_COUNTER_SEND_ERR]++;
        err2 = DHCP_RELAY_LINUX_INJECT_COUNTER_SEND_ERR;
    }

end:
    if (p0->flags & VLIB_BUFFER_IS_TRACED) {
        dhcp_relay_linux_inject_trace_t *t0 = vlib_add_trace(vm, node, p0, sizeof(t0[0]));
        t0->error = err1;
        t0->sys_errno = errno1;
    }
    if (p1->flags & VLIB_BUFFER_IS_TRACED) {
        dhcp_relay_linux_inject_trace_t *t1 = vlib_add_trace(vm, node, p1, sizeof(t1[0]));
        t1->error = err2;
        t1->sys_errno = errno2;
    }
    return;
}

static uword
pds_dhcp_relay_linux_inject (vlib_main_t *vm,
                             vlib_node_runtime_t *node,
                             vlib_frame_t *from_frame)
{
    u32 counter[DHCP_RELAY_LINUX_INJECT_COUNTER_LAST] = {0};

    PDS_PACKET_LOOP_START {
        PDS_PACKET_DUAL_LOOP_START(READ, READ) {
            pds_dhcp_relay_linux_inject_x2(vm, node,
                                           PDS_PACKET_BUFFER(0),
                                           PDS_PACKET_BUFFER(1),
                                           counter);
        } PDS_PACKET_DUAL_LOOP_END;
        PDS_PACKET_SINGLE_LOOP_START {
            pds_dhcp_relay_linux_inject_x1(vm, node,
                                           PDS_PACKET_BUFFER(0),
                                           counter);
        } PDS_PACKET_SINGLE_LOOP_END;
    } PDS_PACKET_LOOP_END_NO_ENQUEUE;

#define _(n, s) \
    vlib_node_increment_counter (vm, pds_dhcp_relay_linux_inject_node.index,   \
                                 DHCP_RELAY_LINUX_INJECT_COUNTER_##n,          \
                                 counter[DHCP_RELAY_LINUX_INJECT_COUNTER_##n]);
    foreach_dhcp_relay_linux_inject_counter
#undef _

    vlib_buffer_free(vm, PDS_PACKET_BUFFER_INDEX_PTR(0), from_frame->n_vectors);
    return from_frame->n_vectors;
}

static char * pds_dhcp_relay_linux_inject_error_strings[] = {
#define _(n,s) s,
    foreach_dhcp_relay_linux_inject_counter
#undef _
};

static u8 *
format_pds_dhcp_relay_linux_inject_trace (u8 *s, va_list *args)
{
    CLIB_UNUSED (vlib_main_t * vm) = va_arg(*args, vlib_main_t *);
    CLIB_UNUSED (vlib_node_t * node) = va_arg(*args, vlib_node_t *);
    dhcp_relay_linux_inject_trace_t *t =
                            va_arg(*args, dhcp_relay_linux_inject_trace_t *);

    s = format(s, "Error - %s, errno - %s",
               pds_dhcp_relay_linux_inject_error_strings[t->error],
               strerror(t->sys_errno));
    return s;
}

VLIB_REGISTER_NODE (pds_dhcp_relay_linux_inject_node) = {
    .function = pds_dhcp_relay_linux_inject,
    .name = "pds-dhcp-relay-linux-inject",
    /* Takes a vector of packets. */
    .vector_size = sizeof (u32),

    .n_errors = DHCP_RELAY_LINUX_INJECT_COUNTER_LAST,
    .error_strings = pds_dhcp_relay_linux_inject_error_strings,
    .format_trace = format_pds_dhcp_relay_linux_inject_trace,
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

    pds_dhcp_relay_svr_fill_tx_hdr_x2(p0, p1);

    pds_dhcp_relay_svr_hdr_fill_next(next0, counter);
    pds_dhcp_relay_svr_hdr_fill_next(next1, counter);

    return;
}

always_inline void
pds_dhcp_relay_svr_hdr_x1 (vlib_buffer_t *p, u16 *next, u32 *counter)
{
    vlib_buffer_advance(p, -(pds_dhcp_relay_svr_hdr_buffer_advance_offset(p)));

    pds_dhcp_relay_svr_fill_tx_hdr_x1(p);

    pds_dhcp_relay_svr_hdr_fill_next(next, counter);

    return;
}
    
static uword
pds_dhcp_relay_svr_hdr (vlib_main_t *vm,
                        vlib_node_runtime_t *node,
                        vlib_frame_t *from_frame)
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

VLIB_REGISTER_NODE(pds_dhcp_relay_svr_tx_node) = {
    .function = pds_dhcp_relay_svr_hdr,
    .name = "pds-dhcp-relay-svr-p4-inject",
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
    return (VPP_ARM_TO_P4_HDR_SZ);
}

always_inline void
pds_dhcp_relay_client_hdr_fill_next (u16 *next, u32 *counter, bool error)
{
    if (!error) {
        *next = PDS_DHCP_RELAY_CLIENT_HDR_NEXT_INTF_OUT;
        counter[DHCP_RELAY_CLIENT_HDR_COUNTER_TX]++;
    } else {
        *next = PDS_DHCP_RELAY_CLIENT_HDR_NEXT_DROP;
    }

    return;
}

always_inline void
pds_dhcp_relay_client_hdr_x2 (vlib_buffer_t *p0, vlib_buffer_t *p1,
                              u16 *next0, u16 *next1, u32 *counter)
{
    bool error0, error1;
    vlib_buffer_advance(p0, -(pds_dhcp_relay_client_hdr_buffer_advance_offset(p0)));
    vlib_buffer_advance(p1, -(pds_dhcp_relay_client_hdr_buffer_advance_offset(p1)));

    // TODO need to fill all the p4 tx header fileds 

    pds_dhcp_relay_client_fill_tx_hdr_x2(p0, p1, &error0, &error1);

    pds_dhcp_relay_client_hdr_fill_next(next0, counter, error0);
    pds_dhcp_relay_client_hdr_fill_next(next1, counter, error1);

    return;
}
    
always_inline void
pds_dhcp_relay_client_hdr_x1 (vlib_buffer_t *p, u16 *next, u32 *counter)
{
    bool error;
    vlib_buffer_advance(p, -(pds_dhcp_relay_client_hdr_buffer_advance_offset(p)));

    // TODO need to fill all the p4 tx header fileds 
    pds_dhcp_relay_client_fill_tx_hdr_x1(p, &error);

    pds_dhcp_relay_client_hdr_fill_next(next, counter, error);

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
            // BD VNID takes 3 bytes and vnicid takes 2 bytes
            o->data[len++] = PDS_DHCP_OPT_82_CIRC_ID_LEN;
            clib_memcpy(&o->data[len], &l2_vni, 3);  // l2 vnid
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
pds_dhcp_client_hdr_callback (vlib_main_t * vm, vlib_buffer_t *b)
{
    udp_header_t *u0;
    ip4_header_t *ip0;
    u32 old0, new0;
    ip_csum_t sum0;
    u16 vnic_id;
    pds_impl_db_vnic_entry_t *vnic_info = NULL;
    pds_impl_db_subnet_entry_t *subnet_info = NULL;

    ethernet_header_t *eth0;

    ip0 = vlib_buffer_get_current(b);
    u0 = (udp_header_t *)(ip0 + 1);

    vlib_buffer_advance(b, - sizeof (ethernet_header_t));
    eth0 = vlib_buffer_get_current(b);

    // extract vnic and subnet info
    vnic_id = vnet_buffer(b)->pds_dhcp_data.vnic_id;
    vnic_info = pds_impl_db_vnic_get(vnic_id);
    if(vnic_info == NULL) {
        return 1;
    }
    subnet_info = pds_impl_db_subnet_get(vnic_info->subnet_hw_id);
    if(subnet_info == NULL) {
        return 1;
    }

    // set dest port
    u0->checksum = 0;
    u0->dst_port = clib_net_to_host_u16(UDP_DST_PORT_dhcp_to_client);

    // ip header filling
    sum0 = ip0->checksum;
    old0 = ip0->dst_address.as_u32;
    new0 = 0xFFFFFFFF;
    ip0->dst_address.as_u32 = new0;
    sum0 = ip_csum_update (sum0, old0, new0, ip4_header_t /* structure */ ,
                           dst_address /* offset of changed member */ );
    ip0->checksum = ip_csum_fold (sum0);

    sum0 = ip0->checksum;
    old0 = ip0->src_address.as_u32;
    new0 = subnet_info->vr_ip.ip4.as_u32;
    new0 = clib_host_to_net_u32(new0);
    ip0->src_address.as_u32 = new0;
    sum0 = ip_csum_update (sum0, old0, new0, ip4_header_t /* structure */ ,
                           src_address /* offset of changed member */ );
    ip0->checksum = ip_csum_fold (sum0);

    // eth header filling
    eth0->type =  clib_net_to_host_u16 (ETHERNET_TYPE_IP4);
    clib_memcpy(eth0->src_address, subnet_info->mac, ETH_ADDR_LEN);
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

static clib_error_t *
pds_dhcp_relay_init (vlib_main_t * vm)
{
    int no_threads = vec_len(vlib_worker_threads);

    clib_memset(&dhcp_relay_main, 0, sizeof(dhcp_relay_main_t));
    vec_validate_init_empty(dhcp_relay_main.inject_fds, (no_threads - 1), -1);

    pds_dhcp_relay_pipeline_init();

    dhcp_register_server_next_node_tx(vm, (u8 *) "pds-dhcp-relay-linux-inject");

    dhcp_register_client_next_node_tx(vm, (u8 *) "pds-dhcp-relay-client-tx");

    dhcp_register_custom_options_cb(&pds_dhcp_options_callback);

    dhcp_register_custom_client_hdr_cb(&pds_dhcp_client_hdr_callback);

    dhcp_register_extract_circ_id_cb(&pds_dhcp_extract_circ_id_callback);

    pds_dhcp_relay_cfg_init();

    return 0;
}

VLIB_INIT_FUNCTION (pds_dhcp_relay_init) =
{
    .runs_after = VLIB_INITS("pds_infra_init", "dhcp4_proxy_init"),
};
