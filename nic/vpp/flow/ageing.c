//
//  {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
//

#include <vlib/vlib.h>
#include <vnet/buffer.h>
#include <session.h>
#include <system.h>
#include <feature.h>
#include <pkt.h>
#include "node.h"
#include "pdsa_hdlr.h"
#include "pdsa_uds_hdlr.h"

typedef void (flow_expiration_handler) (u32 ses_id);

typedef struct flow_age_setup_trace_s {
    u32 session_id;
    u32 timer_hdl;
    pds_flow_state state;
    u8 flags;
} flow_age_setup_trace_t;

vlib_node_registration_t pds_flow_age_node,
                         pds_flow_age_setup_node;

always_inline void
pds_flow_age_setup_syn_x2 (u32 session_id0, u32 session_id1, u16 thread)
{
    pds_flow_main_t *fm = &pds_flow_main;
    pds_flow_hw_ctx_t *ctx0, *ctx1;

    ctx0 = pds_flow_get_hw_ctx(session_id0);
    ctx1 = pds_flow_get_hw_ctx(session_id1);

    ctx0->flow_state = PDS_FLOW_STATE_CONN_SETUP;
    ctx1->flow_state = PDS_FLOW_STATE_CONN_SETUP;

    ctx0->timer_hdl = tw_timer_start_16t_1w_2048sl(&fm->timer_wheel[thread],
                                                   session_id0,
                                                   PDS_FLOW_CONN_SETUP_TIMER,
                                                   fm->tcp_con_setup_timeout);
    ctx1->timer_hdl = tw_timer_start_16t_1w_2048sl(&fm->timer_wheel[thread],
                                                   session_id1,
                                                   PDS_FLOW_CONN_SETUP_TIMER,
                                                   fm->tcp_con_setup_timeout);
    return;
}

always_inline void
pds_flow_age_setup_fin_x2 (u32 session_id0, u32 session_id1, u16 thread)
{
    pds_flow_main_t *fm = &pds_flow_main;
    pds_flow_hw_ctx_t *ctx0, *ctx1;
    //pds_flow_timer timer0, timer1;

    ctx0 = pds_flow_get_hw_ctx_lock(session_id0);
    ctx1 = pds_flow_get_hw_ctx_lock(session_id1);

    // TODO: check if fin is from rflow/iflow and
    // start half close/close timer accordingly.
    ctx0->flow_state = PDS_FLOW_STATE_HALF_CLOSE_IFLOW;
    ctx1->flow_state = PDS_FLOW_STATE_HALF_CLOSE_IFLOW;

    ctx0->timer_hdl = tw_timer_start_16t_1w_2048sl(&fm->timer_wheel[thread],
                                                   session_id0,
                                                   PDS_FLOW_HALF_CLOSE_TIMER,
                                                   fm->tcp_half_close_timeout);
    ctx1->timer_hdl = tw_timer_start_16t_1w_2048sl(&fm->timer_wheel[thread],
                                                   session_id1,
                                                   PDS_FLOW_HALF_CLOSE_TIMER,
                                                   fm->tcp_half_close_timeout);

    pds_flow_hw_ctx_unlock(ctx0);
    pds_flow_hw_ctx_unlock(ctx1);
    return;
}

always_inline void
pds_flow_age_setup_rst_x2 (u32 session_id0, u32 session_id1, u16 thread)
{
    pds_flow_main_t *fm = &pds_flow_main;
    pds_flow_hw_ctx_t *ctx0, *ctx1;

    ctx0 = pds_flow_get_hw_ctx(session_id0);
    ctx1 = pds_flow_get_hw_ctx(session_id1);

    ctx0->flow_state = PDS_FLOW_STATE_CLOSE;
    ctx1->flow_state = PDS_FLOW_STATE_CLOSE;

    ctx0->timer_hdl = tw_timer_start_16t_1w_2048sl(&fm->timer_wheel[thread],
                                                   session_id0,
                                                   PDS_FLOW_CLOSE_TIMER,
                                                   fm->tcp_half_close_timeout);
    ctx1->timer_hdl = tw_timer_start_16t_1w_2048sl(&fm->timer_wheel[thread],
                                                   session_id1,
                                                   PDS_FLOW_CLOSE_TIMER,
                                                   fm->tcp_half_close_timeout);
    return;
}

