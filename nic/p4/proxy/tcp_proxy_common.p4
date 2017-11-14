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
        pad1_tx2rx                      : 4                     ;\


#define RX2TX_SHARED_STATE \
        snd_una                         : SEQ_NUMBER_WIDTH      ;\
        rcv_nxt                         : SEQ_NUMBER_WIDTH      ;\
        snd_wnd                         : SEQ_NUMBER_WIDTH      ;\
        ft_pi                           : 16                    ;\
        snd_cwnd                        : WINDOW_WIDTH          ;\
        rto                             : 16                    ;\
        pending_ft_clear                : 1                     ;\
        pending_ft_reset                : 1                     ;\
        pad1_rx2tx                      : 30                    ;


#define RX2TX_SHARED_EXTRA_STATE \
        ato_deadline                    : TS_WIDTH              ;\
        retx_head_ts                    : TS_WIDTH              ;\
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
        pending_ack_send                : 1                     ;\
        pending_snd_una_update          : 1                     ;\
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
        pad_rx2tx_extra                 : 11                    ;\

#define TCB_TX_SHARED_STATE \
        snd_nxt                         : SEQ_NUMBER_WIDTH      ;\
        retx_snd_una                    : SEQ_NUMBER_WIDTH      ;\
        retx_head_desc                  : HBM_ADDRESS_WIDTH     ;\
        xmit_desc                       : HBM_ADDRESS_WIDTH     ;\
        retx_tail_desc                  : HBM_ADDRESS_WIDTH     ;\
        retx_xmit_cursor                : HBM_ADDRESS_WIDTH     ;\
        xmit_cursor_addr                : HBM_ADDRESS_WIDTH     ;\
        retx_next_desc                  : HBM_ADDRESS_WIDTH     ;\
        xmit_next_desc                  : HBM_ADDRESS_WIDTH     ;\
        retx_head_offset                : 16                    ;\
        retx_head_len                   : 16                    ;\
        xmit_offset                     : 16                    ;\
        xmit_len                        : 16                    ;\
        packets_out                     : 16                    ;\
        sacked_out                      : 16                    ;\
        rto_pi                          : 16                    ;\
        rto_backoff                     : 4                     ;\
        pending_ack_tx                  : 1                     ;\
        pending_delayed_ack_tx          : 1                     ;\
        pending_tso_data                : 1                     ;\
        pending_pad                     : 5                     ;\

#define TCB_TSO_STATE \
        source_lif                      : 16                    ;\
        source_port                     : 16                    ;\
        dest_port                       : 16                    ;\
        prr_out                         : 32                    ;\
        header_len                      : 8                     ;\
        bytes_sent                      : 16                    ;\
        pkts_sent                       : 8                     ;\
        debug_num_phv_to_pkt            : 8                     ;\
        debug_num_mem_to_pkt            : 8                     ;\
        quick_acks_decr                 : 4                     ;\

#define TX_SHARED_PARAMS \
snd_nxt, retx_snd_una, retx_head_desc, xmit_desc, retx_tail_desc,\
retx_xmit_cursor, xmit_cursor_addr, retx_next_desc, xmit_next_desc,\
retx_head_offset, retx_head_len, xmit_offset, xmit_len,\
packets_out, sacked_out, rto_pi, rto_backoff,\
pending_ack_tx,pending_delayed_ack_tx, pending_tso_data, pending_pad

#define TSO_PARAMS                                                        \
source_lif, source_port, dest_port, prr_out, header_len,\
bytes_sent,pkts_sent,debug_num_phv_to_pkt, debug_num_mem_to_pkt,\
quick_acks_decr


#define GENERATE_TX_SHARED_D \
    modify_field(tcp_tx_d.snd_nxt, snd_nxt); \
    modify_field(tcp_tx_d.retx_snd_una, retx_snd_una); \
    modify_field(tcp_tx_d.retx_head_desc, retx_head_desc); \
    modify_field(tcp_tx_d.xmit_desc, xmit_desc); \
    modify_field(tcp_tx_d.retx_tail_desc, retx_tail_desc); \
    modify_field(tcp_tx_d.retx_xmit_cursor, retx_xmit_cursor); \
    modify_field(tcp_tx_d.xmit_cursor_addr, xmit_cursor_addr); \
    modify_field(tcp_tx_d.retx_next_desc, retx_next_desc); \
    modify_field(tcp_tx_d.xmit_next_desc, xmit_next_desc); \
    modify_field(tcp_tx_d.retx_head_offset, retx_head_offset); \
    modify_field(tcp_tx_d.retx_head_len, retx_head_len); \
    modify_field(tcp_tx_d.xmit_offset, xmit_offset); \
    modify_field(tcp_tx_d.xmit_len, xmit_len); \
    modify_field(tcp_tx_d.packets_out, packets_out); \
    modify_field(tcp_tx_d.sacked_out, sacked_out); \
    modify_field(tcp_tx_d.rto_pi, rto_pi); \
    modify_field(tcp_tx_d.rto_backoff, rto_backoff); \
    modify_field(tcp_tx_d.pending_ack_tx, pending_ack_tx); \
    modify_field(tcp_tx_d.pending_delayed_ack_tx, pending_delayed_ack_tx); \
    modify_field(tcp_tx_d.pending_tso_data, pending_tso_data);\
    modify_field(tcp_tx_d.pending_pad, pending_pad);\

#define GENERATE_TSO_SHARED_D                                                                               \
    modify_field(tso_d.source_lif, source_lif); \
    modify_field(tso_d.source_port, source_port); \
    modify_field(tso_d.dest_port, dest_port); \
    modify_field(tso_d.prr_out, prr_out); \
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
