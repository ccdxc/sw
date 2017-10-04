/*
 *  Implements the RX stage of the RxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp_common.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct phv_ p;
struct tcp_rx_tcp_rx_k k;
struct tcp_rx_tcp_rx_d d;

%%
    .param          tcp_rx_rtt_stage2_start
    .param          tcp_rx_read_rnmdr_stage2_start
    .param          tcp_rx_read_rnmpr_stage2_start
    .param          tcp_rx_sack_stage2_start
    .align
tcp_rx_process_stage1_start:
    CAPRI_SET_DEBUG_STAGE0_3(p.s6_s2s_debug_stage0_3_thread, CAPRI_MPU_STAGE_1, CAPRI_MPU_TABLE_0)
    add         r7,r0, r0
    phvwr       p.common_phv_debug_dol, d.u.tcp_rx_d.debug_dol
    /* r4 is loaded at the beginning of the stage with current timestamp value */
    add         r6, r4,r0
    /* Check if the state is TCP_ESTABLISHED. If not packet is given to CPU */
    addi        r1, r0, TCP_ESTABLISHED
    seq         c1, d.u.tcp_rx_d.state, r1
    phvwri.!c1  p.common_phv_write_arq, 1
    bcf         [!c1], flow_rx_process_done
    nop
    /* Setup the to-stage/stage-to-stage variables */
    phvwr       p.common_phv_snd_una, d.u.tcp_rx_d.snd_una
    /* if (cp->seq != tp->rx.rcv_nxt) { */
    sne         c7, k.to_s1_seq, d.u.tcp_rx_d.rcv_nxt
    phvwri.c7   p.common_phv_ooo_rcv, 1
    bcf         [c7], flow_rx_process_done
    nop

    /*   if (!(before(cp->ack_seq, tp->tx.snd_nxt))) { */
    slt         c1, k.to_s1_ack_seq, k.to_s1_snd_nxt
    bcf         [!c1], slow_path
    nop

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
    // XXX: VS - rcv_tsval should come from k
    sub         r1, d.u.tcp_rx_d.rcv_tsval, d.u.tcp_rx_d.ts_recent
    slt         c1, r1, r0
    bcf         [c1],slow_path
    nop

    /* tcp_store_ts_recent(tp) */
    // XXX: VS - rcv_tsval should come from k
    tblwr       d.u.tcp_rx_d.ts_recent, d.u.tcp_rx_d.rcv_tsval
//  tblwr       d.ts_recent_tstamp, r6
    add         r6,r6,r0

    /* r2 contains tcp_data_len */
    sub         r2, k.s1_s2s_end_seq, k.to_s1_seq
    seq         c1, r2, r0
    phvwr.c1    p.common_phv_process_ack_flag, r0
    bcf         [c1],tcp_ack
    nop

tcp_queue_rcv:
#if MODEL_BUG_FIX   
    phvwri      p.common_phv_write_serq, 1
#endif
    
tcp_rcv_nxt_update:
    sub         r1, k.s1_s2s_end_seq, d.u.tcp_rx_d.rcv_nxt
    tblwr       d.u.tcp_rx_d.rcv_nxt, k.s1_s2s_end_seq
    phvwr       p.rx2tx_rcv_nxt, k.s1_s2s_end_seq

bytes_rcvd_stats_update_start:
        CAPRI_STATS_INC(bytes_rcvd, 16, r1, d.u.tcp_rx_d.bytes_rcvd)
bytes_rcvd_stats_update:
        CAPRI_STATS_INC_UPDATE(r1, d.u.tcp_rx_d.bytes_rcvd, p.to_s7_bytes_rcvd)
bytes_rcvd_stats_update_end:
    