always_inline void
pds_flow_age_setup_con_established_x2 (u32 session_id0, u32 session_id1,
                                       u16 thread)
{
    pds_flow_main_t *fm = &pds_flow_main;
    pds_flow_hw_ctx_t *ctx0, *ctx1;

    ctx0 = pds_flow_get_hw_ctx(session_id0);
    ctx1 = pds_flow_get_hw_ctx(session_id1);

    ctx0->flow_state = PDS_FLOW_STATE_ESTABLISHED;
    ctx1->flow_state = PDS_FLOW_STATE_ESTABLISHED;

    ctx0->timer_hdl = tw_timer_start_16t_1w_2048sl(&fm->timer_wheel[thread],
                                                   session_id0,
                                                   PDS_FLOW_IDLE_TIMER,
                                                   fm->idle_timeout[ctx0->proto]);
    ctx1->timer_hdl = tw_timer_start_16t_1w_2048sl(&fm->timer_wheel[thread],
                                                   session_id0,
                                                   PDS_FLOW_IDLE_TIMER,
                                                   fm->idle_timeout[ctx1->proto]);
    return;
}

always_inline void
pds_flow_age_setup_syn_x1 (u32 session_id0, u16 thread)
{
    pds_flow_main_t *fm = &pds_flow_main;
    pds_flow_hw_ctx_t *ctx0;

    ctx0 = pds_flow_get_hw_ctx(session_id0);

    ctx0->flow_state = PDS_FLOW_STATE_CONN_SETUP;

    ctx0->timer_hdl = tw_timer_start_16t_1w_2048sl(&fm->timer_wheel[thread],
                                                   session_id0,
                                                   PDS_FLOW_CONN_SETUP_TIMER,
                                                   fm->tcp_con_setup_timeout);
    return;
}

always_inline void
pds_flow_age_setup_fin_x1 (u32 session_id0, u16 thread)
{
    pds_flow_main_t *fm = &pds_flow_main;
    pds_flow_hw_ctx_t *ctx0;

    ctx0 = pds_flow_get_hw_ctx(session_id0);

    // TODO: check if fin is from rflow/iflow and
    // start half close/close timer accordingly.
    ctx0->flow_state = PDS_FLOW_STATE_HALF_CLOSE_IFLOW;

    ctx0->timer_hdl = tw_timer_start_16t_1w_2048sl(&fm->timer_wheel[thread],
                                                   session_id0,
                                                   PDS_FLOW_HALF_CLOSE_TIMER,
                                                   fm->tcp_half_close_timeout);

    return;
}

always_inline void
pds_flow_age_setup_rst_x1 (u32 session_id0, u16 thread)
{
    pds_flow_main_t *fm = &pds_flow_main;
    pds_flow_hw_ctx_t *ctx0;

    ctx0 = pds_flow_get_hw_ctx(session_id0);

    ctx0->flow_state = PDS_FLOW_STATE_CLOSE;

    ctx0->timer_hdl = tw_timer_start_16t_1w_2048sl(&fm->timer_wheel[thread],
                                                   session_id0,
                                                   PDS_FLOW_CLOSE_TIMER,
                                                   fm->tcp_half_close_timeout);
    return;
}

always_inline void
pds_flow_age_setup_con_established_x1 (u32 session_id0, u16 thread)
{
    pds_flow_main_t *fm = &pds_flow_main;
    pds_flow_hw_ctx_t *ctx0;

    ctx0 = pds_flow_get_hw_ctx(session_id0);

    ctx0->flow_state = PDS_FLOW_STATE_ESTABLISHED;

    ctx0->timer_hdl = tw_timer_start_16t_1w_2048sl(&fm->timer_wheel[thread],
                                                   session_id0,
                                                   PDS_FLOW_IDLE_TIMER, 300);
                                                   //fm->idle_timeout[ctx0->proto]);
    return;
}

