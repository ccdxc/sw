/*****************************************************************************/
/* tcp_proxy_rxdma.p4
/*****************************************************************************/

#define rx_table_s0_t3_action tcp_proxy_dummy_action
#define rx_table_s0_t2_action tcp_proxy_dummy_action
#define rx_table_s0_t1_action tcp_proxy_dummy_action
#define rx_table_s0_t0_action tcp_proxy_dummy_action
#define rx_table_s7_t3_action tcp_proxy_dummy_action
#define rx_table_s7_t2_action tcp_proxy_dummy_action
#define rx_table_s7_t1_action tcp_proxy_dummy_action
#define rx_table_s7_t0_action tcp_proxy_dummy_action
#define rx_table_s6_t3_action tcp_proxy_dummy_action
#define rx_table_s6_t2_action tcp_proxy_dummy_action
#define rx_table_s6_t1_action tcp_proxy_dummy_action
#define rx_table_s5_t3_action tcp_proxy_dummy_action
#define rx_table_s5_t2_action tcp_proxy_dummy_action
#define rx_table_s5_t1_action tcp_proxy_dummy_action
#define rx_table_s5_t0_action tcp_proxy_dummy_action
#define rx_table_s4_t3_action tcp_proxy_dummy_action
#define rx_table_s4_t2_action tcp_proxy_dummy_action
#define rx_table_s4_t1_action tcp_proxy_dummy_action
#define rx_table_s3_t3_action tcp_proxy_dummy_action
#define rx_table_s1_t3_action tcp_proxy_dummy_action
#define rx_table_s1_t2_action tcp_proxy_dummy_action
#define rx_table_s1_t1_action tcp_proxy_dummy_action
#define rx_table_s0_t3_action tcp_proxy_dummy_action
#define rx_table_s0_t2_action tcp_proxy_dummy_action
#define rx_table_s0_t1_action tcp_proxy_dummy_action
#define rx_table_s0_t0_action tcp_proxy_dummy_action

#define common_p4plus_stage0_app_header_table tcp_rx_read_tx2rx
#define common_p4plus_stage0_app_header_table_action read_tx2rx

#define rx_table_s1_t0 tcp_rx_tcp_rx
#define rx_table_s1_t0_action tcp_rx

#define rx_table_s2_t0 tcp_rx_tcp_rtt
#define rx_table_s2_t0_action tcp_rtt

#define rx_table_s2_t1 tcp_rx_read_rnmdr
#define rx_table_s2_t1_action read_rnmdr

#define rx_table_s2_t2 tcp_rx_read_rnmpr
#define rx_table_s2_t2_action read_rnmpr

#define rx_table_s2_t3 tcp_rx_read_serq
#define rx_table_s2_t3_action read_serq

#define rx_table_s3_t0 tcp_rx_tcp_fra
#define rx_table_s3_t0_action tcp_fra

#define rx_table_s3_t1 tcp_rx_rdesc_alloc
#define rx_table_s3_t1_action rdesc_alloc

#define rx_table_s3_t2 tcp_rx_rpage_alloc
#define rx_table_s3_t2_action rpage_alloc

#define rx_table_s4_t0 tcp_rx_tcp_cc
#define rx_table_s4_t0_action tcp_cc

#define rx_table_s6_t0 tcp_rx_write_serq
#define rx_table_s6_t0_action write_serq

#include "../common-p4+/common_rxdma.p4"
#include "tcp_proxy_common.p4"

/******************************************************************************
 * Macros
 *****************************************************************************/
#define GENERATE_GLOBAL_K \
    modify_field(common_global_scratch.fid, common_phv.fid); \
    modify_field(common_global_scratch.rcv_tsecr, common_phv.rcv_tsecr); \
    modify_field(common_global_scratch.snd_una, common_phv.snd_una); \
    modify_field(common_global_scratch.pkts_acked, common_phv.pkts_acked); \
    modify_field(common_global_scratch.quick, common_phv.quick); \
    modify_field(common_global_scratch.ca_event, common_phv.ca_event); \
    modify_field(common_global_scratch.ecn_flags, common_phv.ecn_flags); \
    modify_field(common_global_scratch.process_ack_flag, \
                            common_phv.process_ack_flag); \
    modify_field(common_global_scratch.syn, common_phv.syn); \
    modify_field(common_global_scratch.ece, common_phv.ece); \
    modify_field(common_global_scratch.is_dupack, common_phv.is_dupack); \
    modify_field(common_global_scratch.ooo_rcv, common_phv.ooo_rcv); \
    modify_field(common_global_scratch.write_serq, common_phv.write_serq); \
    modify_field(common_global_scratch.pending_ack_send, common_phv.pending_ack_send); \
    modify_field(common_global_scratch.pending_txdma, common_phv.pending_txdma); \
    modify_field(common_global_scratch.pending_sync_mss, common_phv.pending_sync_mss); \
    modify_field(common_global_scratch.pingpong, common_phv.pingpong);

