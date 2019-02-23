/*****************************************************************************/
/* tcp_proxy_rxdma.p4
/*****************************************************************************/

#include "../common-p4+/common_rxdma_dummy.p4"

/******************************************************************************
 * Table names
 *
 * Table names have to alphabetically be in chronological order (to match with
 * common program table names), so they need to be prefixed by stage# and
 * table#
 *****************************************************************************/
#define rx_table_s1_t0 s1_t0_tcp_rx

#define rx_table_s2_t0 s2_t0_tcp_rx
#define rx_table_s2_t1 s2_t1_tcp_rx
#define rx_table_s2_t2 s2_t2_tcp_rx

#define rx_table_s3_t0 s3_t0_tcp_rx
#define rx_table_s3_t1 s3_t1_tcp_rx
#define rx_table_s3_t2 s3_t2_tcp_rx

#define rx_table_s4_t0 s4_t0_tcp_rx
#define rx_table_s4_t2 s4_t2_tcp_rx
#define rx_table_s4_t3 s4_t3_tcp_rx

#define rx_table_s5_t0 s5_t0_tcp_rx
#define rx_table_s5_t1 s5_t1_tcp_rx
#define rx_table_s5_t2 s5_t2_tcp_rx

#define rx_table_s6_t0 s6_t0_tcp_rx
#define rx_table_s6_t1 s6_t1_tcp_rx
#define rx_table_s6_t2 s6_t2_tcp_rx

#define rx_table_s7_t0 s7_t0_tcp_rx



/******************************************************************************
 * Action names
 *****************************************************************************/
#define rx_table_s1_t0_action tcp_rx

#define rx_table_s2_t0_action tcp_ack
#define rx_table_s2_t1_action read_rnmdr
#define rx_table_s2_t2_action ooo_book_keeping

#define rx_table_s3_t0_action tcp_rtt
#define rx_table_s3_t1_action rdesc_alloc
#define rx_table_s3_t2_action ooo_qbase_sem_idx

#define rx_table_s4_t0_action tcp_cc
#define rx_table_s4_t2_action ooo_qbase_alloc

#define rx_table_s5_t0_action tcp_fc
#define rx_table_s5_t2_action ooo_qbase_cb_load

#define rx_table_s6_t0_action dma
#define rx_table_s6_t1_action write_arq
#define rx_table_s6_t2_action write_ooq

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
    modify_field(common_global_scratch.debug_dol, common_phv.debug_dol); \
    modify_field(common_global_scratch.quick, common_phv.quick); \
    modify_field(common_global_scratch.process_ack_flag, \
                            common_phv.process_ack_flag); \
    modify_field(common_global_scratch.flags, common_phv.flags); \
    modify_field(common_global_scratch.is_dupack, common_phv.is_dupack); \
    modify_field(common_global_scratch.ooo_rcv, common_phv.ooo_rcv); \
    modify_field(common_global_scratch.write_serq, common_phv.write_serq); \
    modify_field(common_global_scratch.pending_del_ack_send, common_phv.pending_del_ack_send); \
    modify_field(common_global_scratch.pending_txdma, common_phv.pending_txdma); \
    modify_field(common_global_scratch.fatal_error, common_phv.fatal_error); \
    modify_field(common_global_scratch.write_arq, common_phv.write_arq); \
    modify_field(common_global_scratch.write_tcp_app_hdr, common_phv.write_tcp_app_hdr); \
    modify_field(common_global_scratch.tsopt_enabled, common_phv.tsopt_enabled); \
    modify_field(common_global_scratch.tsopt_available, common_phv.tsopt_available); \
    modify_field(common_global_scratch.skip_pkt_dma, common_phv.skip_pkt_dma); \
    modify_field(common_global_scratch.ooo_alloc_fail, common_phv.ooo_alloc_fail); \
    modify_field(common_global_scratch.ooq_tx2rx_pkt, common_phv.ooq_tx2rx_pkt); \

#define GENERATE_S1_S2S_K \
    modify_field(s1_s2s_scratch.payload_len, s1_s2s.payload_len); \
    modify_field(s1_s2s_scratch.seq, s1_s2s.seq); \
    modify_field(s1_s2s_scratch.ack_seq, s1_s2s.ack_seq); \
    modify_field(s1_s2s_scratch.snd_nxt, s1_s2s.snd_nxt); \
    modify_field(s1_s2s_scratch.rcv_mss_shft, s1_s2s.rcv_mss_shft); \
    modify_field(s1_s2s_scratch.rcv_tsval, s1_s2s.rcv_tsval); \
    modify_field(s1_s2s_scratch.quick_acks_decr, s1_s2s.quick_acks_decr); \
    modify_field(s1_s2s_scratch.fin_sent, s1_s2s.fin_sent); \
    modify_field(s1_s2s_scratch.rst_sent, s1_s2s.rst_sent); \
    modify_field(s1_s2s_scratch.cc_rto_signal, s1_s2s.cc_rto_signal);

#define GENERATE_T2_S2S_K \
    modify_field(t2_s2s_scratch.payload_len, t2_s2s.payload_len); \
    modify_field(t2_s2s_scratch.seq, t2_s2s.seq); \
    modify_field(t2_s2s_scratch.ooo_qbase_addr, t2_s2s.ooo_qbase_addr); \
    modify_field(t2_s2s_scratch.ooo_queue_id, t2_s2s.ooo_queue_id); \
    modify_field(t2_s2s_scratch.ooo_tail_index, t2_s2s.ooo_tail_index);

