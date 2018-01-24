/*****************************************************************************/
/* tcp_proxy_common.p4
/*****************************************************************************/

/******************************************************************************
 * Rx2Tx shared state in PHV
 *****************************************************************************/
#define SEQ_NUMBER_WIDTH                32
#define TS_WIDTH                        32
#define RING_INDEX_WIDTH                16
#define ADDRESS_WIDTH                   16
#define OFFSET_WIDTH                    16
#define LEN_WIDTH                       16
#define COUNTER32                       32
#define COUNTER16                       16
#define COUNTER8                        8
#define WINDOW_WIDTH                    16
#define MTU_WIDTH                       8
#define TCP_OOO_NUM_CELLS               64 // needs to match entry in tcp-constants.h

#define TX2RX_SHARED_STATE \
        prr_out                         : SEQ_NUMBER_WIDTH      ;\
        snd_nxt                         : SEQ_NUMBER_WIDTH      ;\
        rcv_wup                         : 32                    ;\
        packets_out                     : 16                    ;\
        ecn_flags_tx                    : 8                     ;\
        quick_acks_decr                 : 4                     ;\
        fin_sent                        : 1                     ;\
        pad1_tx2rx                      : 3                     ;\


#define RX2TX_SHARED_STATE \
        snd_una                         : SEQ_NUMBER_WIDTH      ;\
        rcv_nxt                         : SEQ_NUMBER_WIDTH      ;\
        snd_wnd                         : SEQ_NUMBER_WIDTH      ;\
        ft_pi                           : 16                    ;\
        rto                             : 16                    ;\
        rx_flag                         : 8                     ;\
        state                           : 8                     ;\
        pending_ack_send                : 1                     ;\
        saved_pending_ack_send          : 1                     ;\
        pad1_rx2tx                      : 14                    ;


#define RX2TX_SHARED_EXTRA_STATE \
        ato_deadline                    : TS_WIDTH              ;\
        rcv_tsval                       : TS_WIDTH              ;\
        srtt_us                         : TS_WIDTH              ;\
        rcv_wnd                         : WINDOW_WIDTH          ;\
        prior_ssthresh                  : WINDOW_WIDTH          ;\
        high_seq                        : SEQ_NUMBER_WIDTH      ;\
        sacked_out                      : COUNTER16             ;\
        lost_out                        : COUNTER8              ;\
        retrans_out                     : COUNTER8              ;\
        fackets_out                     : COUNTER16             ;\
        ooo_datalen                     : COUNTER16             ;\
        reordering                      : COUNTER32             ;\
        undo_marker                     : SEQ_NUMBER_WIDTH      ;\
        undo_retrans                    : SEQ_NUMBER_WIDTH      ;\
        snd_ssthresh                    : WINDOW_WIDTH          ;\
        loss_cwnd                       : WINDOW_WIDTH          ;\
        write_seq                       : SEQ_NUMBER_WIDTH      ;\
        tso_seq                         : SEQ_NUMBER_WIDTH      ;\
        rcv_mss                         : 16                    ;\
        ca_state                        : 8                     ;\
        ecn_flags                       : 8                     ;\
        num_sacks                       : 8                     ;\
        pending_challenge_ack_send      : 1                     ;\
        pending_sync_mss                : 1                     ;\
        pending_tso_keepalive           : 1                     ;\
        pending_tso_pmtu_probe          : 1                     ;\
        pending_tso_data                : 1                     ;\
        pending_tso_probe_data          : 1                     ;\
        pending_tso_probe               : 1                     ;\
        pending_ooo_se_recv             : 1                     ;\
        pending_tso_retx                : 1                     ;\
        pending_rexmit                  : 2                     ;\
        pending                         : 2                     ;\
        ack_blocked                     : 1                     ;\
        ack_pending                     : 3                     ;\
        snd_wscale                      : 4                     ;\
        rcv_mss_shft                    : 4                     ;\
        quick                           : 4                     ;\
        pingpong                        : 1                     ;\
        pending_reset_backoff           : 1                     ;\
        dsack                           : 1                     ;\
        pad_rx2tx_extra                 : 13                    ;\

#define TCB_RETX_SHARED_STATE \
        retx_snd_una                    : SEQ_NUMBER_WIDTH      ;\
        retx_head_desc                  : HBM_ADDRESS_WIDTH     ;\
        retx_tail_desc                  : HBM_ADDRESS_WIDTH     ;\
        retx_xmit_cursor                : 40                    ;\
        retx_next_desc                  : HBM_ADDRESS_WIDTH     ;\
        retx_head_offset                : 16                    ;\
        retx_head_len                   : 16                    ;\