/******************************************************************************
 * D-vectors
 *****************************************************************************/

// d for stage 0
header_type read_tx2rxd_t {
    fields {
        pc                      : 8;
        rsvd                    : 8;
        cosA                    : 4;
        cosB                    : 4;
        cos_sel                 : 8;
        eval_last               : 8;
        host                    : 4;
        total                   : 4;
        pid                     : 16;

        prr_out                 : 32;
        snd_nxt                 : 32;
        ecn_flags_tx            : 8;
        packets_out             : 16;
    }
}

// d for stage 1
header_type tcp_rx_d_t {
    fields {
        rcv_nxt                 : 32;
        rcv_tsval               : 32;
        rcv_tstamp              : 32;
        ts_recent               : 32;
        lrcv_time               : 32;
        snd_una                 : 32;
        snd_wl1                 : 32;
        retx_head_ts            : 32;
        rto_deadline            : 32;
        max_window              : 16;
        bytes_rcvd              : 16;
        bytes_acked             : 16;
        snd_wnd                 : 16;
        rto                     : 8;
        pred_flags              : 8;
        ecn_flags               : 8;
        ato                     : 4;
        quick                   : 4;
        snd_wscale              : 4;
        pending                 : 3;
        ca_flags                : 2;
        write_serq              : 1;
        pending_txdma           : 1;
        fastopen_rsk            : 1;
        pingpong                : 1;
    }
}

// d for stage 2
header_type tcp_rtt_d_t {
    fields {
        srtt_us                 : 32;
        seq_rtt_us              : 32;
        ca_rtt_us               : 32;
        curr_ts                 : 32;
        rtt_min                 : 32;
        rttvar_us               : 32;
        mdev_us                 : 32;
        mdev_max_us             : 32;
        rtt_seq                 : 32;
        rto                     : 8;
        backoff                 : 4;
    }
}

// d for stage 2 table 1
header_type read_rnmdr_d_t {
    fields {
        rnmdr_pidx              : 16;
    }
}

// d for stage 2 table 2
header_type read_rnmpr_d_t {
    fields {
        rnmpr_pidx              : 16;
    }
}

// d for stage 2 table 3
header_type read_serq_d_t {
    fields {
        serq_pidx               : 16;
    }
}

// d for stage 3 table 0
header_type tcp_fra_d_t {
    fields {
        curr_ts                 : 32;
        reordering              : 32;
        retx_head_ts            : 32;
        high_seq                : 32;
        undo_marker             : 32;
        ack_flag                : 16;
        fackets_out             : 16;
        prior_ssthresh          : 16;
        snd_ssthresh            : 16;
        snd_cwnd                : 16;
        loss_cwnd               : 16;
        pkts_acked              : 8;
        ca_state                : 8;
        fast_rexmit             : 2;
        do_lost                 : 1;
    }
}

// d for stage 3 table 1
header_type rdesc_alloc_d_t {
    fields {
        desc                    : 64;
    }
}

// d for stage 3 table 2
header_type rpage_alloc_d_t {
    fields {
        page                    : 64;
    }
}

// d for stage 4 table 0
header_type tcp_cc_d_t {
    fields {
        curr_ts                 : 32;
        prr_delivered           : 32;
        last_time               : 32;
        epoch_start             : 32;
        cnt                     : 32;
        last_max_cwnd           : 32;
        delayed_ack             : 32;
        snd_cwnd_cnt            : 32;
        snd_cwnd_clamp          : 32;
        prior_cwnd              : 16;
        snd_cwnd                : 16;
        last_cwnd               : 16;
        tune_reordering         : 8;
        sack_reordering         : 8;
        max_packets_out         : 8;
        is_cwnd_limited         : 8;
    }
}