#define GENERATE_S3_T2_S2S_K \
    modify_field(s3_t2_s2s_scratch.ooo_rx2tx_ready_trim0, s3_t2_s2s.ooo_rx2tx_ready_trim0); \
    modify_field(s3_t2_s2s_scratch.ooo_rx2tx_ready_len0, s3_t2_s2s.ooo_rx2tx_ready_len0); \
    modify_field(s3_t2_s2s_scratch.ooo_rx2tx_ready_trim1, s3_t2_s2s.ooo_rx2tx_ready_trim1); \
    modify_field(s3_t2_s2s_scratch.ooo_rx2tx_ready_len1, s3_t2_s2s.ooo_rx2tx_ready_len1); \
    modify_field(s3_t2_s2s_scratch.ooo_rx2tx_ready_trim2, s3_t2_s2s.ooo_rx2tx_ready_trim2); \
    modify_field(s3_t2_s2s_scratch.ooo_rx2tx_ready_len2, s3_t2_s2s.ooo_rx2tx_ready_len2); \
    modify_field(s3_t2_s2s_scratch.ooo_rx2tx_ready_trim3, s3_t2_s2s.ooo_rx2tx_ready_trim3); \
    modify_field(s3_t2_s2s_scratch.ooo_rx2tx_ready_len3, s3_t2_s2s.ooo_rx2tx_ready_len3); \

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
        pid                     : 16;
        rx_ts                   : 64; // 16 bytes

        serq_ring_size          : 16;
        l7_proxy_type           : 8;
        debug_dol               : 16;
        quick_acks_decr_old     : 4;
        pad2                    : 20; // 8 bytes

        serq_cidx               : 16;
        pad1                    : 48; // 8 bytes

        // written by TCP Tx P4+ program
        TX2RX_SHARED_STATE           // offset = 32
    }
}


// d for stage 1
header_type tcp_rx_d_t {
    fields {
        ooq_not_empty           : 8;    // offset 0 (TCP_TCB_RX_OOQ_NOT_EMPTY)
        consumer_ring_shift     : 8;
        bytes_acked             : 16;   // tcp_ack stage
        slow_path_cnt           : 16;
        serq_full_cnt           : 16;
        ooo_cnt                 : 16;
        ato                     : 16;
        del_ack_pi              : 16;
        cfg_flags               : 8;
        rcv_nxt                 : 32;
        rx_drop_cnt             : 16;
        ts_recent               : 32;
        lrcv_time               : 32;
        snd_una                 : 32;   // tcp_ack stage
        snd_wl1                 : 32;   // tcp_ack stage
        pred_flags              : 32;   // tcp_ack stage
        snd_recover             : 32;   // tcp_ack stage
        bytes_rcvd              : 16;
        snd_wnd                 : 16;   // tcp_ack stage
        serq_pidx               : 16;
        rcv_mss                 : 16;
        num_dup_acks            : 16;   // tcp_ack_stage
        cc_flags                : 8;    // tcp_ack stage
        quick                   : 8;
        flag                    : 8;    // used with .l not written back
        rto                     : 8;
        state                   : 8;
        parsed_state            : 8;
        rcv_wscale              : 8;
        limited_transmit        : 2;    // tcp_ack stage
        pending                 : 3;
        write_serq              : 1;
        alloc_descr             : 1;    // used with .l not written back
    }
}

// d for stage 2 table 1
header_type read_rnmdr_d_t {
    fields {
        rnmdr_pidx              : 32;
        rnmdr_pidx_full         : 8;
    }
}

// d for stage 3 table 0
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
        rto                     : 16;
        ts_ganularity_us        : 16;
        ts_shift                : 8;
        backoff                 : 4;
    }
}

#define RTT_D_PARAMS                                            \
    srtt_us, seq_rtt_us, ca_rtt_us, curr_ts, rtt_min, rttvar_us,\
    mdev_us, mdev_max_us, rtt_seq, rto, ts_ganularity_us,       \
    ts_shift, backoff

#define GENERATE_RTT_D                                          \
    modify_field(tcp_rtt_d.srtt_us, srtt_us);                   \
    modify_field(tcp_rtt_d.seq_rtt_us, seq_rtt_us);             \
    modify_field(tcp_rtt_d.ca_rtt_us, ca_rtt_us);               \
    modify_field(tcp_rtt_d.curr_ts, curr_ts);                   \
    modify_field(tcp_rtt_d.rtt_min, rtt_min);                   \
    modify_field(tcp_rtt_d.rttvar_us, rttvar_us);               \
    modify_field(tcp_rtt_d.mdev_us, mdev_us);                   \
    modify_field(tcp_rtt_d.mdev_max_us, mdev_max_us);           \
    modify_field(tcp_rtt_d.rtt_seq, rtt_seq);                   \
    modify_field(tcp_rtt_d.rto, rto);                           \
    modify_field(tcp_rtt_d.ts_ganularity_us, ts_ganularity_us); \
    modify_field(tcp_rtt_d.ts_shift, ts_shift);                 \
    modify_field(tcp_rtt_d.backoff, backoff);                   \


// d for stage 3 table 1
header_type rdesc_alloc_d_t {
    fields {
        desc                    : 64;
        pad                     : 448;
    }
}

// for stage 3 table 2
header_type read_ooo_qbase_index_t {
    fields {
        ooo_qbase_pindex      : 32;
        ooo_qbase_pindex_full : 8;
    }
}

