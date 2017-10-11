/*****************************************************************************/
/* tcp_proxy_common.p4
/*****************************************************************************/

#include "../common-p4+/common_txdma_dummy.p4"

/******************************************************************************
 * Table names
 *****************************************************************************/
#define tx_table_s0_t0 tcp_tx_read_rx2tx

#define tx_table_s1_t0 tcp_tx_read_rx2tx_extra
#define tx_table_s1_t1 tcp_tx_read_sesq_ci

#define tx_table_s2_t0 tcp_tx_s2_t0
#define tx_table_s2_t1 tcp_tx_sesq_consume

#define tx_table_s3_t0 tcp_tx_tcp_tx

#define tx_table_s4_t0 tcp_tx_tso

#define tx_table_s5_t0 tcp_tx_stats


/******************************************************************************
 * Action names
 *****************************************************************************/
#define tx_table_s0_t0_action read_rx2tx

#define tx_table_s1_t0_action read_rx2tx_extra
#define tx_table_s1_t1_action read_sesq_ci

#define tx_table_s2_t0_action read_sesq_descr
#define tx_table_s2_t1_action sesq_consume

#define tx_table_s3_t0_action tcp_tx

#define tx_table_s4_t0_action tso

#define tx_table_s5_t0_action stats

#include "../common-p4+/common_txdma.p4"

#include "tcp_proxy_common.p4"

/******************************************************************************
 * Macros
 *****************************************************************************/
#define GENERATE_GLOBAL_K \
    modify_field(common_global_scratch.fid, common_phv.fid); \
    modify_field(common_global_scratch.qstate_addr, common_phv.qstate_addr); \
    modify_field(common_global_scratch.snd_una, common_phv.snd_una); \
    modify_field(common_global_scratch.rcv_nxt, common_phv.rcv_nxt); \
    modify_field(common_global_scratch.pending_rx2tx, common_phv.pending_rx2tx); \
    modify_field(common_global_scratch.pending_sesq, common_phv.pending_sesq); \
    modify_field(common_global_scratch.pending_ack_send, common_phv.pending_ack_send); \
    modify_field(common_global_scratch.debug_dol_dont_send_ack, common_phv.debug_dol_dont_send_ack);\
    modify_field(common_global_scratch.pending_asesq, common_phv.pending_asesq); \
    modify_field(common_global_scratch.debug_dol_dont_tx, common_phv.debug_dol_dont_tx);

/******************************************************************************
 * D-vectors
 *****************************************************************************/
// d for stage 0
header_type rx2tx_d_t {
    fields {
        CAPRI_QSTATE_HEADER_COMMON // 8 bytes (including 1 byte action pc)

        CAPRI_QSTATE_HEADER_RING(0)

        CAPRI_QSTATE_HEADER_RING(1)

        CAPRI_QSTATE_HEADER_RING(2)

        CAPRI_QSTATE_HEADER_RING(3)

        CAPRI_QSTATE_HEADER_RING(4)

        debug_dol_tx : 32;      // Total 32 bytes

        sesq_base : HBM_ADDRESS_WIDTH; // 4 bytes

        asesq_base : HBM_ADDRESS_WIDTH; // 4 bytes

        RX2TX_SHARED_STATE      // 24
    }
}

// d for stage 1
header_type rx2tx_extra_d_t {
    fields {
        RX2TX_SHARED_EXTRA_STATE
    }
}

header_type read_sesq_ci_d_t {
    fields {
        desc_addr               : 64;
    }
}

// d for stage 2
// Reads sesq descriptor, uses pkt_descr_t

// d for stage 3
header_type tcp_tx_d_t {
    fields {
        TCB_TX_SHARED_STATE
    }
}

// d for stage 4
header_type tso_d_t {
    fields {
        TCB_TX_SHARED_STATE
    }
}

/******************************************************************************
 * Global PHV definitions
 *****************************************************************************/
header_type common_global_phv_t {
    fields {
        // global k (max 128)
        fid                     : 24;
        qstate_addr             : HBM_ADDRESS_WIDTH;
        snd_una                 : SEQ_NUMBER_WIDTH;
        rcv_nxt                 : SEQ_NUMBER_WIDTH;
        pending_rx2tx           : 1;
        pending_sesq            : 1;
        pending_ack_send        : 1;
        debug_dol_dont_send_ack : 1;
        pending_asesq           : 1;
        debug_dol_dont_tx       : 1;
    }
}