#define TCB_CC_AND_FRA_SHARED_STATE \
        prr_out                 : 16;   \
        prr_delivered           : 16;   \
        last_time               : 32;   \
        epoch_start             : 32;   \
        cnt                     : 16;   \
        last_max_cwnd           : 16;   \
        snd_cwnd_cnt            : 16;   \
        snd_cwnd_clamp          : 16;   \
        snd_cwnd                : 16;   \
        prior_cwnd              : 16;   \
        last_cwnd               : 16;   \
        tune_reordering         : 8;    \
        sack_reordering         : 8;    \
        max_packets_out         : 8;    \
        ca_state                : 8;    \
        delayed_ack             : 8;

#define TCB_XMIT_SHARED_STATE \
        snd_nxt                         : SEQ_NUMBER_WIDTH      ;\
        xmit_next_desc                  : HBM_ADDRESS_WIDTH     ;\
        xmit_cursor_addr                : 40                    ;\
        xmit_offset                     : 16                    ;\
        xmit_len                        : 16                    ;\
        packets_out                     : 16                    ;\
        sacked_out                      : 16                    ;\
        rto_pi                          : 16                    ;\
        retrans_out                     : 16                    ;\
        lost_out                        : 16                    ;\
        flag                            : 16                    ;\
        is_cwnd_limited                 : 8                     ;\
        rto_backoff                     : 4                     ;\
        pending_ack_tx                  : 1                     ;\
        pending_delayed_ack_tx          : 1                     ;\
        pending_tso_data                : 1                     ;\
        pending_pad                     : 5                     ;\

#define TCB_TSO_STATE \
        source_lif                      : 16                    ;\
        source_port                     : 16                    ;\
        dest_port                       : 16                    ;\
        header_len                      : 8                     ;\
        bytes_sent                      : 16                    ;\
        pkts_sent                       : 8                     ;\
        debug_num_phv_to_pkt            : 8                     ;\
        debug_num_mem_to_pkt            : 8                     ;\
        quick_acks_decr                 : 4                     ;\

#define RETX_SHARED_PARAMS \
retx_snd_una, retx_head_desc, retx_tail_desc,\
retx_xmit_cursor, retx_next_desc,\
retx_head_offset, retx_head_len

#define CC_AND_FRA_SHARED_PARAMS \
prr_out,\
prr_delivered, last_time, epoch_start, cnt,\
last_max_cwnd, snd_cwnd_cnt, snd_cwnd_clamp,\
snd_cwnd, prior_cwnd, last_cwnd,\
tune_reordering, sack_reordering,\
max_packets_out, ca_state, delayed_ack

#define XMIT_SHARED_PARAMS \
snd_nxt,\
xmit_cursor_addr, xmit_next_desc,\
xmit_offset, xmit_len,\
packets_out, sacked_out, rto_pi, retrans_out, lost_out,\
is_cwnd_limited, flag, ca_state, rto_backoff,\
pending_ack_tx,pending_delayed_ack_tx, pending_tso_data, pending_pad

#define TSO_PARAMS                                                        \
source_lif, source_port, dest_port, header_len,\
bytes_sent,pkts_sent,debug_num_phv_to_pkt, debug_num_mem_to_pkt,\
quick_acks_decr


#define GENERATE_RETX_SHARED_D \
    modify_field(retx_d.retx_snd_una, retx_snd_una); \
    modify_field(retx_d.retx_head_desc, retx_head_desc); \
    modify_field(retx_d.retx_tail_desc, retx_tail_desc); \
    modify_field(retx_d.retx_xmit_cursor, retx_xmit_cursor); \
    modify_field(retx_d.retx_next_desc, retx_next_desc); \
    modify_field(retx_d.retx_head_offset, retx_head_offset); \
    modify_field(retx_d.retx_head_len, retx_head_len); \

