/*
 *  {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
 */

#include <vnet/plugin/plugin.h>
#include <vnet/ip/ip4_packet.h>
#include <nic/vpp/infra/api/intf.h>
#include <init.h>
#include <api.h>
#include <vnic.h>
#include <pdsa_impl_db_hdlr.h>
#include "node.h"

VLIB_PLUGIN_REGISTER () = {
    .description = "Pensando Infra Plugin",
};

static pds_infra_main_t infra_main;
static vlib_node_registration_t pds_ip4_linux_inject_node;
static vlib_node_registration_t pds_p4cpu_hdr_lookup_node;
static vlib_node_registration_t pds_nh_tx_node;
static vlib_node_registration_t pds_vnic_l2_rewrite_node;
static vlib_node_registration_t pds_error_drop_node;

// ipc init routine
extern int pds_vpp_ipc_init(void);
// VPP UDS init
extern int vpp_uds_init(void);

always_inline int
pds_ip4_linux_inject_fd_get (void)
{
    pds_infra_main_t *im = &infra_main;
    int thread_id = vlib_get_thread_index();
    int fd;
    const int sock_opt = 1;

    if (PREDICT_TRUE(im->ip4_linux_inject_fds[thread_id] != -1)) {
        return im->ip4_linux_inject_fds[thread_id];
    }

    fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (fd < 0) {
        return -1;
    }
    // set flag so that socket expects application to frame IP4 header
    if (setsockopt(fd, IPPROTO_IP, IP_HDRINCL, &sock_opt,
                   sizeof(sock_opt)) < 0) {
        return -1;
    }
    im->ip4_linux_inject_fds[thread_id] = fd;
    return fd;
}

always_inline void
pds_ip4_linux_inject_x1 (vlib_main_t *vm,
                         vlib_node_runtime_t *node,
                         vlib_buffer_t *p, u32 *counter)
{
    static struct sockaddr_in sin;
    ip4_header_t *ip4 = vlib_buffer_get_current(p);
    int fd, ret, err;

    fd = pds_ip4_linux_inject_fd_get();
    if (PREDICT_FALSE(-1 == fd)) {
        counter[IP4_LINUX_INJECT_COUNTER_SOCK_ERR]++;
        err = IP4_LINUX_INJECT_COUNTER_SOCK_ERR;
        goto end;
    }
    // set dest addr as hint to Linux kernel to route the packet
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = ip4->dst_address.as_u32;

    ret = sendto(fd, ip4, p->current_length, 0, (struct sockaddr *) &sin,
                 sizeof(struct sockaddr));
    if (ret < 0) {
        counter[IP4_LINUX_INJECT_COUNTER_SEND_ERR]++;
        err = IP4_LINUX_INJECT_COUNTER_SEND_ERR;
        goto end;
    }
    counter[IP4_LINUX_INJECT_COUNTER_TX]++;
    err = IP4_LINUX_INJECT_COUNTER_TX;

end:
    if (p->flags & VLIB_BUFFER_IS_TRACED) {
        ip4_linux_inject_trace_t *t = vlib_add_trace(vm, node, p, sizeof (t[0]));
        t->error = err;
        t->sys_errno = errno;
    }
    return;
}

always_inline void
pds_ip4_linux_inject_x2 (vlib_main_t *vm,
                         vlib_node_runtime_t *node,
                         vlib_buffer_t *p0, vlib_buffer_t *p1,
                         u32 *counter)
{
    static struct sockaddr_in sin0, sin1;
    ip4_header_t *ip40 = vlib_buffer_get_current(p0);
    ip4_header_t *ip41 = vlib_buffer_get_current(p1);
    int fd, ret0, ret1, err1, err2, errno1 = 0, errno2 = 0;

    fd = pds_ip4_linux_inject_fd_get();
    if (PREDICT_FALSE(-1 == fd)) {
        counter[IP4_LINUX_INJECT_COUNTER_SOCK_ERR] += 2;
        err1 = err2 = IP4_LINUX_INJECT_COUNTER_SOCK_ERR;
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
        counter[IP4_LINUX_INJECT_COUNTER_SEND_ERR] += 2;
        err1 = err2 = IP4_LINUX_INJECT_COUNTER_SEND_ERR;
        goto end;
    }

    if (ret0 > 0) {
        counter[IP4_LINUX_INJECT_COUNTER_TX]++;
        err1 = IP4_LINUX_INJECT_COUNTER_TX;
    } else {
        counter[IP4_LINUX_INJECT_COUNTER_SEND_ERR]++;
        err2 = IP4_LINUX_INJECT_COUNTER_SEND_ERR;
    }
    if (ret1 > 0) {
        counter[IP4_LINUX_INJECT_COUNTER_TX]++;
        err2 = IP4_LINUX_INJECT_COUNTER_TX;
    } else {
        counter[IP4_LINUX_INJECT_COUNTER_SEND_ERR]++;
        err2 = IP4_LINUX_INJECT_COUNTER_SEND_ERR;
    }

end:
    if (p0->flags & VLIB_BUFFER_IS_TRACED) {
        ip4_linux_inject_trace_t *t0 = vlib_add_trace(vm, node, p0, sizeof(t0[0]));
        t0->error = err1;
        t0->sys_errno = errno1;
    }
    if (p1->flags & VLIB_BUFFER_IS_TRACED) {
        ip4_linux_inject_trace_t *t1 = vlib_add_trace(vm, node, p1, sizeof(t1[0]));
        t1->error = err2;
        t1->sys_errno = errno2;
    }
    return;
}