header_type to_stage_1_phv_t {
    fields {
        sesq_ci_addr            : HBM_ADDRESS_WIDTH;
        pending_cidx            : 16;
    }
}

header_type to_stage_2_phv_t {
    fields {
        sesq_cidx               : 16;
    }
}

header_type to_stage_3_phv_t {
    fields {
        ooo_datalen             : 16;
        sesq_desc_addr          : HBM_ADDRESS_WIDTH;
        addr                    : HBM_ADDRESS_WIDTH;
        offset                  : OFFSET_WIDTH;
        len                     : LEN_WIDTH;
        rcv_mss_shft            : 4;
        quick                   : 4;
        pingpong                : 1;
    }
}

header_type to_stage_4_phv_t {
    fields {
        xmit_cursor_addr        : HBM_ADDRESS_WIDTH;
        xmit_cursor_offset      : OFFSET_WIDTH;
        xmit_cursor_len         : LEN_WIDTH;

        rcv_wnd                 : SEQ_NUMBER_WIDTH;
        rcv_mss                 : 16;
        ca_state                : 8;

        pending_challenge_ack_send : 1;
        pending_sync_mss        : 1;
        pending_tso_keep_alive  : 1;
        pending_tso_pmtu_probe  : 1;
        pending_tso_data        : 1;
        pending_tso_probe_data  : 1;
        pending_tso_retx        : 1;
    }
}

header_type to_stage_5_phv_t {
    // stats
    fields {
        bytes_sent              : 16;
        pkts_sent               : 8;
        debug_num_mem_to_pkt    : 8;
        debug_num_phv_to_pkt    : 8;
    }
}
/******************************************************************************
 * Stage to stage PHV definitions
 *****************************************************************************/

// 160 bytes
header_type common_t0_s2s_phv_t {
    fields {
        snd_nxt                 : SEQ_NUMBER_WIDTH;
        snd_wnd                 : SEQ_NUMBER_WIDTH;
        snd_cwnd                : WINDOW_WIDTH;
        packets_out             : 16;
        sacked_out              : 16;
        lost_out                : 8;
        retrans_out             : 8;
    }
}

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
metadata pkt_descr_aol_t read_sesq_descr_d;
@pragma scratch_metadata
metadata tcp_tx_d_t tcp_tx_d;
@pragma scratch_metadata
metadata tso_d_t tso_d;

/******************************************************************************
 * Header unions for PHV layout
 *****************************************************************************/
@pragma dont_trim
@pragma pa_header_union ingress app_header
metadata p4plus_to_p4_header_t tcp_app_header;
@pragma pa_header_union ingress common_global
metadata common_global_phv_t common_phv;
@pragma scratch_metadata
metadata common_global_phv_t common_global_scratch;

@pragma pa_header_union ingress to_stage_1
metadata to_stage_1_phv_t to_s1;
@pragma pa_header_union ingress to_stage_2
metadata to_stage_2_phv_t to_s2;
@pragma pa_header_union ingress to_stage_3
metadata to_stage_3_phv_t to_s3;
@pragma pa_header_union ingress to_stage_4
metadata to_stage_4_phv_t to_s4;
@pragma pa_header_union ingress to_stage_5
metadata to_stage_5_phv_t to_s5;

@pragma pa_header_union ingress common_t0_s2s
metadata common_t0_s2s_phv_t t0_s2s;


@pragma scratch_metadata
metadata to_stage_1_phv_t to_s1_scratch;
@pragma scratch_metadata
metadata to_stage_2_phv_t to_s2_scratch;
@pragma scratch_metadata
metadata to_stage_3_phv_t to_s3_scratch;
@pragma scratch_metadata
metadata to_stage_4_phv_t to_s4_scratch;
@pragma scratch_metadata
metadata to_stage_5_phv_t to_s5_scratch;

@pragma scratch_metadata
metadata common_t0_s2s_phv_t t0_s2s_scratch;

/******************************************************************************
 * PHV following k (for app DMA etc.)
 *****************************************************************************/