tcp_event_data_recv:
    /* SCHEDULE_ACK(tp) */
    /* Set the pending txdma in phv for subsequent stage to issue dma
     * commands to update rx2tx shared state and ring the doorbell
     * to schedule the txdma processing for TCP
     */
    phvwr       p.common_phv_pending_txdma, 1
    /* The bit for actual specific pending processing needed in txdma
     * is set in the rx2tx shared state. In this case we need an
     * ack to be sent
     */
    phvwr       p.rx2tx_extra_pending_ack_send, 1
    /* Initialized the delayed ack engine if first ack */
    /*
      if (!tp->fto.ato) {
        tcp_incr_quickack(tp) ;
        tp->fto.ato = TCP_ATO_MIN ;                                                                                    }
     *
     */
    seq         c1, d.u.tcp_rx_d.ato, r0
    bcf         [!c1], delack_engine_init_done
    nop
tcp_incr_quickack:
    /* unsigned int quickacks = tp->fc.rcv_wnd / (2 * tp->rx_opt.rcv_mss); */

    /* rcv_mss_shft = 1 for 1.5k (rounded to 2k), 3 for 9k (rounded to 8k) */

    add         r2, k.s1_s2s_rcv_mss_shft, r0
    addi        r2, r2, RCV_MSS_SHFT_BASE
    add         r1, r0, k.s1_s2s_window
    sllv        r1, r1, r2

    /* r1 = quickacks */

    /*
       if (quickacks == 0)
        quickacks = 2;
     *
     */

    seq.c1      c2, r1, r0
    addi.c2     r1, r0, 2
    /* 
       if (quickacks > tp->tx.quick)
           /* Maximal number of ACKs sent quickly to accelerate slow-start. *
       #define TCP_MAX_QUICKACKS       16U

       tp->tx.quick = min(quickacks, TCP_MAX_QUICKACKS);
     */
    slt         c2, d.u.tcp_rx_d.quick,r1
    addi        r2, r0, TCP_MAX_QUICKACKS
    slt.c2      c3, r2, r1
    tblwr.c3    d.u.tcp_rx_d.quick, TCP_MAX_QUICKACKS
    tblwr.!c3   d.u.tcp_rx_d.quick, r1
#if MODEL_BUG_FIX
    phvwr       p.common_phv_quick, d.u.tcp_rx_d.quick
#endif
    sne         c4, r7, r0
    jr.c4       r7
    add         r7, r0, r0
    
    tblwr       d.u.tcp_rx_d.ato, TCP_ATO_MIN


delack_engine_init_done:
    /*
       else /* if (!tp->fto.ato) * {
        int m = tcp_time_stamp - tp->rx.lrcv_time;

        if (m <= TCP_ATO_MIN / 2) {
            /* The fastest case is the first. *
            tp->fto.ato = (tp->fto.ato >> 1) + TCP_ATO_MIN / 2;
        } else if (m < tp->fto.ato) {
            tp->fto.ato = (tp->fto.ato >> 1) + m;
            if (tp->fto.ato > tp->fto.rto)
                tp->fto.ato = tp->fto.rto;
        } else if (m > tp->fto.rto) {
            /* Too long gap. Apparently sender failed to
             * restart window, so that we send ACKs quickly.
             *
            tcp_incr_quickack(tp);
        }
      }
     */
    /* r1 = m */
    sub         r1, r6, d.u.tcp_rx_d.lrcv_time
    /* r2 = TCP_ATO_MIN / 2 */
    addi        r2, r0, TCP_ATO_MIN
    sll         r2, r2, 1
    slt         c1, r2, r1
    /* tp->fto.ato = tp->fto.ato >> 1 */
    add         r5, d.u.tcp_rx_d.ato, r0
    srl         r5, r5, 1
    tblwr       d.u.tcp_rx_d.ato, r5
    /* tp->fto.ato = tp->fto.ato + TCP_ATO_MIN/2 */
    tbladd.!c1  d.u.tcp_rx_d.ato, r2
    /* clear c2 */
    setcf       c2, [!c0]
    /*  if (m < tp->fto.ato) */
    slt.c1      c2, r1, d.u.tcp_rx_d.ato
    /* tp->fto.ato = (tp->fto.ato >> 1) + m */
    tbladd.c2   d.u.tcp_rx_d.ato, r1
    /* if (tp->fto.ato > tp->fto.rto) */
    slt.c2      c3, d.u.tcp_rx_d.rto, d.u.tcp_rx_d.ato
    /*     tp->fto.ato = tp->fto.rto */
    tblwr.c3    d.u.tcp_rx_d.ato, d.u.tcp_rx_d.rto
    /* if (m > tp->fto.rto */
    slt.c2      c4, d.u.tcp_rx_d.rto, r1
    bal.c2      r7, tcp_incr_quickack
    nop
    /* tp->rx.lrcv_time = tcp_time_stamp */
    tblwr       d.u.tcp_rx_d.lrcv_time, r6

    /* tcp_ecn_check_ce (tp, cp) */
    bal         r7, tcp_ecn_check_ce
    nop
    