// d for stage 3 table 0 - ooo book keeping
header_type ooo_book_keeping_t {
    fields {
        start_seq0      : 32;
        end_seq0        : 32;
        tail_index0     : 16;
        start_seq1      : 32;
        end_seq1        : 32;
        tail_index1     : 16;
        start_seq2      : 32;
        end_seq2        : 32;
        tail_index2     : 16;
        start_seq3      : 32;
        end_seq3        : 32;
        tail_index3     : 16;
        ooo_queue_full  : 32;
    }
}

// offset 0 (TCP_TCB_CC_SND_CWND_OFFSET)

#define CC_D_PARAMS \
        snd_cwnd, cc_algo, smss, smss_squared,  smss_times_abc_l, snd_ssthresh, \
        max_win, abc_bytes_acked, abc_l_var, snd_wscale, cc_flags, t_flags, \
        ip_tos_ecn_received

#define GENERATE_CC_D \
    modify_field(tcp_cc_d.snd_cwnd, snd_cwnd); \
    modify_field(tcp_cc_d.cc_algo, cc_algo); \
    modify_field(tcp_cc_d.smss, smss); \
    modify_field(tcp_cc_d.smss_squared, smss_squared); \
    modify_field(tcp_cc_d.smss_times_abc_l, smss_times_abc_l); \
    modify_field(tcp_cc_d.snd_ssthresh, snd_ssthresh); \
    modify_field(tcp_cc_d.max_win, max_win); \
    modify_field(tcp_cc_d.abc_bytes_acked, abc_bytes_acked); \
    modify_field(tcp_cc_d.abc_l_var, abc_l_var); \
    modify_field(tcp_cc_d.snd_wscale, snd_wscale); \
    modify_field(tcp_cc_d.cc_flags, cc_flags); \
    modify_field(tcp_cc_d.t_flags, t_flags); \
    modify_field(tcp_cc_d.ip_tos_ecn_received, ip_tos_ecn_received);

// d for stage 4 table 0
header_type tcp_cc_d_t {
    fields {
        snd_cwnd                : 32;
        cc_algo                 : 8;
        smss                    : 16;
        smss_squared            : 32;
        smss_times_abc_l        : 32;
        snd_ssthresh            : 32;
        max_win                 : 32;
        abc_bytes_acked         : 32;
        abc_l_var               : 8;
        snd_wscale              : 8;
        cc_flags                : 8;
        t_flags                 : 8;
        ip_tos_ecn_received     : 1;
    }
}

// d for stage 4 table 2
header_type ooq_alloc_d_t {
    fields {
        qbase                   : 64;
    }
}

// d for stage5 table 0
header_type read_ooo_base_addr_t {
    fields {
        ooo_qbase_addr : 64;
    }
}

// d for stage 5 table 0
header_type tcp_fc_d_t {
    fields {
        page                    : 32;
        descr                   : 32;
        rcv_wup                 : 32;
        page_cnt                : 16;
        rcv_wnd                 : 32;
        rcv_scale               : 8;
        cpu_id                  : 8;
    }
}

// d for stage 5 table 2 - ooo qbase addr
header_type ooo_qbase_addr_t {
    fields {
        ooo_qbase_addr0    : 64;
        ooo_qbase_addr1    : 64;
        ooo_qbase_addr2    : 64;
        ooo_qbase_addr3    : 64;

        ooo_rx2tx_qbase    : 64;
        ooo_rx2tx_qbase_pi : 16;
    }
}

// d for stage 6 table 0
header_type tcp_rx_dma_d_t {
    fields {
        serq_base               : 64;
        nde_shift               : 8;
        nde_offset              : 8;
        nde_len                 : 8;
        consumer_lif            : 16;
        consumer_ring           : 8;
        consumer_num_slots_mask : 16;
        rx2tx_send_ack_pi       : 16;
        rx2tx_clean_retx_pi     : 16;
        rx2tx_fast_retx_pi      : 16;
        consumer_qid            : 16;

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
        data_ofs_rsvd           : 8;
        rcv_wup                 : 32;
        seq                     : 32;
        rcv_wnd_adv             : 16;
        serq_cidx               : 12;
        ip_dsfield              : 8;
    }
}

header_type to_stage_2_phv_t {
    // tcp-ack
    fields {
        flag                    : 8;
        window                  : 16;
    }
}

header_type to_stage_3_phv_t {
    // tcp-rtt, read-rnmdr, read-rnmpr
    fields {
        snd_nxt                 : 32;
        rcv_tsval               : 32;
        rcv_tsecr               : 32;
    }
}

header_type to_stage_4_phv_t {
    // tcp-cc
    fields {
        bytes_acked             : 32;
        snd_wnd                 : 16;
        cc_ack_signal           : 8;
        cc_flags                : 8;
    }
}

header_type to_stage_5_phv_t {
    // tcp-fc
    fields {
        serq_cidx               : 12;
        serq_pidx               : 12;
        rcv_nxt                 : 32;
    }
}

header_type to_stage_6_phv_t {
    // write-serq
    fields {
        page                    : 34;
        descr                   : 34;
        payload_len             : 16;
        serq_pidx               : 12;
    }
}

header_type to_stage_7_phv_t {
    // stats
    fields {
        bytes_rcvd              : 16;
        pkts_rcvd               : 8;
        bytes_acked             : 16;
    }
}