@pragma dont_trim
metadata tx2rx_t tx2rx;
#if 0
header_type txdma_phv_pad1_t {
    fields {
        txdma_phv_pad1 : 152;
    }
}
@pragma dont_trim
metadata txdma_phv_pad1_t phv_pad1;
#endif
@pragma dont_trim
metadata tcp_header_t tcp_header;
header_type txdma_max_options_t {
    fields {
        pad1           : 224;
        pad2           : 96;
    }
}
@pragma dont_trim
metadata txdma_max_options_t tcp_header_options;
header_type txdma_pad_before_dma_t {
    fields {
        pad            : 160;
    }
}
@pragma dont_trim
metadata txdma_pad_before_dma_t dma_pad;
@pragma dont_trim
metadata dma_cmd_phv2pkt_t intrinsic_dma;    // dma cmd 0
// TODO: Note intrinsic_txdma is temporary until model gets ability to DMA
// multiple PHV segments in one command
@pragma dont_trim
metadata dma_cmd_phv2pkt_t intrinsic_txdma;    // dma cmd 1
@pragma dont_trim
metadata dma_cmd_phv2pkt_t app_header_dma;   // dma cmd 2
@pragma dont_trim
metadata dma_cmd_mem2pkt_t l2l3_header_dma;  // dma cmd 3
@pragma dont_trim
metadata dma_cmd_phv2pkt_t tcp_header_dma;   // dma cmd 4
@pragma dont_trim
metadata dma_cmd_mem2pkt_t data_dma;         // dma cmd 5
@pragma dont_trim
metadata dma_cmd_phv2mem_t tx2rx_dma;        // dma cmd 6
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd7_dma;     // dma cmd 7
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd8_dma;     // dma cmd 8

/******************************************************************************
 * Action functions to generate k_struct and d_struct
 *
 * These action functions are currently only to generate the k+i and d structs
 * and do not implement any pseudo code
 *****************************************************************************/

#define RX2TX_PARAMS                                                                                  \
rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid, pi_0,ci_0, pi_1, ci_1, pi_2, ci_2, pi_3, ci_3,\
pi_4, ci_4, debug_dol_tx, sesq_base, asesq_base,snd_una, rcv_nxt, snd_wnd, ft_pi, snd_cwnd, rto_deadline, \
pending_ft_clear, pending_ft_reset


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
    modify_field(rx2tx_d.debug_dol_tx, debug_dol_tx);                                                  \
    modify_field(rx2tx_d.sesq_base, sesq_base);                                                        \
    modify_field(rx2tx_d.asesq_base,asesq_base);                                                       \
    modify_field(rx2tx_d.snd_una, snd_una);                                                            \
    modify_field(rx2tx_d.rcv_nxt, rcv_nxt);                                                            \
    modify_field(rx2tx_d.snd_wnd, snd_wnd);                                                            \
    modify_field(rx2tx_d.ft_pi, ft_pi);                                                                \
    modify_field(rx2tx_d.snd_cwnd, snd_cwnd);                                                          \
    modify_field(rx2tx_d.rto_deadline, rto_deadline);                                                  \
    modify_field(rx2tx_d.pending_ft_clear, pending_ft_clear);                                          \
    modify_field(rx2tx_d.pending_ft_reset, pending_ft_reset);



/*
 * Stage 0 table 0 action
 */
action read_rx2tx(RX2TX_PARAMS) {

    // k + i for stage 0

    // from intrinsic - defined in common_txdma.p4

    // d for stage 0
    GENERATE_RX2TX_D
}

/*
 * Stage 1 table 0 action
 */