// d for stage 5 table 0
header_type tcp_fc_d_t {
    fields {
        dummy                   : 16;
    }
}

// d for stage 6 table 0
header_type write_serq_d_t {
    fields {
        nde_addr                : 64;
        nde_offset              : 16;
        nde_len                 : 16;
        curr_ts                 : 32;
    }
}

/******************************************************************************
 * Global PHV definitions
 *****************************************************************************/
header_type to_stage_1_phv_t {
    // tcp-rx
    fields {
        seq                     : 32;
        ack_seq                 : 32;
        rcv_tsval               : 32;
        snd_nxt                 : 32;
    }
}

header_type to_stage_2_phv_t {
    // tcp-rtt, read-rnmdr, read-rnmpr, read-serq
    fields {
        snd_nxt                 : 32;
    }
}

header_type to_stage_3_phv_t {
    // tcp-fra
    fields {
        packets_out             : 32;
        srtt_us                 : 32;
        undo_retrans            : 32;
        sacked_out              : 16;
        lost_out                : 8;
    }
}

header_type to_stage_4_phv_t {
    // tcp-cc
    fields {
        prr_out                 : 32;
        snd_cwnd                : 16;
        loss_cwnd               : 16;
        snd_ssthresh            : 16;
        ca_state                : 8;
        retrans_out             : 8;
    }
}

header_type to_stage_6_phv_t {
    // write-serq
    fields {
        page                    : 32;
        descr                   : 32;
        serq_pidx               : 16;
    }
}

header_type common_global_phv_t {
    fields {
        // global k (max 128)
        fid                     : 24;
        rcv_tsecr               : 32;
        snd_una                 : 32;
        pkts_acked              : 8;
        quick                   : 4;
        ca_event                : 4;
        ecn_flags               : 2;
        process_ack_flag        : 1;
        syn                     : 1;
        ece                     : 1;
        is_dupack               : 1;
        ooo_rcv                 : 1;
        write_serq              : 1;
        pending_ack_send        : 1;
        pending_txdma           : 1;
        pending_sync_mss        : 1;
        pingpong                : 1;
    }
}

/******************************************************************************
 * Stage to stage PHV definitions
 *****************************************************************************/
header_type s1_s2s_phv_t {
    fields {
        end_seq                 : 32;
        rcv_tstamp              : 32;
        packets_out             : 16;
        window                  : 16;
        ip_dsfield              : 8;
        rcv_mss_shft            : 4;
    }
}

header_type s3_t1_s2s_phv_t {
    fields {
        rnmdr_pidx              : 16;
    }
}

header_type s3_t2_s2s_phv_t {
    fields {
        rnmpr_pidx              : 16;
    }
}

header_type s4_s2s_phv_t {
    fields {
        packets_out             : 32;
        sacked_out              : 16;
        lost_out                : 8;
    }
}

/******************************************************************************
 * Header unions for d-vector
 *****************************************************************************/
@pragma scratch_metadata
metadata read_tx2rxd_t read_tx2rxd;
@pragma scratch_metadata
metadata tcp_rx_d_t tcp_rx_d;
@pragma scratch_metadata
metadata tcp_rtt_d_t tcp_rtt_d;
@pragma scratch_metadata
metadata read_rnmdr_d_t read_rnmdr_d;
@pragma scratch_metadata
metadata read_rnmpr_d_t read_rnmpr_d;
@pragma scratch_metadata
metadata read_serq_d_t read_serq_d;
@pragma scratch_metadata
metadata tcp_fra_d_t tcp_fra_d;
@pragma scratch_metadata
metadata tcp_cc_d_t tcp_cc_d;
@pragma scratch_metadata
metadata write_serq_d_t write_serq_d;
@pragma scratch_metadata
metadata rdesc_alloc_d_t rdesc_alloc_d;
@pragma scratch_metadata
metadata rpage_alloc_d_t rpage_alloc_d;

/******************************************************************************
 * Header unions for PHV layout
 *****************************************************************************/
