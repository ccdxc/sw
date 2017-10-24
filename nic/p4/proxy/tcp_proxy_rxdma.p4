/*****************************************************************************/
/* tcp_proxy_rxdma.p4
/*****************************************************************************/

#include "../common-p4+/common_rxdma_dummy.p4"

/******************************************************************************
 * Table names
 *****************************************************************************/
#define rx_table_s1_t0 tcp_rx_tcp_rx

#define rx_table_s2_t0 tcp_rx_tcp_rtt

#define rx_table_s2_t1 tcp_rx_read_rnmdr

#define rx_table_s2_t2 tcp_rx_read_rnmpr

#define rx_table_s2_t3 tcp_rx_read_serq

#define rx_table_s3_t0 tcp_rx_tcp_fra
#define rx_table_s3_t1 tcp_rx_rdesc_alloc
#define rx_table_s3_t2 tcp_rx_rpage_alloc

#define rx_table_s4_t0 tcp_rx_tcp_cc

#define rx_table_s5_t0 tcp_rx_tcp_fc

#define rx_table_s6_t0 tcp_rx_write_serq
#define rx_table_s6_t1 tcp_rx_write_arq

#define rx_table_s7_t0 tcp_rx_stats

#define common_p4plus_stage0_app_header_table tcp_rx_read_tx2rx


/******************************************************************************
 * Action names
 *****************************************************************************/
#define rx_table_s1_t0_action tcp_rx

#define rx_table_s2_t0_action tcp_rtt

#define rx_table_s2_t1_action read_rnmdr

#define rx_table_s2_t2_action read_rnmpr

#define rx_table_s2_t3_action read_serq

#define rx_table_s3_t0_action tcp_fra
#define rx_table_s3_t1_action rdesc_alloc
#define rx_table_s3_t2_action rpage_alloc

#define rx_table_s4_t0_action tcp_cc

#define rx_table_s5_t0_action tcp_fc

#define rx_table_s6_t0_action write_serq
#define rx_table_s6_t1_action write_arq

#define rx_table_s7_t0_action stats

#define common_p4plus_stage0_app_header_table_action_dummy read_tx2rx

#include "../common-p4+/common_rxdma.p4"
#include "../cpu-p4+/cpu_rx_common.p4"
#include "tcp_proxy_common.p4"

/******************************************************************************
 * Macros
 *****************************************************************************/
#define GENERATE_GLOBAL_K \
    modify_field(common_global_scratch.fid, common_phv.fid); \
    modify_field(common_global_scratch.qstate_addr, common_phv.qstate_addr); \
    modify_field(common_global_scratch.snd_una, common_phv.snd_una); \
    modify_field(common_global_scratch.pkts_acked, common_phv.pkts_acked); \
    modify_field(common_global_scratch.debug_dol, common_phv.debug_dol); \
    modify_field(common_global_scratch.quick, common_phv.quick); \
    modify_field(common_global_scratch.ca_event, common_phv.ca_event); \
    modify_field(common_global_scratch.ecn_flags, common_phv.ecn_flags); \
    modify_field(common_global_scratch.process_ack_flag, \
                            common_phv.process_ack_flag); \
    modify_field(common_global_scratch.syn, common_phv.syn); \
    modify_field(common_global_scratch.ece, common_phv.ece); \
    modify_field(common_global_scratch.is_dupack, common_phv.is_dupack); \
    modify_field(common_global_scratch.ooo_rcv, common_phv.ooo_rcv); \
    modify_field(common_global_scratch.ooo_in_rx_q, common_phv.ooo_in_rx_q); \
    modify_field(common_global_scratch.write_serq, common_phv.write_serq); \
    modify_field(common_global_scratch.pending_ack_send, common_phv.pending_ack_send); \
    modify_field(common_global_scratch.pending_del_ack_send, common_phv.pending_del_ack_send); \
    modify_field(common_global_scratch.pending_txdma, common_phv.pending_txdma); \
    modify_field(common_global_scratch.pingpong, common_phv.pingpong); \
    modify_field(common_global_scratch.rnmdr_full, common_phv.rnmdr_full); \
    modify_field(common_global_scratch.rnmpr_full, common_phv.rnmpr_full); \
    modify_field(common_global_scratch.write_arq, common_phv.write_arq); \
    modify_field(common_global_scratch.write_tcp_app_hdr, common_phv.write_tcp_app_hdr);

