//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_FLOW_PLUGIN_FLOW_H__
#define __VPP_FLOW_PLUGIN_FLOW_H__

#include <vlib/vlib.h>
#include <vnet/ip/ip.h>
#include <vnet/udp/udp_packet.h>
#include <nic/p4/common/defines.h>
#include "flow_prog_hw.h"

#define MAX_SESSION_INDEX (8 * 1024 * 1024)

#define MAX_FLOWS_PER_FRAME (VLIB_FRAME_SIZE * 2)

#define foreach_p4cpu_hdr_lookup_next                               \
        _(IP4_FLOW_PROG, "pds-ip4-flow-program" )                   \
        _(IP6_FLOW_PROG, "pds-ip6-flow-program" )                   \
        _(IP4_TUN_FLOW_PROG, "pds-tunnel-ip4-flow-program" )        \
        _(IP6_TUN_FLOW_PROG, "pds-tunnel-ip6-flow-program" )        \
        _(DROP, "error-drop")                                       \

#define foreach_p4cpu_hdr_lookup_counter                            \
        _(IP4_FLOW, "IPv4 flow packets" )                           \
        _(IP6_FLOW, "IPv6 flow packets" )                           \
        _(IP4_TUN_FLOW, "IPv4 tunnel flow packets" )                \
        _(IP6_TUN_FLOW, "IPv6 tunnel flow packets" )                \
        _(UNKOWN, "Unknown flow packets")                           \

#define foreach_flow_prog_next                                      \
        _(FWD_FLOW, "pds-fwd-flow" )                                \
        _(SESSION_PROG, "pds-session-program")                      \
        _(DROP, "error-drop")                                       \

#define foreach_flow_prog_counter                                   \
        _(FLOW_SUCCESS, "Flow programming success" )                \
        _(FLOW_FAILED, "Flow programming failed")                   \
        _(FLOW_DELETE_FAILED, "Flow delete failed")                 \

#define foreach_fwd_flow_next                                       \
        _(INTF_OUT, "interface-tx" )                                \
        _(DROP, "error-drop")                                       \

#define foreach_fwd_flow_counter                                    \
        _(REWRITE_SUCCESS, "Rewrite success" )                      \
        _(REWRITE_FAILED, "Rewrite failed" )                        \

#define foreach_session_prog_next                                   \
        _(FWD_FLOW, "pds-fwd-flow" )                                \
        _(DROP, "error-drop")                                       \

#define foreach_session_prog_counter                                \
        _(SESSION_SUCCESS, "Session programming success" )          \
        _(SESSION_FAILED, "Session programming failed")             \

typedef enum
{
#define _(s,n) P4CPU_HDR_LOOKUP_NEXT_##s,
    foreach_p4cpu_hdr_lookup_next
#undef _
    P4CPU_HDR_LOOKUP_N_NEXT,
} p4cpu_hdr_lookup_next_t;

typedef enum
{
#define _(n,s) P4CPU_HDR_LOOKUP_COUNTER_##n,
    foreach_p4cpu_hdr_lookup_counter
#undef _
    P4CPU_HDR_LOOKUP_COUNTER_LAST,
} p4cpu_hdr_lookup_counter_t;

typedef enum
{
#define _(s,n) FLOW_PROG_NEXT_##s,
    foreach_flow_prog_next
#undef _
    FLOW_PROG_N_NEXT,
} flow_prog_next_t;

typedef enum
{
#define _(n,s) FLOW_PROG_COUNTER_##n,
    foreach_flow_prog_counter
#undef _
    FLOW_PROG_COUNTER_LAST,
} flow_prog_counter_t;

typedef enum
{
#define _(s,n) FWD_FLOW_NEXT_##s,
    foreach_fwd_flow_next
#undef _
    FWD_FLOW_N_NEXT,
} fwd_flow_next_t;

typedef enum
{
#define _(n,s) FWD_FLOW_COUNTER_##n,
    foreach_fwd_flow_counter
#undef _
    FWD_FLOW_COUNTER_LAST,
} fwd_flow_counter_t;

typedef enum
{
#define _(s,n) SESSION_PROG_NEXT_##s,
    foreach_session_prog_next
#undef _
    SESSION_PROG_N_NEXT,
} flow_session_next_t;

typedef enum
{
#define _(n,s) SESSION_PROG_COUNTER_##n,
    foreach_session_prog_counter
#undef _
    SESSION_PROG_COUNTER_LAST,
} flow_session_counter_t;

typedef struct fwd_flow_trace_s {
    u32 hw_index;
} fwd_flow_trace_t;

typedef struct flow_prog_trace_s {
    ip46_address_t src, dst;
    u16 src_port, dst_port;
    u8 protocol;
} flow_prog_trace_t;

typedef struct p4cpu_hdr_lookup_trace_s {
    u32 l2_offset;
    u32 l3_offset;
    u32 l4_offset;
    u32 vnic;
    u32 flow_hash;
    u32 flags;
} p4cpu_hdr_lookup_trace_t;

typedef struct session_prog_trace_s {
    u32 session_id;
} session_prog_trace_t;

typedef struct pds_flow_params_s {
    union {
        ftlv4_entry_t entry4;
        ftlv6_entry_t entry6;
    };
    u32 hash;
} pds_flow_params_t;

typedef struct pds_flow_hw_ctx_s {
    u8 dummy;
} pds_flow_hw_ctx_t;

#define PDS_FLOW_SESSION_POOL_COUNT_MAX VLIB_FRAME_SIZE

typedef struct pds_flow_session_id_thr_local_pool_s {
    int16_t         pool_count;
    u32             session_ids[PDS_FLOW_SESSION_POOL_COUNT_MAX];
} pds_flow_session_id_thr_local_pool_t;