@pragma pa_header_union ingress app_header
metadata p4_to_p4plus_tcp_proxy_base_header_t tcp_app_header;
@pragma scratch_metadata
metadata p4_to_p4plus_tcp_proxy_base_header_t tcp_scratch_app;

@pragma pa_header_union ingress to_stage_1
metadata to_stage_1_phv_t to_s1;
@pragma pa_header_union ingress to_stage_2
metadata to_stage_2_phv_t to_s2;
@pragma pa_header_union ingress to_stage_3
metadata to_stage_3_phv_t to_s3;
@pragma pa_header_union ingress to_stage_4
metadata to_stage_4_phv_t to_s4;
@pragma pa_header_union ingress to_stage_6
metadata to_stage_6_phv_t to_s6;
@pragma pa_header_union ingress common_global
metadata common_global_phv_t common_phv;

@pragma scratch_metadata
metadata to_stage_1_phv_t to_s1_scratch;
@pragma scratch_metadata
metadata to_stage_2_phv_t to_s2_scratch;
@pragma scratch_metadata
metadata to_stage_3_phv_t to_s3_scratch;
@pragma scratch_metadata
metadata to_stage_4_phv_t to_s4_scratch;
@pragma scratch_metadata
metadata to_stage_6_phv_t to_s6_scratch;
@pragma scratch_metadata
metadata s1_s2s_phv_t s1_s2s_scratch;
@pragma scratch_metadata
metadata s3_t1_s2s_phv_t s3_t1_s2s_scratch;
@pragma scratch_metadata
metadata s3_t2_s2s_phv_t s3_t2_s2s_scratch;
@pragma scratch_metadata
metadata s4_s2s_phv_t s4_s2s_scratch;
@pragma scratch_metadata
metadata common_global_phv_t common_global_scratch;

@pragma pa_header_union ingress common_t0_s2s s4_s2s
metadata s1_s2s_phv_t s1_s2s;
metadata s4_s2s_phv_t s4_s2s;

@pragma pa_header_union ingress common_t1_s2s
metadata s3_t1_s2s_phv_t s3_t1_s2s;

@pragma pa_header_union ingress common_t2_s2s
metadata s3_t2_s2s_phv_t s3_t2_s2s;

/******************************************************************************
 * PHV following k (for app DMA etc.)
 *****************************************************************************/
@pragma dont_trim
metadata rx2tx_t rx2tx;
@pragma dont_trim
metadata pkt_descr_t aol; 
@pragma dont_trim
metadata ring_entry_t ring_entry; 
@pragma dont_trim
metadata doorbell_data_t db_data;
@pragma dont_trim
metadata dma_cmd_pkt2mem_t dma_cmd0;
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd1;
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd2;
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd3;
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd4;
@pragma dont_trim
metadata dma_cmd_generic_t dma_cmd5;

/******************************************************************************
 * Action functions to generate k_struct and d_struct
 *
 * These action functions are currently only to generate the k+i and d structs
 * and do not implement any pseudo code
 *****************************************************************************/

/*
 * Stage 0 table 0 action
 */
action read_tx2rx(rsvd, prr_out, snd_nxt, ecn_flags_tx, packets_out) {
    // k + i for stage 0
    //modify_field(tcp_scratch_app.app_type, tcp_app_header.app_type);
    modify_field(tcp_scratch_app.num_sack_blocks, tcp_app_header.num_sack_blocks);
    modify_field(tcp_scratch_app.payload_len, tcp_app_header.payload_len);
    //modify_field(tcp_scratch_app.srcPort, tcp_app_header.srcPort);
    //modify_field(tcp_scratch_app.dstPort, tcp_app_header.dstPort);
    modify_field(tcp_scratch_app.seqNo, tcp_app_header.seqNo);
    modify_field(tcp_scratch_app.ackNo, tcp_app_header.ackNo);
    modify_field(tcp_scratch_app.dataOffset, tcp_app_header.dataOffset);
    modify_field(tcp_scratch_app.res, tcp_app_header.res);
    modify_field(tcp_scratch_app.flags, tcp_app_header.flags);
    modify_field(tcp_scratch_app.window, tcp_app_header.window);
    modify_field(tcp_scratch_app.urgentPtr, tcp_app_header.urgentPtr);
    modify_field(tcp_scratch_app.ts, tcp_app_header.ts);
    modify_field(tcp_scratch_app.prev_echo_ts, tcp_app_header.prev_echo_ts);
    //modify_field(tcp_scratch_app.gft_flow_id, tcp_app_header.gft_flow_id);

    // d for stage 0
    modify_field(read_tx2rxd.rsvd, rsvd);
    modify_field(read_tx2rxd.prr_out, prr_out);
    modify_field(read_tx2rxd.snd_nxt, snd_nxt);
    modify_field(read_tx2rxd.ecn_flags_tx, ecn_flags_tx);
    modify_field(read_tx2rxd.packets_out, packets_out);
}

