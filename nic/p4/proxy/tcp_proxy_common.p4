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

#define TXDMA_PARAMS_BASE                                                                             \
rsvd, cosA, cosB, cos_sel, eval_last, host, total, pid\

#define GENERATE_TXDMA_PARAMS_BASE(__struct)                                                           \
    modify_field(__struct.rsvd, rsvd);                                                                 \
    modify_field(__struct.cosA, cosA);                                                                 \
    modify_field(__struct.cosB, cosB);                                                                 \
    modify_field(__struct.cos_sel, cos_sel);                                                           \
    modify_field(__struct.eval_last, eval_last);                                                       \
    modify_field(__struct.host, host);                                                                 \
    modify_field(__struct.total, total);                                                               \
    modify_field(__struct.pid, pid);                                                                   \


#define TX2RX_SHARED_STATE \
        prr_out                         : SEQ_NUMBER_WIDTH      ;\
        snd_nxt                         : SEQ_NUMBER_WIDTH      ;\
        rcv_wup                         : 32                    ;\
        packets_out                     : 16                    ;\
        quick_acks_decr                 : 4                     ;\
        fin_sent                        : 1                     ;\
        rst_sent                        : 1                     ;\
        rto_event                       : 1                     ;\
        pad1_tx2rx                      : 9                     ;\

// offset 0 (TCP_TCB_RX2TX_EXTRA_SND_CWND_OFFSET)
#define RX2TX_SHARED_EXTRA_STATE \
        snd_cwnd                        : 32                    ;\
        rcv_nxt                         : SEQ_NUMBER_WIDTH      ;\
        snd_wnd                         : 16                    ;\
        rcv_wnd                         : 16                    ;\
        rto                             : 16                    ;\
        snd_una                         : SEQ_NUMBER_WIDTH      ;\
        rcv_tsval                       : TS_WIDTH              ;\
        rcv_mss                         : 16                    ;\
        cc_flags                        : 16                    ;\
        t_flags                         : 8                     ;\
        limited_transmit                : 8                     ;\
        state                           : 8                     ;\
        pending_dup_ack_send            : 1                     ;\
        pending_challenge_ack_send      : 1                     ;\
        rx2tx_end_marker                : 6                     ;\
        pad_rx2tx_extra                 : 272                   ;\

#define TCB_RETX_SHARED_STATE \
        retx_snd_una                    : SEQ_NUMBER_WIDTH      ;\
        sesq_ci_addr                    : HBM_ADDRESS_WIDTH     ;\
        gc_base                         : 64                    ;\
        last_ack                        : 32                    ;\
        stretch_ack_cnt                 : 32                    ;\
        tx_ring_pi                      : 16                    ;\
        last_snd_wnd                    : 16                    ;\
        tx_rst_sent                     : 1                     ;\

#define TCB_XMIT_SHARED_STATE \
        snd_nxt                         : SEQ_NUMBER_WIDTH      ;\
        initial_window                  : 32                    ;\
        snd_wscale                      : 8                     ;\
        xmit_cursor_addr                : 40                    ;\
        sesq_tx_ci                      : 16                    ;\
        xmit_offset                     : 16                    ;\
        xmit_len                        : 16                    ;\
        packets_out                     : 16                    ;\
        sacked_out                      : 16                    ;\
        retrans_out                     : 16                    ;\
        lost_out                        : 16                    ;\
        smss                            : 16                    ;\
        limited_transmit                : 8                     ;\
        is_cwnd_limited                 : 8                     ;\
        rto_backoff                     : 8                     ;\
        no_window                       : 1                     ;\

#define TCB_TSO_STATE \
        ip_id                           : 32                    ;\
        source_lif                      : 16                    ;\
        source_port                     : 16                    ;\
        dest_port                       : 16                    ;\
        header_len                      : 16                    ;\
        bytes_sent                      : 16                    ;\
        pkts_sent                       : 8                     ;\
        debug_num_phv_to_pkt            : 8                     ;\
        debug_num_mem_to_pkt            : 8                     ;\
        quick_acks_decr                 : 4                     ;\

#define RETX_SHARED_PARAMS \
retx_snd_una,\
sesq_ci_addr,\
gc_base,\
last_ack,\
stretch_ack_cnt,\
tx_ring_pi,\
last_snd_wnd,\
tx_rst_sent

#define XMIT_SHARED_PARAMS \
snd_nxt, initial_window, snd_wscale,\
xmit_cursor_addr, sesq_tx_ci,\
xmit_offset, xmit_len,\
packets_out, sacked_out, retrans_out, lost_out,\
smss, is_cwnd_limited, limited_transmit, rto_backoff, no_window

#define TSO_PARAMS                                                        \
ip_id, source_lif, source_port, dest_port, header_len,\
bytes_sent,pkts_sent,debug_num_phv_to_pkt, debug_num_mem_to_pkt,\
quick_acks_decr


#define GENERATE_RETX_SHARED_D \
    modify_field(retx_d.retx_snd_una, retx_snd_una); \
    modify_field(retx_d.sesq_ci_addr, sesq_ci_addr); \
    modify_field(retx_d.gc_base, gc_base); \
    modify_field(retx_d.last_ack, last_ack); \
    modify_field(retx_d.stretch_ack_cnt, stretch_ack_cnt); \
    modify_field(retx_d.tx_ring_pi, tx_ring_pi); \
    modify_field(retx_d.last_snd_wnd, last_snd_wnd); \
    modify_field(retx_d.tx_rst_sent, tx_rst_sent); \

#define GENERATE_XMIT_SHARED_D \
    modify_field(xmit_d.snd_nxt, snd_nxt); \
    modify_field(xmit_d.initial_window, initial_window); \
    modify_field(xmit_d.snd_wscale, snd_wscale); \
    modify_field(xmit_d.xmit_cursor_addr, xmit_cursor_addr); \
    modify_field(xmit_d.sesq_tx_ci, sesq_tx_ci); \
    modify_field(xmit_d.xmit_offset, xmit_offset); \
    modify_field(xmit_d.xmit_len, xmit_len); \
    modify_field(xmit_d.packets_out, packets_out); \
    modify_field(xmit_d.sacked_out, sacked_out); \
    modify_field(xmit_d.retrans_out, retrans_out); \
    modify_field(xmit_d.lost_out, lost_out); \
    modify_field(xmit_d.smss, smss); \
    modify_field(xmit_d.is_cwnd_limited, is_cwnd_limited); \
    modify_field(xmit_d.limited_transmit, limited_transmit); \
    modify_field(xmit_d.rto_backoff, rto_backoff); \
    modify_field(xmit_d.no_window, no_window); \

#define GENERATE_TSO_SHARED_D \
    modify_field(tso_d.ip_id, ip_id); \
    modify_field(tso_d.source_lif, source_lif); \
    modify_field(tso_d.source_port, source_port); \
    modify_field(tso_d.dest_port, dest_port); \
    modify_field(tso_d.header_len, header_len); \
    modify_field(tso_d.bytes_sent, bytes_sent);\
    modify_field(tso_d.pkts_sent, pkts_sent);\
    modify_field(tso_d.debug_num_phv_to_pkt, debug_num_phv_to_pkt);\
    modify_field(tso_d.debug_num_mem_to_pkt, debug_num_mem_to_pkt);\
    modify_field(tso_d.quick_acks_decr, quick_acks_decr);\


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