tcp_event_data_rcv_done:
    /* c1 = (ack_seq == snd_una) */
    seq         c1, k.to_s1_ack_seq, d.u.tcp_rx_d.snd_una
    /* clear process_ack_flag if ack_seq != snd_una */
    phvwr.!c1   p.common_phv_process_ack_flag, r0
    bcf         [c1], flow_rx_process_done
    nop
    /* Fall thru to tcp_ack if ack_seq != snd_una */

    
tcp_ack:
    /* If the ack is older than previous acks
     * then we can probably ignore it.
         *
    if (before(ack, prior_snd_una)) {
        /* RFC 5961 5.2 .Blind Data Injection Attack..Mitigation *
        if (before(ack, prior_snd_una - tp->rx.max_window)) {
            tp->pending.challenge_ack_send = 1;
            return -1;
        }
        goto old_ack;
    }
    *
    */
    slt         c1,k.to_s1_ack_seq, d.u.tcp_rx_d.snd_una
    sub.c1      r1, d.u.tcp_rx_d.snd_una, d.u.tcp_rx_d.max_window
    slt.c1      c2, k.to_s1_ack_seq, r1
    phvwr.c2    p.rx2tx_extra_pending_challenge_ack_send, 1
    bcf         [c2], flow_rx_process_done
    nop
    b.c1        old_ack
    nop

    /* If the ack includes data we haven't sent yet, discard
     * this segment (RFC793 Section 3.9).
     *
    if (after(ack, tp->tx.snd_nxt))
        goto invalid_ack;
     *
     */
    slt         c1, k.to_s1_snd_nxt, k.to_s1_ack_seq
    bcf         [c1],invalid_ack
    nop

    /*
     *
    if (tp->pending.pending == ICSK_TIME_EARLY_RETRANS ||
        tp->pending.pending == ICSK_TIME_LOSS_PROBE)
       tcp_rearm_rto(tp);
     *
     */
    sne         c1, d.u.tcp_rx_d.pending, ICSK_TIME_EARLY_RETRANS
    sne         c2, d.u.tcp_rx_d.pending, ICSK_TIME_LOSS_PROBE
    bcf         [c1 & c2], no_rearm_rto
    nop
    bal         r7, tcp_rearm_rto
    nop
no_rearm_rto:   
    /*
     *
    if (after(ack, prior_snd_una)) {
       md->process_ack_flag |= FLAG_SND_UNA_ADVANCED;
        }
     *
     */
    slt         c1, d.u.tcp_rx_d.snd_una, k.to_s1_ack_seq
    add         r5, k.common_phv_process_ack_flag, r0
    ori.c1      r5, r5, FLAG_SND_UNA_ADVANCED
    phvwr.c1    p.common_phv_process_ack_flag, r5

    /* ts_recent update must be made after we are sure that the packet
     * is in window.
     *
    if (md->process_ack_flag & FLAG_UPDATE_TS_RECENT)
        tcp_replace_ts_recent(tp, cp->seq);
     *
     */
    add         r5, k.common_phv_process_ack_flag, r0
    smeqh       c2, r5 , FLAG_UPDATE_TS_RECENT, FLAG_UPDATE_TS_RECENT
    bal.c2      r7, tcp_replace_ts_recent
    nop
    /*
     *
      if (!(md->process_ack_flag & FLAG_SLOWPATH) && after(ack, prior_snd_una)) {
        /* Window is constant, pure forward advance.
         * No more checks are required.
         * Note, we use the fact that SND.UNA>=SND.WL2.
         *
        tcp_update_wl(tp, ack_seq);
        tcp_snd_una_update(tp);
        md->process_ack_flag |= FLAG_WIN_UPDATE;

        tcp_in_ack_event(tp, CA_ACK_WIN_UPDATE);
      }
     *
     */
    add         r5, k.common_phv_process_ack_flag, r0
    smneh       c2, r5, FLAG_SLOWPATH, FLAG_SLOWPATH