action read_rx2tx_extra(
       ato_deadline, retx_head_ts, srtt_us, rcv_wnd, prior_ssthresh, high_seq,
       sacked_out, lost_out, retrans_out, fackets_out, ooo_datalen,
       reordering, undo_marker, undo_retrans, snd_ssthresh, loss_cwnd,
       write_seq, tso_seq, rcv_mss, rto, ca_state, ecn_flags, num_sacks,
       pending_ack_send, pending_challenge_ack_send, pending_sync_mss,
       pending_tso_keepalive, pending_tso_pmtu_probe, pending_tso_data,
       pending_tso_probe_data, pending_tso_probe,
       pending_ooo_se_recv, pending_tso_retx, pending_rexmit, pending,
       ack_blocked, ack_pending, snd_wscale, rcv_mss_shft, quick,
       pingpong, backoff, dsack) {

    // from ki global
    GENERATE_GLOBAL_K

    // from to_stage 1
    modify_field(to_s1_scratch.sesq_ci_addr, to_s1.sesq_ci_addr);
    modify_field(to_s1_scratch.pending_cidx, to_s1.pending_cidx);

    // d for stage 1
    modify_field(rx2tx_extra_d.ato_deadline, ato_deadline);
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
    modify_field(rx2tx_extra_d.reordering, reordering);
    modify_field(rx2tx_extra_d.undo_marker, undo_marker);
    modify_field(rx2tx_extra_d.undo_retrans, undo_retrans);
    modify_field(rx2tx_extra_d.snd_ssthresh, snd_ssthresh);
    modify_field(rx2tx_extra_d.loss_cwnd, loss_cwnd);
    modify_field(rx2tx_extra_d.write_seq, write_seq);
    modify_field(rx2tx_extra_d.tso_seq, tso_seq);
    modify_field(rx2tx_extra_d.rcv_mss, rcv_mss);
    modify_field(rx2tx_extra_d.rto, rto);
    modify_field(rx2tx_extra_d.ca_state, ca_state);
    modify_field(rx2tx_extra_d.ecn_flags, ecn_flags);
    modify_field(rx2tx_extra_d.num_sacks, num_sacks);
    modify_field(rx2tx_extra_d.pending_ack_send, pending_ack_send);
    modify_field(rx2tx_extra_d.pending_challenge_ack_send, pending_challenge_ack_send);
    modify_field(rx2tx_extra_d.pending_sync_mss, pending_sync_mss);
    modify_field(rx2tx_extra_d.pending_tso_keepalive, pending_tso_keepalive);
    modify_field(rx2tx_extra_d.pending_tso_pmtu_probe, pending_tso_pmtu_probe);
    modify_field(rx2tx_extra_d.pending_tso_data, pending_tso_data);
    modify_field(rx2tx_extra_d.pending_tso_probe_data, pending_tso_probe_data);
    modify_field(rx2tx_extra_d.pending_tso_probe, pending_tso_probe);
    modify_field(rx2tx_extra_d.pending_ooo_se_recv, pending_ooo_se_recv);
    modify_field(rx2tx_extra_d.pending_tso_retx, pending_tso_retx);
    modify_field(rx2tx_extra_d.pending, pending);
    modify_field(rx2tx_extra_d.pending_rexmit, pending_rexmit);
    modify_field(rx2tx_extra_d.ack_blocked, ack_blocked);
    modify_field(rx2tx_extra_d.ack_pending, ack_pending);
    modify_field(rx2tx_extra_d.snd_wscale, snd_wscale);
    modify_field(rx2tx_extra_d.rcv_mss_shft, rcv_mss_shft);
    modify_field(rx2tx_extra_d.quick, quick);
    modify_field(rx2tx_extra_d.pingpong, pingpong);
    modify_field(rx2tx_extra_d.backoff, backoff);
    modify_field(rx2tx_extra_d.dsack, dsack);
}

/*
 * Stage 1 table 1 action
 */
action read_sesq_ci(desc_addr) {

    // from ki global
    GENERATE_GLOBAL_K

    // from to_stage 1
    modify_field(to_s1_scratch.sesq_ci_addr, to_s1.sesq_ci_addr);
    modify_field(to_s1_scratch.pending_cidx, to_s1.pending_cidx);

    // d for stage 1
    modify_field(read_sesq_ci_d.desc_addr, desc_addr);
}

/*
 * Stage 2 table 0 action
 */
action read_sesq_descr(A0, O0, L0, A1, O1, L1, A2, O2, L2, next_addr, next_pkt) {
    // from ki global
    GENERATE_GLOBAL_K

    // from to_stage 2
    modify_field(to_s2_scratch.sesq_cidx, to_s2.sesq_cidx);

    // from stage to stage

    // d for stage 2
    modify_field(read_sesq_descr_d.A0, A0);
    modify_field(read_sesq_descr_d.O0, O0);
    modify_field(read_sesq_descr_d.L0, L0);
    modify_field(read_sesq_descr_d.A1, A1);
    modify_field(read_sesq_descr_d.O1, O1);
    modify_field(read_sesq_descr_d.L1, L1);
    modify_field(read_sesq_descr_d.A2, A2);
    modify_field(read_sesq_descr_d.O2, O2);
    modify_field(read_sesq_descr_d.L2, L2);
    modify_field(read_sesq_descr_d.next_addr, next_addr);
    modify_field(read_sesq_descr_d.next_pkt, next_pkt);
}

/*
 * Stage 2 table 1 action
 */
action sesq_consume() {

    // from ki global
    GENERATE_GLOBAL_K

    // from to_stage 2
    modify_field(to_s2_scratch.sesq_cidx, to_s2.sesq_cidx);

    // d for stage 2
}