#define GENERATE_CC_AND_FRA_SHARED_D \
    modify_field(cc_and_fra_d.prr_out, prr_out); \
    modify_field(cc_and_fra_d.prr_delivered, prr_delivered); \
    modify_field(cc_and_fra_d.last_time, last_time); \
    modify_field(cc_and_fra_d.epoch_start, epoch_start); \
    modify_field(cc_and_fra_d.cnt, cnt); \
    modify_field(cc_and_fra_d.last_max_cwnd, last_max_cwnd); \
    modify_field(cc_and_fra_d.snd_cwnd_cnt, snd_cwnd_cnt); \
    modify_field(cc_and_fra_d.snd_cwnd_clamp, snd_cwnd_clamp); \
    modify_field(cc_and_fra_d.snd_cwnd, snd_cwnd); \
    modify_field(cc_and_fra_d.prior_cwnd, prior_cwnd); \
    modify_field(cc_and_fra_d.last_cwnd, last_cwnd); \
    modify_field(cc_and_fra_d.tune_reordering, tune_reordering); \
    modify_field(cc_and_fra_d.sack_reordering, sack_reordering); \
    modify_field(cc_and_fra_d.max_packets_out, max_packets_out); \
    modify_field(cc_and_fra_d.ca_state, ca_state); \
    modify_field(cc_and_fra_d.delayed_ack, delayed_ack); \

#define GENERATE_XMIT_SHARED_D \
    modify_field(xmit_d.snd_nxt, snd_nxt); \
    modify_field(xmit_d.xmit_cursor_addr, xmit_cursor_addr); \
    modify_field(xmit_d.xmit_next_desc, xmit_next_desc); \
    modify_field(xmit_d.xmit_offset, xmit_offset); \
    modify_field(xmit_d.xmit_len, xmit_len); \
    modify_field(xmit_d.packets_out, packets_out); \
    modify_field(xmit_d.sacked_out, sacked_out); \
    modify_field(xmit_d.rto_pi, rto_pi); \
    modify_field(xmit_d.retrans_out, retrans_out); \
    modify_field(xmit_d.lost_out, lost_out); \
    modify_field(xmit_d.flag, flag); \
    modify_field(xmit_d.is_cwnd_limited, is_cwnd_limited); \
    modify_field(xmit_d.rto_backoff, rto_backoff); \
    modify_field(xmit_d.pending_ack_tx, pending_ack_tx); \
    modify_field(xmit_d.pending_delayed_ack_tx, pending_delayed_ack_tx); \
    modify_field(xmit_d.pending_tso_data, pending_tso_data);\
    modify_field(xmit_d.pending_pad, pending_pad);\

#define GENERATE_TSO_SHARED_D                                                                               \
    modify_field(tso_d.source_lif, source_lif); \
    modify_field(tso_d.source_port, source_port); \
    modify_field(tso_d.dest_port, dest_port); \
    modify_field(tso_d.header_len, header_len); \
    modify_field(tso_d.bytes_sent, bytes_sent);\
    modify_field(tso_d.pkts_sent, pkts_sent);\
    modify_field(tso_d.debug_num_phv_to_pkt, debug_num_phv_to_pkt);\
    modify_field(tso_d.debug_num_mem_to_pkt, debug_num_mem_to_pkt);\
    modify_field(tso_d.quick_acks_decr, quick_acks_decr);


header_type rx2tx_t {
    fields {
        RX2TX_SHARED_STATE
    }
}

header_type rx2tx_pad_t {
    fields {
        rx2tx_pad : 128;
    }
}


header_type rx2tx_extra_t {
    fields {
        RX2TX_SHARED_EXTRA_STATE
    }
}

header_type doorbell_data_pad_t {
    fields {
        db_data_pad : 16;
    }
}

header_type tx2rx_t {
    fields {
        TX2RX_SHARED_STATE
    }
}

/******************************************************************************
 * DMA commands
 *****************************************************************************/
 header_type dma_cmds_t {
     fields {
         cmd0_cmd           : 8;
         cmd0_size          : 16;
         cmd0_pad           : 40;
         cmd0_addr          : 64;

         cmd1_cmd           : 8;
         cmd1_size          : 16;
         cmd1_pad           : 40;
         cmd1_addr          : 64;

         cmd2_cmd           : 8;
         cmd2_size          : 16;
         cmd2_pad           : 40;
         cmd2_addr          : 64;

         cmd3_cmd           : 8;
         cmd3_size          : 16;
         cmd3_pad           : 40;
         cmd3_addr          : 64;

         cmd4_cmd           : 8;
         cmd4_size          : 16;
         cmd4_pad           : 40;
         cmd4_addr          : 64;

         cmd5_cmd           : 8;
         cmd5_size          : 16;
         cmd5_pad           : 40;
         cmd5_addr          : 64;
     }
 }

/******************************************************************************
 * AOL
 *****************************************************************************/
 header_type aol_t {
     fields {
         scratch        : 64;
         desc           : 16;
         page           : 16;
         offset         : 16;
         len            : 16;
         addr           : 30;
         free_pending   : 1;
         aol_valid      : 1;
         page_alloc     : 1;
     }
 }
