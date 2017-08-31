/*****************************************************************************/
/* tcp_proxy_common.p4
/*****************************************************************************/

#include "../common-p4+/common_txdma_dummy.p4"

#define tx_table_s0_t0 tcp_tx_read_rx2tx

#define tx_table_s1_t0 tcp_tx_read_rx2tx_extra

#define tx_table_s2_t0 tcp_tx_read_sesq_ci

#define tx_table_s3_t0 tcp_tx_read_sesq

#define tx_table_s4_t0 tcp_proxy_dummy_action  // TODO

#define tx_table_s4_t1 tcp_tx_sesq_consume


#define tx_table_s0_t0_action read_rx2tx

#define tx_table_s1_t0_action read_rx2tx_extra

#define tx_table_s2_t0_action read_sesq_ci

#define tx_table_s3_t0_action read_sesq

#define tx_table_s4_t0_action tcp_proxy_dummy_action  // TODO

#define tx_table_s4_t1_action sesq_consume

#include "../common-p4+/common_txdma.p4"

#include "tcp_proxy_common.p4"

/******************************************************************************
 * Macros
 *****************************************************************************/
#define GENERATE_GLOBAL_K \
    modify_field(common_global_scratch.fid, common_phv.fid);

/******************************************************************************
 * D-vectors
 *****************************************************************************/
// d for stage 0
header_type rx2tx_d_t {
    fields {
        CAPRI_QSTATE_HEADER_COMMON

	CAPRI_QSTATE_HEADER_RING(0)

	CAPRI_QSTATE_HEADER_RING(1)

	CAPRI_QSTATE_HEADER_RING(2)

	CAPRI_QSTATE_HEADER_RING(3)

	CAPRI_QSTATE_HEADER_RING(4)

	CAPRI_QSTATE_HEADER_RING(5)

        RX2TX_SHARED_STATE
    }
}

// d for stage 1
header_type rx2tx_extra_d_t {
    fields {
        RX2TX_SHARED_EXTRA_STATE
    }
}

// d for stage 2
header_type read_sesq_ci_d_t {
    fields {
        sesq_cidx               : 16;
    }
}

// d for stage 3
header_type read_sesq_d_t {
    fields {
        sesq_fid                : 16;
        sesq_desc               : 16;
    }
}

// d for stage 4
header_type sesq_consume_d_t {
    fields {
        sesq_cidx               : 16;
    }
}

// d for stage 5
header_type tcp_tx_d_t {
    fields {
        retx_ci                 : 16;
        retx_pi                 : 16;
        sched_flag              : 8;
        retx_snd_una            : 32;
        retx_snd_nxt            : 32;
        retx_head_desc          : 16;
        retx_snd_una_cursor     : 16;
        retx_tail_desc          : 16;
        retx_snd_nxt_cursor     : 16;
        retx_xmit_cursor        : 16;
        xmit_cursor_addr        : 16;
        rcv_wup                 : 16;
        pending_ack_tx          : 1;
        pending_delayed_ack_tx  : 1;
        pending_tso_data        : 1;
    }
}

/******************************************************************************
 * Global PHV definitions
 *****************************************************************************/
header_type common_global_phv_t {
    fields {
        // global k (max 128)
        fid                     : 24;
    }
}

/******************************************************************************
 * Stage to stage PHV definitions
 *****************************************************************************/

/******************************************************************************
 * Header unions for d-vector
 *****************************************************************************/
@pragma scratch_metadata
metadata rx2tx_d_t rx2tx_d;
@pragma scratch_metadata
metadata rx2tx_extra_d_t rx2tx_extra_d;
@pragma scratch_metadata
metadata read_sesq_ci_d_t read_sesq_ci_d;
@pragma scratch_metadata
metadata read_sesq_d_t read_sesq_d;
@pragma scratch_metadata
metadata sesq_consume_d_t sesq_consume_d;
//metadata tcp_tx_d_t tcp_tx_d;

/******************************************************************************
 * Header unions for PHV layout
 *****************************************************************************/
@pragma pa_header_union ingress common_global
metadata common_global_phv_t common_phv;
@pragma scratch_metadata
metadata common_global_phv_t common_global_scratch;

/******************************************************************************
 * PHV following k (for app DMA etc.)
 *****************************************************************************/

/******************************************************************************
 * Action functions to generate k_struct and d_struct
 *
 * These action functions are currently only to generate the k+i and d structs
 * and do not implement any pseudo code
 *****************************************************************************/

#define RX2TX_PARAMS                                                                                  \
rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid, pi_0,ci_0, pi_1, ci_1, pi_2, ci_2, pi_3, ci_3, pi_4, ci_4, pi_5,ci_5,snd_una, rcv_nxt, snd_wnd, snd_wl1, snd_up, write_seq, tso_seq, snd_cwnd
#