tcp_update_wl:
    tblwr.c2    d.u.tcp_rx_d.snd_wl1, k.to_s1_ack_seq
tcp_snd_una_update:
    /* Increment bytes acked by the delta between ack_seq and snd_una */
    sub.c2      r1, k.to_s1_ack_seq, d.u.tcp_rx_d.snd_una
    tbladd.c2   d.u.tcp_rx_d.bytes_acked, r1
    /* Update snd_una */
    tblwr.c2    d.u.tcp_rx_d.snd_una, k.to_s1_ack_seq
    phvwr.c2    p.rx2tx_snd_una, k.to_s1_ack_seq

    add.c2      r5, k.common_phv_process_ack_flag, r0
    ori.c2      r5, r5, FLAG_WIN_UPDATE
    phvwr.c2    p.common_phv_process_ack_flag, r5
    /* ack_ev_flags = CA_ACK_SLOWPATH */
    ori.c2      r1, r0, CA_ACK_SLOWPATH
    bcf         [c2], tcp_in_ack_event
    nop
    /* else {
        u32 ack_ev_flags = CA_ACK_SLOWPATH;

        if (ack_seq != cp->end_seq)
        md->process_ack_flag |= FLAG_DATA;
    
        md->process_ack_flag |= tcp_ack_update_window(tp, cp, ack, ack_seq);

        if (tcp_ecn_rcv_ecn_echo(tp, cp)) {
        md->process_ack_flag |= FLAG_ECE;
        ack_ev_flags |= CA_ACK_ECE;
        }

        if (md->process_ack_flag & FLAG_WIN_UPDATE)
        ack_ev_flags |= CA_ACK_WIN_UPDATE;

        tcp_in_ack_event(tp, ack_ev_flags);

     */
    /* r1 is ack_ev_flags */
    /* u32 ack_ev_flags = CA_ACK_SLOWPATH; */
    ori         r1, r0, CA_ACK_SLOWPATH
    /* if (ack_seq != cp->end_seq) */
    sne         c3, k.to_s1_seq, k.s1_s2s_end_seq
    /*     md->process_ack_flag |= FLAG_DATA; */
    add         r5, k.common_phv_process_ack_flag, r0
    ori         r5, r5, FLAG_DATA
    phvwr.c3    p.common_phv_process_ack_flag, r5
    /* Update our send window.
     *
     * Window update algorithm, described in RFC793/RFC1122 (used in linux-2.2
     * and in FreeBSD. NetBSD's one is even worse.) is wrong.
     */
tcp_ack_update_window:
    /* r2 contains nwin */
    /* nwin = cp->window */
    add         r2, k.s1_s2s_window,0
    /* if (!cp->syn) */
    seq         c1, k.common_phv_syn, r0
    /*     nwin <<= tp->rx_opt.snd_wscale; */
    add.c1      r5, d.u.tcp_rx_d.snd_wscale, r0
    sllv.c1     r2, r2, r5
