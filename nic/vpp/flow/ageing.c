//
//  {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
//

#include <vlib/vlib.h>
#include <vnet/buffer.h>
#include <vnet/tcp/tcp_packet.h>
#include <session.h>
#include <mapping.h>
#include <system.h>
#include <feature.h>
#include <pkt.h>
#include <vnic.h>
#include <nic/vpp/infra/api/intf.h>
#include "utils.h"
#include "node.h"
#include "pdsa_hdlr.h"
#include "pdsa_uds_hdlr.h"

typedef void (flow_expiration_handler) (u32 ses_id);

typedef struct flow_age_setup_trace_s {
    u32 session_id;
    u32 timer_hdl;
    pds_flow_state state;
    u8 flags;
    u8 tcp_flags;
} flow_age_setup_trace_t;

// timer_hdl is 23 bits, so check against 0x7fffff
#define FLOW_AGE_TIMER_STOP(_tw, _hdl)                  \
{                                                       \
    if (_hdl != 0x7fffff) {                             \
        tw_timer_stop_16t_1w_2048sl(_tw, _hdl);         \
        _hdl = ~0;                                      \
    }                                                   \
}                                                       \

vlib_node_registration_t pds_flow_age_node,
                         pds_flow_age_setup_node;

always_inline void
pds_flow_age_setup_syn_x2 (u32 session_id0, u32 session_id1, u16 thread)
{
    pds_flow_main_t *fm = &pds_flow_main;
    pds_flow_hw_ctx_t *ctx0, *ctx1;

    ctx0 = pds_flow_get_hw_ctx(session_id0);
    ctx1 = pds_flow_get_hw_ctx(session_id1);

    // If session is already in conn_setup state, then this is a duplicate packet, 
    // so ignore
    if (PREDICT_FALSE(ctx0->flow_state == PDS_FLOW_STATE_CONN_SETUP)) {
        goto ctx1;
    }
    ctx0->flow_state = PDS_FLOW_STATE_CONN_SETUP;
    ctx0->timer_hdl = tw_timer_start_16t_1w_2048sl(&fm->timer_wheel[thread],
                                                   session_id0,
                                                   PDS_FLOW_CONN_SETUP_TIMER,
                                                   fm->tcp_con_setup_timeout);
    
ctx1:
    if (PREDICT_FALSE(ctx1->flow_state == PDS_FLOW_STATE_CONN_SETUP)) {
        goto end;
    }
    ctx1->flow_state = PDS_FLOW_STATE_CONN_SETUP;
    ctx1->timer_hdl = tw_timer_start_16t_1w_2048sl(&fm->timer_wheel[thread],
                                                   session_id1,
                                                   PDS_FLOW_CONN_SETUP_TIMER,
                                                   fm->tcp_con_setup_timeout);
end:
    return;
}

always_inline void
pds_flow_age_setup_rst_x2 (u32 session_id0, u32 session_id1, u16 thread)
{
    pds_flow_main_t *fm = &pds_flow_main;
    pds_flow_hw_ctx_t *ctx0, *ctx1;

    ctx0 = pds_flow_get_hw_ctx(session_id0);
    ctx1 = pds_flow_get_hw_ctx(session_id1);

    if (PREDICT_FALSE(ctx0->flow_state == PDS_FLOW_STATE_CLOSE)) {
        goto ctx1;
    }

    ctx0->flow_state = PDS_FLOW_STATE_CLOSE;
    FLOW_AGE_TIMER_STOP(&fm->timer_wheel[thread], ctx0->timer_hdl);
    ctx0->timer_hdl = tw_timer_start_16t_1w_2048sl(&fm->timer_wheel[thread],
                                                   session_id0,
                                                   PDS_FLOW_CLOSE_TIMER,
                                                   fm->tcp_close_timeout);
    
ctx1:
    if (PREDICT_FALSE(ctx1->flow_state == PDS_FLOW_STATE_CLOSE)) {
        goto end;
    }

    ctx1->flow_state = PDS_FLOW_STATE_CLOSE;
    FLOW_AGE_TIMER_STOP(&fm->timer_wheel[thread], ctx1->timer_hdl);
    ctx1->timer_hdl = tw_timer_start_16t_1w_2048sl(&fm->timer_wheel[thread],
                                                   session_id1,
                                                   PDS_FLOW_CLOSE_TIMER,
                                                   fm->tcp_close_timeout);
end:
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

    if (PREDICT_FALSE(ctx0->flow_state == PDS_FLOW_STATE_ESTABLISHED)) {
        goto ctx1;
    }

    ctx0->flow_state = PDS_FLOW_STATE_ESTABLISHED;
    ctx0->timer_hdl = tw_timer_start_16t_1w_2048sl(&fm->timer_wheel[thread],
                                                   session_id0,
                                                   PDS_FLOW_IDLE_TIMER,
                                                   fm->idle_timeout[ctx0->proto]);

ctx1:
    if (PREDICT_FALSE(ctx1->flow_state == PDS_FLOW_STATE_ESTABLISHED)) {
        goto end;
    }

    ctx1->flow_state = PDS_FLOW_STATE_ESTABLISHED;
    ctx1->timer_hdl = tw_timer_start_16t_1w_2048sl(&fm->timer_wheel[thread],
                                                   session_id1,
                                                   PDS_FLOW_IDLE_TIMER,
                                                   fm->idle_timeout[ctx1->proto]);

end:
    return;
}

