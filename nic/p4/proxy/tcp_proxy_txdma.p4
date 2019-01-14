/*****************************************************************************/
/* tcp_proxy_common.p4
/*****************************************************************************/

#include "../common-p4+/common_txdma_dummy.p4"
#include "platform/capri/capri_common.hpp"

/******************************************************************************
 * Table names
 *
 * Table names have to alphabetically be in chronological order (to match with
 * common program table names), so they need * to be prefixed by stage# and
 * table#
 *****************************************************************************/
#define tx_table_s0_t0 s0_t0_tcp_tx

#define tx_table_s1_t0 s1_t0_tcp_tx
#define tx_table_s1_t1 s1_t1_tcp_tx
#define tx_table_s1_t2 s1_t2_tcp_tx

#define tx_table_s2_t0 s2_t0_tcp_tx
#define tx_table_s2_t1 s2_t1_tcp_tx
#define tx_table_s2_t2 s2_t2_tcp_tx

#define tx_table_s3_t0 s3_t0_tcp_tx

#define tx_table_s4_t0 s4_t0_tcp_tx
#define tx_table_s4_t1 s4_t1_tcp_tx

#define tx_table_s5_t0 s5_t0_tcp_tx

#define tx_table_s6_t0 s6_t0_tcp_tx

#define tx_table_s7_t0 s7_t0_tcp_tx


/******************************************************************************
 * Action names
 *****************************************************************************/
#define tx_table_s0_t0_action read_rx2tx

#define tx_table_s1_t0_action read_rx2tx_extra
#define tx_table_s1_t1_action read_sesq_ci
#define tx_table_s1_t1_action1 pending
#define tx_table_s1_t1_action2 read_sesq_retx_ci
#define tx_table_s1_t2_action read_xmit

#define tx_table_s2_t0_action read_descr
#define tx_table_s2_t1_action read_tcp_flags

#define tx_table_s3_t0_action retx

#define tx_table_s4_t0_action cc_and_fra
#define tx_table_s4_t1_action read_nmdr_gc_pi

#define tx_table_s5_t0_action xmit

#define tx_table_s6_t0_action tso

#define tx_table_s7_t0_action stats

#include "../common-p4+/common_txdma.p4"

#include "tcp_proxy_common.p4"

/******************************************************************************
 * Macros
 *****************************************************************************/