static u8 *
format_pds_flow_age_setup_trace (u8 * s, va_list * args)
{
    CLIB_UNUSED(vlib_main_t * vm) = va_arg(*args, vlib_main_t *);
    CLIB_UNUSED(vlib_node_t * node) = va_arg(*args, vlib_node_t *);
    flow_age_setup_trace_t *t = va_arg(*args, flow_age_setup_trace_t *);

    s = format(s, "Session ID %u, Timer handle %u, Flow state %u, Flags 0x%x",
               t->session_id, t->timer_hdl, t->state, t->flags);
    return s;
}

void
pds_flow_age_setup_trace_add (vlib_main_t *vm,
                              vlib_node_runtime_t *node,
                              vlib_frame_t *from_frame)
{
    pds_flow_hw_ctx_t *ctx;

    PDS_PACKET_TRACE_LOOP_START {
        PDS_PACKET_TRACE_DUAL_LOOP_START {
            vlib_buffer_t *b0, *b1;
            flow_age_setup_trace_t *t0, *t1;

            b0 = PDS_PACKET_BUFFER(0);
            b1 = PDS_PACKET_BUFFER(1);

            if (b0->flags & VLIB_BUFFER_IS_TRACED) {
                t0 = vlib_add_trace(vm, node, b0, sizeof(t0[0]));
                t0->session_id = vnet_buffer(b0)->pds_flow_data.ses_id;
                ctx = pds_flow_get_hw_ctx(t0->session_id);
                t0->timer_hdl = ctx->timer_hdl;
                t0->state = ctx->flow_state;
                t0->flags = vnet_buffer(b0)->pds_flow_data.flags;
            }

            if (b1->flags & VLIB_BUFFER_IS_TRACED) {
                t1 = vlib_add_trace(vm, node, b1, sizeof(t1[0]));
                t1->session_id = vnet_buffer(b1)->pds_flow_data.ses_id;
                ctx = pds_flow_get_hw_ctx(t1->session_id);
                t1->timer_hdl = ctx->timer_hdl;
                t1->state = ctx->flow_state;
                t1->flags = vnet_buffer(b1)->pds_flow_data.flags;
            }

        } PDS_PACKET_TRACE_DUAL_LOOP_END;

        PDS_PACKET_TRACE_SINGLE_LOOP_START {
            vlib_buffer_t *b0;
            flow_age_setup_trace_t *t0;

            b0 = PDS_PACKET_BUFFER(0);

            if (b0->flags & VLIB_BUFFER_IS_TRACED) {
                t0 = vlib_add_trace(vm, node, b0, sizeof(t0[0]));
                t0->session_id = vnet_buffer(b0)->pds_flow_data.ses_id;
                ctx = pds_flow_get_hw_ctx(t0->session_id);
                t0->timer_hdl = ctx->timer_hdl;
                t0->state = ctx->flow_state;
                t0->flags = vnet_buffer(b0)->pds_flow_data.flags;
            }
        } PDS_PACKET_TRACE_SINGLE_LOOP_END;
    } PDS_PACKET_TRACE_LOOP_END;
}