/*
 * Stage 1 table 0 action
 */
action tcp_rx(rcv_nxt, rcv_tsval, rcv_tstamp, ts_recent, lrcv_time, snd_una,
        snd_wl1, retx_head_ts, rto_deadline, max_window, bytes_rcvd,
        bytes_acked, snd_wnd, rto, pred_flags, ecn_flags, ato, quick,
        snd_wscale, pending, ca_flags, write_serq, pending_txdma, fastopen_rsk,
        pingpong) {
    // k + i for stage 1

    // from to_stage 1
    modify_field(to_s1_scratch.seq, to_s1.seq);
    modify_field(to_s1_scratch.ack_seq, to_s1.ack_seq);
    modify_field(to_s1_scratch.rcv_tsval, to_s1.rcv_tsval);
    modify_field(to_s1_scratch.snd_nxt, to_s1.snd_nxt);

    // from ki global
    GENERATE_GLOBAL_K

    // from stage 0 to stage 1
    modify_field(s1_s2s_scratch.end_seq, s1_s2s.end_seq);
    modify_field(s1_s2s_scratch.packets_out, s1_s2s.packets_out);
    modify_field(s1_s2s_scratch.window, s1_s2s.window);
    modify_field(s1_s2s_scratch.ip_dsfield, s1_s2s.ip_dsfield);
    modify_field(s1_s2s_scratch.rcv_mss_shft, s1_s2s.rcv_mss_shft);
    modify_field(s1_s2s_scratch.rcv_tstamp, s1_s2s.rcv_tstamp);

    // d for stage 1 tcp-rx
    modify_field(tcp_rx_d.rcv_nxt, rcv_nxt);
    modify_field(tcp_rx_d.rcv_tsval, rcv_tsval);
    modify_field(tcp_rx_d.rcv_tstamp, rcv_tstamp);
    modify_field(tcp_rx_d.ts_recent, ts_recent);
    modify_field(tcp_rx_d.write_serq, write_serq);
    modify_field(tcp_rx_d.bytes_rcvd, bytes_rcvd);
    modify_field(tcp_rx_d.pending_txdma, pending_txdma);
    modify_field(tcp_rx_d.ato, ato);
    modify_field(tcp_rx_d.quick, quick);
    modify_field(tcp_rx_d.rto, rto);
    modify_field(tcp_rx_d.lrcv_time, lrcv_time);
    modify_field(tcp_rx_d.snd_una, snd_una);
    modify_field(tcp_rx_d.max_window, max_window);
    modify_field(tcp_rx_d.pending, pending);
    modify_field(tcp_rx_d.snd_wl1, snd_wl1);
    modify_field(tcp_rx_d.bytes_acked, bytes_acked);
    modify_field(tcp_rx_d.snd_wscale, snd_wscale);
    modify_field(tcp_rx_d.snd_wnd, snd_wnd);
    modify_field(tcp_rx_d.pred_flags, pred_flags);
    modify_field(tcp_rx_d.ecn_flags, ecn_flags);
    modify_field(tcp_rx_d.ca_flags, ca_flags);
    modify_field(tcp_rx_d.fastopen_rsk, fastopen_rsk);
    modify_field(tcp_rx_d.retx_head_ts, retx_head_ts);
    modify_field(tcp_rx_d.rto_deadline, rto_deadline);
    modify_field(tcp_rx_d.pingpong, pingpong);
}

/*
 * Stage 2 table 0 action
 */