tcp_may_update_window:
    /* after(ack, snd_una) */
    slt         c1, k.to_s1_ack_seq, d.u.tcp_rx_d.snd_una
    /* after(ack_seq, snd_wl1) */
    slt         c2, k.to_s1_seq, d.u.tcp_rx_d.snd_wl1
    /* ack_seq == snd_wl1 */
    slt         c3, k.to_s1_seq, d.u.tcp_rx_d.snd_wl1
    /* nwin > snd_wnd */
    slt         c4, d.u.tcp_rx_d.snd_wnd, r2
    setcf       c5, [!c3 | !c4]
    setcf       c5, [!c1 & !c2 & c5]
    bcf         [c5], tcp_update_window_bypass
    nop

tcp_update_window:
    /* flag |= FLAG_WIN_UPDATE */
    add         r5, k.common_phv_process_ack_flag, r0
    ori         r5, r5, FLAG_WIN_UPDATE
    phvwr       p.common_phv_process_ack_flag, r5
    /* ack_ev_flags |= CA_ACK_WIN_UPDATE */
    ori         r1, r1, CA_ACK_WIN_UPDATE
    /* tcp_update_wl */
    tblwr       d.u.tcp_rx_d.snd_wl1, k.to_s1_seq
    /*
        if (tp->tx.snd_wnd != nwin) {
        tp->tx.snd_wnd = nwin;

        /* Note, it is the only place, where
         * fast path is recovered for sending TCP.
         *
        tp->rx.pred_flags = 0;

                if (nwin > tp->rx.max_window) {
                tp->rx.max_window = nwin;
            tcp_sync_mss(tp, tp->rx_opt.pmtu);
                }
            }
     *
     */

    /* if (tp->tx.snd_wnd != nwin) { */
    sne         c1,d.u.tcp_rx_d.snd_wnd, r2
    /*   tp->tx.snd_wnd = nwin; */

    tblwr.c1    d.u.tcp_rx_d.snd_wnd, r2
    /*   tp->rx.pred_flags = 0; */
    tblwr.c1    d.u.tcp_rx_d.pred_flags, 0
        /*   if (nwin > tp->rx.max_window) { */
    slt.c1      c2, d.u.tcp_rx_d.max_window, r2
        /*        tp->rx.max_window = nwin; */
    tblwr.c2    d.u.tcp_rx_d.max_window, r2
    /*        tcp_sync_mss(tp, tp->rx_opt.pmtu);
     *        tcp_sync_mss will be triggered in tx stage based on pending bit
     */
    phvwr.c2    p.common_phv_pending_sync_mss, 1
    

tcp_update_window_bypass:
    /* tcp_snd_una_update */
    /* Increment bytes acked by the delta between ack_seq and snd_una */
    sub         r3, k.to_s1_ack_seq, d.u.tcp_rx_d.snd_una
    tbladd      d.u.tcp_rx_d.bytes_acked, r3
    /* Update snd_una */
    tblwr       d.u.tcp_rx_d.snd_una, k.to_s1_ack_seq
    phvwr.c2    p.rx2tx_snd_una, k.to_s1_ack_seq

tcp_ecn_rcv_ecn_echo:
    /* ecn_flags & TCP_ECN_OK */
    smeqb       c3, d.u.tcp_rx_d.ecn_flags, TCP_ECN_OK, TCP_ECN_OK
    /* c4 = !cp->ece */
    seq         c4, k.common_phv_ece, r0
    /* c5 = !cp->syn */
    seq         c5, k.common_phv_syn, r0
    bcf         [c4 | !c5 | !c3], tcp_ece_flag_set_bypass
    nop
    /* md->process_ack_flag |= FLAG_ECE */
    add         r5, k.common_phv_process_ack_flag, r0
    ori         r5, r5, FLAG_ECE
    phvwr       p.common_phv_process_ack_flag, r5
    /* ack_ev_flags |= CA_ACK_ECE */
    ori         r1, r1, CA_ACK_ECE
    /* Fall thru to tcp_in_ack_event with ack_ev_flags in r1 */