always_inline uword
pds_flow_age_setup (vlib_main_t *vm,
                    vlib_node_runtime_t *node,
                    vlib_frame_t *from_frame)
{
    u32 counter[FLOW_AGE_SETUP_COUNTER_LAST] = {0};

    PDS_PACKET_LOOP_START {
        PDS_PACKET_DUAL_LOOP_START(LOAD, LOAD) {
            vlib_buffer_t *b0, *b1;
            u32 session_id0, session_id1;
            u16 flags0, flags1;

            b0 = PDS_PACKET_BUFFER(0);
            b1 = PDS_PACKET_BUFFER(1);

            session_id0 = vnet_buffer(b0)->pds_flow_data.ses_id;
            session_id1 = vnet_buffer(b1)->pds_flow_data.ses_id;
            flags0 = vnet_buffer(b0)->pds_flow_data.tcp_flags;
            flags1 = vnet_buffer(b1)->pds_flow_data.tcp_flags;
            if (flags0 == flags1) {
                if (flags0 & TCP_FLAG_SYN) {
                    pds_flow_age_setup_syn_x2(session_id0, session_id1,
                                              node->thread_index);
                    counter[FLOW_AGE_SETUP_COUNTER_SYN] += 2;
                } else if (flags0 & TCP_FLAG_FIN) {
                    pds_flow_age_setup_fin_x2(session_id0, session_id1,
                                              node->thread_index);
                    counter[FLOW_AGE_SETUP_COUNTER_FIN] += 2;
                } else if (PREDICT_FALSE(flags0 & TCP_FLAG_RST)) {
                    pds_flow_age_setup_rst_x2(session_id0, session_id1,
                                              node->thread_index);
                    counter[FLOW_AGE_SETUP_COUNTER_RST] += 2;
                } else {
                    // no connection tracking required,
                    // so move state to established and start idle timeout
                    pds_flow_age_setup_con_established_x2(session_id0,
                                                          session_id1,
                                                          node->thread_index);
                    counter[FLOW_AGE_SETUP_COUNTER_OTHER] += 2;
                }
            } else {
                if (flags0 & TCP_FLAG_SYN) {
                    pds_flow_age_setup_syn_x1(session_id0, node->thread_index);
                    counter[FLOW_AGE_SETUP_COUNTER_SYN]++;
                } else if (flags0 & TCP_FLAG_FIN) {
                    pds_flow_age_setup_fin_x1(session_id0, node->thread_index);
                    counter[FLOW_AGE_SETUP_COUNTER_FIN]++;
                } else if (PREDICT_FALSE(flags0 & TCP_FLAG_RST)) {
                    pds_flow_age_setup_rst_x1(session_id0, node->thread_index);
                    counter[FLOW_AGE_SETUP_COUNTER_RST]++;
                } else {
                    // no connection tracking required,
                    // so move state to established and start idle timeout
                    pds_flow_age_setup_con_established_x1(session_id0,
                                                          node->thread_index);
                    counter[FLOW_AGE_SETUP_COUNTER_OTHER]++;
                }
                if (flags1 & TCP_FLAG_SYN) {
                    pds_flow_age_setup_syn_x1(session_id1, node->thread_index);
                    counter[FLOW_AGE_SETUP_COUNTER_SYN]++;
                } else if (flags1 & TCP_FLAG_FIN) {
                    pds_flow_age_setup_fin_x1(session_id1, node->thread_index);
                    counter[FLOW_AGE_SETUP_COUNTER_FIN]++;
                } else if (PREDICT_FALSE(flags1 & TCP_FLAG_RST)) {
                    pds_flow_age_setup_rst_x1(session_id1, node->thread_index);
                    counter[FLOW_AGE_SETUP_COUNTER_RST]++;
                } else {
                    // no connection tracking required,
                    // so move state to established and start idle timeout
                    pds_flow_age_setup_con_established_x1(session_id1,
                                                          node->thread_index);
                    counter[FLOW_AGE_SETUP_COUNTER_OTHER]++;
                }
            }
            *PDS_PACKET_NEXT_NODE_PTR(0) = FLOW_AGE_SETUP_NEXT_FWD_FLOW;
            *PDS_PACKET_NEXT_NODE_PTR(1) = FLOW_AGE_SETUP_NEXT_FWD_FLOW;
        } PDS_PACKET_DUAL_LOOP_END;

        PDS_PACKET_SINGLE_LOOP_START {
            vlib_buffer_t *b0;
            u32 session_id0;
            u8 flags0;

            b0 = PDS_PACKET_BUFFER(0);

            session_id0 = vnet_buffer(b0)->pds_flow_data.ses_id;
            flags0 = vnet_buffer(b0)->pds_flow_data.tcp_flags;
            if (flags0 & TCP_FLAG_SYN) {
                pds_flow_age_setup_syn_x1(session_id0, node->thread_index);
                counter[FLOW_AGE_SETUP_COUNTER_SYN]++;
            } else if (flags0 & TCP_FLAG_FIN) {
                pds_flow_age_setup_fin_x1(session_id0, node->thread_index);
                counter[FLOW_AGE_SETUP_COUNTER_FIN]++;
            } else if (PREDICT_FALSE(flags0 & TCP_FLAG_RST)) {
                pds_flow_age_setup_rst_x1(session_id0, node->thread_index);
                counter[FLOW_AGE_SETUP_COUNTER_RST]++;
            } else {
                // no connection tracking required,
                // so move state to established and start idle timeout
                pds_flow_age_setup_con_established_x1(session_id0,
                                                      node->thread_index);
                counter[FLOW_AGE_SETUP_COUNTER_OTHER]++;
            }
            *PDS_PACKET_NEXT_NODE_PTR(0) = FLOW_AGE_SETUP_NEXT_FWD_FLOW;
        } PDS_PACKET_SINGLE_LOOP_END;
    } PDS_PACKET_LOOP_END;

#define _(n, s)                                                            \
    vlib_node_increment_counter(vm, node->node_index,                      \
                                FLOW_AGE_SETUP_COUNTER_##n,                \
                                counter[FLOW_AGE_SETUP_COUNTER_##n]);
    foreach_flow_age_setup_counter
#undef _

    if (node->flags & VLIB_NODE_FLAG_TRACE) {
        pds_flow_age_setup_trace_add(vm, node, from_frame);
    }

    return from_frame->n_vectors;
}

static char * flow_age_setup_error_strings[] = {
#define _(n,s) s,
    foreach_flow_age_setup_next
#undef _
};

VLIB_REGISTER_NODE (pds_flow_age_setup_node) = {
    .function = pds_flow_age_setup,
    .name = "pds-flow-age-setup",
    .vector_size = sizeof(u32),

    .n_errors = FLOW_AGE_SETUP_COUNTER_LAST,
    .error_strings = flow_age_setup_error_strings,

    .n_next_nodes = FLOW_AGE_SETUP_N_NEXT,
    .next_nodes = {
#define _(s,n) [FLOW_AGE_SETUP_NEXT_##s] = n,
    foreach_flow_age_setup_next
#undef _
    },

    .format_trace = format_pds_flow_age_setup_trace,
};

u8
pds_flow_age_session_expired (pds_flow_hw_ctx_t *session, u64 cur_time,
                              u64 ses_time)
{
    u64 timeout;
    pds_flow_main_t *fm = &pds_flow_main;

    timeout = fm->idle_timeout_ticks[session->proto];
    // flow is very much active
    if (cur_time < ses_time) {
        return 0;
    }
    // flow timedout
    if ((cur_time - ses_time) >= timeout) {
        return 1;
    }
    return 0;
}

void
pds_flow_start_keep_alive (pds_flow_hw_ctx_t *session)
{
    // TODO: start tcp keep-alive to both ends
    return;
}

void
pds_flow_delete_session (pds_flow_hw_ctx_t *session)
{
    // Delete both iflow and rflow
    if (session->v4) {
        ftlv4 *table4 = (ftlv4 *)pds_flow_get_table4();
        ftlv4_remove_with_handle(table4, session->iflow.table_id, 
                                 session->iflow.primary);
        ftlv4_remove_with_handle(table4, session->rflow.table_id,
                                 session->rflow.primary);    
    } else {
        ftl *table = (ftl *)pds_flow_get_table6_or_l2();
        ftl_remove_with_handle(table, session->iflow.table_id,
                               session->iflow.primary);
        ftl_remove_with_handle(table, session->rflow.table_id,
                               session->rflow.primary);
    }
    return;
}

static uword
pds_flow_age_process (vlib_main_t *vm,
                      vlib_node_runtime_t *rt,
                      vlib_frame_t *f)
{
    pds_flow_main_t *fm = &pds_flow_main;
    f64 now = vlib_time_now(vm);

    // for thread 0 - main thread skip
    if (!rt->thread_index) {
        return 0;
    }

    // If the timers are not yet initialized, then skip
    if ((vec_len(fm->timer_wheel) != fm->no_threads) ||
        !fm->timer_wheel[rt->thread_index].timer_interval) {
       return 0;
    }

    tw_timer_expire_timers_16t_1w_2048sl(&fm->timer_wheel[rt->thread_index],
                                         now);
    return 0;
}

VLIB_REGISTER_NODE (pds_flow_age_node) = {
    .function = pds_flow_age_process,
    .type = VLIB_NODE_TYPE_INPUT,
    .name = "pds-flow-age-process",
    .state = VLIB_NODE_STATE_POLLING,
};

static void
pds_flow_connection_timeout (u32 ses_id)
{
    return;
}

static void
pds_flow_idle_timeout (u32 ses_id)
{
    u64 timestamp;
    pds_flow_main_t *fm = &pds_flow_main;
    pds_flow_hw_ctx_t *session = pds_flow_get_hw_ctx_lock(ses_id);
    u64 cur_time = pds_system_get_current_tick();

    if (0 == session->inuse) {
        // this means session allocated from pool but still in cache, not used
        goto end;
    }
    timestamp = pds_session_get_timestamp(ses_id);
    if (pds_flow_age_session_expired(session, cur_time, timestamp)) {
        if (!fm->con_track_en || session->proto != PDS_FLOW_PROTO_TCP) {
            pds_flow_delete_session(session);
            goto end;
        }
        // connection tracking enabled and protocol is TCP, send keep-alive
        pds_flow_start_keep_alive(session);
    }
end:
    pds_flow_hw_ctx_unlock(session);
    return;
}

static void
pds_flow_keep_alive_timeout (u32 ses_id)
{

}

static void
pds_flow_half_close_timeout (u32 ses_id)
{

}

static void
pds_flow_close_timeout (u32 ses_id)
{

}

static void
pds_flow_drop_timeout (u32 ses_id)
{

}

static flow_expiration_handler *flow_exp_handlers[PDS_FLOW_N_TIMERS] =
{
    pds_flow_connection_timeout,
    pds_flow_idle_timeout,
    pds_flow_keep_alive_timeout,
    pds_flow_half_close_timeout,
    pds_flow_close_timeout,
    pds_flow_drop_timeout,
};

static void
pds_flow_expired_timers_dispatch (u32 * expired_timers)
{
    int i;
    u32 ses_index, timer_id;

    for (i = 0; i < vec_len(expired_timers); i++) {
        /* Get session index and timer id */
        ses_index = expired_timers[i] & 0x0FFFFFFF;
        timer_id = expired_timers[i] >> 28;

        /* Handle expiration */
        (*flow_exp_handlers[timer_id]) (ses_index);
    }
}

uword
pds_flow_timer_init (vlib_main_t *vm, vlib_node_runtime_t *rt, vlib_frame_t *f)
{
    pds_flow_main_t *fm = &pds_flow_main;
    tw_timer_wheel_16t_1w_2048sl_t *tw;

    vlib_worker_thread_barrier_sync(vm);
    // we need one timer wheel per worker, nothing for main thread.
    vec_validate(fm->timer_wheel, (fm->no_threads - 1));

    foreach_vlib_main (({
        // skip main thread
        if (ii == 0) {
            continue;
        }
        tw = vec_elt_at_index(fm->timer_wheel, ii);
        tw_timer_wheel_init_16t_1w_2048sl(tw, pds_flow_expired_timers_dispatch,
                                          PDS_FLOW_TIMER_TICK, ~0);
        tw->last_run_time = vlib_time_now(this_vlib_main);
    }));
    
    vlib_worker_thread_barrier_release(vm);
    return 0;
}

VLIB_REGISTER_NODE (pds_flow_timer_init_node, static) =
{
  .function = pds_flow_timer_init,
  .type = VLIB_NODE_TYPE_PROCESS,
  .name = "pds-flow-timer-init",
};

static clib_error_t *
pds_flow_age_init (vlib_main_t *vm)
{
    pds_flow_main_t *fm = &pds_flow_main;

    vec_validate_init_empty(fm->idle_timeout, PDS_FLOW_PROTO_OTHER, 0);
    vec_validate_init_empty(fm->idle_timeout_ticks, PDS_FLOW_PROTO_OTHER, 0);
    vec_validate_init_empty(fm->drop_timeout, PDS_FLOW_PROTO_OTHER, 0);
    vec_validate_init_empty(fm->drop_timeout_ticks, PDS_FLOW_PROTO_OTHER, 0);

    return 0;
}

VLIB_INIT_FUNCTION(pds_flow_age_init) =
{
    .runs_after = VLIB_INITS("pds_flow_init"),
};