always_inline void
pds_flow_age_setup_syn_x1 (u32 session_id0, u16 thread)
{
    pds_flow_main_t *fm = &pds_flow_main;
    pds_flow_hw_ctx_t *ctx0;

    ctx0 = pds_flow_get_hw_ctx(session_id0);

    if (PREDICT_FALSE(ctx0->flow_state == PDS_FLOW_STATE_CONN_SETUP)) {
        return;
    }

    ctx0->flow_state = PDS_FLOW_STATE_CONN_SETUP;
    ctx0->timer_hdl = tw_timer_start_16t_1w_2048sl(&fm->timer_wheel[thread],
                                                   session_id0,
                                                   PDS_FLOW_CONN_SETUP_TIMER,
                                                   fm->tcp_con_setup_timeout);
    return;
}

always_inline void
pds_flow_age_setup_fin_x1 (u32 session_id0, u16 thread, bool rflow)
{
    pds_flow_main_t *fm = &pds_flow_main;
    pds_flow_hw_ctx_t *ctx0;
    u32 timer;
    u64 timeout;
    u8 state;

    ctx0 = pds_flow_get_hw_ctx(session_id0);

    if (PREDICT_FALSE(ctx0->flow_state < PDS_FLOW_STATE_ESTABLISHED)) {
        return;
    }

    if (rflow) {
        if (PREDICT_FALSE(ctx0->flow_state == PDS_FLOW_STATE_HALF_CLOSE_RFLOW ||
                          ctx0->flow_state == PDS_FLOW_STATE_CLOSE)) {
            return;
        }
        if (ctx0->flow_state == PDS_FLOW_STATE_HALF_CLOSE_IFLOW) {
            timer = PDS_FLOW_CLOSE_TIMER;
            timeout = fm->tcp_close_timeout;
            state = PDS_FLOW_STATE_CLOSE;
        } else {
            timer = PDS_FLOW_HALF_CLOSE_TIMER;
            timeout = fm->tcp_half_close_timeout;
            state = PDS_FLOW_STATE_HALF_CLOSE_RFLOW;
        }
    } else {
        if (PREDICT_FALSE(ctx0->flow_state == PDS_FLOW_STATE_HALF_CLOSE_IFLOW ||
                          ctx0->flow_state == PDS_FLOW_STATE_CLOSE)) {
            return;
        }
        if (ctx0->flow_state == PDS_FLOW_STATE_HALF_CLOSE_RFLOW) {
            timer = PDS_FLOW_CLOSE_TIMER;
            timeout = fm->tcp_close_timeout;
            state = PDS_FLOW_STATE_CLOSE;
        } else {
            timer = PDS_FLOW_HALF_CLOSE_TIMER;
            timeout = fm->tcp_half_close_timeout;
            state = PDS_FLOW_STATE_HALF_CLOSE_IFLOW;
        }
    }
    FLOW_AGE_TIMER_STOP(&fm->timer_wheel[thread], ctx0->timer_hdl);
    ctx0->timer_hdl = tw_timer_start_16t_1w_2048sl(&fm->timer_wheel[thread],
                                                   session_id0,
                                                   timer,
                                                   timeout);
    ctx0->flow_state = state;
    return;
}

