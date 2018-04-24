/*
 *  Implements the RX stage of the RxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s2_t0_tcp_rx_k.h"

struct phv_ p;
struct s2_t0_tcp_rx_k_ k;
struct s2_t0_tcp_rx_d d;

%%
    .param          tcp_ack_start
    .param          tcp_rx_read_rnmdr_start
    .param          tcp_rx_read_rnmpr_start
    .param          tcp_rx_l7_read_rnmdr_start
    .align

    /*
     * Global conditional variables
     *
     * c6 = ooo in Rx Queue, skip some stages
     * c7 = Drop packet
     */
tcp_rx_process_start:
#ifdef CAPRI_IGNORE_TIMESTAMP
    add             r4, r0, r0
    add             r6, r0, r0
#endif

    /*
     * Adjust quick based on acks sent in tx pipeline
     */
    tblssub         d.u.tcp_rx_d.quick, k.s1_s2s_quick_acks_decr

    /*
     * Fast Path checks
     */
    sne             c1, d.u.tcp_rx_d.state, TCP_ESTABLISHED
    seq.!c1         c1, k.s1_s2s_fin_sent, 1
    sne             c2, d.u.tcp_rx_d.rcv_nxt, k.to_s2_seq
    slt             c3, k.s1_s2s_snd_nxt, k.s1_s2s_ack_seq

    // disable timestamp checking for now
    //setcf           c4, [c0]
    //slt             c4, r4, d.u.tcp_rx_d.ts_recent

    /*
     * c4 = serq is full
     */
    add             r2, d.u.tcp_rx_d.serq_pidx, 1
    and             r2, r2, CAPRI_SERQ_RING_SLOTS - 1
    seq             c4, r2, k.to_s2_serq_cidx

    /*
     * Header Prediction
     *
     * Checks if hlen and flags are as expected in fast path
     *
     * TODO : check for sender window change as well later
     *
     * TODO : check for hlen difference. The issue with checking for
     * hlen is that DOL and e2e have different hlens (e2e has
     * timestamp and dol does not). Either find a way to configure
     * the CBs accordingly, or add timestamp option to DOL
     */
    and             r1, k.{to_s2_flags}, TCPHDR_HP_FLAG_BITS
    sne             c5, r1, d.u.tcp_rx_d.pred_flags[23:16]

    sne             c6, d.u.tcp_rx_d.ooo_in_rx_q, r0

    bcf             [c1 | c2 | c3 | c4 | c5 | c6], tcp_rx_slow_path

tcp_rx_fast_path:

tcp_store_ts_recent:
    /*
     * tp->rcv_nxt == tp->rcv_wup
     *      tcp_store_ts_recent(tp)
     *
     */
    seq             c1, k.to_s2_rcv_wup, d.u.tcp_rx_d.rcv_nxt
    tblwr.c1        d.u.tcp_rx_d.ts_recent, k.s1_s2s_rcv_tsval

    /*
     * Do we have payload? If not we are done.
     */
    seq             c1, k.s1_s2s_payload_len, 0
    b.c1            flow_rx_process_done
    nop

    phvwri          p.common_phv_write_serq, 1
    tblwr.l         d.u.tcp_rx_d.alloc_descr, 1

    phvwr           p.to_s6_serq_pidx, d.u.tcp_rx_d.serq_pidx
    tblmincri       d.u.tcp_rx_d.serq_pidx, CAPRI_SERQ_RING_SLOTS_SHIFT, 1

tcp_rcv_nxt_update:
    tbladd          d.u.tcp_rx_d.rcv_nxt, k.s1_s2s_payload_len

bytes_rcvd_stats_update_start:
    CAPRI_STATS_INC2(bytes_rcvd, k.s1_s2s_payload_len, d.u.tcp_rx_d.bytes_rcvd, p.to_s7_bytes_rcvd)