header_type common_global_phv_t {
    fields {
        // global k (max 128)
        fid                     : 24;
        qstate_addr             : 34;
        snd_una                 : 32;
        debug_dol               : 8;
        flags                   : 8;
        quick                   : 3;
        process_ack_flag        : 1;
        is_dupack               : 1;
        ooo_rcv                 : 1;
        write_serq              : 1;
        pending_del_ack_send    : 1;
        pending_txdma           : 4;
        fatal_error             : 1;
        write_arq               : 1;
        write_tcp_app_hdr       : 1;
        tsopt_enabled           : 1;
        tsopt_available         : 1;
        skip_pkt_dma            : 1;
        ip_tos_ecn              : 2;
        ooo_alloc_fail          : 1;
        ooq_tx2rx_pkt           : 1;
    }
}

/******************************************************************************
 * Stage to stage PHV definitions
 *****************************************************************************/
header_type s1_s2s_phv_t {
    fields {
        payload_len             : 16;
        seq                     : 32;
        ack_seq                 : 32;
        snd_nxt                 : 32;
        rcv_tsval               : 32;
        rcv_mss_shft            : 4;
        quick_acks_decr         : 4;
        fin_sent                : 1;
        rst_sent                : 1;
        cc_rto_signal           : 1;
    }
}

header_type s3_t1_s2s_phv_t {
    fields {
        rnmdr_pidx              : 16;
    }
}

header_type s6_s2s_phv_t {
    fields {
        payload_len             : 16;
    }
}

header_type s6_t1_s2s_phv_t {
    fields {
        rnmdr_pidx              : 16;
        cpu_id                  : 8;
    }
}

header_type t2_s2s_phv_t {
    fields {
        payload_len             : 16;
        seq                     : 32;
        ooo_qbase_addr          : 64;
        ooo_tail_index          : 16;
        ooo_queue_id            : 4;
        ooo_rx2tx_ready_qid     : 4;
    }
}