action tcp_rtt(srtt_us, rto, backoff, seq_rtt_us, ca_rtt_us,
        curr_ts, rtt_min, rttvar_us, mdev_us, mdev_max_us,
        rtt_seq) {
    // k + i for stage 2

    // from to_stage 2
    modify_field(to_s2_scratch.snd_nxt, to_s2.snd_nxt);

    // from ki global
    GENERATE_GLOBAL_K

    // from stage 1 to stage 2

    // d for stage 2 tcp-rtt
    modify_field(tcp_rtt_d.srtt_us, srtt_us);
    modify_field(tcp_rtt_d.rto, rto);
    modify_field(tcp_rtt_d.backoff, backoff);
    modify_field(tcp_rtt_d.seq_rtt_us, seq_rtt_us);
    modify_field(tcp_rtt_d.ca_rtt_us, ca_rtt_us);
    modify_field(tcp_rtt_d.curr_ts, curr_ts);
    modify_field(tcp_rtt_d.rtt_min, rtt_min);
    modify_field(tcp_rtt_d.rttvar_us, rttvar_us);
    modify_field(tcp_rtt_d.mdev_us, mdev_us);
    modify_field(tcp_rtt_d.mdev_max_us, mdev_max_us);
    modify_field(tcp_rtt_d.rtt_seq, rtt_seq);
}

/*
 * Stage 2 table 1 action
 */
action read_rnmdr(rnmdr_pidx) {
    // d for stage 2 table 1 read-rnmdr-idx
    modify_field(read_rnmdr_d.rnmdr_pidx, rnmdr_pidx);
}

/*
 * Stage 2 table 2 action
 */
action read_rnmpr(rnmpr_pidx) {
    // d for stage 2 table 2 read-rnmpr-idx
    modify_field(read_rnmpr_d.rnmpr_pidx, rnmpr_pidx);
}

/*
 * Stage 2 table 3 action
 */
action read_serq(serq_pidx) {
    // d for stage 2 table 3 read-serq-idx
    modify_field(read_serq_d.serq_pidx, serq_pidx);
}

/*
 * Stage 3 table 0 action
 */
action tcp_fra(curr_ts, reordering, retx_head_ts, high_seq, undo_marker,
        ack_flag, fackets_out, prior_ssthresh, snd_ssthresh, snd_cwnd,
        loss_cwnd, pkts_acked, ca_state, fast_rexmit, do_lost) {
    // k + i for stage 3

    // from to_stage 3
    modify_field(to_s3_scratch.packets_out, to_s3.packets_out);
    modify_field(to_s3_scratch.srtt_us, to_s3.srtt_us);
    modify_field(to_s3_scratch.undo_retrans, to_s3.undo_retrans);
    modify_field(to_s3_scratch.sacked_out, to_s3.sacked_out);
    modify_field(to_s3_scratch.lost_out, to_s3.lost_out);

    // from ki global
    GENERATE_GLOBAL_K

    // from stage 2 to stage 3

    // d for stage 3 table 0
    modify_field(tcp_fra_d.curr_ts, curr_ts);
    modify_field(tcp_fra_d.reordering, reordering);
    modify_field(tcp_fra_d.retx_head_ts, retx_head_ts);
    modify_field(tcp_fra_d.high_seq, high_seq);
    modify_field(tcp_fra_d.undo_marker, undo_marker);
    modify_field(tcp_fra_d.ack_flag, ack_flag);
    modify_field(tcp_fra_d.fackets_out, fackets_out);
    modify_field(tcp_fra_d.prior_ssthresh, prior_ssthresh);
    modify_field(tcp_fra_d.snd_ssthresh, snd_ssthresh);
    modify_field(tcp_fra_d.snd_cwnd, snd_cwnd);
    modify_field(tcp_fra_d.loss_cwnd, loss_cwnd);
    modify_field(tcp_fra_d.pkts_acked, pkts_acked);
    modify_field(tcp_fra_d.ca_state, ca_state);
    modify_field(tcp_fra_d.fast_rexmit, fast_rexmit);
    modify_field(tcp_fra_d.do_lost, do_lost);
}

/*
 * Stage 3 table 1 action
 */
action rdesc_alloc(desc) {
    // k + i for stage 3 table 1

    // from to_stage 3

    // from ki global
    GENERATE_GLOBAL_K

    // from stage 2 to stage 3
    modify_field(s3_t1_s2s_scratch.rnmdr_pidx, s3_t1_s2s.rnmdr_pidx);

    // d for stage 3 table 1
    modify_field(rdesc_alloc_d.desc, desc);
}