always_inline void
pds_flow_age_setup_rst_x1 (u32 session_id0, u16 thread)
{
    pds_flow_main_t *fm = &pds_flow_main;
    pds_flow_hw_ctx_t *ctx0;

    ctx0 = pds_flow_get_hw_ctx(session_id0);

    if (ctx0->flow_state == PDS_FLOW_STATE_CLOSE) {
        return;
    }

    ctx0->flow_state = PDS_FLOW_STATE_CLOSE;
    FLOW_AGE_TIMER_STOP(&fm->timer_wheel[thread], ctx0->timer_hdl);
    ctx0->timer_hdl = tw_timer_start_16t_1w_2048sl(&fm->timer_wheel[thread],
                                                   session_id0,
                                                   PDS_FLOW_CLOSE_TIMER,
                                                   fm->tcp_close_timeout);
    return;
}

always_inline void
pds_flow_age_setup_con_established_x1 (u32 session_id0, u16 thread)
{
    pds_flow_main_t *fm = &pds_flow_main;
    pds_flow_hw_ctx_t *ctx0;

    ctx0 = pds_flow_get_hw_ctx(session_id0);

    if (PREDICT_FALSE(ctx0->flow_state == PDS_FLOW_STATE_ESTABLISHED)) {
        return;
    }

    ctx0->flow_state = PDS_FLOW_STATE_ESTABLISHED;
    ctx0->timer_hdl = tw_timer_start_16t_1w_2048sl(&fm->timer_wheel[thread],
                                                   session_id0,
                                                   PDS_FLOW_IDLE_TIMER,
                                                   fm->idle_timeout[ctx0->proto]);
    return;
}