header_type s3_t2_s2s_phv_t {
    fields {
        ooo_rx2tx_ready_trim0   : 16;
        ooo_rx2tx_ready_len0    : 16;
        ooo_rx2tx_ready_trim1   : 16;
        ooo_rx2tx_ready_len1    : 16;
        ooo_rx2tx_ready_trim2   : 16;
        ooo_rx2tx_ready_len2    : 16;
        ooo_rx2tx_ready_trim3   : 16;
        ooo_rx2tx_ready_len3    : 16;
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
metadata tcp_cc_d_t tcp_cc_d;
@pragma scratch_metadata
metadata tcp_fc_d_t tcp_fc_d;
@pragma scratch_metadata
metadata tcp_rx_dma_d_t tcp_rx_dma_d;
@pragma scratch_metadata
metadata rdesc_alloc_d_t rdesc_alloc_d;
@pragma scratch_metadata
metadata arq_pi_d_t arq_rx_pi_d;

@pragma scratch_metadata
metadata ooo_book_keeping_t ooo_book_keeping;
@pragma scratch_metadata
metadata read_ooo_qbase_index_t read_ooo_qbase_index;
@pragma scratch_metadata
metadata ooq_alloc_d_t ooq_alloc_d;
@pragma scratch_metadata
metadata ooo_qbase_addr_t ooo_qbase_addr;
@pragma scratch_metadata
metadata read_ooo_base_addr_t read_ooo_base_addr;

/******************************************************************************
 * Header unions for PHV layout
 *****************************************************************************/
@pragma pa_header_union ingress app_header
metadata p4_to_p4plus_tcp_proxy_base_header_t tcp_app_header;
@pragma scratch_metadata
metadata p4_to_p4plus_tcp_proxy_base_header_t tcp_scratch_app;

@pragma pa_header_union ingress to_stage_1 cpu_hdr1
metadata to_stage_1_phv_t to_s1;
metadata p4_to_p4plus_cpu_pkt_1_t cpu_hdr1;

@pragma pa_header_union ingress to_stage_2 cpu_hdr2
metadata to_stage_2_phv_t to_s2;
metadata p4_to_p4plus_cpu_pkt_2_t cpu_hdr2;

@pragma pa_header_union ingress to_stage_3 cpu_hdr3
metadata to_stage_3_phv_t to_s3;
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
metadata p4_to_p4plus_cpu_pkt_1_t to_cpu1_scratch;
@pragma scratch_metadata
metadata to_stage_1_phv_t to_s1_scratch;
@pragma scratch_metadata
metadata p4_to_p4plus_cpu_pkt_2_t to_cpu2_scratch;
@pragma scratch_metadata
metadata to_stage_2_phv_t to_s2_scratch;
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
metadata s6_s2s_phv_t s6_s2s_scratch;
@pragma scratch_metadata
metadata s6_t1_s2s_phv_t s6_t1_s2s_scratch;
@pragma scratch_metadata
metadata t2_s2s_phv_t t2_s2s_scratch;
@pragma scratch_metadata
metadata s3_t2_s2s_phv_t s3_t2_s2s_scratch;
@pragma scratch_metadata
metadata common_global_phv_t common_global_scratch;

@pragma pa_header_union ingress common_t0_s2s s6_s2s
metadata s1_s2s_phv_t s1_s2s;
//metadata s4_s2s_phv_t s4_s2s;
metadata s6_s2s_phv_t s6_s2s;

@pragma pa_header_union ingress common_t1_s2s s6_t1_s2s
metadata s3_t1_s2s_phv_t s3_t1_s2s;
metadata s6_t1_s2s_phv_t s6_t1_s2s;

@pragma pa_header_union ingress common_t2_s2s t2_s2s s3_t2_s2s
metadata t2_s2s_phv_t t2_s2s;
metadata s3_t2_s2s_phv_t s3_t2_s2s;

/******************************************************************************
 * PHV following k (for app DMA etc.)
 *****************************************************************************/
@pragma dont_trim
metadata rx2tx_extra_t rx2tx_extra;
@pragma dont_trim
metadata doorbell_data_t db_data;
@pragma dont_trim
metadata doorbell_data_t db_data2;
@pragma dont_trim
metadata doorbell_data_t db_data3;
@pragma dont_trim
metadata doorbell_data_t rx2tx_ooq_ready_db_data;

header_type ooq_rx2tx_queue_entry_opaque_t {
    fields {
        entry : 64;
    }
}

/*
 * ring_entry and aol have to be contiguous in phv
 */
@pragma dont_trim
metadata hbm_al_ring_entry_t ring_entry;
@pragma dont_trim
metadata pkt_descr_aol_t aol;

/*
 * Upto 4 OOO queues can become in-order and DMAed to rx2tx program
 */
@pragma dont_trim
metadata ooq_rx2tx_queue_entry_opaque_t ooq_rx2tx_queue_entry1;
@pragma dont_trim
metadata ooq_rx2tx_queue_entry_opaque_t ooq_rx2tx_queue_entry2;
@pragma dont_trim
metadata ooq_rx2tx_queue_entry_opaque_t ooq_rx2tx_queue_entry3;
@pragma dont_trim
metadata ooq_rx2tx_queue_entry_opaque_t ooq_rx2tx_queue_entry4;
@pragma dont_trim
metadata ooq_rx2tx_queue_entry_opaque_t dummy_to_get_around_ncc_bug;



@pragma pa_align 128
@pragma dont_trim
metadata dma_cmd_pkt2mem_t pkt_dma;                 // dma cmd 1
@pragma dont_trim
@pragma pa_header_union ingress pkt_dma
metadata dma_cmd_skip_t pkt_dma_skip;

@pragma dont_trim
@pragma pa_header_union ingress pkt_dma
metadata dma_cmd_pkt2mem_t ooo_pkt_dma;

@pragma dont_trim
metadata dma_cmd_phv2mem_t pkt_descr_dma;           // dma cmd 2

@pragma dont_trim
metadata dma_cmd_phv2mem_t tcp_flags_dma;           // dma cmd 3
@pragma dont_trim
@pragma pa_header_union ingress tcp_flags_dma
metadata dma_cmd_phv2mem_t cpu_hdr_dma;

@pragma dont_trim
metadata dma_cmd_phv2mem_t ring_slot;               // dma cmd 4

@pragma dont_trim
metadata dma_cmd_phv2mem_t rx2tx_extra_dma;         // dma cmd 5

@pragma dont_trim
metadata dma_cmd_phv2mem_t tx_doorbell1;            // dma cmd 6

@pragma dont_trim
metadata dma_cmd_phv2mem_t tx_doorbell;             // dma cmd 7

@pragma dont_trim
metadata dma_cmd_phv2mem_t rx2tx_ooq_ready;         // dma cmd 8

@pragma dont_trim
metadata dma_cmd_phv2mem_t rx2tx_ooq_doorbell;      // dma cmd 9

@pragma dont_trim
metadata dma_cmd_phv2mem_t tls_doorbell;            // dma cmd 10

/******************************************************************************
 * Action functions to generate k_struct and d_struct
 *
 * These action functions are currently only to generate the k+i and d structs
 * and do not implement any pseudo code
 *****************************************************************************/

/*
 * Stage 0 table 0 action
 */
action read_tx2rx(rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid, rx_ts,
                  serq_ring_size, l7_proxy_type, debug_dol, quick_acks_decr_old,
                  pad2, serq_cidx, pad1, snd_nxt, rcv_wup, rcv_wnd_adv,
                  quick_acks_decr, fin_sent, rst_sent, rto_event,
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
    modify_field(tcp_scratch_app.prev_echo_ts, tcp_app_header.prev_echo_ts);

    // d for stage 0
    modify_field(read_tx2rxd.rsvd, rsvd);
    modify_field(read_tx2rxd.cosA, cosA);
    modify_field(read_tx2rxd.cosB, cosB);
    modify_field(read_tx2rxd.cos_sel, cos_sel);
    modify_field(read_tx2rxd.eval_last, eval_last);
    modify_field(read_tx2rxd.host, host);
    modify_field(read_tx2rxd.total, total);
    modify_field(read_tx2rxd.pid, pid);
    modify_field(read_tx2rxd.rx_ts, rx_ts);
    modify_field(read_tx2rxd.serq_ring_size, serq_ring_size);
    modify_field(read_tx2rxd.l7_proxy_type, l7_proxy_type);
    modify_field(read_tx2rxd.debug_dol, debug_dol);
    modify_field(read_tx2rxd.quick_acks_decr_old, quick_acks_decr_old);
    modify_field(read_tx2rxd.pad2, pad2);
    modify_field(read_tx2rxd.serq_cidx, serq_cidx);
    modify_field(read_tx2rxd.pad1, pad1);
    modify_field(read_tx2rxd.snd_nxt, snd_nxt);
    modify_field(read_tx2rxd.rcv_wup, rcv_wup);
    modify_field(read_tx2rxd.rcv_wnd_adv, rcv_wnd_adv);
    modify_field(read_tx2rxd.quick_acks_decr, quick_acks_decr);
    modify_field(read_tx2rxd.fin_sent, fin_sent);
    modify_field(read_tx2rxd.rst_sent, rst_sent);
    modify_field(read_tx2rxd.rto_event, rto_event);
    modify_field(read_tx2rxd.pad1_tx2rx, pad1_tx2rx);
}

#define TCP_RX_CB_PARAMS \
        ooq_not_empty, consumer_ring_shift, \
        bytes_acked, slow_path_cnt, serq_full_cnt, ooo_cnt, \
        ato, del_ack_pi, cfg_flags, \
        rcv_nxt, rx_drop_cnt, ts_recent, lrcv_time, \
        snd_una, snd_wl1, pred_flags, snd_recover, bytes_rcvd, \
        snd_wnd, serq_pidx, rcv_mss, num_dup_acks, cc_flags, quick, \
        flag, rto, state, parsed_state, rcv_wscale, limited_transmit, pending, \
        write_serq, alloc_descr

#define TCP_RX_CB_D \
    modify_field(tcp_rx_d.ooq_not_empty, ooq_not_empty); \
    modify_field(tcp_rx_d.consumer_ring_shift, consumer_ring_shift); \
    modify_field(tcp_rx_d.bytes_acked, bytes_acked); \
    modify_field(tcp_rx_d.slow_path_cnt, slow_path_cnt); \
    modify_field(tcp_rx_d.serq_full_cnt, serq_full_cnt); \
    modify_field(tcp_rx_d.ooo_cnt, ooo_cnt); \
    modify_field(tcp_rx_d.ato, ato); \
    modify_field(tcp_rx_d.del_ack_pi, del_ack_pi); \
    modify_field(tcp_rx_d.cfg_flags, cfg_flags); \
    modify_field(tcp_rx_d.rcv_nxt, rcv_nxt); \
    modify_field(tcp_rx_d.rx_drop_cnt, rx_drop_cnt); \
    modify_field(tcp_rx_d.ts_recent, ts_recent); \
    modify_field(tcp_rx_d.lrcv_time, lrcv_time); \
    modify_field(tcp_rx_d.snd_una, snd_una); \
    modify_field(tcp_rx_d.snd_wl1, snd_wl1); \
    modify_field(tcp_rx_d.pred_flags, pred_flags); \
    modify_field(tcp_rx_d.snd_recover, snd_recover); \
    modify_field(tcp_rx_d.bytes_rcvd, bytes_rcvd); \
    modify_field(tcp_rx_d.snd_wnd, snd_wnd); \
    modify_field(tcp_rx_d.serq_pidx, serq_pidx); \
    modify_field(tcp_rx_d.rcv_mss, rcv_mss); \
    modify_field(tcp_rx_d.num_dup_acks, num_dup_acks); \
    modify_field(tcp_rx_d.cc_flags, cc_flags); \
    modify_field(tcp_rx_d.quick, quick); \
    modify_field(tcp_rx_d.flag, flag); \
    modify_field(tcp_rx_d.rto, rto); \
    modify_field(tcp_rx_d.state, state); \
    modify_field(tcp_rx_d.parsed_state, parsed_state); \
    modify_field(tcp_rx_d.rcv_wscale, rcv_wscale); \
    modify_field(tcp_rx_d.limited_transmit, limited_transmit); \
    modify_field(tcp_rx_d.pending, pending); \
    modify_field(tcp_rx_d.write_serq, write_serq); \
    modify_field(tcp_rx_d.alloc_descr, alloc_descr);

/*
 * Stage 1 table 0 action
 */
action tcp_rx(TCP_RX_CB_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // k + i for stage 1
    if (write_serq == 0) {
        modify_field(to_s1_scratch.data_ofs_rsvd, to_s1.data_ofs_rsvd);
        modify_field(to_s1_scratch.rcv_wup, to_s1.rcv_wup);
        modify_field(to_s1_scratch.seq, to_s1.seq);
        modify_field(to_s1_scratch.rcv_wnd_adv, to_s1.rcv_wnd_adv);
        modify_field(to_s1_scratch.serq_cidx, to_s1.serq_cidx);
        modify_field(to_s1_scratch.ip_dsfield, to_s1.ip_dsfield);
    }

    if (write_serq == 1) {
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

    GENERATE_S1_S2S_K

    // d for stage 1 tcp-rx
    TCP_RX_CB_D
}

/*
 * Stage 2 table 0 action
 */
action tcp_ack(TCP_RX_CB_PARAMS) {
    // k + i for stage 2

    // from ki global
    GENERATE_GLOBAL_K

    if (write_serq == 0) {
        modify_field(to_cpu2_scratch.l3_offset_2, cpu_hdr2.l3_offset_2);
        modify_field(to_cpu2_scratch.l4_offset, cpu_hdr2.l4_offset);
        modify_field(to_cpu2_scratch.payload_offset, cpu_hdr2.payload_offset);
        modify_field(to_cpu2_scratch.tcp_flags, cpu_hdr2.tcp_flags);
        modify_field(to_cpu2_scratch.tcp_seqNo, cpu_hdr2.tcp_seqNo);
        modify_field(to_cpu2_scratch.tcp_AckNo_1, cpu_hdr2.tcp_AckNo_1);
    }
    // from to_stage 2
    if (write_serq == 1) {
        modify_field(to_s2_scratch.flag, to_s2.flag);
        modify_field(to_s2_scratch.window, to_s2.window);
    }

    // from stage to stage
    GENERATE_S1_S2S_K

    // d for stage 2 (reuse tcp-rx cb)
    TCP_RX_CB_D
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
action ooo_book_keeping (start_seq0, end_seq0, tail_index0,
                             start_seq1, end_seq1, tail_index1,
                             start_seq2, end_seq2, tail_index2,
                             start_seq3, end_seq3, tail_index3,
                             ooo_queue_full)
{
    GENERATE_GLOBAL_K

    GENERATE_T2_S2S_K

    modify_field(ooo_book_keeping.start_seq0, start_seq0);
    modify_field(ooo_book_keeping.end_seq0, end_seq0);
    modify_field(ooo_book_keeping.tail_index0, tail_index0);
    modify_field(ooo_book_keeping.start_seq1, start_seq1);
    modify_field(ooo_book_keeping.end_seq1, end_seq1);
    modify_field(ooo_book_keeping.tail_index1, tail_index1);
    modify_field(ooo_book_keeping.start_seq2, start_seq2);
    modify_field(ooo_book_keeping.end_seq2, end_seq2);
    modify_field(ooo_book_keeping.tail_index2, tail_index2);
    modify_field(ooo_book_keeping.start_seq3, start_seq3);
    modify_field(ooo_book_keeping.end_seq3, end_seq3);
    modify_field(ooo_book_keeping.tail_index3, tail_index3);
    modify_field(ooo_book_keeping.ooo_queue_full, ooo_queue_full);
}


/*
 * Stage 3 table 0 action
 */
action tcp_rtt(RTT_D_PARAMS) {
    // k + i for stage 3

    // from ki global
    GENERATE_GLOBAL_K

    // from to_stage 3
    if (backoff == 0) {
        modify_field(to_s3_scratch.snd_nxt, to_s3.snd_nxt);
        modify_field(to_s3_scratch.rcv_tsval, to_s3.rcv_tsval);
        modify_field(to_s3_scratch.rcv_tsecr, to_s3.rcv_tsecr);
    }
    if (backoff == 1) {
        modify_field(to_cpu3_scratch.tcp_AckNo_2, cpu_hdr3.tcp_AckNo_2);
        modify_field(to_cpu3_scratch.tcp_window, cpu_hdr3.tcp_window);
        modify_field(to_cpu3_scratch.tcp_options, cpu_hdr3.tcp_options);
        modify_field(to_cpu3_scratch.tcp_mss, cpu_hdr3.tcp_mss);
        modify_field(to_cpu3_scratch.tcp_ws, cpu_hdr3.tcp_ws);
    }

    // from stage to stage

    // d for rtt stage
    GENERATE_RTT_D
}

/*
 * Stage 3 table 1 action
 */
action rdesc_alloc(desc, pad) {
    // k + i for stage 3 table 1

    // from to_stage 3

    // from ki global
    GENERATE_GLOBAL_K

    // from stage to stage
    modify_field(s3_t1_s2s_scratch.rnmdr_pidx, s3_t1_s2s.rnmdr_pidx);

    // d for stage 3 table 1
    modify_field(rdesc_alloc_d.desc, desc);
    modify_field(rdesc_alloc_d.pad, pad);
}

/*
 * Stage 3 table 2 action
 */
action ooo_qbase_sem_idx(ooo_qbase_pindex, ooo_qbase_pindex_full)
{
    GENERATE_GLOBAL_K

    GENERATE_T2_S2S_K

    modify_field(read_ooo_qbase_index.ooo_qbase_pindex, ooo_qbase_pindex);
    modify_field(read_ooo_qbase_index.ooo_qbase_pindex_full, ooo_qbase_pindex_full);
}


/*
 * Stage 4 table 0 action
 */
action tcp_cc(CC_D_PARAMS) {
    // k + i for stage 4

    // from to_stage
    modify_field(to_s4_scratch.bytes_acked, to_s4.bytes_acked);
    modify_field(to_s4_scratch.snd_wnd, to_s4.snd_wnd);
    modify_field(to_s4_scratch.cc_ack_signal, to_s4.cc_ack_signal);
    modify_field(to_s4_scratch.cc_flags, to_s4.cc_flags);

    // from ki global
    GENERATE_GLOBAL_K

    // from stage to stage

    // d for stage 4 tcp-cc
    GENERATE_CC_D
}

/*
 * Stage 4 table 2 action
 */
action ooo_qbase_alloc(qbase)
{
    GENERATE_GLOBAL_K

    GENERATE_T2_S2S_K

    modify_field(ooq_alloc_d.qbase, qbase);
}

/*
 * Stage 5 table 0 action1
 */
action tcp_fc(page, descr, page_cnt, rcv_wnd, rcv_wup, rcv_scale, cpu_id) {
    // k + i for stage 5

    // from to_stage 5
    modify_field(to_s5_scratch.serq_cidx, to_s5.serq_cidx);
    modify_field(to_s5_scratch.serq_pidx, to_s5.serq_pidx);
    modify_field(to_s5_scratch.rcv_nxt, to_s5.rcv_nxt);

    // from ki global
    GENERATE_GLOBAL_K

    // from stage to stage

    // d for stage 5 table 0
    modify_field(tcp_fc_d.page, page);
    modify_field(tcp_fc_d.descr, descr);
    modify_field(tcp_fc_d.page_cnt, page_cnt);
    modify_field(tcp_fc_d.rcv_wnd, rcv_wnd);
    modify_field(tcp_fc_d.rcv_wup, rcv_wup);
    modify_field(tcp_fc_d.rcv_scale, rcv_scale);
    modify_field(tcp_fc_d.cpu_id, cpu_id);
}

/*
 * Stage 5 table 2 action
 */
action ooo_qbase_cb_load(ooo_qbase_addr0, ooo_qbase_addr1,
                         ooo_qbase_addr2, ooo_qbase_addr3,
                         ooo_rx2tx_qbase, ooo_rx2tx_qbase_pi)
{
    GENERATE_GLOBAL_K

    if (ooo_rx2tx_qbase_pi == 0) {
        GENERATE_T2_S2S_K
    }
    if (ooo_rx2tx_qbase_pi == 1) {
        GENERATE_S3_T2_S2S_K
    }

    modify_field(ooo_qbase_addr.ooo_qbase_addr0, ooo_qbase_addr0);
    modify_field(ooo_qbase_addr.ooo_qbase_addr1, ooo_qbase_addr1);
    modify_field(ooo_qbase_addr.ooo_qbase_addr2, ooo_qbase_addr2);
    modify_field(ooo_qbase_addr.ooo_qbase_addr3, ooo_qbase_addr3);

    modify_field(ooo_qbase_addr.ooo_rx2tx_qbase, ooo_rx2tx_qbase);
    modify_field(ooo_qbase_addr.ooo_rx2tx_qbase_pi, ooo_rx2tx_qbase_pi);

}


/*
 * Stage 6 table 0 action
 */
action dma(serq_base, nde_shift, nde_offset, nde_len,
        consumer_lif, consumer_ring, consumer_num_slots_mask,
        pkts_rcvd, rx2tx_send_ack_pi,
        rx2tx_clean_retx_pi, rx2tx_fast_retx_pi, consumer_qid) {
    // k + i for stage 6

    // from to_stage 6
    modify_field(to_s6_scratch.page, to_s6.page);
    modify_field(to_s6_scratch.descr, to_s6.descr);
    modify_field(to_s6_scratch.payload_len, to_s6.payload_len);
    modify_field(to_s6_scratch.serq_pidx, to_s6.serq_pidx);

    // from ki global
    GENERATE_GLOBAL_K

    // from stage to stage
    modify_field(s6_s2s_scratch.payload_len, s6_s2s.payload_len);

    // d for stage 5 table 0
    modify_field(tcp_rx_dma_d.serq_base, serq_base);
    modify_field(tcp_rx_dma_d.nde_shift, nde_shift);
    modify_field(tcp_rx_dma_d.nde_offset, nde_offset);
    modify_field(tcp_rx_dma_d.nde_len, nde_len);
    modify_field(tcp_rx_dma_d.consumer_lif, consumer_lif);
    modify_field(tcp_rx_dma_d.consumer_ring, consumer_ring);
    modify_field(tcp_rx_dma_d.consumer_num_slots_mask, consumer_num_slots_mask);
    modify_field(tcp_rx_dma_d.pkts_rcvd, pkts_rcvd);
    modify_field(tcp_rx_dma_d.rx2tx_send_ack_pi, rx2tx_send_ack_pi);
    modify_field(tcp_rx_dma_d.rx2tx_clean_retx_pi, rx2tx_clean_retx_pi);
    modify_field(tcp_rx_dma_d.rx2tx_fast_retx_pi, rx2tx_fast_retx_pi);
    modify_field(tcp_rx_dma_d.consumer_qid, consumer_qid);
}

/*
 * Stage 6 table 1 action
 */
action write_arq(ARQ_PI_PARAMS) {

    // k + i for stage 6

    // from to_stage 6
    modify_field(to_s6_scratch.page, to_s6.page);
    modify_field(to_s6_scratch.descr, to_s6.descr);
    modify_field(to_s6_scratch.payload_len, to_s6.payload_len);


    // from stage to stage
    modify_field(s6_t1_s2s_scratch.rnmdr_pidx, s6_t1_s2s.rnmdr_pidx);
    modify_field(s6_t1_s2s_scratch.cpu_id, s6_t1_s2s.cpu_id);

    // from ki global
    GENERATE_GLOBAL_K

    // from stage to stage

    // d for stage 6 table 1
    GENERATE_ARQ_PI_D(arq_rx_pi_d)
}

/*
 * Stage 6 table 2 action
 */
action write_ooq() {
    // k + i for stage 6

    // from to_stage 6
    modify_field(to_s6_scratch.page, to_s6.page);
    modify_field(to_s6_scratch.descr, to_s6.descr);
    modify_field(to_s6_scratch.payload_len, to_s6.payload_len);
    modify_field(to_s6_scratch.serq_pidx, to_s6.serq_pidx);

    // from ki global
    GENERATE_GLOBAL_K

    GENERATE_T2_S2S_K
}

/*
 * Stage 7 table 0 action
 */
action stats() {
    // k + i for stage 7

    // from to_stage 7
    modify_field(to_s7_scratch.bytes_rcvd, to_s7.bytes_rcvd);
    modify_field(to_s7_scratch.pkts_rcvd, to_s7.pkts_rcvd);
    modify_field(to_s7_scratch.bytes_acked, to_s7.bytes_acked);

    // from ki global
    GENERATE_GLOBAL_K

    // from stage to stage

    // d for stage 7 table 0
}