bytes_rcvd_stats_update_end:

    /* SCHEDULE_ACK(tp) */
    /* Set the pending txdma in phv for subsequent stage to issue dma
     * commands to update rx2tx shared state and ring the doorbell
     * to schedule the txdma processing for TCP
     */
    phvwrmi         p.common_phv_pending_txdma, TCP_PENDING_TXDMA_ACK_SEND, \
                        TCP_PENDING_TXDMA_ACK_SEND
    /* The bit for actual specific pending processing needed in txdma
     * is set in the rx2tx shared state. In this case we need an
     * ack to be sent
     */
    phvwr           p.rx2tx_pending_ack_send, 1

tcp_event_data_recv:
    /* Initialize the delayed ack engine if first ack
     *
     * if (!tp->fto.ato) {
     *   tcp_incr_quickack(tp) ;
     *   tp->fto.ato = TCP_ATO_MIN ;
     * }
     */
    seq             c1, d.u.tcp_rx_d.ato, r0
    bal.c1          r7, tcp_incr_quickack
    nop

delack_engine_init_done:
    /*
     * else /* if (!tp->fto.ato) * {
     *   int m = tcp_time_stamp - tp->rx.lrcv_time;
     *
     *   if (m <= TCP_ATO_MIN / 2) {
     *       /* The fastest case is the first. *
     *       tp->fto.ato = (tp->fto.ato >> 1) + TCP_ATO_MIN / 2;
     *   } else if (m < tp->fto.ato) {
     *       tp->fto.ato = (tp->fto.ato >> 1) + m;
     *       if (tp->fto.ato > tp->fto.rto)
     *           tp->fto.ato = tp->fto.rto;
     *   } else if (m > tp->fto.rto) {
     *       /* Too long gap. Apparently sender failed to
     *        * restart window, so that we send ACKs quickly.
     *        *
     *       tcp_incr_quickack(tp);
     *   }
     * }
     */
    /* r1 = m */
    sub             r1, r4, d.u.tcp_rx_d.lrcv_time

    /*
     * if (m <= TCP_ATO_MIN / 2) {
     *   tp->fto.ato = (tp->fto.ato >>> 1) + TCP_ATO_MIN/2
     * }
     */
    slt             c1, TCP_ATO_MIN / 2, r1
    srl             r5, d.u.tcp_rx_d.ato, 1
    add             r5, r5, TCP_ATO_MIN / 2
    tblwr.!c1       d.u.tcp_rx_d.ato, r5
    bcf             [!c1], tcp_event_data_rcv_done

    // TODO optimize this
    /* clear c2 */
    setcf           c2, [!c0]
    /*  if (m < tp->fto.ato) */
    slt.c1          c2, r1, d.u.tcp_rx_d.ato
    /* tp->fto.ato = (tp->fto.ato >> 1) + m */
    tbladd.c2       d.u.tcp_rx_d.ato, r1
    /* if (tp->fto.ato > tp->fto.rto) */
    slt.c2          c3, d.u.tcp_rx_d.rto, d.u.tcp_rx_d.ato
    /*     tp->fto.ato = tp->fto.rto */
    tblwr.c3        d.u.tcp_rx_d.ato, d.u.tcp_rx_d.rto
    phvwr.c3        p.s6_t1_s2s_ato, d.u.tcp_rx_d.rto
    /* if (m > tp->fto.rto */
    slt.c2          c4, d.u.tcp_rx_d.rto, r1
    bal.c2          r7, tcp_incr_quickack
    nop
    /* tp->rx.lrcv_time = tcp_time_stamp */

tcp_event_data_rcv_done:
    /*
     * if (tp->rx.ecn_flags & TCP_ECN_OK)
     */
    smeqb           c1, d.u.tcp_rx_d.ecn_flags, TCP_ECN_OK, TCP_ECN_OK
    bcf             [c1], tcp_ecn_check_ce
    nop

    tblwr           d.u.tcp_rx_d.lrcv_time, r4