/******************************************************************************
 * D-vectors
 *****************************************************************************/

// d for stage 0
header_type read_tx2rxd_t {
    fields {
        rsvd                    : 8;
        cosA                    : 4;
        cosB                    : 4;
        cos_sel                 : 8;
        eval_last               : 8;
        host                    : 4;
        total                   : 4;
        pid                     : 16; // 8 bytes

        serq_ring_size          : 16;
        pad2                    : 48; // 8 bytes

        // written by TLS P4+ program
        serq_cidx               : 16;
        pad1                    : 48; // 8 bytes

        // written by TCP Tx P4+ program
        TX2RX_SHARED_STATE           // offset = 24
    }
}

// d for stage 1
header_type tcp_rx_d_t {
    fields {
        ooo_rcv_bitmap          : TCP_OOO_NUM_CELLS;
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
        rcv_mss                 : 16;
        ato                     : 16;
        debug_dol               : 8;
        rto                     : 8;
        pred_flags              : 8;
        ecn_flags               : 8;
        state                   : 8;
        quick                   : 4;
        snd_wscale              : 4;
        quick_acks_decr         : 4;
        pending                 : 3;
        ca_flags                : 2;
        write_serq              : 1;
        pending_txdma           : 1;
        fastopen_rsk            : 1;
        pingpong                : 1;
        ooo_in_rx_q             : 1;
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
        rnmdr_pidx              : 32;
        rnmdr_pidx_full         : 1;
    }
}

// d for stage 2 table 2
header_type read_rnmpr_d_t {
    fields {
        rnmpr_pidx              : 32;
        rnmpr_pidx_full         : 1;
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
        pad                     : 448;
    }
}

// d for stage 3 table 2
header_type rpage_alloc_d_t {
    fields {
        page                    : 64;
        pad                     : 448;
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
        serq_base               : 32;
        page                    : 32;
        descr                   : 32;
        page_cnt                : 16;
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
        debug_stage0_3_thread   : 16;
        debug_stage4_7_thread   : 16;
        ft_pi                   : 16;
        serq_pidx               : 16; 

        // stats
        pkts_rcvd               : 8;
        pages_alloced           : 8;
        desc_alloced            : 8;
        debug_num_pkt_to_mem    : 8;
        debug_num_phv_to_mem    : 8;
    }
}

/******************************************************************************
 * Global PHV definitions
 *****************************************************************************/
header_type to_stage_1_phv_t {
    // tcp-rx
    fields {
        flags                   : 8;
        rcv_tsval               : 24;
        seq                     : 32;
        ack_seq                 : 32;
        snd_nxt                 : 32;
    }
}

header_type to_stage_2_phv_t {
    // tcp-rtt, read-rnmdr, read-rnmpr, read-serq
    fields {
        pad1                    : 48;
        snd_nxt                 : 32;
        rcv_tsecr               : 32;
        pad2                    : 16;
    }
}