/*
 * Stage 3 table 0 action
 */
action tcp_tx(TX_SHARED_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // from to_stage 3
    modify_field(to_s3_scratch.ooo_datalen, to_s3.ooo_datalen);
    modify_field(to_s3_scratch.sesq_desc_addr, to_s3.sesq_desc_addr);
    modify_field(to_s3_scratch.addr, to_s3.addr);
    modify_field(to_s3_scratch.offset, to_s3.offset);
    modify_field(to_s3_scratch.len, to_s3.len);
    modify_field(to_s3_scratch.rcv_mss_shft, to_s3.rcv_mss_shft);
    modify_field(to_s3_scratch.quick, to_s3.quick);
    modify_field(to_s3_scratch.pingpong, to_s3.pingpong);

    // from stage to stage
    modify_field(t0_s2s_scratch.snd_nxt, t0_s2s.snd_nxt);
    modify_field(t0_s2s_scratch.snd_wnd, t0_s2s.snd_wnd);
    modify_field(t0_s2s_scratch.snd_cwnd, t0_s2s.snd_cwnd);
    modify_field(t0_s2s_scratch.packets_out, t0_s2s.packets_out);
    modify_field(t0_s2s_scratch.sacked_out, t0_s2s.sacked_out);
    modify_field(t0_s2s_scratch.lost_out, t0_s2s.lost_out);
    modify_field(t0_s2s_scratch.retrans_out, t0_s2s.retrans_out);

    // d for stage 3 table 0
    GENERATE_TX_SHARED_D
}


/*
 * Stage 4 table 0 action
 */
action tso(TX_SHARED_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // from to_stage 4
    modify_field(to_s4_scratch.xmit_cursor_addr, to_s4.xmit_cursor_addr);
    modify_field(to_s4_scratch.xmit_cursor_offset, to_s4.xmit_cursor_offset);
    modify_field(to_s4_scratch.xmit_cursor_len, to_s4.xmit_cursor_len);
    modify_field(to_s4_scratch.rcv_wnd, to_s4.rcv_wnd);
    modify_field(to_s4_scratch.rcv_mss, to_s4.rcv_mss);
    modify_field(to_s4_scratch.ca_state, to_s4.ca_state);
    modify_field(to_s4_scratch.pending_challenge_ack_send, to_s4.pending_challenge_ack_send);
    modify_field(to_s4_scratch.pending_sync_mss, to_s4.pending_sync_mss);
    modify_field(to_s4_scratch.pending_tso_keep_alive, to_s4.pending_tso_keep_alive);
    modify_field(to_s4_scratch.pending_tso_pmtu_probe, to_s4.pending_tso_pmtu_probe);
    modify_field(to_s4_scratch.pending_tso_data, to_s4.pending_tso_data);
    modify_field(to_s4_scratch.pending_tso_probe_data, to_s4.pending_tso_probe_data);
    modify_field(to_s4_scratch.pending_tso_retx, to_s4.pending_tso_retx);

    // from stage to stage
    modify_field(t0_s2s_scratch.snd_nxt, t0_s2s.snd_nxt);
    modify_field(t0_s2s_scratch.snd_wnd, t0_s2s.snd_wnd);
    modify_field(t0_s2s_scratch.snd_cwnd, t0_s2s.snd_cwnd);
    modify_field(t0_s2s_scratch.packets_out, t0_s2s.packets_out);
    modify_field(t0_s2s_scratch.sacked_out, t0_s2s.sacked_out);
    modify_field(t0_s2s_scratch.lost_out, t0_s2s.lost_out);
    modify_field(t0_s2s_scratch.retrans_out, t0_s2s.retrans_out);

    // d for stage 4 table 0
    GENERATE_TX_SHARED_D
}

/*
 * Stage 5 table 0 action
 */
action stats() {
    // from ki global
    GENERATE_GLOBAL_K

    // from to_stage 5
    modify_field(to_s5_scratch.bytes_sent, to_s5.bytes_sent);
    modify_field(to_s5_scratch.pkts_sent, to_s5.pkts_sent);
    modify_field(to_s5_scratch.debug_num_phv_to_pkt, to_s5.debug_num_phv_to_pkt);
    modify_field(to_s5_scratch.debug_num_mem_to_pkt, to_s5.debug_num_mem_to_pkt);

    // d for stage 5 table 0
}