tcp_ack_snd_check:
    /* r1 = rcv_nxt - rcv_wup */
    sub             r1, d.u.tcp_rx_d.rcv_nxt , k.to_s2_rcv_wup
    /* r2 = rcv_mss */
    /* c1 = ((rcv_nxt - rcv_wup) > rcv_mss) */
    /* c1 = more than one full frame received */
    slt             c1, d.u.tcp_rx_d.rcv_mss, r1

    /* xxx: c2 = (new rcv window >= rcv_wnd) */
    sne             c2, r0, r0 // TODO
    /* c3 = in quick ack mode */
    /* quick && !pingpong */
    sne             c4, d.u.tcp_rx_d.quick, r0
    sne             c5, d.u.tcp_rx_d.pingpong, r0
    setcf           c3, [c4 & !c5]
    /* c4 = we have out of order data */
    //sne             c4, k.to_s3_ooo_datalen, r0 TODO
    sne             c4, r0, r0 // TODO
    setcf           c1, [c1 | c2 | c3 | c4]

    phvwr.!c1        p.common_phv_pending_del_ack_send, 1
    phvwr.c1         p.common_phv_pending_ack_send, 1

slow_path:
flow_rx_process_done:
table_read_setup_next:
    phvwr           p.rx2tx_snd_wnd, d.u.tcp_rx_d.snd_wnd
    phvwr           p.rx2tx_extra_rcv_mss, d.u.tcp_rx_d.rcv_mss
    phvwr           p.rx2tx_rcv_nxt, d.u.tcp_rx_d.rcv_nxt
    phvwr           p.to_s3_flag, d.u.tcp_rx_d.flag
    /*
     * c7 = drop
     */
    bcf             [c7], flow_rx_drop

table_read_RTT:
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN, tcp_ack_start,
                        k.common_phv_qstate_addr, TCP_TCB_RX_OFFSET,
                        TABLE_SIZE_512_BITS)
    /*
     * c6 = OOO in Rx queue, do not allocate buffers
     */
    bcf             [c6], tcp_rx_end

table_read_RNMDR_ALLOC_IDX:
    /*
     * Allocate page and descriptor if alloc_descr is 1.
     * TODO : we can optimize to not allocate page if
     * payload_len is 0
     */
    seq             c3, d.u.tcp_rx_d.alloc_descr, 1
    phvwr.!c3       p.common_phv_l7_proxy_en, 0
    phvwr.!c3       p.common_phv_l7_proxy_type_redirect, 0
    //bcf             [!c3], table_read_RNMPR_ALLOC_IDX
    bcf             [!c3], tcp_rx_end
    CAPRI_NEXT_TABLE_READ_i(1, TABLE_LOCK_DIS, tcp_rx_read_rnmdr_start,
                        RNMDR_ALLOC_IDX, TABLE_SIZE_64_BITS)
table_read_RNMPR_ALLOC_IDX:
    //seq             c3, k.s1_s2s_payload_len, r0
    //bcf             [c3], tcp_rx_end
    CAPRI_NEXT_TABLE_READ_i(2, TABLE_LOCK_DIS, tcp_rx_read_rnmpr_start,
                        RNMPR_ALLOC_IDX, TABLE_SIZE_64_BITS)
table_read_L7_RNDMR_ALLOC_IDX:
    seq             c1, k.common_phv_l7_proxy_en, 1
    b.!c1.e         tcp_rx_end
    smeqb           c1, d.u.tcp_rx_d.parsed_state, \
                            TCP_PARSED_STATE_HANDLE_IN_CPU, \
                            TCP_PARSED_STATE_HANDLE_IN_CPU
    phvwr.c1        p.common_phv_l7_proxy_en, 0
    b.c1            tcp_rx_end
    seq             c2, k.common_phv_l7_proxy_type_redirect, 1
    phvwri.c2       p.app_header_table1_valid, 0
    phvwri.c2       p.common_phv_write_serq, 0
    CAPRI_NEXT_TABLE_READ_i(3, TABLE_LOCK_DIS, tcp_rx_l7_read_rnmdr_start,
                        RNMDR_ALLOC_IDX, TABLE_SIZE_64_BITS)