tcp_ece_flag_set_bypass:    
tcp_in_ack_event:   

    /* We passed data and got it acked, remove any soft error
     * log. Something worked...
     *
     * tp->rx.rcv_tstamp = tcp_time_stamp;
     * if (!prior_packets)
     *     goto no_queue;
     */
     tblwr      d.u.tcp_rx_d.rcv_tstamp, r6
     seq        c3, k.s1_s2s_packets_out, r0
     bcf        [!c3], no_queue
     nop

tcp_ecn_check_ce:
    sne         c4, r7, r0

    /*   if (tp->rx.ecn_flags & TCP_ECN_OK) */
    smeqb       c1, d.u.tcp_rx_d.ecn_flags, TCP_ECN_OK, TCP_ECN_OK
    bcf         [!c1], tcp_ecn_check_ce_done
    nop
    add         r1, k.s1_s2s_ip_dsfield, r0
    andi        r1, r1, INET_ECN_MASK

    smeqb       c5, r1, INET_ECN_NOT_ECT, INET_ECN_NOT_ECT
    smeqb       c4, r1, INET_ECN_ECT_1, INET_ECN_ECT_1
    smeqb       c3, r1, INET_ECN_ECT_0, INET_ECN_ECT_0
    smeqb       c2, r1, INET_ECN_CE, INET_ECN_CE

    //.cscase   INET_ECN_NOT_ECT
    setcf       c1, [c0]
    smeqb.c5    c1, d.u.tcp_rx_d.ecn_flags, TCP_ECN_SEEN, TCP_ECN_SEEN
    bal.c1      r7, tcp_enter_quickack_mode
    nop

    //.cscase INET_ECN_ECT_1
    setcf       c1,[c0]
    smeqb.c4    c1, d.u.tcp_rx_d.ca_flags, TCP_CONG_NEEDS_ECN, TCP_CONG_NEEDS_ECN
    addi.c1     r2, r0, CA_EVENT_ECN_NO_CE
    phvwr.c1    p.common_phv_ca_event, r2
    tblor.c4    d.u.tcp_rx_d.ecn_flags, TCP_ECN_SEEN

    //.cscase INET_ECN_ECT_0
    setcf       c1, [c0]
    smeqb.c3    c1, d.u.tcp_rx_d.ca_flags, TCP_CONG_NEEDS_ECN, TCP_CONG_NEEDS_ECN
    addi.c1     r2, r0, CA_EVENT_ECN_NO_CE
    phvwr.c1    p.common_phv_ca_event, r2
    tblor.c3    d.u.tcp_rx_d.ecn_flags, TCP_ECN_SEEN

    //.cscase INET_ECN_CE
    setcf       c1, [c0]
    smeqb.c2    c1, d.u.tcp_rx_d.ca_flags, TCP_CONG_NEEDS_ECN, TCP_CONG_NEEDS_ECN
    addi.c1     r2, r0, CA_EVENT_ECN_IS_CE
    phvwr.c1    p.common_phv_ca_event, r2
    setcf       c1, [c0]
    smeqb.c2    c1, d.u.tcp_rx_d.ecn_flags, TCP_ECN_DEMAND_CWR, TCP_ECN_DEMAND_CWR
    bal.!c1     r7, tcp_enter_quickack_mode
    tblor.!c1   d.u.tcp_rx_d.ecn_flags, TCP_ECN_DEMAND_CWR
    tblor       d.u.tcp_rx_d.ecn_flags, TCP_ECN_SEEN
    //.csend
    
tcp_ecn_check_ce_done:  
    jr.c4       r7
    setcf       c4,[!c0]
     
/* Restart timer after forward progress on connection.
 * RFC2988 recommends to restart timer to now+rto.
 */
