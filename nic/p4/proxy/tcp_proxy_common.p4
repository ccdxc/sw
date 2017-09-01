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

#define TX2RX_SHARED_STATE \
        prr_out                         : SEQ_NUMBER_WIDTH      ;\
        snd_nxt                         : SEQ_NUMBER_WIDTH      ;\
        pad1_tx2rx                      : 8                     ;\
        ecn_flags_tx                    : 8                     ;\
        packets_out                     : 16                    ;\
        pad2_tx2rx                      : 32                    ;

#define RX2TX_SHARED_STATE \
        snd_una                         : SEQ_NUMBER_WIDTH      ;\
        rcv_nxt                         : SEQ_NUMBER_WIDTH      ;\
        snd_wnd                         : SEQ_NUMBER_WIDTH      ;\
        snd_wl1                         : SEQ_NUMBER_WIDTH      ;\
        snd_up                          : SEQ_NUMBER_WIDTH      ;\
        snd_cwnd                        : WINDOW_WIDTH          ;\
        rto_deadline                    : TS_WIDTH              ;\
        pending_ft_clear                : 1                     ;\
        pending_ft_reset                : 1                     ;


#define RX2TX_SHARED_EXTRA_STATE \
        ack_blocked                     : 1                     ;\
        ack_pending                     : 3                     ;\
        snd_wscale                      : 4                     ;\
        rcv_mss_shft                    : 4                     ;\
        rcv_mss                         : 16                    ;\
        rto                             : 8                     ;\
        ca_state                        : 8                     ;\
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
        quick                           : 4                     ;\
        pingpong                        : 1                     ;\
        backoff                         : 4                     ;\
        dsack                           : 1                     ;\
        num_sacks                       : 8                     ;\
        reordering                      : COUNTER32             ;\
        undo_marker                     : SEQ_NUMBER_WIDTH      ;\
        undo_retrans                    : SEQ_NUMBER_WIDTH      ;\
        snd_ssthresh                    : WINDOW_WIDTH          ;\
        loss_cwnd                       : WINDOW_WIDTH          ;\
        write_seq                       : SEQ_NUMBER_WIDTH      ;\
        tso_seq                         : SEQ_NUMBER_WIDTH      ;\
        ecn_flags                       : 8                     ;\
        pending_challenge_ack_send      : 1                     ;\
        pending_ack_send                : 1                     ;\
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

#define TCB_TX_SHARED_STATE \
	retx_ci                         : 16                    ;\
	retx_pi                         : 16                    ;\
        sched_flag                      : 8                     ;\
	retx_snd_una			: SEQ_NUMBER_WIDTH      ;\
	retx_snd_nxt			: SEQ_NUMBER_WIDTH      ;\
	retx_head_desc			: HBM_ADDRESS_WIDTH         ;\
	retx_snd_una_cursor		: HBM_ADDRESS_WIDTH         ;\
	retx_tail_desc			: HBM_ADDRESS_WIDTH         ;\
	retx_snd_nxt_cursor		: HBM_ADDRESS_WIDTH         ;\
	retx_xmit_cursor		: HBM_ADDRESS_WIDTH         ;\
	xmit_cursor_addr		: HBM_ADDRESS_WIDTH         ;\
	rcv_wup				: WINDOW_WIDTH          ;\
        prr_out                         : 32                    ;\
	pending_ack_tx			: 1                     ;\
	pending_delayed_ack_tx		: 1                     ;\
	pending_tso_data		: 1                     ;

#define TX_SHARED_PARAMS                                                                                  \
retx_ci, retx_pi, sched_flag, retx_snd_una, retx_snd_nxt,\
retx_head_desc, retx_snd_una_cursor, retx_tail_desc, retx_snd_nxt_cursor,\
retx_xmit_cursor, xmit_cursor_addr, rcv_wup, prr_out, pending_ack_tx,\
pending_delayed_ack_tx, pending_tso_data


#define GENERATE_TX_SHARED_D                                                                               \
    modify_field(tcp_tx_d.retx_ci, retx_ci); \
    modify_field(tcp_tx_d.retx_pi, retx_pi); \
    modify_field(tcp_tx_d.sched_flag, sched_flag); \
    modify_field(tcp_tx_d.retx_snd_una, retx_snd_una); \
    modify_field(tcp_tx_d.retx_snd_nxt, retx_snd_nxt); \
    modify_field(tcp_tx_d.retx_head_desc, retx_head_desc); \
    modify_field(tcp_tx_d.retx_snd_una_cursor, retx_snd_una_cursor); \
    modify_field(tcp_tx_d.retx_tail_desc, retx_tail_desc); \
    modify_field(tcp_tx_d.retx_snd_nxt_cursor, retx_snd_nxt_cursor); \
    modify_field(tcp_tx_d.retx_xmit_cursor, retx_xmit_cursor); \
    modify_field(tcp_tx_d.xmit_cursor_addr, xmit_cursor_addr); \
    modify_field(tcp_tx_d.rcv_wup, rcv_wup); \
    modify_field(tcp_tx_d.prr_out, prr_out); \
    modify_field(tcp_tx_d.pending_ack_tx, pending_ack_tx); \
    modify_field(tcp_tx_d.pending_delayed_ack_tx, pending_delayed_ack_tx); \
    modify_field(tcp_tx_d.pending_tso_data, pending_tso_data);

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

header_type rx2tx_extra_pad_t {
    fields {
        rx2tx_extra_pad : 6;
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