tcp_rx_end:
    nop.e
    nop



tcp_ecn_check_ce:
    sne             c4, r7, r0

    add             r1, k.to_s2_ip_dsfield, r0
    andi            r1, r1, INET_ECN_MASK

    smeqb           c5, r1, INET_ECN_NOT_ECT, INET_ECN_NOT_ECT
    smeqb           c4, r1, INET_ECN_ECT_1, INET_ECN_ECT_1
    smeqb           c3, r1, INET_ECN_ECT_0, INET_ECN_ECT_0
    smeqb           c2, r1, INET_ECN_CE, INET_ECN_CE

    //.cscase   INET_ECN_NOT_ECT
    setcf           c1, [c0]
    smeqb.c5        c1, d.u.tcp_rx_d.ecn_flags, TCP_ECN_SEEN, TCP_ECN_SEEN
    bal.c1          r7, tcp_enter_quickack_mode
    nop

    //.cscase INET_ECN_ECT_1
    setcf           c1,[c0]
    smeqb.c4        c1, d.u.tcp_rx_d.ca_flags, TCP_CONG_NEEDS_ECN, TCP_CONG_NEEDS_ECN
    addi.c1         r2, r0, CA_EVENT_ECN_NO_CE
    //phvwr.c1        p.common_phv_ca_event, r2
    tblor.c4        d.u.tcp_rx_d.ecn_flags, TCP_ECN_SEEN

    //.cscase INET_ECN_ECT_0
    setcf           c1, [c0]
    smeqb.c3        c1, d.u.tcp_rx_d.ca_flags, TCP_CONG_NEEDS_ECN, TCP_CONG_NEEDS_ECN
    addi.c1         r2, r0, CA_EVENT_ECN_NO_CE
    //phvwr.c1        p.common_phv_ca_event, r2
    tblor.c3        d.u.tcp_rx_d.ecn_flags, TCP_ECN_SEEN

    //.cscase INET_ECN_CE
    setcf           c1, [c0]
    smeqb.c2        c1, d.u.tcp_rx_d.ca_flags, TCP_CONG_NEEDS_ECN, TCP_CONG_NEEDS_ECN
    addi.c1         r2, r0, CA_EVENT_ECN_IS_CE
    //phvwr.c1        p.common_phv_ca_event, r2
    setcf           c1, [c0]
    smeqb.c2        c1, d.u.tcp_rx_d.ecn_flags, TCP_ECN_DEMAND_CWR, TCP_ECN_DEMAND_CWR
    bal.!c1         r7, tcp_enter_quickack_mode
    tblor.!c1       d.u.tcp_rx_d.ecn_flags, TCP_ECN_DEMAND_CWR
    tblor           d.u.tcp_rx_d.ecn_flags, TCP_ECN_SEEN
    //.csend

tcp_ecn_check_ce_done:
    jr.c4           r7
    setcf           c4,[!c0]

tcp_incr_quickack:
    /* unsigned int quickacks = tp->fc.rcv_wnd / (2 * tp->rx_opt.rcv_mss); */

    /* rcv_mss_shft = 1 for 1.5k (rounded to 2k), 3 for 9k (rounded to 8k) */

    add             r2, k.s1_s2s_rcv_mss_shft, RCV_MSS_SHFT_BASE
    sll             r1, r2, k.s1_s2s_window

    /* r1 = quickacks */

    /*
       if (quickacks == 0)
        quickacks = 2;
     *
     */

    seq.c1          c2, r1, r0
    addi.c2         r1, r0, 2
    /*
       if (quickacks > tp->tx.quick)
           /* Maximal number of ACKs sent quickly to accelerate slow-start. *
       #define TCP_MAX_QUICKACKS       16U

       tp->tx.quick = min(quickacks, TCP_MAX_QUICKACKS);
     */
    slt             c2, d.u.tcp_rx_d.quick,r1
    addi            r2, r0, TCP_MAX_QUICKACKS
    slt.c2          c3, r2, r1
    tblwr.c3        d.u.tcp_rx_d.quick, TCP_MAX_QUICKACKS
    tblwr.!c3       d.u.tcp_rx_d.quick, r1

    tblwr           d.u.tcp_rx_d.ato, TCP_ATO_MIN
    jr              r7
    phvwr           p.s6_t1_s2s_ato, TCP_ATO_MIN