tcp_rearm_rto:

    CAPRI_CLEAR_TABLE0_VALID

    sne         c4, r7, r0
    /* If the retrans timer is currently being used by Fast Open
     * for SYN-ACK retrans purpose, stay put.
     *
      if (tp->rx_opt.fastopen_rsk)
        return;
     */
    sne         c1, d.u.tcp_rx_d.fastopen_rsk, r0
    bcf         [c1 & c4], tcp_rearm_rto_done
    nop
    seq         c1, k.s1_s2s_packets_out, r0
    phvwr.c1    p.rx2tx_pending_ft_clear,1
    phvwr.c1    p.common_phv_pending_txdma, 1
    bcf         [c1], tcp_rearm_rto_done
    tbladd.c1   d.u.tcp_rx_d.rto,  -1
    /* r3 = rto */
    add         r3, d.u.tcp_rx_d.rto, r0
    seq         c2, d.u.tcp_rx_d.pending, ICSK_TIME_EARLY_RETRANS
    seq         c3, d.u.tcp_rx_d.pending, ICSK_TIME_LOSS_PROBE
    bcf         [!c2 & !c3], early_retx_or_tlp
    nop
    /* rto_time_stamp = tp->rx.retx_head_ts + rto */
    add         r1, d.u.tcp_rx_d.retx_head_ts, r3
    /* delta = (s32) (rto_time_stamp - tcp_time_stamp) */
    sub         r2, r1, r6
    /* if (delta > 0) */
    slt         c2, r0, r2
    /*  rto = delta */
    add.c2      r3, r2, r0
early_retx_or_tlp:
    /* tp->fto.rto_deadline = tcp_time_stamp + min(rto, TCP_RTO_MAX) */
    addi        r4, r0, TCP_RTO_MAX
    slt         c1, r3, r4
    add.c1      r5, r3, r6
    tblwr.c1    d.u.tcp_rx_d.rto_deadline, r5
    add.!c1     r5, r4, r6
    tblwr.!c1   d.u.tcp_rx_d.rto_deadline, r5
    phvwr       p.rx2tx_pending_ft_reset,1
    phvwr.c1    p.common_phv_pending_txdma, 1
tcp_rearm_rto_done: 
    jr.c4       r7
    add         r7, r0, r0

    
tcp_replace_ts_recent:
    sne         c4, r7, r0
    jr.c4       r7
    add         r7, r0, r0

tcp_enter_quickack_mode:
    bal         r7, tcp_incr_quickack
    nop
    tblwr       d.u.tcp_rx_d.pingpong, r0
    phvwr       p.common_phv_pingpong, d.u.tcp_rx_d.pingpong
    tblwr       d.u.tcp_rx_d.ato, TCP_ATO_MIN
    sne         c4, r7, r0
    jr.c4       r7
    add         r7, r0, r0

slow_path:  
old_ack:
invalid_ack:
no_queue:
flow_rx_process_done:
table_read_setup_next:
    phvwr       p.rx2tx_snd_wnd, d.u.tcp_rx_d.snd_wnd
    phvwr       p.rx2tx_extra_rcv_mss, d.u.tcp_rx_d.rcv_mss
    bcf         [c7], table_read_SACK
    nop
table_read_RTT:
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN, tcp_rx_rtt_stage2_start,
                        k.common_phv_qstate_addr, TCP_TCB_RTT_OFFSET,
                        TABLE_SIZE_512_BITS)
table_read_RNMDR_ALLOC_IDX:
    addi        r3, r0, RNMDR_ALLOC_IDX
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS, tcp_rx_read_rnmdr_stage2_start,
                        r3, TABLE_SIZE_64_BITS)
table_read_RNMPR_ALLOC_IDX:
    addi        r3, r0, RNMPR_ALLOC_IDX
    CAPRI_NEXT_TABLE_READ(2, TABLE_LOCK_DIS, tcp_rx_read_rnmpr_stage2_start,
                        r3, TABLE_SIZE_64_BITS)
table_read_SERQ_PRODUCER_IDX:
    phvwri      p.common_phv_write_serq, 1
    phvwr       p.to_s6_xrq_base, d.u.tcp_rx_d.serq_base
    nop.e
    nop
table_read_SACK:    
#if 0
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN, tcp_rx_sack_stage2_start,
                        k.common_phv_qstate_addr, TCP_TCB_SACK_OFFSET,
                        TABLE_SIZE_512_BITS)
#endif
    nop.e
    nop