typedef struct pds_flow_main_s {
    u64 no_threads;
    volatile u32 *flow_prog_lock;
    ftlv4 **table4;
    ftlv6 **table6;
    pds_flow_params_t **ip4_flow_params;
    pds_flow_params_t **ip6_flow_params;
    pds_flow_hw_ctx_t *session_index_pool;
    pds_flow_session_id_thr_local_pool_t *session_id_thr_local_pool;
} pds_flow_main_t;

extern pds_flow_main_t pds_flow_main;

always_inline void pds_flow_prog_lock(void)
{
    pds_flow_main_t *fm = &pds_flow_main;
    if (fm->no_threads <= 2) {
        return;
    }

    while (clib_atomic_test_and_set(fm->flow_prog_lock));
}

always_inline void pds_flow_prog_unlock(void)
{
    pds_flow_main_t *fm = &pds_flow_main;
    if (fm->no_threads <= 2) {
        return;
    }

    clib_atomic_release(fm->flow_prog_lock);
}

always_inline void * pds_flow_prog_get_table4(void)
{
    pds_flow_main_t *fm = &pds_flow_main;

    return fm->table4[vlib_get_thread_index()];
}

always_inline void * pds_flow_prog_get_table6(void)
{
    pds_flow_main_t *fm = &pds_flow_main;

    return fm->table6[vlib_get_thread_index()];
}

always_inline void pds_session_id_alloc2(u32 *ses_id0, u32 *ses_id1)
{
    pds_flow_main_t *fm = &pds_flow_main;
    pds_flow_session_id_thr_local_pool_t    *thr_local_pool = 
                     &fm->session_id_thr_local_pool[vlib_get_thread_index()];
    uint16_t        refill_count;
    pds_flow_hw_ctx_t *ctx;

    if (PREDICT_TRUE(thr_local_pool->pool_count >= 1)) {
        *ses_id1 = thr_local_pool->session_ids[thr_local_pool->pool_count - 1];
        *ses_id0 = thr_local_pool->session_ids[thr_local_pool->pool_count];
        thr_local_pool->pool_count -= 2;
        return;
    }
    /* refill pool */
    refill_count = PDS_FLOW_SESSION_POOL_COUNT_MAX;
    if (thr_local_pool->pool_count == 0) {
        refill_count--;;
    }
    pds_flow_prog_lock();
    while (refill_count) {
        if (PREDICT_FALSE(MAX_SESSION_INDEX <= pool_elts(fm->session_index_pool))) {
            break;
        }
        thr_local_pool->pool_count++;
        pool_get(fm->session_index_pool, ctx);
        thr_local_pool->session_ids[thr_local_pool->pool_count] =
             ctx - fm->session_index_pool + 1;
        refill_count--;
    }
    pds_flow_prog_unlock();
    if (PREDICT_TRUE(thr_local_pool->pool_count >= 1)) {
        *ses_id1 = thr_local_pool->session_ids[thr_local_pool->pool_count - 1];
        *ses_id0 = thr_local_pool->session_ids[thr_local_pool->pool_count];
        thr_local_pool->pool_count -= 2;
        return;
    }
    return;
}

always_inline u32 pds_session_id_alloc(void)
{
    pds_flow_main_t *fm = &pds_flow_main;
    pds_flow_session_id_thr_local_pool_t    *thr_local_pool =
       &fm->session_id_thr_local_pool[vlib_get_thread_index()];
    uint16_t        refill_count;
    pds_flow_hw_ctx_t *ctx;
    uint32_t        ret = 0;

    if (PREDICT_TRUE(thr_local_pool->pool_count >= 0)) {
        ret = thr_local_pool->session_ids[thr_local_pool->pool_count];
        thr_local_pool->pool_count--;
        return ret;
    }
    /* refill pool */
    refill_count = PDS_FLOW_SESSION_POOL_COUNT_MAX;
    pds_flow_prog_lock();
    while (refill_count) {
        if (PREDICT_FALSE(MAX_SESSION_INDEX <= pool_elts(fm->session_index_pool))) {
            break;
        }
        thr_local_pool->pool_count++;
        pool_get(fm->session_index_pool, ctx);
        thr_local_pool->session_ids[thr_local_pool->pool_count] =
            ctx - fm->session_index_pool + 1;
        refill_count--;
    }
    pds_flow_prog_unlock();
    if (PREDICT_TRUE(thr_local_pool->pool_count >= 0)) {
        ret = thr_local_pool->session_ids[thr_local_pool->pool_count];
        thr_local_pool->pool_count--;
        return ret;
    }
    return 0; //0 means invalid
}

always_inline void pds_session_id_dealloc(u32 ses_id)
{
    pds_flow_main_t *fm = &pds_flow_main;
            
    pds_flow_prog_lock();
    pool_put_index(fm->session_index_pool, (ses_id + 1));
    pds_flow_prog_unlock();
    return;
}

always_inline void pds_session_id_flush(void)
{
    pds_flow_main_t *fm = &pds_flow_main;
    pds_flow_session_id_thr_local_pool_t *thr_local_pool =
        fm->session_id_thr_local_pool;
    pds_flow_hw_ctx_t *ctx;

    pds_flow_prog_lock();
    pool_flush(ctx, fm->session_index_pool,
          ({
          }));
    for (u32 i = 0; i < vec_len(thr_local_pool); i++) {
        thr_local_pool[i].pool_count = -1;
    }
    pds_flow_prog_unlock();
    return;
}

#endif    // __VPP_FLOW_PLUGIN_FLOW_H__