header_type to_stage_3_phv_t {
    // tcp-fra
    fields {
        pad1                    : 8;
        packets_out             : 16;
        pad2                    : 8;
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

header_type to_stage_5_phv_t {
    // tcp-fc
    fields {
        page_count              : 32;
        page                    : 32;
        descr                   : 32;
    }
}

header_type to_stage_6_phv_t {
    // write-serq
    fields {
        page                    : 32;
        descr                   : 32;
        xrq_base                : 32;
        xrq_pidx                : 16;
        payload_len             : 16;
    }
}

header_type to_stage_7_phv_t {
    // stats
    fields {
        bytes_rcvd              : 16;
        pkts_rcvd               : 8;
        pages_alloced           : 8;
        desc_alloced            : 8;
        debug_num_pkt_to_mem    : 8;
        debug_num_phv_to_mem    : 8;

        stats4                  : 16;
        stats5                  : 16;
        stats6                  : 16;
        stats7                  : 16;
    }
}

header_type common_global_phv_t {
    fields {
        // global k (max 128)
        fid                     : 24;
        qstate_addr             : 32;
        snd_una                 : 32;
        pkts_acked              : 8;
        debug_dol               : 8;
        quick                   : 3;
        ca_event                : 4;
        ecn_flags               : 2;
        process_ack_flag        : 1;
        syn                     : 1;
        ece                     : 1;
        is_dupack               : 1;
        ooo_rcv                 : 1;
        ooo_in_rx_q             : 1;
        write_serq              : 1;
        pending_ack_send        : 1;
        pending_del_ack_send    : 1;
        pending_txdma           : 1;
        pingpong                : 1;
        rnmdr_full              : 1;
        rnmpr_full              : 1;
        write_arq               : 1;
        write_tcp_app_hdr       : 1;
    }
}

/******************************************************************************
 * Stage to stage PHV definitions
 *****************************************************************************/
header_type s1_s2s_phv_t {
    fields {
        debug_stage0_3_thread   : 16;
        rcv_wup                 : 32;
        end_seq                 : 32;
        rcv_tstamp              : 32;
        packets_out             : 16;
        window                  : 16;
        ip_dsfield              : 8;
        rcv_mss_shft            : 4;
        quick_acks_decr         : 4;
    }
}

header_type s3_t1_s2s_phv_t {
    fields {
        debug_stage0_3_thread   : 16;
        debug_stage4_7_thread   : 16;
        rnmdr_pidx              : 16;
    }
}

header_type s3_t2_s2s_phv_t {
    fields {
        debug_stage0_3_thread   : 16;
        debug_stage4_7_thread   : 16;
        rnmpr_pidx              : 16;
    }
}

header_type s4_s2s_phv_t {
    fields {
        debug_stage0_3_thread   : 16;
        debug_stage4_7_thread   : 16;
        ato                     : 16;
        ooo_offset              : 16;
        packets_out             : 32;
        sacked_out              : 16;
        lost_out                : 8;
    }
}

header_type s6_s2s_phv_t {
    fields {
        debug_stage0_3_thread   : 16;
        debug_stage4_7_thread   : 16;
        ato                     : 16;
        ooo_offset              : 16;
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
metadata tcp_fc_d_t tcp_fc_d;
@pragma scratch_metadata
metadata write_serq_d_t write_serq_d;
@pragma scratch_metadata
metadata rdesc_alloc_d_t rdesc_alloc_d;
@pragma scratch_metadata
metadata rpage_alloc_d_t rpage_alloc_d;
@pragma scratch_metadata
metadata arq_rx_pi_d_t arq_rx_pi_d;

/******************************************************************************
 * Header unions for PHV layout
 *****************************************************************************/
@pragma pa_header_union ingress app_header
metadata p4_to_p4plus_tcp_proxy_base_header_t tcp_app_header;
@pragma scratch_metadata
metadata p4_to_p4plus_tcp_proxy_base_header_t tcp_scratch_app;

@pragma pa_header_union ingress to_stage_1 cpu_hdr1
metadata to_stage_1_phv_t to_s1;
@pragma dont_trim
metadata p4_to_p4plus_cpu_pkt_1_t cpu_hdr1;

@pragma pa_header_union ingress to_stage_2 cpu_hdr2
metadata to_stage_2_phv_t to_s2;
@pragma dont_trim
metadata p4_to_p4plus_cpu_pkt_2_t cpu_hdr2;

@pragma pa_header_union ingress to_stage_3 cpu_hdr3
metadata to_stage_3_phv_t to_s3;
@pragma dont_trim
metadata p4_to_p4plus_cpu_pkt_3_t cpu_hdr3;


@pragma pa_header_union ingress to_stage_4
metadata to_stage_4_phv_t to_s4;
@pragma pa_header_union ingress to_stage_5
metadata to_stage_5_phv_t to_s5;
@pragma pa_header_union ingress to_stage_6
metadata to_stage_6_phv_t to_s6;
@pragma pa_header_union ingress to_stage_7
metadata to_stage_7_phv_t to_s7;
@pragma pa_header_union ingress common_global
metadata common_global_phv_t common_phv;

@pragma scratch_metadata
metadata to_stage_1_phv_t to_s1_scratch;
@pragma scratch_metadata
metadata p4_to_p4plus_cpu_pkt_1_t to_cpu1_scratch;
@pragma scratch_metadata
metadata to_stage_2_phv_t to_s2_scratch;
@pragma scratch_metadata
metadata p4_to_p4plus_cpu_pkt_2_t to_cpu2_scratch;
@pragma scratch_metadata
metadata to_stage_3_phv_t to_s3_scratch;
@pragma scratch_metadata
metadata p4_to_p4plus_cpu_pkt_3_t to_cpu3_scratch;
@pragma scratch_metadata
metadata to_stage_4_phv_t to_s4_scratch;
@pragma scratch_metadata
metadata to_stage_5_phv_t to_s5_scratch;
@pragma scratch_metadata
metadata to_stage_6_phv_t to_s6_scratch;
@pragma scratch_metadata
metadata to_stage_7_phv_t to_s7_scratch;
@pragma scratch_metadata
metadata s1_s2s_phv_t s1_s2s_scratch;
@pragma scratch_metadata
metadata s3_t1_s2s_phv_t s3_t1_s2s_scratch;
@pragma scratch_metadata
metadata s3_t2_s2s_phv_t s3_t2_s2s_scratch;
@pragma scratch_metadata
metadata s4_s2s_phv_t s4_s2s_scratch;
@pragma scratch_metadata
metadata s6_s2s_phv_t s6_s2s_scratch;
@pragma scratch_metadata
metadata common_global_phv_t common_global_scratch;

@pragma pa_header_union ingress common_t0_s2s s4_s2s s6_s2s
metadata s1_s2s_phv_t s1_s2s;
metadata s4_s2s_phv_t s4_s2s;
metadata s6_s2s_phv_t s6_s2s;

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
metadata rx2tx_extra_t rx2tx_extra;
@pragma dont_trim
metadata ring_entry_t ring_entry; 
@pragma dont_trim
metadata doorbell_data_t db_data;
@pragma dont_trim
metadata doorbell_data_t db_data2;
@pragma dont_trim
metadata pkt_descr_t aol; 
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
metadata dma_cmd_phv2mem_t dma_cmd5;
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd6;
@pragma dont_trim
metadata dma_cmd_phv2mem_t dma_cmd7;

/******************************************************************************
 * Action functions to generate k_struct and d_struct
 *
 * These action functions are currently only to generate the k+i and d structs
 * and do not implement any pseudo code
 *****************************************************************************/

/*
 * Stage 0 table 0 action
 */
action read_tx2rx(rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid,
                  serq_ring_size, pad2, serq_cidx, pad1, prr_out,
                  snd_nxt, rcv_wup, packets_out, ecn_flags_tx, quick_acks_decr,
                  pad1_tx2rx) {
    // k + i for stage 0

    // from intrinsic
    modify_field(p4_intr_global_scratch.lif, p4_intr_global.lif);
    modify_field(p4_intr_global_scratch.tm_iq, p4_intr_global.tm_iq);
    modify_field(p4_rxdma_intr_scratch.qid, p4_rxdma_intr.qid);
    modify_field(p4_rxdma_intr_scratch.qtype, p4_rxdma_intr.qtype);
    modify_field(p4_rxdma_intr_scratch.qstate_addr, p4_rxdma_intr.qstate_addr);

    // from app header
    modify_field(tcp_scratch_app.p4plus_app_id, tcp_app_header.p4plus_app_id);
    modify_field(tcp_scratch_app.table0_valid, tcp_app_header.table0_valid);
    modify_field(tcp_scratch_app.table1_valid, tcp_app_header.table1_valid);
    modify_field(tcp_scratch_app.table2_valid, tcp_app_header.table2_valid);
    modify_field(tcp_scratch_app.table3_valid, tcp_app_header.table3_valid);
    modify_field(tcp_scratch_app.gft_flow_id, app_header.gft_flow_id);

    modify_field(tcp_scratch_app.num_sack_blocks, tcp_app_header.num_sack_blocks);
    modify_field(tcp_scratch_app.payload_len, tcp_app_header.payload_len);
    modify_field(tcp_scratch_app.srcPort, tcp_app_header.srcPort);
    modify_field(tcp_scratch_app.dstPort, tcp_app_header.dstPort);
    modify_field(tcp_scratch_app.seqNo, tcp_app_header.seqNo);
    modify_field(tcp_scratch_app.ackNo, tcp_app_header.ackNo);
    modify_field(tcp_scratch_app.dataOffset, tcp_app_header.dataOffset);
    modify_field(tcp_scratch_app.flags, tcp_app_header.flags);
    modify_field(tcp_scratch_app.window, tcp_app_header.window);
    modify_field(tcp_scratch_app.urgentPtr, tcp_app_header.urgentPtr);
    modify_field(tcp_scratch_app.ts, tcp_app_header.ts);
    modify_field(tcp_scratch_app.tcp_pad1, tcp_app_header.tcp_pad1);
    modify_field(tcp_scratch_app.prev_echo_ts, tcp_app_header.prev_echo_ts);
    modify_field(tcp_scratch_app.tcp_pad, tcp_app_header.tcp_pad);

    // d for stage 0
    modify_field(read_tx2rxd.rsvd, rsvd);
    modify_field(read_tx2rxd.cosA, cosA);
    modify_field(read_tx2rxd.cosB, cosB);
    modify_field(read_tx2rxd.cos_sel, cos_sel);
    modify_field(read_tx2rxd.eval_last, eval_last);
    modify_field(read_tx2rxd.host, host);
    modify_field(read_tx2rxd.total, total);
    modify_field(read_tx2rxd.pid, pid);
    modify_field(read_tx2rxd.serq_ring_size, serq_ring_size);
    modify_field(read_tx2rxd.pad2, pad2);
    modify_field(read_tx2rxd.serq_cidx, serq_cidx);
    modify_field(read_tx2rxd.pad1, pad1);
    modify_field(read_tx2rxd.prr_out, prr_out);
    modify_field(read_tx2rxd.snd_nxt, snd_nxt);
    modify_field(read_tx2rxd.rcv_wup, rcv_wup);
    modify_field(read_tx2rxd.packets_out, packets_out);
    modify_field(read_tx2rxd.ecn_flags_tx, ecn_flags_tx);
    modify_field(read_tx2rxd.quick_acks_decr, quick_acks_decr);
    modify_field(read_tx2rxd.pad1_tx2rx, pad1_tx2rx);
}

/*
 * Stage 1 table 0 action
 */
action tcp_rx(ooo_rcv_bitmap, rcv_nxt, rcv_tsval, rcv_tstamp, ts_recent, lrcv_time,
        snd_una, snd_wl1, retx_head_ts, rto_deadline, max_window, bytes_rcvd,
        bytes_acked, snd_wnd, rcv_mss, debug_dol, rto, pred_flags, ecn_flags, state,
        ato, quick, snd_wscale, quick_acks_decr, pending, ca_flags, write_serq,
        pending_txdma, fastopen_rsk, pingpong, ooo_in_rx_q) {
    // k + i for stage 1


    // from to_stage 1
    if (write_serq == 1) {
        modify_field(to_s1_scratch.flags, to_s1.flags);
        modify_field(to_s1_scratch.rcv_tsval, to_s1.rcv_tsval);
        modify_field(to_s1_scratch.seq, to_s1.seq);
        modify_field(to_s1_scratch.ack_seq, to_s1.ack_seq);

        modify_field(to_s1_scratch.snd_nxt, to_s1.snd_nxt);
    }

    if (write_serq == 0) {
        modify_field(to_cpu1_scratch.src_lif, cpu_hdr1.src_lif);
        modify_field(to_cpu1_scratch.lif, cpu_hdr1.lif);
        modify_field(to_cpu1_scratch.qtype, cpu_hdr1.qtype);
        modify_field(to_cpu1_scratch.qid, cpu_hdr1.qid);
        modify_field(to_cpu1_scratch.lkp_vrf, cpu_hdr1.lkp_vrf);
        modify_field(to_cpu1_scratch.pad, cpu_hdr1.pad);
        modify_field(to_cpu1_scratch.lkp_dir, cpu_hdr1.lkp_dir);
        modify_field(to_cpu1_scratch.lkp_inst, cpu_hdr1.lkp_inst);
        modify_field(to_cpu1_scratch.lkp_type, cpu_hdr1.lkp_type);
        modify_field(to_cpu1_scratch.flags, cpu_hdr1.flags);
        modify_field(to_cpu1_scratch.l2_offset, cpu_hdr1.l2_offset);
        modify_field(to_cpu1_scratch.l3_offset_1, cpu_hdr1.l3_offset_1);
    }


    // from ki global
    GENERATE_GLOBAL_K

    // from stage 0 to stage 1
    modify_field(s1_s2s_scratch.rcv_wup, s1_s2s.rcv_wup);
    modify_field(s1_s2s_scratch.end_seq, s1_s2s.end_seq);
    modify_field(s1_s2s_scratch.packets_out, s1_s2s.packets_out);
    modify_field(s1_s2s_scratch.window, s1_s2s.window);
    modify_field(s1_s2s_scratch.ip_dsfield, s1_s2s.ip_dsfield);
    modify_field(s1_s2s_scratch.rcv_mss_shft, s1_s2s.rcv_mss_shft);
    modify_field(s1_s2s_scratch.rcv_tstamp, s1_s2s.rcv_tstamp);
    modify_field(s1_s2s_scratch.quick_acks_decr, s1_s2s.quick_acks_decr);

    // d for stage 1 tcp-rx
    modify_field(tcp_rx_d.ooo_rcv_bitmap, ooo_rcv_bitmap);
    modify_field(tcp_rx_d.rcv_nxt, rcv_nxt);
    modify_field(tcp_rx_d.rcv_tsval, rcv_tsval);
    modify_field(tcp_rx_d.rcv_tstamp, rcv_tstamp);
    modify_field(tcp_rx_d.ts_recent, ts_recent);
    modify_field(tcp_rx_d.lrcv_time, lrcv_time);
    modify_field(tcp_rx_d.snd_una, snd_una);
    modify_field(tcp_rx_d.snd_wl1, snd_wl1);
    modify_field(tcp_rx_d.retx_head_ts, retx_head_ts);
    modify_field(tcp_rx_d.rto_deadline, rto_deadline);
    modify_field(tcp_rx_d.max_window, max_window);
    modify_field(tcp_rx_d.bytes_rcvd, bytes_rcvd);
    modify_field(tcp_rx_d.bytes_acked, bytes_acked);
    modify_field(tcp_rx_d.snd_wnd, snd_wnd);
    modify_field(tcp_rx_d.rcv_mss, rcv_mss);
    modify_field(tcp_rx_d.debug_dol, debug_dol);
    modify_field(tcp_rx_d.rto, rto);
    modify_field(tcp_rx_d.pred_flags, pred_flags);
    modify_field(tcp_rx_d.ecn_flags, ecn_flags);
    modify_field(tcp_rx_d.state, state);
    modify_field(tcp_rx_d.ato, ato);
    modify_field(tcp_rx_d.quick, quick);
    modify_field(tcp_rx_d.snd_wscale, snd_wscale);
    modify_field(tcp_rx_d.quick_acks_decr, quick_acks_decr);
    modify_field(tcp_rx_d.pending, pending);
    modify_field(tcp_rx_d.ca_flags, ca_flags);
    modify_field(tcp_rx_d.write_serq, write_serq);
    modify_field(tcp_rx_d.pending_txdma, pending_txdma);
    modify_field(tcp_rx_d.fastopen_rsk, fastopen_rsk);
    modify_field(tcp_rx_d.pingpong, pingpong);
    modify_field(tcp_rx_d.ooo_in_rx_q, ooo_in_rx_q);
}

/*
 * Stage 2 table 0 action
 */
action tcp_rtt(srtt_us, rto, backoff, seq_rtt_us, ca_rtt_us,
        curr_ts, rtt_min, rttvar_us, mdev_us, mdev_max_us,
        rtt_seq) {
    // k + i for stage 2

    // from to_stage 2
    if (backoff == 0) {
        modify_field(to_s2_scratch.snd_nxt, to_s2.snd_nxt);
    }
    if (backoff == 1) {
        modify_field(to_cpu2_scratch.l3_offset_2, cpu_hdr2.l3_offset_2);
        modify_field(to_cpu2_scratch.l4_offset, cpu_hdr2.l4_offset);
        modify_field(to_cpu2_scratch.payload_offset, cpu_hdr2.payload_offset);
        modify_field(to_cpu2_scratch.tcp_flags, cpu_hdr2.tcp_flags);
        modify_field(to_cpu2_scratch.tcp_seqNo, cpu_hdr2.tcp_seqNo);
        modify_field(to_cpu2_scratch.tcp_AckNo, cpu_hdr2.tcp_AckNo);
        modify_field(to_cpu2_scratch.tcp_window, cpu_hdr2.tcp_window);
    }

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
action read_rnmdr(rnmdr_pidx, rnmdr_pidx_full) {
    // d for stage 2 table 1 read-rnmdr-idx
    modify_field(read_rnmdr_d.rnmdr_pidx, rnmdr_pidx);
    modify_field(read_rnmdr_d.rnmdr_pidx_full, rnmdr_pidx_full);
}

/*
 * Stage 2 table 2 action
 */
action read_rnmpr(rnmpr_pidx, rnmpr_pidx_full) {
    // d for stage 2 table 2 read-rnmpr-idx
    modify_field(read_rnmpr_d.rnmpr_pidx, rnmpr_pidx);
    modify_field(read_rnmpr_d.rnmpr_pidx_full, rnmpr_pidx_full);
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
    if (reordering == 0) {
        modify_field(to_s3_scratch.pad1, to_s3.pad1);
        modify_field(to_s3_scratch.packets_out, to_s3.packets_out);
        modify_field(to_s3_scratch.pad2, to_s3.pad2);
        modify_field(to_s3_scratch.srtt_us, to_s3.srtt_us);
        modify_field(to_s3_scratch.undo_retrans, to_s3.undo_retrans);
        modify_field(to_s3_scratch.sacked_out, to_s3.sacked_out);
        modify_field(to_s3_scratch.lost_out, to_s3.lost_out);
    }
    if (reordering == 1) {
        modify_field(to_cpu3_scratch.tcp_options, cpu_hdr3.tcp_options);
        modify_field(to_cpu3_scratch.tcp_mss, cpu_hdr3.tcp_mss);
        modify_field(to_cpu3_scratch.tcp_ws, cpu_hdr3.tcp_ws);
    }

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
action rdesc_alloc(desc, pad) {
    // k + i for stage 3 table 1

    // from to_stage 3

    // from ki global
    GENERATE_GLOBAL_K

    // from stage 2 to stage 3
    modify_field(s3_t1_s2s_scratch.rnmdr_pidx, s3_t1_s2s.rnmdr_pidx);

    // d for stage 3 table 1
    modify_field(rdesc_alloc_d.desc, desc);
    modify_field(rdesc_alloc_d.pad, pad);
}

/*
 * Stage 3 table 2 action
 */
action rpage_alloc(page, pad) {
    // k + i for stage 3 table 2

    // from to_stage 3

    // from ki global
    GENERATE_GLOBAL_K

    // from stage 2 to stage 3
    modify_field(s3_t2_s2s_scratch.rnmpr_pidx, s3_t2_s2s.rnmpr_pidx);

    // d for stage 3 table 2
    modify_field(rpage_alloc_d.page, page);
    modify_field(rpage_alloc_d.pad, pad);
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
    modify_field(s4_s2s_scratch.debug_stage0_3_thread, s4_s2s.debug_stage0_3_thread);
    modify_field(s4_s2s_scratch.debug_stage4_7_thread, s4_s2s.debug_stage4_7_thread);
    modify_field(s4_s2s_scratch.ato, s4_s2s.ato);
    modify_field(s4_s2s_scratch.ooo_offset, s4_s2s.ooo_offset);
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
action tcp_fc(serq_base, page, descr, page_cnt) {
    // k + i for stage 5

    // from to_stage 5
    modify_field(to_s5_scratch.page, to_s5.page);
    modify_field(to_s5_scratch.descr, to_s5.descr);

    // from ki global
    GENERATE_GLOBAL_K

    // from stage 4 to stage 5

    // d for stage 5 table 0
    modify_field(tcp_fc_d.serq_base, serq_base);
    modify_field(tcp_fc_d.page, page);
    modify_field(tcp_fc_d.descr, descr);
    modify_field(tcp_fc_d.page_cnt, page_cnt);
}

/*
 * Stage 6 table 0 action
 */
action write_serq(nde_addr, nde_offset, nde_len, curr_ts,
        debug_stage0_3_thread, debug_stage4_7_thread, serq_pidx, ato,
        ooo_offset,
        pkts_rcvd, pages_alloced, desc_alloced, debug_num_pkt_to_mem,
        debug_num_phv_to_mem, ft_pi) {
    // k + i for stage 6

    // from to_stage 6
    modify_field(to_s6_scratch.page, to_s6.page);
    modify_field(to_s6_scratch.descr, to_s6.descr);
    modify_field(to_s6_scratch.xrq_pidx, to_s6.xrq_pidx);
    modify_field(to_s6_scratch.xrq_base, to_s6.xrq_base);
    modify_field(to_s6_scratch.payload_len, to_s6.payload_len);

    // from ki global
    GENERATE_GLOBAL_K

    // from stage to stage
    modify_field(s6_s2s_scratch.debug_stage0_3_thread, s6_s2s.debug_stage0_3_thread);
    modify_field(s6_s2s_scratch.debug_stage4_7_thread, s6_s2s.debug_stage4_7_thread);
    modify_field(s6_s2s_scratch.ato, s6_s2s.ato);
    modify_field(s6_s2s_scratch.ooo_offset, s6_s2s.ooo_offset);

    // d for stage 6 table 0
    modify_field(write_serq_d.nde_addr, nde_addr);
    modify_field(write_serq_d.nde_offset, nde_offset);
    modify_field(write_serq_d.nde_len, nde_len);
    modify_field(write_serq_d.curr_ts, curr_ts);
    modify_field(write_serq_d.debug_stage0_3_thread, debug_stage0_3_thread);
    modify_field(write_serq_d.debug_stage4_7_thread, debug_stage4_7_thread);
    modify_field(write_serq_d.serq_pidx, serq_pidx);
    modify_field(write_serq_d.pkts_rcvd, pkts_rcvd);
    modify_field(write_serq_d.pages_alloced, pages_alloced);
    modify_field(write_serq_d.desc_alloced, desc_alloced);
    modify_field(write_serq_d.debug_num_pkt_to_mem, debug_num_pkt_to_mem);
    modify_field(write_serq_d.debug_num_phv_to_mem, debug_num_phv_to_mem);
    modify_field(write_serq_d.ft_pi, ft_pi);
}

/*
 * Stage 6 table 1 action
 */
action write_arq(ARQ_RX_PI_PARAMS) {

    // k + i for stage 6

    // from to_stage 6
    modify_field(to_s6_scratch.page, to_s6.page);
    modify_field(to_s6_scratch.descr, to_s6.descr);
    modify_field(to_s6_scratch.xrq_pidx, to_s6.xrq_pidx);
    modify_field(to_s6_scratch.xrq_base, to_s6.xrq_base);
    modify_field(to_s6_scratch.payload_len, to_s6.payload_len);

    // from ki global
    GENERATE_GLOBAL_K

    // from stage to stage

    // d for stage 6 table 1
    GENERATE_ARQ_RX_PI_D
}

/*
 * Stage 7 table 0 action
 */
action stats() {
    // k + i for stage 7

    // from to_stage 7
    modify_field(to_s7_scratch.bytes_rcvd, to_s7.bytes_rcvd);
    modify_field(to_s7_scratch.pkts_rcvd, to_s7.pkts_rcvd);
    modify_field(to_s7_scratch.pages_alloced, to_s7.pages_alloced);
    modify_field(to_s7_scratch.desc_alloced, to_s7.desc_alloced);
    modify_field(to_s7_scratch.debug_num_pkt_to_mem, to_s7.debug_num_pkt_to_mem);
    modify_field(to_s7_scratch.debug_num_phv_to_mem, to_s7.debug_num_phv_to_mem);

    // from ki global
    GENERATE_GLOBAL_K

    // from stage 6 to stage 7

    // d for stage 7 table 0
}