#define GENERATE_GLOBAL_K \
    modify_field(common_global_scratch.fid, common_phv.fid); \
    modify_field(common_global_scratch.qstate_addr, common_phv.qstate_addr); \
    modify_field(common_global_scratch.snd_una, common_phv.snd_una); \
    modify_field(common_global_scratch.pending_retx_cleanup, common_phv.pending_retx_cleanup); \
    modify_field(common_global_scratch.fin, common_phv.fin); \
    modify_field(common_global_scratch.rst, common_phv.rst); \
    modify_field(common_global_scratch.pending_rx2tx, common_phv.pending_rx2tx); \
    modify_field(common_global_scratch.pending_sesq, common_phv.pending_sesq); \
    modify_field(common_global_scratch.pending_dup_ack_send, common_phv.pending_dup_ack_send); \
    modify_field(common_global_scratch.pending_ack_send, common_phv.pending_ack_send); \
    modify_field(common_global_scratch.pending_rto, common_phv.pending_rto); \
    modify_field(common_global_scratch.pending_fast_retx, common_phv.pending_fast_retx); \
    modify_field(common_global_scratch.partial_retx_cleanup, common_phv.partial_retx_cleanup); \
    modify_field(common_global_scratch.debug_dol_dont_send_ack, common_phv.debug_dol_dont_send_ack);\
    modify_field(common_global_scratch.pending_asesq, common_phv.pending_asesq); \
    modify_field(common_global_scratch.debug_dol_dont_tx, common_phv.debug_dol_dont_tx); \
    modify_field(common_global_scratch.debug_dol_free_rnmdr, common_phv.debug_dol_free_rnmdr);\
    modify_field(common_global_scratch.debug_dol_dont_start_retx_timer, common_phv.debug_dol_dont_start_retx_timer);\
    modify_field(common_global_scratch.debug_dol_bypass_barco, common_phv.debug_dol_bypass_barco);\
    modify_field(common_global_scratch.debug_dol_force_tbl_setaddr, common_phv.debug_dol_force_tbl_setaddr);

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

        CAPRI_QSTATE_HEADER_RING(5)

        CAPRI_QSTATE_HEADER_RING(6)

        CAPRI_QSTATE_HEADER_RING(7) // offset 36, total 40 bytes

        // **Need to match offsets in tcp-table.h**
        sesq_tx_ci : 16;        // offset 40 (TCP_TCB_RX2TX_TX_CI_OFFSET)

        sesq_retx_ci : 16;      // offset 42 (TCP_TCB_RX2TX_RETX_CI_OFFSET)
        asesq_retx_ci: 16;      // offset 44
        clean_retx_pending : 8; // offset 46 (TCP_TCB_RX2TX_RETX_PENDING_OFFSET)

        debug_dol_tblsetaddr : 8;

        // delayed ack timeout decremented on every timer tick
        ato_deadline : 16;      // offset 48 (TCP_TCB_RX2TX_ATO_OFFSET)

        // retransmission timeout decremented on every timer tick
        rto_deadline : 16;      // offset 50 (TCP_TCB_RX2TX_RTO_OFFSET)

        debug_dol_tx : 16;

        sesq_base : HBM_ADDRESS_WIDTH;

        perpetual_timer_started : 1;
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

header_type read_sesq_retx_ci_d_t {
    fields {
        pad1                    : 16;
        len1                    : 14;
        descr1_addr             : 34;
        pad2                    : 16;
        len2                    : 14;
        descr2_addr             : 34;
        pad3                    : 16;
        len3                    : 14;
        descr3_addr             : 34;
        pad4                    : 16;
        len4                    : 14;
        descr4_addr             : 34;
        pad5                    : 16;
        len5                    : 14;
        descr5_addr             : 34;
        pad6                    : 16;
        len6                    : 14;
        descr6_addr             : 34;
        pad7                    : 16;
        len7                    : 14;
        descr7_addr             : 34;
        pad8                    : 16;
        len8                    : 14;
        descr8_addr             : 34;
    }
}

header_type tcp_tx_pending_d_t {
    fields {
        TCB_RETX_SHARED_STATE
    }
}

// d for stage 2
// Reads sesq or retx_head descriptor, uses pkt_descr_aol_t

// d for stage 2 table 1
header_type tcp_tx_read_tcp_flags_d_t {
    fields {
        tcp_flags               : 8;
    }
}

// d for stage 3
header_type tcp_retx_d_t {
    fields {
        TCB_RETX_SHARED_STATE
    }
}

// d for stage 4
header_type tcp_cc_and_fra_d_t {
    fields {
        TCB_CC_AND_FRA_SHARED_STATE
    }
}

header_type read_nmdr_gc_d_t {
    fields {
        sw_pi                   : 16;
        sw_ci                   : 16;
    }
}

// d for stage 5
header_type tcp_xmit_d_t {
    fields {
        TCB_XMIT_SHARED_STATE
    }
}

// d for stage 6
header_type tso_d_t {
    fields {
        TCB_TSO_STATE
    }
}

/******************************************************************************
 * Global PHV definitions
 *****************************************************************************/
header_type common_global_phv_t {
    fields {
        // global k (max 128)
        fid                     : 24;
        qstate_addr             : 34;
        snd_una                 : SEQ_NUMBER_WIDTH;
        fin                     : 1;
        rst                     : 1;
        pending_retx_cleanup    : 1;
        pending_rx2tx           : 1;
        pending_sesq            : 1;
        pending_dup_ack_send    : 1;
        pending_ack_send        : 1;
        pending_rto             : 1;
        pending_fast_retx       : 1;
        partial_retx_cleanup    : 1;
        debug_dol_dont_send_ack : 1;
        pending_asesq           : 1;
        debug_dol_dont_tx       : 1;
        debug_dol_free_rnmdr    : 1;
        debug_dol_dont_start_retx_timer : 1;
        debug_dol_bypass_barco : 1;
        debug_dol_force_tbl_setaddr : 1;
    }
}

