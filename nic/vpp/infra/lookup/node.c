/*
 *  {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
 */

#include <vnet/plugin/plugin.h>
#include <init.h>
#include <api.h>
#include "node.h"

// *INDENT-OFF*
VLIB_PLUGIN_REGISTER () = {
    .description = "Pensando Infra Plugin",
};
// *INDENT-ON*

// ipc init routine
extern int pds_vpp_ipc_init(void);
// VPP UDS init
extern int vpp_uds_init(void);

vlib_node_registration_t pds_p4cpu_hdr_lookup_node;

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
    int ret = pds_vpp_ipc_init();

    if (ret != 0) {
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