#if 0
    // TODO : this needs to move to tx pipeline
/* Restart timer after forward progress on connection.
 * RFC2988 recommends to restart timer to now+rto.
 */
tcp_rearm_rto:

    CAPRI_CLEAR_TABLE0_VALID

    sne             c4, r7, r0
    /* If the retrans timer is currently being used by Fast Open
     * for SYN-ACK retrans purpose, stay put.
     *
      if (tp->rx_opt.fastopen_rsk)
        return;
     */
    sne             c1, d.u.tcp_rx_d.fastopen_rsk, r0
    bcf             [c1 & c4], tcp_rearm_rto_done
    nop
    seq             c1, k.s1_s2s_packets_out, r0
    phvwr.c1        p.rx2tx_pending_ft_clear,1
    phvwr.c1        p.common_phv_pending_txdma, 1
    bcf             [c1], tcp_rearm_rto_done
    tbladd.c1       d.u.tcp_rx_d.rto,  -1
    /* r3 = rto */
    add             r3, d.u.tcp_rx_d.rto, r0
    seq             c2, d.u.tcp_rx_d.pending, ICSK_TIME_EARLY_RETRANS
    seq             c3, d.u.tcp_rx_d.pending, ICSK_TIME_LOSS_PROBE
    bcf             [!c2 & !c3], early_retx_or_tlp
    nop
    /* rto_time_stamp = tp->rx.retx_head_ts + rto */
    add             r1, d.u.tcp_rx_d.retx_head_ts, r3
    /* delta = (s32) (rto_time_stamp - tcp_time_stamp) */
    sub             r2, r1, r6
    /* if (delta > 0) */
    slt             c2, r0, r2
    /*  rto = delta */
    add.c2          r3, r2, r0
early_retx_or_tlp:
    /* tp->fto.rto_deadline = tcp_time_stamp + min(rto, TCP_RTO_MAX) */
    addi            r4, r0, TCP_RTO_MAX
    slt             c1, r3, r4
    add.c1          r5, r3, r6
    tblwr.c1        d.u.tcp_rx_d.rto_deadline, r5
    add.!c1         r5, r4, r6
    tblwr.!c1       d.u.tcp_rx_d.rto_deadline, r5
    phvwr           p.rx2tx_pending_ft_reset,1
    phvwr.c1        p.common_phv_pending_txdma, 1
tcp_rearm_rto_done:
    jr.c4           r7
    add             r7, r0, r0
#endif


tcp_enter_quickack_mode:
    bal             r7, tcp_incr_quickack
    nop
    tblwr           d.u.tcp_rx_d.pingpong, r0
    phvwr           p.common_phv_pingpong, d.u.tcp_rx_d.pingpong
    tblwr           d.u.tcp_rx_d.ato, TCP_ATO_MIN
    phvwr           p.s6_t1_s2s_ato, TCP_ATO_MIN
    sne             c4, r7, r0
    jr.c4           r7
    add             r7, r0, r0