static u8 *
format_pds_flow_age_setup_trace (u8 * s, va_list * args)
{
    CLIB_UNUSED(vlib_main_t * vm) = va_arg(*args, vlib_main_t *);
    CLIB_UNUSED(vlib_node_t * node) = va_arg(*args, vlib_node_t *);
    flow_age_setup_trace_t *t = va_arg(*args, flow_age_setup_trace_t *);

    s = format(s, "Session ID %u, Timer handle %u, Flow state %u, Flags 0x%x "
               "TCP flags 0x%x", t->session_id, t->timer_hdl, t->state, t->flags,
               t->tcp_flags);
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
                t0->tcp_flags = vnet_buffer(b0)->pds_flow_data.tcp_flags;
            }

            if (b1->flags & VLIB_BUFFER_IS_TRACED) {
                t1 = vlib_add_trace(vm, node, b1, sizeof(t1[0]));
                t1->session_id = vnet_buffer(b1)->pds_flow_data.ses_id;
                ctx = pds_flow_get_hw_ctx(t1->session_id);
                t1->timer_hdl = ctx->timer_hdl;
                t1->state = ctx->flow_state;
                t1->flags = vnet_buffer(b1)->pds_flow_data.flags;
                t1->tcp_flags = vnet_buffer(b1)->pds_flow_data.tcp_flags;
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
                t0->tcp_flags = vnet_buffer(b0)->pds_flow_data.tcp_flags;
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
    pds_flow_main_t *fm = &pds_flow_main;

    PDS_PACKET_LOOP_START {
        PDS_PACKET_DUAL_LOOP_START(LOAD, LOAD) {
            vlib_buffer_t *b0, *b1;
            u32 session_id0, session_id1;
            u16 tcp_flags0, tcp_flags1;
            bool rflow0, rflow1;

            b0 = PDS_PACKET_BUFFER(0);
            b1 = PDS_PACKET_BUFFER(1);

            session_id0 = vnet_buffer(b0)->pds_flow_data.ses_id;
            session_id1 = vnet_buffer(b1)->pds_flow_data.ses_id;
            tcp_flags0 = vnet_buffer(b0)->pds_flow_data.tcp_flags;
            tcp_flags1 = vnet_buffer(b1)->pds_flow_data.tcp_flags;
            if (tcp_flags0 == tcp_flags1) {
                if (fm->con_track_en && (tcp_flags0 & TCP_FLAG_SYN)) {
                    pds_flow_age_setup_syn_x2(session_id0, session_id1,
                                              node->thread_index);
                    counter[FLOW_AGE_SETUP_COUNTER_SYN] += 2;
                } else if (tcp_flags0 & TCP_FLAG_FIN) {
                    rflow0 = pds_is_rflow(b0);
                    rflow1 = pds_is_rflow(b1);
                    pds_flow_age_setup_fin_x1(session_id0,
                                              node->thread_index, rflow0);
                    pds_flow_age_setup_fin_x1(session_id1,
                                              node->thread_index, rflow1);
                    counter[FLOW_AGE_SETUP_COUNTER_FIN] += 2;
                } else if (PREDICT_FALSE(tcp_flags0 & TCP_FLAG_RST)) {
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
                if (fm->con_track_en && (tcp_flags0 & TCP_FLAG_SYN)) {
                    pds_flow_age_setup_syn_x1(session_id0, node->thread_index);
                    counter[FLOW_AGE_SETUP_COUNTER_SYN]++;
                } else if (tcp_flags0 & TCP_FLAG_FIN) {
                    rflow0 = pds_is_rflow(b0);
                    pds_flow_age_setup_fin_x1(session_id0, node->thread_index, 
                                              rflow0);
                    counter[FLOW_AGE_SETUP_COUNTER_FIN]++;
                } else if (PREDICT_FALSE(tcp_flags0 & TCP_FLAG_RST)) {
                    pds_flow_age_setup_rst_x1(session_id0, node->thread_index);
                    counter[FLOW_AGE_SETUP_COUNTER_RST]++;
                } else {
                    // no connection tracking required,
                    // so move state to established and start idle timeout
                    pds_flow_age_setup_con_established_x1(session_id0,
                                                          node->thread_index);
                    counter[FLOW_AGE_SETUP_COUNTER_OTHER]++;
                }
                if (fm->con_track_en && (tcp_flags1 & TCP_FLAG_SYN)) {
                    pds_flow_age_setup_syn_x1(session_id1, node->thread_index);
                    counter[FLOW_AGE_SETUP_COUNTER_SYN]++;
                } else if (tcp_flags1 & TCP_FLAG_FIN) {
                    rflow1 = pds_is_rflow(b1);
                    pds_flow_age_setup_fin_x1(session_id1, node->thread_index,
                                              rflow1);
                    counter[FLOW_AGE_SETUP_COUNTER_FIN]++;
                } else if (PREDICT_FALSE(tcp_flags1 & TCP_FLAG_RST)) {
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
            u8 tcp_flags0;
            bool rflow0;

            b0 = PDS_PACKET_BUFFER(0);

            session_id0 = vnet_buffer(b0)->pds_flow_data.ses_id;
            tcp_flags0 = vnet_buffer(b0)->pds_flow_data.tcp_flags;
            if (fm->con_track_en && (tcp_flags0 & TCP_FLAG_SYN)) {
                pds_flow_age_setup_syn_x1(session_id0, node->thread_index);
                counter[FLOW_AGE_SETUP_COUNTER_SYN]++;
            } else if (tcp_flags0 & TCP_FLAG_FIN) {
                rflow0 = pds_is_rflow(b0);
                pds_flow_age_setup_fin_x1(session_id0, node->thread_index, rflow0);
                counter[FLOW_AGE_SETUP_COUNTER_FIN]++;
            } else if (PREDICT_FALSE(tcp_flags0 & TCP_FLAG_RST)) {
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
    foreach_flow_age_setup_counter
#undef _
};

VLIB_REGISTER_NODE(pds_flow_age_setup_node) = {
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
                              u64 ses_time, u64 *diff_time)
{
    u64 timeout, diff_tick;
    pds_flow_main_t *fm = &pds_flow_main;

    timeout = fm->idle_timeout_ticks[session->proto];

    // h/w timestamp is 48 bit wide. handle roll-over case.
    // note - capri frequncy is 416 MHz = 2.4038461538462 ns per tick.
    // so for 48 bit roll-over it take over 6 days. 
    // max configurable timeout is 2 days, so multiple roll-over
    // case not possible.
    // redo this check for elba and see if this holds good even there
    if (PREDICT_FALSE(cur_time < ses_time)) {
        diff_tick = 0xFFFFFFFFFFFF - ses_time + cur_time;
    } else {
        diff_tick = cur_time - ses_time;
    }
    // flow timedout
    if (diff_tick >= timeout) {
        return 1;
    }

    *diff_time = (u64) (PDS_FLOW_SEC_TO_TIMER_TICK(
                        pds_system_get_secs(diff_tick)));
    return 0;
}

void
pds_flow_start_keep_alive (pds_flow_hw_ctx_t *session)
{
    // TODO: start tcp keep-alive to both ends
    return;
}

always_inline uint8_t
pds_flow_get_ctr_idx (uint8_t proto, bool isv4)
{
    switch(proto) {
    case IP_PROTOCOL_TCP:
        if (isv4) {
            return FLOW_TYPE_COUNTER_TCPV4;
        } else {
            return FLOW_TYPE_COUNTER_TCPV6;
        }
    case IP_PROTOCOL_UDP:
        if (isv4) {
            return FLOW_TYPE_COUNTER_UDPV4;
        } else {
            return FLOW_TYPE_COUNTER_UDPV6;
        }
    case IP_PROTOCOL_ICMP:
        if (isv4) {
            return FLOW_TYPE_COUNTER_ICMPV4;
        } else {
            return FLOW_TYPE_COUNTER_ICMPV6;
        }
    default:
        if (isv4) {
            return FLOW_TYPE_COUNTER_OTHERV4;
        } else {
            return FLOW_TYPE_COUNTER_OTHERV6;
        }
    }
    // TODO: what about L2?
    return 0;
}

void
pds_flow_delete_session (u32 ses_id)
{
    pds_flow_hw_ctx_t *session = pds_flow_get_hw_ctx(ses_id);
    pds_flow_main_t *fm = &pds_flow_main;
    int flow_log_enabled = 0;
    int thread = vlib_get_thread_index();
    uint8_t ctr_idx;

    pds_vnic_flow_log_en_get(session->vnic_id, &flow_log_enabled);
    FLOW_AGE_TIMER_STOP(&fm->timer_wheel[thread], session->timer_hdl);
    // Delete both iflow and rflow
    if (session->v4) {
        ftlv4 *table4 = (ftlv4 *)pds_flow_get_table4();
        if (flow_log_enabled) {
            ftlv4_export_with_handle(table4, session->iflow.table_id,
                                     session->iflow.primary,
                                     session->rflow.table_id,
                                     session->rflow.primary,
                                     FLOW_EXPORT_REASON_DEL,
                                     session->host_origin);
        }
        session = pds_flow_get_hw_ctx_lock(ses_id);
        if (PREDICT_FALSE(ftlv4_get_with_handle(table4, session->iflow.table_id,
                                                session->iflow.primary) != 0)) {
            goto end;
        }
        pds_flow_hw_ctx_unlock(session);
        if (PREDICT_FALSE(ftlv4_remove_cached_entry(table4)) != 0) {
            return;
        }

        session = pds_flow_get_hw_ctx_lock(ses_id);
        if (PREDICT_FALSE(ftlv4_get_with_handle(table4, session->rflow.table_id,
                                                session->rflow.primary) != 0)) {
            goto end;
        }
        pds_flow_hw_ctx_unlock(session);
        if (PREDICT_FALSE(ftlv4_remove_cached_entry(table4)) != 0) {
            return;
        }
    } else {
        ftl *table = (ftl *)pds_flow_get_table6_or_l2();
        if (flow_log_enabled) {
            ftl_export_with_handle(table, session->iflow.table_id,
                                   session->iflow.primary,
                                   FLOW_EXPORT_REASON_DEL);
        }
        session = pds_flow_get_hw_ctx_lock(ses_id);
        if (PREDICT_FALSE(ftlv6_get_with_handle(table, session->iflow.table_id,
                                                session->iflow.primary) != 0)) {
            goto end;
        }
        pds_flow_hw_ctx_unlock(session);
        if (PREDICT_FALSE(ftlv6_remove_cached_entry(table)) != 0) {
            return;
        }

        session = pds_flow_get_hw_ctx_lock(ses_id);
        if (PREDICT_FALSE(ftlv6_get_with_handle(table, session->rflow.table_id,
                                                session->rflow.primary) != 0)) {
           goto end;
        }
        pds_flow_hw_ctx_unlock(session);
        if (PREDICT_FALSE(ftlv6_remove_cached_entry(table)) != 0) {
            return;
        }
    }

    pds_vnic_active_sessions_decrement(session->vnic_id);
    ctr_idx = pds_flow_get_ctr_idx(session->proto, session->v4);
    clib_atomic_fetch_add(&fm->stats.counter[ctr_idx], -1);
    pds_session_id_dealloc(ses_id);
    pds_session_stats_clear(ses_id);
    return;

end:
    pds_flow_hw_ctx_unlock(session);
    return;
}

static u8
pds_flow_send_keep_alive_helper (pds_flow_hw_ctx_t *session,
                                 bool iflow)
{
    pds_flow_index_t flow_index;
    ip4_and_tcp_header_t *h;
    pds_flow_main_t *fm = &pds_flow_main;
    vlib_buffer_t *b;
    u32 bi;
    u32 ses_id = session - fm->session_index_pool + 1;
    vlib_main_t *vm = vlib_get_main();
    session_info_t info;
    u32 sip, dip;
    u16 sport, dport, lkp_id;
    u32 *to_next = NULL;
    bool to_host = false;

    h = vlib_packet_template_get_packet(vlib_get_main(),
                                        &fm->tcp_keepalive_packet_template, &bi);
    // we ran out of packet buffers
    if (PREDICT_FALSE(!h)) {
        return -1;
    }
        
    if (iflow) {
        flow_index = session->iflow;
        // iflow is from the network to host, so the keepalive is also sent
        // from network to host
        to_host = session->iflow_rx;
    } else {
        flow_index = session->rflow;
        if (session->packet_type == PDS_PKT_TYPE_L2L) {
            to_host = true;
        } else {
            to_host = !session->iflow_rx;
        }
    }

    if (session->v4) {
        ftlv4 *table4 = pds_flow_prog_get_table4();
        if (ftlv4_get_with_handle(table4, flow_index.table_id,
                                  flow_index.primary) != 0) {
            return -1;
        }

        ftlv4_get_last_read_session_info(&sip, &dip, &sport, &dport, &lkp_id);
        // Fill in IP and TCP header details
        h->ip4_hdr.src_address.as_u32 = sip;
        h->ip4_hdr.dst_address.as_u32 = dip;
        h->tcp_hdr.src_port = sport;
        h->tcp_hdr.dst_port = dport;
    } else {
        // Will implement v6 later
        return -1;
    }
#if 0
    } else {

        ftlv6 *table = pds_flow_prog_get_table6_or_l2();
        if (ftlv6_get_with_handle(table, flow_index.table_id,
                                  flow_index.primary) != 0) {
            return -1;
        }

        ftlv6_get_last_read_session_info(sip6, dip6, &sport, &dport, &lkp_id);
        h->ip_hdr.ip6_hdr.protocol = IP_PROTOCOL_TCP;
        memcpy(&h->ip_hdr.ip6_hdr.src_address.as_u8, sip6, sizeof(ip6_address_t));
        memcpy(&h->ip_hdr.ip6_hdr.dst_address.as_u8, dip6, sizeof(ip6_address_t));
        h->tcp_hdr.src_port = sport;
        h->tcp_hdr.dst_port = dport;
    }
#endif 

    pds_session_get_info(ses_id, &info);

    if (iflow) {
        h->tcp_hdr.seq_number = info.iflow_tcp_seq_num;
        h->tcp_hdr.ack_number = info.iflow_tcp_ack_num;
        h->tcp_hdr.window = info.iflow_tcp_win_size;
    } else {
        h->tcp_hdr.seq_number = info.rflow_tcp_seq_num;
        h->tcp_hdr.ack_number = info.rflow_tcp_ack_num;
        h->tcp_hdr.window = info.rflow_tcp_win_size;
    }

    // Send the packet
    if (to_host) {
        u16 vnic_id = 0, vnic_nh_hw_id = 0;
        // Get the VNIC information
        int ret = pds_dst_vnic_info_get(lkp_id, dip, &vnic_id, 
                                                  &vnic_nh_hw_id);
        if (PREDICT_FALSE(ret == -1)) {
            return -1;
        }

        b = vlib_get_buffer(vm, bi);
        vnet_buffer(b)->pds_tx_data.vnic_id = vnic_id;
        vnet_buffer(b)->pds_tx_data.vnic_nh_hw_id = vnic_nh_hw_id;

        static vlib_node_t *vnic_node = NULL;
        if (!vnic_node) {
            vnic_node = vlib_get_node_by_name(vm, (u8 *) "pds-vnic-l2-rewrite");
        }
        vlib_frame_t *to_frame = vlib_get_frame_to_node(vm, vnic_node->index);
        to_next = vlib_frame_vector_args(to_frame);
        to_next[0] = bi;
        to_frame->n_vectors++;
        vlib_put_frame_to_node(vm, vnic_node->index, to_frame);
    } else {
        ethernet_header_t *eth0;
        mac_addr_t mac;
        u16 host_lif_hw_id = 0;

        // Get the VNIC to which the packet belongs
        int ret = pds_src_vnic_info_get(lkp_id, sip, mac, &host_lif_hw_id);
        if (PREDICT_FALSE(ret == -1)) {
            return -1;
        }

        // Add the ethernet header
        b = vlib_get_buffer(vm, bi);
        vlib_buffer_advance(b, - sizeof (ethernet_header_t));
        eth0 = vlib_buffer_get_current(b);
        // Src MAC will be VNIC mac and dst mac is dummy
        clib_memcpy(&eth0->src_address, mac, ETH_ADDR_LEN);
        
        static u32 cpu_mnic_if_index = ~0;
        if (((u32) ~0) == cpu_mnic_if_index) {
            cpu_mnic_if_index = pds_infra_get_sw_ifindex_by_name((u8*)"cpu_mnic0");
        }
        vnet_buffer(b)->sw_if_index[VLIB_RX] = cpu_mnic_if_index;
        vnet_buffer(b)->pds_flow_data.lif = host_lif_hw_id;
        vnet_buffer(b)->pds_flow_data.ses_id = ses_id;
        
        static vlib_node_t *fwd_node = NULL;
        if (!fwd_node) {
            fwd_node = vlib_get_node_by_name(vm, (u8 *) "pds-fwd-flow");
        }
        vlib_frame_t *to_frame = vlib_get_frame_to_node(vm, fwd_node->index);
        to_next = vlib_frame_vector_args(to_frame);
        to_next[0] = bi;
        to_frame->n_vectors++;
        vlib_put_frame_to_node(vm, fwd_node->index, to_frame);
    }
    return 0;
}

static void
pds_flow_send_keep_alive (pds_flow_hw_ctx_t *session)
{
    u8 ret1, ret2;
    pds_flow_main_t *fm = &pds_flow_main;
    int thread = vlib_get_thread_index();
    u32 ses_id = session - fm->session_index_pool + 1;

    // send keepalive packet for iflow
    ret1 = pds_flow_send_keep_alive_helper(session, true);
    if (ret1 != 0) {
        goto end;
    }
    
    // if send for iflow succeeded, send keepalive for rflow
    ret2 = pds_flow_send_keep_alive_helper(session, false);
    
    // if we successfully sent the packets, then start the keepalive timer to
    // retry if required, otherwise delete the session
    if (ret2 == 0) {
        session->keep_alive_retry += 1;
        // if send was successful, then start the keepalive timer to retry.
        session->timer_hdl = tw_timer_start_16t_1w_2048sl(&fm->timer_wheel[thread],
                                                          ses_id,
                                                          PDS_FLOW_KEEP_ALIVE_TIMER, 
                                                          fm->tcp_keep_alive_timeout);
        session->flow_state = PDS_FLOW_STATE_KEEPALIVE_SENT;
        return;
    }

end:
    pds_flow_delete_session(ses_id);
    return;
}

static uword
pds_flow_age_process (vlib_main_t *vm,
                      vlib_node_runtime_t *rt,
                      vlib_frame_t *f)
{
    pds_flow_main_t *fm = &pds_flow_main;
    f64 now = vlib_time_now(vm);

    tw_timer_expire_timers_16t_1w_2048sl(&fm->timer_wheel[rt->thread_index],
                                         now);
    return 0;
}

VLIB_REGISTER_NODE(pds_flow_age_node) = {
    .function = pds_flow_age_process,
    .type = VLIB_NODE_TYPE_INPUT,
    .name = "pds-flow-age-process",
    .state = VLIB_NODE_STATE_DISABLED,
};

static void
pds_flow_connection_timeout (u32 ses_id)
{
    pds_flow_main_t *fm = &pds_flow_main;
    pds_flow_hw_ctx_t *session;
    uint8_t iflow_state, rflow_state;
    int thread = vlib_get_thread_index();

    // If both iflow and rflow are in established state, start idle timer.
    // If not, then delete the session.
    session = pds_flow_get_hw_ctx_lock(ses_id);
    pds_session_get_session_state(ses_id, &iflow_state, &rflow_state);
    pds_flow_hw_ctx_unlock(session);
    if (pds_session_state_established(iflow_state) && 
        pds_session_state_established(rflow_state)) {
        session->flow_state = PDS_FLOW_STATE_ESTABLISHED;
        session->timer_hdl = tw_timer_start_16t_1w_2048sl(&fm->timer_wheel[thread],
                                                          ses_id,
                                                          PDS_FLOW_IDLE_TIMER,
                                                          fm->idle_timeout[session->proto]);
    } else {
        pds_flow_delete_session(ses_id);
        return;
    }

    return;
}

static void
pds_flow_idle_timeout (u32 ses_id)
{
    u64 timestamp;
    pds_flow_main_t *fm = &pds_flow_main;
    pds_flow_hw_ctx_t *session;
    u64 cur_time;
    u64 diff_time;
    int thread = vlib_get_thread_index();

    session = pds_flow_get_hw_ctx_lock(ses_id);
    timestamp = pds_session_get_timestamp(ses_id);
    pds_flow_hw_ctx_unlock(session);
    cur_time = pds_system_get_current_tick();
    if (pds_flow_age_session_expired(session, cur_time, timestamp, &diff_time)) {
        pds_flow_delete_session(ses_id);
        return;
    // Enable once keepalive is tested
#if 0
        if (!fm->con_track_en || session->proto != PDS_FLOW_PROTO_TCP) {
            pds_flow_delete_session(ses_id);
            return;
        }
        // connection tracking enabled and protocol is TCP, send keep-alive
        pds_flow_send_keep_alive(session);
#endif
    } else {
        // restart idle timeout with the time left for timeout
        session->timer_hdl = tw_timer_start_16t_1w_2048sl(&fm->timer_wheel[thread],
                                                          ses_id,
                                                          PDS_FLOW_IDLE_TIMER,
                                                          fm->idle_timeout[session->proto] - diff_time);
    }

    return;
}

static void
pds_flow_keep_alive_timeout (u32 ses_id)
{
    pds_flow_main_t *fm = &pds_flow_main;
    int thread = vlib_get_thread_index();
    pds_flow_hw_ctx_t *session;
    u64 timestamp;
    u64 cur_time, diff_time;

    // If the session timestamp is updated, then start idle timeout, otherwise retry
    // sending keepalives
    session = pds_flow_get_hw_ctx_lock(ses_id);
    timestamp = pds_session_get_timestamp(ses_id);
    pds_flow_hw_ctx_unlock(session);
    cur_time = pds_system_get_current_tick();
    if (timestamp && 
        !pds_flow_age_session_expired(session, cur_time, timestamp, &diff_time)) {
        // restart idle timeout with the time left for timeout
        session->timer_hdl = tw_timer_start_16t_1w_2048sl(&fm->timer_wheel[thread],
                                                          ses_id,
                                                          PDS_FLOW_IDLE_TIMER,
                                                          fm->idle_timeout[session->proto] - diff_time);
        return;
    }

    if (session->keep_alive_retry > TCP_KEEP_ALIVE_RETRY_COUNT_MAX) {
        pds_flow_delete_session(ses_id);
    }
    // send a keep alive message for both flows
    pds_flow_send_keep_alive(session);
}

static void
pds_flow_half_close_timeout (u32 ses_id)
{
    pds_flow_delete_session(ses_id);
    return;
}

static void
pds_flow_close_timeout (u32 ses_id)
{
    pds_flow_delete_session(ses_id);
    return;
}

static void
pds_flow_drop_timeout (u32 ses_id)
{
    pds_flow_delete_session(ses_id);
    return;
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

void
pds_flow_expired_timers_dispatch (u32 * expired_timers)
{
    int i;
    u32 ses_index, timer_id;
    pds_flow_hw_ctx_t *session;

    for (i = 0; i < vec_len(expired_timers); i++) {
        /* Get session index and timer id */
        ses_index = expired_timers[i] & 0x0FFFFFFF;
        timer_id = expired_timers[i] >> 28;

        /* Handle expiration */
        session = pds_flow_get_hw_ctx(ses_index);
        session->timer_hdl = ~0;
        (*flow_exp_handlers[timer_id]) (ses_index);
    }
}

uword
pds_flow_timer_init (vlib_main_t *vm, vlib_node_runtime_t *rt, vlib_frame_t *f)
{
    pds_flow_main_t *fm = &pds_flow_main;
    tw_timer_wheel_16t_1w_2048sl_t *tw;
    vlib_node_t *node = vlib_get_node_by_name(vm, (u8 *) "pds-flow-age-process");

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
        vlib_node_set_state(this_vlib_main, node->index, VLIB_NODE_STATE_POLLING);
    }));

    vlib_worker_thread_barrier_release(vm);
    return 0;
}

VLIB_REGISTER_NODE(pds_flow_timer_init_node, static) =
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

    pds_local_mapping_table_init();

    return 0;
}

VLIB_INIT_FUNCTION(pds_flow_age_init) =
{
    .runs_after = VLIB_INITS("pds_flow_init"),
};
