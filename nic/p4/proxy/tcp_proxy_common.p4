/*****************************************************************************/
/* tcp_proxy_common.p4
/*****************************************************************************/

/******************************************************************************
 * Rx2Tx shared state in PHV
 *****************************************************************************/

#define RX2TX_SHARED_STATE \
     snd_una                : 32; \
     rcv_nxt                : 32; \
     snd_wnd                : 32; \
     snd_wl1                : 32; \
     snd_up                 : 32; \
     write_seq              : 32; \
     tso_seq                : 32; \
     snd_cwnd               : 16; \
     snd_wscale             : 4; \
     rcv_mss_shft           : 4; \
     rcv_mss                : 8; \
     rto                    : 8; \
     ca_state               : 8; \
     pending_challenge_ack_send : 1; \
     pending_ack_send       : 1; \
     pending_sync_mss       : 1; \
     pending_tso_keepalive  : 1; \
     pending_tso_pmtu_probe : 1; \
     pending_tso_data       : 1; \
     pending_tso_probe_data : 1; \
     pending_tso_probe      : 1; \
     pending_ooo_se_recv    : 1; \
     pending_tso_retx       : 1; \
     pending_ft_clear       : 1; \
     pending_ft_reset       : 1; \
     pending_rexmit         : 2; \
     pending                : 3; \
     ack_blocked            : 1; \
     ack_pending            : 3; \
     ato_deadline           : 32; \
     rto_deadline           : 32; \
     retx_head_ts           : 32; \
     srtt_us                : 32; \
     rcv_wnd                : 16;

header_type rx2tx_t {
    fields {
        RX2TX_SHARED_STATE
    }
}

header_type rx2tx_d_t {
    fields {
        pc                  : 8;
        rsvd                : 8;
        cosA                : 4;
        cosB                : 4;
        cos_sel             : 8;
        eval_last           : 8;
        host                : 4;
        total               : 4;
        pid                 : 16;
        p_index0            : 16;
        c_index0            : 16;
        p_index1            : 16;
        c_index1            : 16;
        p_index2            : 16;
        c_index2            : 16;
        p_index3            : 16;
        c_index3            : 16;

        RX2TX_SHARED_STATE
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