#define GENERATE_RX2TX_D                                                                               \
    modify_field(rx2tx_d.rsvd, rsvd);                                                                  \
    modify_field(rx2tx_d.cosA, cosA);                                                                  \
    modify_field(rx2tx_d.cosB, cosB);                                                                  \
    modify_field(rx2tx_d.cos_sel, cos_sel);                                                            \
    modify_field(rx2tx_d.eval_last, eval_last);                                                        \
    modify_field(rx2tx_d.host, host);                                                                  \
    modify_field(rx2tx_d.total, total);                                                                \
    modify_field(rx2tx_d.pid, pid);                                                                    \
    modify_field(rx2tx_d.pi_0, pi_0);                                                                  \
    modify_field(rx2tx_d.ci_0, ci_0);                                                                  \
    modify_field(rx2tx_d.pi_1, pi_1);                                                                  \
    modify_field(rx2tx_d.ci_1, ci_1);                                                                  \
    modify_field(rx2tx_d.pi_2, pi_2);                                                                  \
    modify_field(rx2tx_d.ci_2, ci_2);                                                                  \
    modify_field(rx2tx_d.pi_3, pi_3);                                                                  \
    modify_field(rx2tx_d.ci_3, ci_3);                                                                  \
    modify_field(rx2tx_d.pi_4, pi_4);                                                                  \
    modify_field(rx2tx_d.ci_4, ci_4);                                                                  \
    modify_field(rx2tx_d.pi_5, pi_5);                                                                  \
    modify_field(rx2tx_d.ci_5, ci_5);                                                                  \
    modify_field(rx2tx_d.snd_una, snd_una);                                                            \
    modify_field(rx2tx_d.rcv_nxt, rcv_nxt);                                                            \
    modify_field(rx2tx_d.snd_wnd, snd_wnd);                                                            \
    modify_field(rx2tx_d.snd_wl1, snd_wl1);                                                            \
    modify_field(rx2tx_d.snd_up, snd_up);                                                              \
    modify_field(rx2tx_d.write_seq, write_seq);                                                        \
    modify_field(rx2tx_d.tso_seq, tso_seq);                                                            \
    modify_field(rx2tx_d.snd_cwnd, snd_cwnd);



/*
 * Stage 0 table 0 action
 */
action read_rx2tx(RX2TX_PARAMS) {

    // k + i for stage 0

    // from ki global
    GENERATE_GLOBAL_K

    // d for stage 0
    GENERATE_RX2TX_D
}

/*
 * Stage 1 table 0 action
 */
action read_rx2tx_extra(pending_challenge_ack_send, pending_ack_send, pending_sync_mss, pending_tso_keepalive,
       pending_tso_pmtu_probe, pending_tso_data, pending_tso_probe_data, pending_tso_probe,
       pending_ooo_se_recv, pending_tso_retx, pending_ft_clear, pending_ft_reset, pending_xmit, pending,
       ack_blocked, ack_pending, snd_wscale, rcv_mss_shft, rcv_mss, rto, ca_state, ato_deadline, rto_deadline,
       retx_head_ts, srtt_us, rcv_wnd, prior_ssthresh, high_seq,  sacked_out,
       lost_out, retrans_out, fackets_out, ooo_datalen, quick,
       pingpong, backoff, dsack, num_sacks, reordering, undo_marker,
       undo_retrans, snd_ssthresh, loss_cwnd, ecn_flags) {

    // from ki global
    GENERATE_GLOBAL_K

    // d for stage 0
    modify_field(rx2tx_extra_d.pending_challenge_ack_send, pending_challenge_ack_send);
    modify_field(rx2tx_extra_d.pending_ack_send, pending_ack_send);
    modify_field(rx2tx_extra_d.pending_sync_mss, pending_sync_mss);
    modify_field(rx2tx_extra_d.pending_tso_keepalive, pending_tso_keepalive);
    modify_field(rx2tx_extra_d.pending_tso_pmtu_probe, pending_tso_pmtu_probe);
    modify_field(rx2tx_extra_d.pending_tso_data, pending_tso_data);
    modify_field(rx2tx_extra_d.pending_tso_probe_data, pending_tso_probe_data);
    modify_field(rx2tx_extra_d.pending_tso_probe, pending_tso_probe);
    modify_field(rx2tx_extra_d.pending_ooo_se_recv, pending_ooo_se_recv);
    modify_field(rx2tx_extra_d.pending_tso_retx, pending_tso_retx);
    modify_field(rx2tx_extra_d.pending_ft_clear, pending_ft_clear);
    modify_field(rx2tx_extra_d.pending_ft_reset, pending_ft_reset);
    modify_field(rx2tx_extra_d.pending, pending);
    modify_field(rx2tx_extra_d.ack_blocked, ack_blocked);
    modify_field(rx2tx_extra_d.ack_pending, ack_pending);
    modify_field(rx2tx_extra_d.snd_wscale, snd_wscale);
    modify_field(rx2tx_extra_d.rcv_mss_shft, rcv_mss_shft);
    modify_field(rx2tx_extra_d.rcv_mss, rcv_mss);
    modify_field(rx2tx_extra_d.rto, rto);
    modify_field(rx2tx_extra_d.ca_state, ca_state);
    modify_field(rx2tx_extra_d.ato_deadline, ato_deadline);
    modify_field(rx2tx_extra_d.rto_deadline, rto_deadline);
    modify_field(rx2tx_extra_d.retx_head_ts, retx_head_ts);
    modify_field(rx2tx_extra_d.srtt_us, srtt_us);
    modify_field(rx2tx_extra_d.rcv_wnd, rcv_wnd);
    modify_field(rx2tx_extra_d.prior_ssthresh, prior_ssthresh);
    modify_field(rx2tx_extra_d.high_seq, high_seq);
    modify_field(rx2tx_extra_d.sacked_out, sacked_out);
    modify_field(rx2tx_extra_d.lost_out, lost_out);
    modify_field(rx2tx_extra_d.retrans_out, retrans_out);
    modify_field(rx2tx_extra_d.fackets_out, fackets_out);
    modify_field(rx2tx_extra_d.ooo_datalen, ooo_datalen);
    modify_field(rx2tx_extra_d.quick, quick);
    modify_field(rx2tx_extra_d.pingpong, pingpong);
    modify_field(rx2tx_extra_d.backoff, backoff);
    modify_field(rx2tx_extra_d.dsack, dsack);
    modify_field(rx2tx_extra_d.num_sacks, num_sacks);
    modify_field(rx2tx_extra_d.reordering, reordering);
    modify_field(rx2tx_extra_d.undo_marker, undo_marker);
    modify_field(rx2tx_extra_d.undo_retrans, undo_retrans);
    modify_field(rx2tx_extra_d.snd_ssthresh, snd_ssthresh);
    modify_field(rx2tx_extra_d.loss_cwnd, loss_cwnd);
    modify_field(rx2tx_extra_d.ecn_flags, ecn_flags);
}