/*
 * Stage 3 table 2 action
 */
action rpage_alloc(page) {
    // k + i for stage 3 table 2

    // from to_stage 3

    // from ki global
    GENERATE_GLOBAL_K

    // from stage 2 to stage 3
    modify_field(s3_t2_s2s_scratch.rnmpr_pidx, s3_t2_s2s.rnmpr_pidx);

    // d for stage 3 table 2
    modify_field(rpage_alloc_d.page, page);
}

/*
 * Stage 4 table 0 action
 */
action tcp_cc(curr_ts, prr_delivered, last_time, epoch_start, cnt,
        last_max_cwnd, delayed_ack, snd_cwnd_cnt, snd_cwnd_clamp, prior_cwnd,
        snd_cwnd, last_cwnd, tune_reordering, sack_reordering, max_packets_out,
        is_cwnd_limited) {
    // k + i for stage 4

    // from to_stage 4
    modify_field(to_s4_scratch.prr_out, to_s4.prr_out);
    modify_field(to_s4_scratch.snd_cwnd, to_s4.snd_cwnd);
    modify_field(to_s4_scratch.loss_cwnd, to_s4.loss_cwnd);
    modify_field(to_s4_scratch.snd_ssthresh, to_s4.snd_ssthresh);
    modify_field(to_s4_scratch.ca_state, to_s4.ca_state);
    modify_field(to_s4_scratch.retrans_out, to_s4.retrans_out);

    // from ki global
    GENERATE_GLOBAL_K

    // from stage 3 to stage 4
    modify_field(s4_s2s_scratch.packets_out, s4_s2s.packets_out);
    modify_field(s4_s2s_scratch.sacked_out, s4_s2s.sacked_out);
    modify_field(s4_s2s_scratch.lost_out, s4_s2s.lost_out);

    // d for stage 4 table 0
    modify_field(tcp_cc_d.curr_ts, curr_ts);
    modify_field(tcp_cc_d.prr_delivered, prr_delivered);
    modify_field(tcp_cc_d.last_time, last_time);
    modify_field(tcp_cc_d.epoch_start, epoch_start);
    modify_field(tcp_cc_d.cnt, cnt);
    modify_field(tcp_cc_d.last_max_cwnd, last_max_cwnd);
    modify_field(tcp_cc_d.delayed_ack, delayed_ack);
    modify_field(tcp_cc_d.snd_cwnd_cnt, snd_cwnd_cnt);
    modify_field(tcp_cc_d.snd_cwnd_clamp, snd_cwnd_clamp);
    modify_field(tcp_cc_d.prior_cwnd, prior_cwnd);
    modify_field(tcp_cc_d.snd_cwnd, snd_cwnd);
    modify_field(tcp_cc_d.last_cwnd, last_cwnd);
    modify_field(tcp_cc_d.tune_reordering, tune_reordering);
    modify_field(tcp_cc_d.sack_reordering, sack_reordering);
    modify_field(tcp_cc_d.max_packets_out, max_packets_out);
    modify_field(tcp_cc_d.is_cwnd_limited, is_cwnd_limited);
}

/*
 * Stage 5 table 0 action
 */
action tcp_fc() {
    // k + i for stage 5

    // from to_stage 5

    // from ki global
    GENERATE_GLOBAL_K

    // from stage 4 to stage 5

    // d for stage 5 table 0
}

/*
 * Stage 6 table 0 action
 */
action write_serq(nde_addr, nde_offset, nde_len, curr_ts) {
    // k + i for stage 6

    // from to_stage 6
    modify_field(to_s6_scratch.page, to_s6.page);
    modify_field(to_s6_scratch.descr, to_s6.descr);
    modify_field(to_s6_scratch.serq_pidx, to_s6.serq_pidx);

    // from ki global
    GENERATE_GLOBAL_K

    // from stage 5 to stage 6

    // d for stage 6 table 0
    modify_field(write_serq_d.nde_addr, nde_addr);
    modify_field(write_serq_d.nde_offset, nde_offset);
    modify_field(write_serq_d.nde_len, nde_len);
    modify_field(write_serq_d.curr_ts, curr_ts);
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