tcp_rx_slow_path:
    seq             c1, k.s1_s2s_payload_len, r0
    tblwr.c1.l      d.u.tcp_rx_d.alloc_descr, 0
    phvwri.c1       p.common_phv_write_serq, 0

    seq             c1, k.common_phv_syn, 1
    tblwr.c1.l      d.u.tcp_rx_d.alloc_descr, 1

    smeqb           c1, d.u.tcp_rx_d.parsed_state, \
                            TCP_PARSED_STATE_HANDLE_IN_CPU, \
                            TCP_PARSED_STATE_HANDLE_IN_CPU
    phvwri.c1       p.common_phv_write_arq, 1
    tblwr.c1.l      d.u.tcp_rx_d.alloc_descr, 1

    seq             c2, d.u.tcp_rx_d.state, TCP_LISTEN
    phvwri.c2       p.common_phv_write_tcp_app_hdr,1
    phvwr.c2        p.cpu_hdr2_tcp_seqNo, k.{to_s2_seq}.wx
    phvwr.c2        p.cpu_hdr2_tcp_AckNo, k.{s1_s2s_ack_seq}.wx
    phvwr.c2        p.cpu_hdr2_tcp_flags, k.to_s2_flags

    bcf             [c1], flow_rx_process_done
    setcf           c7, [!c0]
    /* Setup the to-stage/stage-to-stage variables */

    /* if (cp->seq != tp->rx.rcv_nxt) { */
    /* if pkt->seq > rcv_nxt, do ooo (SACK) processing */
    slt             c7, d.u.tcp_rx_d.rcv_nxt, k.to_s2_seq
    bcf             [c7], ooo_received
    /*
     * if pkt->seq < rcv_nxt, its a retransmission, drop the packet,
     * but send ack. We don't partially accept unacknowledged bytes yet,
     * the entire frame is dropped.
     */
    slt             c1, k.to_s2_seq, d.u.tcp_rx_d.rcv_nxt
    phvwrmi.c1      p.common_phv_pending_txdma, TCP_PENDING_TXDMA_ACK_SEND, \
                        TCP_PENDING_TXDMA_ACK_SEND
    phvwr.c1        p.rx2tx_pending_ack_send, 1
    sne             c2, k.s1_s2s_payload_len, 0
    setcf           c3, [c1 & c2]
    phvwr.c3        p.common_phv_skip_pkt_dma, 1
    phvwr.c3        p.common_phv_write_serq, 0
    // set c6 so that descriptors are not allocated
    setcf.c3        c6, [c0]
    bcf             [c3], flow_rx_process_done

    /*
     * If we have received data and serq is full,
     * drop the frame
     */
    sne             c1, k.s1_s2s_payload_len, r0
    add             r2, d.u.tcp_rx_d.serq_pidx, 1
    and             r2, r2, CAPRI_SERQ_RING_SLOTS - 1
    seq             c2, r2, k.to_s2_serq_cidx
    setcf           c7, [c1 & c2]
    phvwri.c7       p.p4_intr_global_drop, 1
    b.c7            flow_rx_process_done

    /*
     * Handle close (fin sent in tx pipeline)
     *
     * State (EST) --> FIN_WAIT_1
     * State (CLOSE_WAIT) --> LAST_ACK
     */
    setcf           c2, [!c0]
    seq             c1, k.s1_s2s_fin_sent, 1

    seq.c1          c2, d.u.tcp_rx_d.state, TCP_ESTABLISHED
    tblwr.c2        d.u.tcp_rx_d.state, TCP_FIN_WAIT1

    seq.c1          c2, d.u.tcp_rx_d.state, TCP_CLOSE_WAIT
    tblwr.c2        d.u.tcp_rx_d.state, TCP_LAST_ACK

    /*
     * EST (recv: FIN) --> CLOSE_WAIT, increment sequence number
     * FIN_WAIT_1 (recv: FIN) --> CLOSING, increment sequence number
     * FIN_WAIT_2 (recv: FIN) --> TIME_WAIT, increment sequence number
     * 
     */
    seq             c1, k.common_phv_fin, 1
    b.!c1           tcp_rx_slow_path_post_fin_handling

    seq             c1, d.u.tcp_rx_d.state, TCP_ESTABLISHED
    tblwr.c1        d.u.tcp_rx_d.state, TCP_CLOSE_WAIT

    seq             c2, d.u.tcp_rx_d.state, TCP_FIN_WAIT1
    tblwr.c2        d.u.tcp_rx_d.state, TCP_CLOSING

    // TODO : hand off to arm or start time_wait timer
    seq             c3, d.u.tcp_rx_d.state, TCP_FIN_WAIT2
    tblwr.c3        d.u.tcp_rx_d.state, TCP_TIME_WAIT

    /*
     * TODO : Until TLS code is ready, do not send FIN packets to TLS
     * unless bypass_barco is set
     */
    smeqb           c4, k.common_phv_debug_dol, TCP_DDOL_BYPASS_BARCO, TCP_DDOL_BYPASS_BARCO
    b.!c4           tcp_rx_slow_path_post_fin_handling

    setcf           c1, [c1 | c2 | c3]
    b.!c1           tcp_rx_slow_path_post_fin_handling
    tbladd.c1       d.u.tcp_rx_d.rcv_nxt, 1
    tblwr.c1.l      d.u.tcp_rx_d.alloc_descr, 1
    phvwri.c1       p.common_phv_write_serq, 1
    phvwr.c1        p.to_s6_serq_pidx, d.u.tcp_rx_d.serq_pidx
    tblmincri.c1    d.u.tcp_rx_d.serq_pidx, CAPRI_SERQ_RING_SLOTS_SHIFT, 1
    phvwr.c1        p.rx2tx_pending_ack_send, 1
    phvwrmi.c1      p.common_phv_pending_txdma, TCP_PENDING_TXDMA_ACK_SEND, \
                        TCP_PENDING_TXDMA_ACK_SEND

    phvwr           p.rx2tx_state, d.u.tcp_rx_d.state