header_type to_stage_1_phv_t {
    fields {
        sesq_ci_addr            : HBM_ADDRESS_WIDTH;
        num_retx_pkts           : 16;
    }
}

header_type to_stage_2_phv_t {
    fields {
        sesq_cidx               : 16;
    }
}

header_type to_stage_3_phv_t {
    fields {
        sesq_retx_ci            : CAPRI_SESQ_RING_SLOTS_SHIFT;
        clean_retx_pi           : 16;
        window_not_restricted   : 1;
    }
}

header_type to_stage_4_phv_t {
    fields {
        packets_out             : 16;
        sacked_out              : 16;
        lost_out                : 16;
        retrans_out             : 16;
        snd_ssthresh            : 16;
        is_cwnd_limited         : 8;
    }
}

header_type to_stage_5_phv_t {
    fields {
        snd_cwnd                : 16;
        rto                     : 16;
        sesq_tx_ci              : 16;
        rcv_mss_shft            : 4;
        quick                   : 4;
        pingpong                : 1;
        window_open             : 1;
    }
}

header_type to_stage_6_phv_t {
    fields {
        rcv_mss                 : 16;
        rcv_wnd                 : 16;

        pending_challenge_ack_send : 1;
        pending_sync_mss        : 1;
        pending_tso_keep_alive  : 1;
        pending_tso_pmtu_probe  : 1;
        pending_tso_data        : 1;
        pending_tso_probe_data  : 1;
        pending_tso_retx        : 1;
    }
}

header_type to_stage_7_phv_t {
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
        snd_wnd                 : 16;
        state                   : 8;
        packets_out_decr        : 4;
        addr                    : HBM_FULL_ADDRESS_WIDTH;
        len                     : 16;
        snd_nxt                 : SEQ_NUMBER_WIDTH;
        rcv_nxt                 : 32;
    }
}

header_type common_t0_s2s_clean_retx_phv_t {
    fields {
        snd_wnd                 : 16;
        state                   : 8;
        packets_out_decr        : 4;
        num_retx_pkts           : 8;
        pkts_acked              : 8;
        len1                    : 14;
        len2                    : 14;
        len3                    : 14;
        len4                    : 14;
        len5                    : 14;
        len6                    : 14;
        len7                    : 14;
        len8                    : 14;
    }
}