static uword
pds_ip4_linux_inject (vlib_main_t *vm,
                      vlib_node_runtime_t *node,
                      vlib_frame_t *from_frame)
{
    u32 counter[IP4_LINUX_INJECT_COUNTER_LAST] = {0};

    PDS_PACKET_LOOP_START {
        PDS_PACKET_DUAL_LOOP_START(READ, READ) {
            pds_ip4_linux_inject_x2(vm, node,
                                    PDS_PACKET_BUFFER(0),
                                    PDS_PACKET_BUFFER(1),
                                    counter);
        } PDS_PACKET_DUAL_LOOP_END;
        PDS_PACKET_SINGLE_LOOP_START {
            pds_ip4_linux_inject_x1(vm, node,
                                    PDS_PACKET_BUFFER(0),
                                    counter);
        } PDS_PACKET_SINGLE_LOOP_END;
    } PDS_PACKET_LOOP_END_NO_ENQUEUE;

#define _(n, s) \
    vlib_node_increment_counter (vm, pds_ip4_linux_inject_node.index,   \
                                 IP4_LINUX_INJECT_COUNTER_##n,          \
                                 counter[IP4_LINUX_INJECT_COUNTER_##n]);
    foreach_ip4_linux_inject_counter
#undef _

    vlib_buffer_free(vm, PDS_PACKET_BUFFER_INDEX_PTR(0), from_frame->n_vectors);
    return from_frame->n_vectors;
}

static char * pds_ip4_linux_inject_error_strings[] = {
#define _(n,s) s,
    foreach_ip4_linux_inject_counter
#undef _
};

static u8 *
format_pds_ip4_linux_inject_trace (u8 *s, va_list *args)
{
    CLIB_UNUSED (vlib_main_t * vm) = va_arg(*args, vlib_main_t *);
    CLIB_UNUSED (vlib_node_t * node) = va_arg(*args, vlib_node_t *);
    ip4_linux_inject_trace_t *t =
                            va_arg(*args, ip4_linux_inject_trace_t *);

    s = format(s, "Status - %s",
               pds_ip4_linux_inject_error_strings[t->error]);
    if (IP4_LINUX_INJECT_COUNTER_TX != t->error) {
        s = format(s, " errno - %s", strerror(t->sys_errno));
    }
    return s;
}

VLIB_REGISTER_NODE(pds_ip4_linux_inject_node, static) = {
    .function = pds_ip4_linux_inject,
    .name = "pds-ip4-linux-inject",
    /* Takes a vector of packets. */
    .vector_size = sizeof (u32),

    .n_errors = IP4_LINUX_INJECT_COUNTER_LAST,
    .error_strings = pds_ip4_linux_inject_error_strings,
    .format_trace = format_pds_ip4_linux_inject_trace,
};

static u8 *
format_pds_p4cpu_hdr_lookup_trace (u8 * s, va_list * args)
{
    CLIB_UNUSED (vlib_main_t * vm) = va_arg (*args, vlib_main_t *);
    CLIB_UNUSED (vlib_node_t * node) = va_arg (*args, vlib_node_t *);
    p4_rx_cpu_hdr_t *t = va_arg (*args, p4_rx_cpu_hdr_t *);

    s = format(s, "%U", format_pds_p4_rx_cpu_hdr, t);
    return s;
}

void
pds_p4cpu_hdr_lookup_trace_add (vlib_main_t *vm,
                                vlib_node_runtime_t *node,
                                vlib_frame_t *from_frame)
{
    PDS_PACKET_TRACE_LOOP_START {
        PDS_PACKET_TRACE_DUAL_LOOP_START {
            vlib_buffer_t *b0, *b1;
            p4_rx_cpu_hdr_t *t0, *t1;

            b0 = PDS_PACKET_BUFFER(0);
            b1 = PDS_PACKET_BUFFER(1);

            if (b0->flags & VLIB_BUFFER_IS_TRACED)
            {
                t0 = vlib_add_trace(vm, node, b0, sizeof (t0[0]));
                clib_memcpy(t0, vlib_buffer_get_current(b0), sizeof(t0[0]));
            }
            if (b1->flags & VLIB_BUFFER_IS_TRACED)
            {
                t1 = vlib_add_trace(vm, node, b1, sizeof (t1[0]));
                clib_memcpy(t1, vlib_buffer_get_current(b1), sizeof(t1[0]));
            }
        } PDS_PACKET_TRACE_DUAL_LOOP_END;
        PDS_PACKET_TRACE_SINGLE_LOOP_START {
            vlib_buffer_t *b0;
            p4_rx_cpu_hdr_t *t0;

            b0 = PDS_PACKET_BUFFER(0);

            if (b0->flags & VLIB_BUFFER_IS_TRACED)
            {
                t0 = vlib_add_trace(vm, node, b0, sizeof (t0[0]));
                clib_memcpy(t0, vlib_buffer_get_current(b0), sizeof(t0[0]));
            }
        } PDS_PACKET_TRACE_SINGLE_LOOP_END;
    } PDS_PACKET_TRACE_LOOP_END;
}

always_inline void
pds_infra_adjust_packet_offset_x2 (vlib_buffer_t *p0, vlib_buffer_t *p1,
                                   pds_packet_offset offset0,
                                   pds_packet_offset offset1)
{
    // TODO: Implement offset adjustment here
    return;
}

always_inline void
pds_infra_adjust_packet_offset_x1 (vlib_buffer_t *p0,
                                   pds_packet_offset offset0)
{
    // TODO: Implement offset adjustment here
    return;
}

static uword
pds_p4cpu_hdr_lookup (vlib_main_t *vm,
                      vlib_node_runtime_t *node,
                      vlib_frame_t *from_frame)
{
    u32 counter[P4CPU_HDR_LOOKUP_COUNTER_LAST] = {0};
    u32 n_enq = 0;
    u16 thread_ids[VLIB_FRAME_SIZE], *thread = thread_ids;
    u16 next_enqueue_pak = 0, curr_pak = 0;
    u32 last_frame_queue_id = ~0;

    if (node->flags & VLIB_NODE_FLAG_TRACE) {
        pds_p4cpu_hdr_lookup_trace_add(vm, node, from_frame);
    }

    PDS_PACKET_LOOP_START {
        PDS_PACKET_DUAL_LOOP_START(READ, READ) {
            u16 nacl_data0, nacl_data1;
            pds_packet_offset offset0, offset1;
            u32 frame_queue_index0 = ~0, frame_queue_index1 = ~0;

            pds_get_nacl_data_x2(PDS_PACKET_BUFFER(0),
                                 PDS_PACKET_BUFFER(1),
                                 &nacl_data0, &nacl_data1);
            pds_get_handoff_data_x2(nacl_data0, nacl_data1,
                                    PDS_PACKET_NEXT_NODE_PTR(0),
                                    PDS_PACKET_NEXT_NODE_PTR(1),
                                    &frame_queue_index0,
                                    &frame_queue_index1,
                                    thread, thread+1,
                                    &offset0, &offset1);
            pds_infra_adjust_packet_offset_x2(PDS_PACKET_BUFFER(0),
                                              PDS_PACKET_BUFFER(1),
                                              offset0, offset1);
            if (PREDICT_FALSE(thread[0] != vm->thread_index)) {
                if (last_frame_queue_id == (u32)~0) {
                    if (curr_pak - next_enqueue_pak) {
                        vlib_buffer_enqueue_to_next(
                            vm, node,
                            PDS_PACKET_BUFFER_INDEX_PTR(next_enqueue_pak),
                            PDS_PACKET_NEXT_NODE_ARR + next_enqueue_pak,
                            curr_pak - next_enqueue_pak);
                        next_enqueue_pak = curr_pak;
                    }
                    last_frame_queue_id = frame_queue_index0;
                } else if (last_frame_queue_id != frame_queue_index0) {
                    last_frame_queue_id = frame_queue_index0;
                    if (curr_pak - next_enqueue_pak) {
                        n_enq = vlib_buffer_enqueue_to_thread(
                                vm, frame_queue_index0,
                                PDS_PACKET_BUFFER_INDEX_PTR(next_enqueue_pak),
                                thread_ids + next_enqueue_pak,
                                curr_pak - next_enqueue_pak, 1);
                        next_enqueue_pak = curr_pak;
                    }
                }
            } else {
                if (last_frame_queue_id != (u32)~0) {
                    if (curr_pak - next_enqueue_pak) {
                        n_enq = vlib_buffer_enqueue_to_thread(
                                vm, frame_queue_index0,
                                PDS_PACKET_BUFFER_INDEX_PTR(next_enqueue_pak),
                                thread_ids + next_enqueue_pak,
                                curr_pak - next_enqueue_pak, 1);
                        next_enqueue_pak = curr_pak;
                    }
                    last_frame_queue_id = ~0;
                }
            }
            curr_pak++;
            if (PREDICT_FALSE(thread[1] != vm->thread_index)) {
                if (last_frame_queue_id == (u32)~0) {
                    if (curr_pak - next_enqueue_pak) {
                        vlib_buffer_enqueue_to_next(
                                vm, node,
                                PDS_PACKET_BUFFER_INDEX_PTR(next_enqueue_pak),
                                PDS_PACKET_NEXT_NODE_ARR + next_enqueue_pak,
                                curr_pak - next_enqueue_pak);
                        next_enqueue_pak = curr_pak;
                    }
                    last_frame_queue_id = frame_queue_index1;
                } else if (last_frame_queue_id != frame_queue_index1) {
                    last_frame_queue_id = frame_queue_index1;
                    if (curr_pak - next_enqueue_pak) {
                        n_enq = vlib_buffer_enqueue_to_thread(
                                vm, frame_queue_index1,
                                PDS_PACKET_BUFFER_INDEX_PTR(next_enqueue_pak),
                                thread_ids + next_enqueue_pak,
                                curr_pak - next_enqueue_pak, 1);
                        next_enqueue_pak = curr_pak;
                    }
                }
            } else {
                if (last_frame_queue_id != (u32)~0) {
                    if (curr_pak - next_enqueue_pak) {
                        n_enq = vlib_buffer_enqueue_to_thread(
                                vm, frame_queue_index1,
                                PDS_PACKET_BUFFER_INDEX_PTR(next_enqueue_pak),
                                thread_ids + next_enqueue_pak,
                                curr_pak - next_enqueue_pak, 1);
                        next_enqueue_pak = curr_pak;
                    }
                    last_frame_queue_id = ~0;
                }
            }
            curr_pak++;
            thread += 2;
        } PDS_PACKET_DUAL_LOOP_END;
        PDS_PACKET_SINGLE_LOOP_START {
            u16 nacl_data0;
            pds_packet_offset offset0;
            u32 frame_queue_index0 = ~0;
            pds_get_nacl_data_x1(PDS_PACKET_BUFFER(0), &nacl_data0);
            pds_get_handoff_data_x1(nacl_data0,
                                    PDS_PACKET_NEXT_NODE_PTR(0),
                                    &frame_queue_index0,
                                    thread,
                                    &offset0);
            pds_infra_adjust_packet_offset_x1(PDS_PACKET_BUFFER(0),
                                              offset0);
            if (PREDICT_FALSE(thread[0] != vm->thread_index)) {
                if (last_frame_queue_id == (u32)~0) {
                    if (curr_pak - next_enqueue_pak) {
                        vlib_buffer_enqueue_to_next(
                                vm, node,
                                PDS_PACKET_BUFFER_INDEX_PTR(next_enqueue_pak),
                                PDS_PACKET_NEXT_NODE_ARR + next_enqueue_pak,
                                curr_pak - next_enqueue_pak);
                        next_enqueue_pak = curr_pak;
                    }
                    last_frame_queue_id = frame_queue_index0;
                } else if (last_frame_queue_id != frame_queue_index0) {
                    last_frame_queue_id = frame_queue_index0;
                    if (curr_pak - next_enqueue_pak) {
                        n_enq = vlib_buffer_enqueue_to_thread(
                                vm, frame_queue_index0,
                                PDS_PACKET_BUFFER_INDEX_PTR(next_enqueue_pak),
                                thread_ids + next_enqueue_pak,
                                curr_pak - next_enqueue_pak, 1);
                        next_enqueue_pak = curr_pak;
                    }
                }
            } else {
                if (last_frame_queue_id != (u32)~0) {
                    if (curr_pak - next_enqueue_pak) {
                        n_enq = vlib_buffer_enqueue_to_thread(
                                vm, frame_queue_index0,
                                PDS_PACKET_BUFFER_INDEX_PTR(next_enqueue_pak),
                                thread_ids + next_enqueue_pak,
                                curr_pak - next_enqueue_pak, 1);
                        next_enqueue_pak = curr_pak;
                    }
                    last_frame_queue_id = ~0;
                }
            }
            curr_pak++;
            thread++;
        } PDS_PACKET_SINGLE_LOOP_END;
    } PDS_PACKET_LOOP_END_NO_ENQUEUE;

    if (last_frame_queue_id == (u32)~0) {
        if (curr_pak - next_enqueue_pak) {
            vlib_buffer_enqueue_to_next(vm, node,
                    PDS_PACKET_BUFFER_INDEX_PTR(next_enqueue_pak),
                    PDS_PACKET_NEXT_NODE_ARR + next_enqueue_pak,
                    curr_pak - next_enqueue_pak);
            next_enqueue_pak = curr_pak;
        }
    } else if (curr_pak - next_enqueue_pak) {
        n_enq = vlib_buffer_enqueue_to_thread(
                vm, last_frame_queue_id,
                PDS_PACKET_BUFFER_INDEX_PTR(next_enqueue_pak),
                thread_ids + next_enqueue_pak,
                curr_pak - next_enqueue_pak, 1);
        next_enqueue_pak = curr_pak;
    }
    counter[1] = n_enq;
#define _(n, s) \
    vlib_node_increment_counter (vm, pds_p4cpu_hdr_lookup_node.index,   \
            P4CPU_HDR_LOOKUP_COUNTER_##n,                               \
            counter[P4CPU_HDR_LOOKUP_COUNTER_##n]);
    foreach_p4cpu_hdr_lookup_counter
#undef _

    return from_frame->n_vectors;
}

static char * p4cpu_error_strings[] = {
#define _(n,s) s,
    foreach_p4cpu_hdr_lookup_counter
#undef _
};

VLIB_REGISTER_NODE(pds_p4cpu_hdr_lookup_node, static) = {
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

always_inline void
pds_vnic_l2_rewrite_internal (vlib_buffer_t *p, u16 *next, u32 *counter)
{
    // Ethernet
    u16 vnic_id;
    u8 *eth0;
    u8 *rewrite;
    u16 nh_id;
    word rewrite_len;

    // extract vnic and subnet info
    vnic_id = vnet_buffer(p)->pds_tx_data.vnic_id;
    rewrite = pds_vnic_l2_rewrite_info_get(vnic_id, &nh_id);
    if (!rewrite) {
        *next = VNIC_L2_REWRITE_NEXT_UNKNOWN;
        counter[VNIC_L2_REWRITE_COUNTER_VNIC_NOT_FOUND] += 1;
        return;
    }

    rewrite_len = vec_len(rewrite);
    vlib_buffer_advance(p, (-rewrite_len));
    eth0 = vlib_buffer_get_current(p);
    clib_memcpy(eth0, rewrite, rewrite_len);
    if (vnet_buffer(p)->pds_tx_data.ether_type != 0) {
        u16 *eth_type = (u16 *) (rewrite + rewrite_len - sizeof(u16));
        *eth_type = clib_net_to_host_u16(vnet_buffer(p)->pds_tx_data.ether_type);
    }
    vnet_buffer(p)->pds_tx_data.nh_id = nh_id;
    *next = VNIC_L2_REWRITE_NEXT_TX_OUT;
    counter[VNIC_L2_REWRITE_COUNTER_TX_OUT] += 1;

    vlib_buffer_advance(p, - VPP_ARM_TO_P4_HDR_SZ);
    return;
}

static uword
pds_vnic_l2_rewrite (vlib_main_t *vm,
                     vlib_node_runtime_t *node,
                     vlib_frame_t *from_frame)
{
    u32 counter[VNIC_L2_REWRITE_COUNTER_LAST] = {0};
    u32 *from;

    from = vlib_frame_vector_args (from_frame);
    if (node->flags & VLIB_NODE_FLAG_TRACE)
        vlib_trace_frame_buffers_only (vm, node, from, from_frame->n_vectors,
                                       /* stride */ 1,
                                       sizeof (vnic_l2_rewrite_trace_t));

    PDS_PACKET_LOOP_START {

        PDS_PACKET_DUAL_LOOP_START(READ, READ) {
            pds_vnic_l2_rewrite_internal(PDS_PACKET_BUFFER(0),
                                   PDS_PACKET_NEXT_NODE_PTR(0), counter);
            pds_vnic_l2_rewrite_internal(PDS_PACKET_BUFFER(1),
                                   PDS_PACKET_NEXT_NODE_PTR(1), counter);
        } PDS_PACKET_DUAL_LOOP_END;

        PDS_PACKET_SINGLE_LOOP_START {
            pds_vnic_l2_rewrite_internal(PDS_PACKET_BUFFER(0),
                                   PDS_PACKET_NEXT_NODE_PTR(0), counter);
        } PDS_PACKET_SINGLE_LOOP_END;

    } PDS_PACKET_LOOP_END;

#define _(n, s)                                                         \
    vlib_node_increment_counter (vm, pds_vnic_l2_rewrite_node.index,    \
                                 VNIC_L2_REWRITE_COUNTER_##n,           \
                                 counter[VNIC_L2_REWRITE_COUNTER_##n]);
    foreach_vnic_l2_rewrite_counter
#undef _

    return from_frame->n_vectors;
}

static char * pds_vnic_l2_rewrite_error_strings[] = {
#define _(n,s) s,
    foreach_vnic_l2_rewrite_counter
#undef _
};

static u8 *
format_pds_vnic_l2_rewrite_trace (u8 *s, va_list *args)
{
    CLIB_UNUSED (vlib_main_t * vm) = va_arg(*args, vlib_main_t *);
    CLIB_UNUSED (vlib_node_t * node) = va_arg(*args, vlib_node_t *);
    vnic_l2_rewrite_trace_t *t = va_arg(*args, vnic_l2_rewrite_trace_t *);

    s = format (s, "%U",
                format_ip4_header, t->packet_data, sizeof (t->packet_data));
    return s;
}

VLIB_REGISTER_NODE(pds_vnic_l2_rewrite_node, static) = {
    .function = pds_vnic_l2_rewrite,
    .name = "pds-vnic-l2-rewrite",

    .vector_size = sizeof(u32),  // takes a vector of packets

    .n_errors = VNIC_L2_REWRITE_COUNTER_LAST,
    .error_strings = pds_vnic_l2_rewrite_error_strings,
    .n_next_nodes = VNIC_L2_REWRITE_N_NEXT,
    .next_nodes = {
#define _(s,n) [VNIC_L2_REWRITE_NEXT_##s] = n,
    foreach_vnic_l2_rewrite_next
#undef _
    },

    .format_trace = format_pds_vnic_l2_rewrite_trace,
};

always_inline void
pds_nh_tx_internal (vlib_main_t *vm, vlib_buffer_t *p, u16 *next, u32 *counter)
{
    u16 nh_id;
    vlib_node_runtime_t *node;
    static u32 cpu_mnic_if_index = ~0;

    nh_id = vnet_buffer(p)->pds_tx_data.nh_id;
    if (PREDICT_FALSE(nh_id == 0)) {
        *next = NH_TX_NEXT_DROP;
        counter[NH_TX_COUNTER_FAILED]++;
    } else {
        pds_nh_add_tx_hdrs(p, nh_id);
        if (PREDICT_FALSE(((u32) ~0) == cpu_mnic_if_index)) {
            cpu_mnic_if_index = pds_infra_get_sw_ifindex_by_name((u8*)"cpu_mnic0");
        }
        vnet_buffer(p)->sw_if_index[VLIB_TX] = cpu_mnic_if_index;
        *next = NH_TX_NEXT_INTF_OUT;
        counter[NH_TX_COUNTER_SUCCESS]++;
    }

    node = vlib_node_get_runtime(vm, pds_nh_tx_node.index);
    if (node->flags & VLIB_NODE_FLAG_TRACE) {
        nh_tx_trace_t *t0;
        if (p->flags & VLIB_BUFFER_IS_TRACED) {
            t0 = vlib_add_trace(vm, node, p, sizeof(t0[0]));
            t0->nh_id = nh_id;
        }
    }
}

static uword
pds_nh_tx (vlib_main_t *vm,
           vlib_node_runtime_t *node,
           vlib_frame_t *from_frame)
{
    u32 counter[NH_TX_COUNTER_LAST] = {0};

    PDS_PACKET_LOOP_START {

        PDS_PACKET_DUAL_LOOP_START(READ, WRITE) {
            pds_nh_tx_internal(vm, PDS_PACKET_BUFFER(0),
                               PDS_PACKET_NEXT_NODE_PTR(0), counter);
            pds_nh_tx_internal(vm, PDS_PACKET_BUFFER(1),
                               PDS_PACKET_NEXT_NODE_PTR(1), counter);
        } PDS_PACKET_DUAL_LOOP_END;

        PDS_PACKET_SINGLE_LOOP_START {
            pds_nh_tx_internal(vm, PDS_PACKET_BUFFER(0),
                               PDS_PACKET_NEXT_NODE_PTR(0), counter);
        } PDS_PACKET_SINGLE_LOOP_END;

    } PDS_PACKET_LOOP_END;

#define _(n, s)                                                     \
    vlib_node_increment_counter(vm, pds_nh_tx_node.index,           \
                                NH_TX_COUNTER_##n,                  \
                                counter[NH_TX_COUNTER_##n]);
    foreach_nh_tx_counter
#undef _

    return from_frame->n_vectors;
}

static char * pds_nh_tx_error_strings[] = {
#define _(n,s) s,
    foreach_nh_tx_counter
#undef _
};

static u8 *
format_pds_nh_tx_trace (u8 *s, va_list *args)
{
    CLIB_UNUSED(vlib_main_t * vm) = va_arg(*args, vlib_main_t *);
    CLIB_UNUSED(vlib_node_t * node) = va_arg(*args, vlib_node_t *);
    nh_tx_trace_t *t = va_arg(*args, nh_tx_trace_t *);

    s = format(s, "Nexthop id %d", t->nh_id);
    return s;
}

VLIB_REGISTER_NODE(pds_nh_tx_node, static) = {
    .function = pds_nh_tx,
    .name = "pds-nexthop-tx",

    .vector_size = sizeof(u32),  // takes a vector of packets

    .n_errors = NH_TX_COUNTER_LAST,
    .error_strings = pds_nh_tx_error_strings,
    .n_next_nodes = NH_TX_N_NEXT,
    .next_nodes = {
#define _(s,n) [NH_TX_NEXT_##s] = n,
    foreach_nh_tx_next
#undef _
    },

    .format_trace = format_pds_nh_tx_trace,
};

always_inline void
pds_drop_trace (vlib_main_t *vm,
                vlib_frame_t *f)
{
    u32 *from;
    u32 n_left = f->n_vectors;
    vlib_buffer_t *b0, *p1;
    u32 bi0;
    i16 save_current_data;
    u16 save_current_length;
    pds_infra_main_t *im = &infra_main;

    from = vlib_frame_vector_args(f);
    clib_spinlock_lock_if_init(&im->packet_dump_lock);
    if (PREDICT_FALSE(im->packet_dump_fd == NULL)) {
        im->packet_dump_fd = fopen(im->packet_dump_path, "a+");
        if (NULL == im->packet_dump_fd) {
            clib_spinlock_unlock_if_init(&im->packet_dump_lock);
            return;
        }
    }
    clib_spinlock_unlock_if_init(&im->packet_dump_lock);

    while (n_left > 0) {
        if (PREDICT_TRUE (n_left > 1)) {
            p1 = vlib_get_buffer(vm, from[1]);
            vlib_prefetch_buffer_header(p1, LOAD);
        }

        bi0 = from[0];
        b0 = vlib_get_buffer(vm, bi0);
        from++;
        n_left--;

        // save current offsets
        save_current_data = b0->current_data;
        save_current_length = b0->current_length;
        // rewind the buffer to start of packet
        if (b0->current_data > 0) {
            vlib_buffer_advance(b0, (word) - b0->current_data);
        }
        u32 n = vlib_buffer_length_in_chain(vm, b0);
        i32 n_left = clib_min(im->packet_dump_len, n);
        vlib_buffer_t *b = b0;
        clib_spinlock_lock_if_init(&im->packet_dump_lock);
        while (1) {
            u32 copy_length = clib_min((u32) n_left, b->current_length);
            char *pak = (char *) (b->data + b->current_data);
            for (int i = 0; i < copy_length; i++) {
                (void)fprintf(im->packet_dump_fd, "%02x ", pak[i]);
            }
            n_left -= b->current_length;
            if (n_left <= 0) {
                break;
            }
            ASSERT (b->flags & VLIB_BUFFER_NEXT_PRESENT);
            b = vlib_get_buffer(vm, b->next_buffer);
        }
        (void)fprintf(im->packet_dump_fd, "\n");
        clib_spinlock_unlock_if_init(&im->packet_dump_lock);
        // restore old data offsets
        b0->current_data = save_current_data;
        b0->current_length = save_current_length;
    }
    fflush(im->packet_dump_fd);
    return;
}

always_inline uword
interface_drop (vlib_main_t *vm,
                vlib_node_runtime_t *node,
                vlib_frame_t *frame)
{
    u32 *from, n_left, thread_index, *sw_if_index;
    vlib_buffer_t *bufs[VLIB_FRAME_SIZE], **b;
    u32 sw_if_indices[VLIB_FRAME_SIZE];
    vlib_simple_counter_main_t *cm;
    u16 nexts[VLIB_FRAME_SIZE];
    vnet_main_t *vnm;

    vnm = vnet_get_main ();
    thread_index = vm->thread_index;
    from = vlib_frame_vector_args (frame);
    n_left = frame->n_vectors;
    b = bufs;
    sw_if_index = sw_if_indices;

    vlib_get_buffers (vm, from, bufs, n_left);

    // all going to drop regardless, this is just a counting exercise
    clib_memset(nexts, 0, sizeof (nexts));

    cm = vec_elt_at_index(vnm->interface_main.sw_if_counters,
                          VNET_INTERFACE_COUNTER_DROP);

    // collect the array of interfaces first
    while (n_left >= 4) {
        if (n_left >= 12) {
            // prefetch 8 ahead - there's not much going on in each iteration
            vlib_prefetch_buffer_header (b[4], LOAD);
            vlib_prefetch_buffer_header (b[5], LOAD);
            vlib_prefetch_buffer_header (b[6], LOAD);
            vlib_prefetch_buffer_header (b[7], LOAD);
        }
        sw_if_index[0] = vnet_buffer (b[0])->sw_if_index[VLIB_RX];
        sw_if_index[1] = vnet_buffer (b[1])->sw_if_index[VLIB_RX];
        sw_if_index[2] = vnet_buffer (b[2])->sw_if_index[VLIB_RX];
        sw_if_index[3] = vnet_buffer (b[3])->sw_if_index[VLIB_RX];

        sw_if_index += 4;
        n_left -= 4;
        b += 4;
    }
    while (n_left) {
        sw_if_index[0] = vnet_buffer (b[0])->sw_if_index[VLIB_RX];

        sw_if_index += 1;
        n_left -= 1;
        b += 1;
    }

    // count against them in blocks
    n_left = frame->n_vectors;

    while (n_left) {
        vnet_sw_interface_t *sw_if0;
        u16 off, count;

        off = frame->n_vectors - n_left;

        sw_if_index = sw_if_indices + off;

        count = clib_count_equal_u32 (sw_if_index, n_left);
        n_left -= count;

        vlib_increment_simple_counter (cm, thread_index, sw_if_index[0], count);

        // increment super-interface drop/punt counters for
        // sub-interfaces.
        sw_if0 = vnet_get_sw_interface (vnm, sw_if_index[0]);
        if (sw_if0->sup_sw_if_index != sw_if_index[0])
            vlib_increment_simple_counter
                (cm, thread_index, sw_if0->sup_sw_if_index, count);
    }

    vlib_buffer_enqueue_to_next(vm, node, from, nexts, frame->n_vectors);

    return frame->n_vectors;
}

static uword
pds_error_drop (vlib_main_t *vm,
                vlib_node_runtime_t *node,
                vlib_frame_t *frame)
{
    pds_infra_main_t *im = &infra_main;

    if (PREDICT_FALSE(im->packet_dump_en)) { 
        (void)pds_drop_trace(vm, frame);
    }

    return interface_drop(vm, node, frame);
}

VLIB_REGISTER_NODE(pds_error_drop_node, static) = {
    .function = pds_error_drop,
    .name = "pds-error-drop",
    .vector_size = sizeof(u32),  // takes a vector of packets
    .n_next_nodes = 1,
    .next_nodes = {
        [0] = "drop",
    },
};

void
pds_packet_dump_en_dis (bool enable, char *file, u16 size)
{
    pds_infra_main_t *im = &infra_main;

    if (im->packet_dump_fd) {
        fclose(im->packet_dump_fd);
        im->packet_dump_fd = NULL;
    }

    if (file) {
        strncpy(im->packet_dump_path, file, MAX_FILE_PATH-1);
    } else {
        im->packet_dump_path[0] = 0;
    }

    im->packet_dump_len = size;
    im->packet_dump_en = enable;
}

void
pds_packet_dump_show (vlib_main_t *vm)
{
    pds_infra_main_t *im = &infra_main;

    vlib_cli_output(vm, "Dump Enable : %s",
                    im->packet_dump_en ? "True" : "False");
    vlib_cli_output(vm, "File : %s",
                    im->packet_dump_path[0] ? im->packet_dump_path : \
                    "Not configured");
    vlib_cli_output(vm, "File Descriptor : %d", im->packet_dump_fd);
    vlib_cli_output(vm, "Dump Length : %u", im->packet_dump_len);
}

/* Don't change this function name as all other plugin
 * inits have to run after this init. So all other plugins
 * Refer to this function name
 */
static clib_error_t *
pds_infra_init (vlib_main_t * vm)
{
    int ret;
    int no_threads = vec_len(vlib_worker_threads);

    clib_memset(&infra_main, 0, sizeof(pds_infra_main_t));
    vec_validate_init_empty(infra_main.ip4_linux_inject_fds,
                            (no_threads - 1), -1);
    infra_main.packet_dump_en = false;
    infra_main.packet_dump_len = DEAFULT_PACKET_DUMP_SIZE;
    clib_spinlock_init(&(infra_main.packet_dump_lock));

    ret = pds_vpp_ipc_init();
    if (ret != 0) {
        ASSERT(0);
    }

    if (0 != pds_cfg_db_init()) {
        ASSERT(0);
    }

    if (0 != pds_impl_db_init()) {
        ASSERT(0);
    }

    if (0 != initialize_pds()) {
        ASSERT(0);
    }

    if (0 != vpp_uds_init()) {
        ASSERT(0);
    }

    return 0;
}

VLIB_INIT_FUNCTION (pds_infra_init);