tcp_rx_slow_path_post_fin_handling:
    /*   /* If PAWS failed, check it more carefully in slow path */
    /* if ((s32)(tp->rx_opt.rcv_tsval - tp->rx_opt.ts_recent) < 0) {

           /* DO NOT update ts_recent here, if checksum fails
        * and timestamp was corrupted part, it will result
        * in a hung connection since we will drop all
                * future packets due to the PAWS test.
                *
            goto slow_path  ;
    }
    */
    sub             r1, r4, d.u.tcp_rx_d.ts_recent
    slt             c1, r1, r0
    bcf             [c1],slow_path
    nop

    seq             c1, k.s1_s2s_payload_len, r0
    b.c1            flow_rx_process_done
    nop

tcp_queue_rcv:
    /*
     * c6 is global conditional variable indicating OOO pkt in Rx Q
     */
    bcf             [!c6], tcp_rcv_nxt_update

    /*
     * We have prior OOO packets, see if we have more data we can send
     *
     * r1 = end_seq - rcv_nxt
     *
     * r0 (0) is the rightmost (lowest) bit set
     * r2 is the leftmost (highest) bit set
     */
    add             r1, k.to_s2_seq, k.s1_s2s_payload_len
    sub             r1, r1, d.u.tcp_rx_d.rcv_nxt
    srl             r2, r1, TCP_OOO_CELL_SIZE_SHIFT
    and             r3, r1, TCP_OOO_CELL_SIZE_MASK
    seq             c3, r3, r0
    sub             r2, r2, 1
    sub.!c3         r2, r2, 1

    fsetv           r4, d.u.tcp_rx_d.ooo_rcv_bitmap, r2, r0
    tblwr           d.u.tcp_rx_d.ooo_rcv_bitmap, r4
    /*
     * Get number of consecutive 1s in ooo_rcv_bitmap
     * Multiply that by TCP_OOO_CELL_SIZE to get DMA len
     * Add it to rcv_nxt to get new rcv_nxt
     *
     * r1 = end_seq - rcv_nxt
     * r3 = len based on consecutive 1s
     * Set r1 (bytes_rcvd) based on max(r1, r3)
     */
    ffcv            r3, d.u.tcp_rx_d.ooo_rcv_bitmap, r0
    sll             r3, r3, TCP_OOO_CELL_SIZE_SHIFT
    slt             c1, r1, r3
    add.c1          r1, r3, r0
    phvwr           p.to_s6_payload_len, r1
    tbladd          d.u.tcp_rx_d.rcv_nxt, r1
    phvwri          p.common_phv_write_serq, 1
    phvwr           p.common_phv_ooo_in_rx_q, 1

    /*
     * Test instructions just to test 2 remaining sack instructions.
     * Remove later. TODO
     */
     // START OF TEST
     ffsv           r3, d.u.tcp_rx_d.ooo_rcv_bitmap, r0
     seq            c1, r3, r0
     b.!c1          flow_rx_drop

     fclrv          r3, d.u.tcp_rx_d.ooo_rcv_bitmap, r2, r0
     tblwr          d.u.tcp_rx_d.ooo_rcv_bitmap, r3

     ffsv           r3, d.u.tcp_rx_d.ooo_rcv_bitmap, r0
     tblwr          d.u.tcp_rx_d.ooo_rcv_bitmap, r0
     sub            r3, r3, 1
     seq            c1, r3, r2
     b.!c1          flow_rx_drop

     ffsv           r3, d.u.tcp_rx_d.ooo_rcv_bitmap, r0
     seq            c1, r3, -1
     b.!c1          flow_rx_drop
     // END OF TEST