/*
 * Stage 2 table 0 action
 */
action read_sesq_ci(sesq_cidx) {

    // from ki global
    GENERATE_GLOBAL_K

    // d for stage 0
    modify_field(read_sesq_ci_d.sesq_cidx, sesq_cidx);
}

/*
 * Stage 3 table 0 action
 */
action read_sesq(sesq_fid, sesq_desc) {

    // from ki global
    GENERATE_GLOBAL_K

    // d for stage 0
    modify_field(read_sesq_d.sesq_fid, sesq_fid);
    modify_field(read_sesq_d.sesq_desc, sesq_desc);
}

/*
 * Stage 4 table 0 action
 */
#if 0
action tcp_tx(retx_ci, retx_pi, sched_flag, retx_snd_una, retx_snd_nxt,
        retx_head_desc, retx_snd_una_cursor, retx_tail_desc, retx_snd_nxt_cursor,
        retx_xmit_cursor, xmit_cursor_addr, rcv_wup, pending_ack_tx,
        pending_delayed_ack_tx, pending_tso_data) {

    // from ki global
    GENERATE_GLOBAL_K

    // d for stage 0
    modify_field(tcp_tx_d.retx_ci, retx_ci);
    modify_field(tcp_tx_d.retx_pi, retx_pi);
    modify_field(tcp_tx_d.sched_flag, sched_flag);
    modify_field(tcp_tx_d.retx_snd_una, retx_snd_una);
    modify_field(tcp_tx_d.retx_snd_nxt, retx_snd_nxt);
    modify_field(tcp_tx_d.retx_head_desc, retx_head_desc);
    modify_field(tcp_tx_d.retx_snd_una_cursor, retx_snd_una_cursor);
    modify_field(tcp_tx_d.retx_tail_desc, retx_tail_desc);
    modify_field(tcp_tx_d.retx_snd_nxt_cursor, retx_snd_nxt_cursor);
    modify_field(tcp_tx_d.retx_xmit_cursor, retx_xmit_cursor);
    modify_field(tcp_tx_d.xmit_cursor_addr, xmit_cursor_addr);
    modify_field(tcp_tx_d.rcv_wup, rcv_wup);
    modify_field(tcp_tx_d.pending_ack_tx, pending_ack_tx);
    modify_field(tcp_tx_d.pending_delayed_ack_tx, pending_delayed_ack_tx);
    modify_field(tcp_tx_d.pending_tso_data, pending_tso_data);
}
#endif

/*
 * Stage 4 table 1 action
 */
action sesq_consume(sesq_cidx) {

    // from ki global
    GENERATE_GLOBAL_K

    // d for stage 0
    modify_field(sesq_consume_d.sesq_cidx, sesq_cidx);
}

/*
 * Dummy
 */
action tcp_proxy_dummy_action(data0, data1, data2, data3, data4, data5, data6, data7) {
    modify_field(scratch_metadata0.data0, data0);
    modify_field(scratch_metadata0.data1, data1);
    modify_field(scratch_metadata0.data2, data2);
    modify_field(scratch_metadata0.data3, data3);
    modify_field(scratch_metadata0.data4, data4);
    modify_field(scratch_metadata0.data5, data5);
    modify_field(scratch_metadata0.data6, data6);
    modify_field(scratch_metadata0.data7, data7);
}