header_type common_t1_s2s_phv_t {
    fields {
        num_pkts_freed          : 8;
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
metadata hbm_al_ring_entry_t read_sesq_ci_d;
@pragma scratch_metadata
metadata read_sesq_retx_ci_d_t read_sesq_retx_ci_d;
@pragma scratch_metadata
metadata tcp_tx_pending_d_t pending_d;
@pragma scratch_metadata
metadata pkt_descr_aol_t read_descr_d;
@pragma scratch_metadata
metadata tcp_tx_read_tcp_flags_d_t read_tcp_flags_d;
@pragma scratch_metadata
metadata tcp_retx_d_t retx_d;
@pragma scratch_metadata
metadata tcp_cc_and_fra_d_t cc_and_fra_d;
@pragma scratch_metadata
metadata tcp_xmit_d_t xmit_d;
@pragma scratch_metadata
metadata read_nmdr_gc_d_t read_nmdr_gc_d;
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
@pragma pa_header_union ingress to_stage_6
metadata to_stage_6_phv_t to_s6;
@pragma pa_header_union ingress to_stage_7
metadata to_stage_7_phv_t to_s7;

@pragma pa_header_union ingress common_t0_s2s t0_s2s_clean_retx
metadata common_t0_s2s_phv_t t0_s2s;
metadata common_t0_s2s_clean_retx_phv_t t0_s2s_clean_retx;
@pragma pa_header_union ingress common_t1_s2s
metadata common_t1_s2s_phv_t t1_s2s;


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
metadata to_stage_6_phv_t to_s6_scratch;
@pragma scratch_metadata
metadata to_stage_7_phv_t to_s7_scratch;

@pragma scratch_metadata
metadata common_t0_s2s_phv_t t0_s2s_scratch;
@pragma scratch_metadata
metadata common_t0_s2s_clean_retx_phv_t t0_s2s_clean_retx_scratch;
@pragma scratch_metadata
metadata common_t1_s2s_phv_t t1_s2s_scratch;

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
metadata tcp_header_t tcp_header;               // 20 bytes
@pragma dont_trim
metadata tcp_header_pad_t tcp_nop_opt1;          // 1 byte
@pragma dont_trim
metadata tcp_header_pad_t tcp_nop_opt2;          // 1 byte
@pragma dont_trim
metadata tcp_header_ts_option_t tcp_ts_opt;     // 10 bytes

header_type txdma_max_options_t {
    fields {
        pad1           : 36;
        pad2           : 104;
    }
}
@pragma dont_trim
metadata txdma_max_options_t tcp_header_options;

@pragma pa_align 64
@pragma dont_trim
metadata ring_entry_t ring_entry1;

@pragma dont_trim
metadata ring_entry_t ring_entry2;
@pragma dont_trim
metadata ring_entry_t ring_entry3;
@pragma dont_trim
metadata ring_entry_t ring_entry4;
@pragma dont_trim
metadata ring_entry_t ring_entry5;
@pragma dont_trim
metadata ring_entry_t ring_entry6;
@pragma dont_trim
metadata ring_entry_t ring_entry7;
@pragma dont_trim
metadata ring_entry_t ring_entry8;
@pragma dont_trim
metadata doorbell_data_t db_data;

@pragma pa_align 128
@pragma dont_trim
metadata dma_cmd_phv2pkt_t intrinsic_dma;    // dma cmd 1
@pragma dont_trim
metadata dma_cmd_mem2pkt_t l2l3_header_dma;  // dma cmd 2
@pragma dont_trim
metadata dma_cmd_phv2pkt_t tcp_header_dma;   // dma cmd 3
@pragma dont_trim
metadata dma_cmd_phv2mem_t ringentry_dma;    // dma cmd 4
@pragma dont_trim
metadata dma_cmd_phv2mem_t ringentry2_dma;    // dma cmd 5
@pragma dont_trim
metadata dma_cmd_phv2mem_t doorbell_dma;     // dma cmd 6
@pragma dont_trim
metadata dma_cmd_mem2pkt_t data_dma;         // dma cmd 7
@pragma dont_trim
metadata dma_cmd_phv2mem_t tx2rx_dma;        // dma cmd 8

/******************************************************************************
 * Action functions to generate k_struct and d_struct
 *
 * These action functions are currently only to generate the k+i and d structs
 * and do not implement any pseudo code
 *****************************************************************************/

#define RX2TX_PARAMS                                                                                  \
rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid, pi_0,ci_0, pi_1, ci_1,\
pi_2, ci_2, pi_3, ci_3, pi_4, ci_4, pi_5, ci_5, pi_6, ci_6,\
pi_7, ci_7, sesq_tx_ci, sesq_retx_ci, asesq_retx_ci, clean_retx_pending,\
debug_dol_tblsetaddr, ato_deadline, rto_deadline, debug_dol_tx, sesq_base,\
perpetual_timer_started

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
    modify_field(rx2tx_d.pi_6, pi_6);                                                                  \
    modify_field(rx2tx_d.ci_6, ci_6);                                                                  \
    modify_field(rx2tx_d.pi_7, pi_7);                                                                  \
    modify_field(rx2tx_d.ci_7, ci_7);                                                                  \
    modify_field(rx2tx_d.sesq_tx_ci, sesq_tx_ci);                                                      \
    modify_field(rx2tx_d.sesq_retx_ci, sesq_retx_ci);                                                  \
    modify_field(rx2tx_d.asesq_retx_ci, asesq_retx_ci);                                                \
    modify_field(rx2tx_d.clean_retx_pending, clean_retx_pending);                                      \
    modify_field(rx2tx_d.debug_dol_tblsetaddr, debug_dol_tblsetaddr);                                  \
    modify_field(rx2tx_d.ato_deadline, ato_deadline);                                                  \
    modify_field(rx2tx_d.rto_deadline, rto_deadline);                                                  \
    modify_field(rx2tx_d.debug_dol_tx, debug_dol_tx);                                                  \
    modify_field(rx2tx_d.sesq_base, sesq_base);                                                        \
    modify_field(rx2tx_d.perpetual_timer_started, perpetual_timer_started);                            \

#define GENERATE_T0_S2S                                                                 \
    modify_field(t0_s2s_scratch.snd_wnd, t0_s2s.snd_wnd);                               \
    modify_field(t0_s2s_scratch.state, t0_s2s.state);                                   \
    modify_field(t0_s2s_scratch.packets_out_decr, t0_s2s.packets_out_decr);             \
    modify_field(t0_s2s_scratch.addr, t0_s2s.addr);                                     \
    modify_field(t0_s2s_scratch.len, t0_s2s.len);                                       \
    modify_field(t0_s2s_scratch.snd_nxt, t0_s2s.snd_nxt);                               \
    modify_field(t0_s2s_scratch.rcv_nxt, t0_s2s.rcv_nxt);

#define GENERATE_T0_S2S_CLEAN_RETX                                                      \
    modify_field(t0_s2s_clean_retx_scratch.snd_wnd, t0_s2s_clean_retx.snd_wnd); \
    modify_field(t0_s2s_clean_retx_scratch.state, t0_s2s_clean_retx.state); \
    modify_field(t0_s2s_clean_retx_scratch.packets_out_decr, t0_s2s_clean_retx.packets_out_decr); \
    modify_field(t0_s2s_clean_retx_scratch.num_retx_pkts, t0_s2s_clean_retx.num_retx_pkts); \
    modify_field(t0_s2s_clean_retx_scratch.pkts_acked, t0_s2s_clean_retx.pkts_acked); \
    modify_field(t0_s2s_clean_retx_scratch.len1, t0_s2s_clean_retx.len1); \
    modify_field(t0_s2s_clean_retx_scratch.len2, t0_s2s_clean_retx.len2); \
    modify_field(t0_s2s_clean_retx_scratch.len3, t0_s2s_clean_retx.len3); \
    modify_field(t0_s2s_clean_retx_scratch.len4, t0_s2s_clean_retx.len4); \
    modify_field(t0_s2s_clean_retx_scratch.len5, t0_s2s_clean_retx.len5); \
    modify_field(t0_s2s_clean_retx_scratch.len6, t0_s2s_clean_retx.len6); \
    modify_field(t0_s2s_clean_retx_scratch.len7, t0_s2s_clean_retx.len7); \
    modify_field(t0_s2s_clean_retx_scratch.len8, t0_s2s_clean_retx.len8);

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
       rcv_nxt, snd_wnd, rcv_wnd, rto, snd_una, rcv_tsval, srtt_us,
       prior_ssthresh, high_seq, ooo_datalen,
       reordering, undo_retrans, snd_ssthresh, loss_cwnd,
       write_seq, rcv_mss, state, ca_state, ecn_flags, num_sacks,
       pending_dup_ack_send, pending_challenge_ack_send,
       pending_sync_mss, pending_tso_keepalive, pending_tso_pmtu_probe,
       pending_tso_data, pending_tso_probe_data, pending_tso_probe,
       pending_ooo_se_recv, pending_tso_retx, pending_rexmit, pending,
       ack_blocked, ack_pending, snd_wscale, rcv_mss_shft, quick,
       pingpong, pending_reset_backoff, dsack, pad_rx2tx_extra) {

    // from ki global
    GENERATE_GLOBAL_K

    // from to_stage 1
    modify_field(to_s1_scratch.sesq_ci_addr, to_s1.sesq_ci_addr);
    modify_field(to_s1_scratch.num_retx_pkts, to_s1.num_retx_pkts);

    // from stage to stage
    GENERATE_T0_S2S

    // d for stage 1
    modify_field(rx2tx_extra_d.rcv_nxt, rcv_nxt);
    modify_field(rx2tx_extra_d.snd_wnd, snd_wnd);
    modify_field(rx2tx_extra_d.rcv_wnd, rcv_wnd);
    modify_field(rx2tx_extra_d.rto, rto);
    modify_field(rx2tx_extra_d.snd_una, snd_una);
    modify_field(rx2tx_extra_d.rcv_tsval, rcv_tsval);
    modify_field(rx2tx_extra_d.srtt_us, srtt_us);
    modify_field(rx2tx_extra_d.prior_ssthresh, prior_ssthresh);
    modify_field(rx2tx_extra_d.high_seq, high_seq);
    modify_field(rx2tx_extra_d.ooo_datalen, ooo_datalen);
    modify_field(rx2tx_extra_d.reordering, reordering);
    modify_field(rx2tx_extra_d.undo_retrans, undo_retrans);
    modify_field(rx2tx_extra_d.snd_ssthresh, snd_ssthresh);
    modify_field(rx2tx_extra_d.loss_cwnd, loss_cwnd);
    modify_field(rx2tx_extra_d.write_seq, write_seq);
    modify_field(rx2tx_extra_d.rcv_mss, rcv_mss);
    modify_field(rx2tx_extra_d.state, state);
    modify_field(rx2tx_extra_d.ca_state, ca_state);
    modify_field(rx2tx_extra_d.ecn_flags, ecn_flags);
    modify_field(rx2tx_extra_d.num_sacks, num_sacks);
    modify_field(rx2tx_extra_d.pending_dup_ack_send, pending_dup_ack_send);
    modify_field(rx2tx_extra_d.pending_challenge_ack_send, pending_challenge_ack_send);
    modify_field(rx2tx_extra_d.pending_sync_mss, pending_sync_mss);
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
    modify_field(rx2tx_extra_d.pending_reset_backoff, pending_reset_backoff);
    modify_field(rx2tx_extra_d.dsack, dsack);
    modify_field(rx2tx_extra_d.pad_rx2tx_extra, pad_rx2tx_extra);
}

/*
 * Stage 1 table 1 action
 */
action read_sesq_ci(pad, len, descr_addr) {

    // from ki global
    GENERATE_GLOBAL_K

    // from to_stage 1

    // d for stage 1
    modify_field(read_sesq_ci_d.pad, pad);
    modify_field(read_sesq_ci_d.len, len);
    modify_field(read_sesq_ci_d.descr_addr, descr_addr);
}

/*
 * Stage 1 table 1 action 1
 */
action pending(RETX_SHARED_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // d (use RETX D)
    GENERATE_RETX_SHARED_D
}

/*
 * Stage 1 table 1 action 2
 */
action read_sesq_retx_ci(pad1, len1, descr1_addr,
pad2, len2, descr2_addr, pad3, len3, descr3_addr,
pad4, len4, descr4_addr, pad5, len5, descr5_addr,
pad6, len6, descr6_addr, pad7, len7, descr7_addr,
pad8, len8, descr8_addr) {

    // from ki global
    GENERATE_GLOBAL_K

    // from to_stage 1

    // d for stage 1
    modify_field(read_sesq_retx_ci_d.pad1, pad1);
    modify_field(read_sesq_retx_ci_d.len1, len1);
    modify_field(read_sesq_retx_ci_d.descr1_addr, descr1_addr);

    modify_field(read_sesq_retx_ci_d.pad2, pad2);
    modify_field(read_sesq_retx_ci_d.len2, len2);
    modify_field(read_sesq_retx_ci_d.descr2_addr, descr2_addr);

    modify_field(read_sesq_retx_ci_d.pad3, pad3);
    modify_field(read_sesq_retx_ci_d.len3, len3);
    modify_field(read_sesq_retx_ci_d.descr3_addr, descr3_addr);

    modify_field(read_sesq_retx_ci_d.pad4, pad4);
    modify_field(read_sesq_retx_ci_d.len4, len4);
    modify_field(read_sesq_retx_ci_d.descr4_addr, descr4_addr);

    modify_field(read_sesq_retx_ci_d.pad5, pad5);
    modify_field(read_sesq_retx_ci_d.len5, len5);
    modify_field(read_sesq_retx_ci_d.descr5_addr, descr5_addr);

    modify_field(read_sesq_retx_ci_d.pad6, pad6);
    modify_field(read_sesq_retx_ci_d.len6, len6);
    modify_field(read_sesq_retx_ci_d.descr6_addr, descr6_addr);

    modify_field(read_sesq_retx_ci_d.pad7, pad7);
    modify_field(read_sesq_retx_ci_d.len7, len7);
    modify_field(read_sesq_retx_ci_d.descr7_addr, descr7_addr);

    modify_field(read_sesq_retx_ci_d.pad8, pad8);
    modify_field(read_sesq_retx_ci_d.len8, len8);
    modify_field(read_sesq_retx_ci_d.descr8_addr, descr8_addr);
}


/*
 * Stage 1 table 2 action 1
 */
action read_xmit(XMIT_SHARED_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // d (use XMIT D)
    GENERATE_XMIT_SHARED_D
}
/*
 * Stage 2 table 0 action
 */
action read_descr(A0, O0, L0, A1, O1, L1, A2, O2, L2, next_addr, next_pkt) {
    // from ki global
    GENERATE_GLOBAL_K

    // from stage to stage
    GENERATE_T0_S2S

    // d for stage 2
    modify_field(read_descr_d.A0, A0);
    modify_field(read_descr_d.O0, O0);
    modify_field(read_descr_d.L0, L0);
    modify_field(read_descr_d.A1, A1);
    modify_field(read_descr_d.O1, O1);
    modify_field(read_descr_d.L1, L1);
    modify_field(read_descr_d.A2, A2);
    modify_field(read_descr_d.O2, O2);
    modify_field(read_descr_d.L2, L2);
    modify_field(read_descr_d.next_addr, next_addr);
    modify_field(read_descr_d.next_pkt, next_pkt);
}

/*
 * Stage 2 table 1 action
 */
action read_tcp_flags(tcp_flags) {
    // from ki global
    GENERATE_GLOBAL_K

    // d for stage 2 table 1
    modify_field(read_tcp_flags_d.tcp_flags, tcp_flags);
}
/*
 * Stage 3 table 0 action
 */
action retx(RETX_SHARED_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // from to_stage 3
    modify_field(to_s3_scratch.sesq_retx_ci, to_s3.sesq_retx_ci);
    modify_field(to_s3_scratch.clean_retx_pi, to_s3.clean_retx_pi);
    modify_field(to_s3_scratch.window_not_restricted, to_s3.window_not_restricted);

    // from stage to stage
    if (tx_rst_sent == 1) {
        GENERATE_T0_S2S
    } else {
        GENERATE_T0_S2S_CLEAN_RETX
    }

    // d for stage 3 table 0
    GENERATE_RETX_SHARED_D
}

/*
 * Stage 4 table 0 action
 */
action cc_and_fra(CC_AND_FRA_SHARED_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // from to_stage 4
    modify_field(to_s4_scratch.packets_out, to_s4.packets_out);
    modify_field(to_s4_scratch.sacked_out, to_s4.sacked_out);
    modify_field(to_s4_scratch.lost_out, to_s4.lost_out);
    modify_field(to_s4_scratch.retrans_out, to_s4.retrans_out);
    modify_field(to_s4_scratch.snd_ssthresh, to_s4.snd_ssthresh);
    modify_field(to_s4_scratch.is_cwnd_limited, to_s4.is_cwnd_limited);

    // from stage to stage
    if (prr_out == 1) {
        GENERATE_T0_S2S
    } else {
        GENERATE_T0_S2S_CLEAN_RETX
    }

    // d for stage 4 table 0
    GENERATE_CC_AND_FRA_SHARED_D
}

/*
 * Stage 4 table 1 action
 */
action read_nmdr_gc_pi(sw_pi, sw_ci) {
    // from ki global
    GENERATE_GLOBAL_K

    // from stage to stage
    modify_field(t1_s2s_scratch.num_pkts_freed, t1_s2s.num_pkts_freed);

    // d for stage 4 table 1 read-rnmdr-idx
    modify_field(read_nmdr_gc_d.sw_pi, sw_pi);
    modify_field(read_nmdr_gc_d.sw_ci, sw_ci);
}

/*
 * Stage 5 table 0 action
 */
action xmit(XMIT_SHARED_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // from to_stage 5
    modify_field(to_s5_scratch.snd_cwnd, to_s5.snd_cwnd);
    modify_field(to_s5_scratch.rto, to_s5.rto);
    modify_field(to_s5_scratch.sesq_tx_ci, to_s5.sesq_tx_ci);
    modify_field(to_s5_scratch.rcv_mss_shft, to_s5.rcv_mss_shft);
    modify_field(to_s5_scratch.quick, to_s5.quick);
    modify_field(to_s5_scratch.pingpong, to_s5.pingpong);
    modify_field(to_s5_scratch.window_open, to_s5.window_open);

    // from stage to stage
    GENERATE_T0_S2S

    // d for stage 4 table 0
    GENERATE_XMIT_SHARED_D
}

/*
 * Stage 6 table 0 action
 */
action tso(TSO_PARAMS) {
    // from ki global
    GENERATE_GLOBAL_K

    // from to_stage 6
    modify_field(to_s6_scratch.rcv_mss, to_s6.rcv_mss);
    modify_field(to_s6_scratch.rcv_wnd, to_s6.rcv_wnd);
    modify_field(to_s6_scratch.pending_challenge_ack_send, to_s6.pending_challenge_ack_send);
    modify_field(to_s6_scratch.pending_sync_mss, to_s6.pending_sync_mss);
    modify_field(to_s6_scratch.pending_tso_keep_alive, to_s6.pending_tso_keep_alive);
    modify_field(to_s6_scratch.pending_tso_pmtu_probe, to_s6.pending_tso_pmtu_probe);
    modify_field(to_s6_scratch.pending_tso_data, to_s6.pending_tso_data);
    modify_field(to_s6_scratch.pending_tso_probe_data, to_s6.pending_tso_probe_data);
    modify_field(to_s6_scratch.pending_tso_retx, to_s6.pending_tso_retx);

    // from stage to stage
    GENERATE_T0_S2S

    // d for stage 4 table 0
    GENERATE_TSO_SHARED_D
}

/*
 * Stage 7 table 0 action
 */
action stats() {
    // from ki global
    GENERATE_GLOBAL_K

    // from to_stage 7
    modify_field(to_s7_scratch.bytes_sent, to_s7.bytes_sent);
    modify_field(to_s7_scratch.pkts_sent, to_s7.pkts_sent);
    modify_field(to_s7_scratch.debug_num_phv_to_pkt, to_s7.debug_num_phv_to_pkt);
    modify_field(to_s7_scratch.debug_num_mem_to_pkt, to_s7.debug_num_mem_to_pkt);

    // d for stage 7 table 0
}