ooo_bytes_rcvd_stats_update_start:
    CAPRI_STATS_INC2(ooo_bytes_rcvd, r1, d.u.tcp_rx_d.bytes_rcvd, p.to_s7_bytes_rcvd)
ooo_bytes_rcvd_stats_update_end:
    b               bytes_rcvd_stats_update_end
    setcf           c6, [c0]

flow_rx_drop:
    CAPRI_CLEAR_TABLE0_VALID
    nop.e
    nop

ooo_received:
    /*
     * We need to send immediate ack (dupack)
     */
    phvwrmi         p.common_phv_pending_txdma, TCP_PENDING_TXDMA_ACK_SEND, \
                        TCP_PENDING_TXDMA_ACK_SEND
    phvwr           p.rx2tx_pending_ack_send, 1
    /*
     * We can receive upto window of rcv_nxt to rcv_nxt + CELL_SIZE *
     * NUM_CELLS into the future.  If we receive an end sequence number beyond
     * that, drop the packet Else set the ooo_rcv_bitmap accordingly
     * r1 = seq - rcv_nxt
     * r2 = end_seq - rcv_nxt
     * r3 = max end_seq
     *
     * c7 = drop packet
     */
    sub             r1, k.to_s2_seq, d.u.tcp_rx_d.rcv_nxt
    add             r2, r1, k.s1_s2s_payload_len
    add             r3, d.u.tcp_rx_d.rcv_nxt, TCP_OOO_NUM_CELLS * TCP_OOO_CELL_SIZE
    add             r4, k.to_s2_seq, k.s1_s2s_payload_len
    slt             c7, r3, r4
    phvwri.c7       p.p4_intr_global_drop, 1
    bcf             [c7], flow_rx_process_done

    /*
     * r3 is the rightmost (lowest) bit set
     */
    srl             r3, r1, TCP_OOO_CELL_SIZE_SHIFT
    and             r5, r1, TCP_OOO_CELL_SIZE_MASK
    seq             c3, r5, r0
    add.!c3         r3, r3, 1

    /*
     * r4 is the leftmost (highest) bit set
     */
    srl             r4, r2, TCP_OOO_CELL_SIZE_SHIFT
    and             r5, r2, TCP_OOO_CELL_SIZE_MASK
    seq             c3, r5, r0
    sub.!c3         r4, r4, 1

    fsetv           r5, d.u.tcp_rx_d.ooo_rcv_bitmap, r4, r3
    tblwr           d.u.tcp_rx_d.ooo_rcv_bitmap, r5
    phvwr           p.to_s6_ooo_offset, r1
    tblwr           d.u.tcp_rx_d.ooo_in_rx_q, 1
    tblwr.l         d.u.tcp_rx_d.alloc_descr, 1
    b               flow_rx_process_done
    phvwr           p.common_phv_ooo_rcv, 1
