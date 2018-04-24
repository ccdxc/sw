/*
 *  TCP ACK processing (ack received from peer)
 */


#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp-constants.h"  
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s3_t0_tcp_rx_k.h"
    
struct phv_ p;
struct s3_t0_tcp_rx_k_ k;
struct s3_t0_tcp_rx_tcp_ack_d d;
    
%%
    .align
    .param          tcp_rx_rtt_start

tcp_ack_slow:
    seq             c1, d.state, TCP_ESTABLISHED
    sne             c2, k.s1_s2s_snd_nxt, k.s1_s2s_ack_seq
    bcf             [c1 | c2], tcp_ack_established_slow

    /*
     *                    (recv ack)
     * State (LAST_ACK)   ----------> TCP_CLOSE
     * State (FIN_WAIT_1) ----------> FIN_WAIT_2
     * State (CLOSING) ----------> TIME_WAIT
     */
    seq             c1, d.state, TCP_LAST_ACK
    tblwr.c1        d.state, TCP_CLOSE
    // TODO : Inform ARM that the connection is closed

    seq             c1, d.state, TCP_FIN_WAIT1
    tblwr.c1        d.state, TCP_FIN_WAIT2

    // TODO : either inform ARM or start time wait timer
    seq             c1, d.state, TCP_CLOSING
    tblwr.c1        d.state, TCP_TIME_WAIT

tcp_ack_established_slow:
    /* If the ack is older than previous acks
     * then we can probably ignore it.
     *
     *  if (before(ack, prior_snd_una)) {
     *          // RFC 5961 5.2 [Blind Data Injection Attack].[Mitigation]
     *          if (before(ack, prior_snd_una - tp->max_window)) {
     *                  tcp_send_challenge_ack(sk, skb);
     *                  return -1;
     *          }
     *          goto old_ack;
     *  }
     *
     */
    slt             c1, k.s1_s2s_ack_seq, d.snd_una
    sub.c1          r1, d.snd_una, d.max_window
    slt.c1          c2, k.s1_s2s_ack_seq, r1
    phvwr.c2        p.rx2tx_extra_pending_challenge_ack_send, 1
    bcf             [c2], tcp_ack_slow_launch_next_stage
    nop
    b.c1            old_ack
    nop

    /* If the ack includes data we haven't sent yet, discard
     * this segment (RFC793 Section 3.9).
     *
     *  if (after(ack, tp->snd_nxt))
     *          goto invalid_ack;
     *
     */
    slt             c1, k.s1_s2s_snd_nxt, k.s1_s2s_ack_seq
    bcf             [c1], invalid_ack
    nop

#if 0
    // TODO : this needs to move to tx pipeline
    /*
     *
    if (tp->pending.pending == ICSK_TIME_EARLY_RETRANS ||
        tp->pending.pending == ICSK_TIME_LOSS_PROBE)
       tcp_rearm_rto(tp);
     *
     */
    sne             c1, d.pending, ICSK_TIME_EARLY_RETRANS
    sne             c2, d.pending, ICSK_TIME_LOSS_PROBE
    bcf             [c1 & c2], no_rearm_rto
    nop
    bal             r7, tcp_rearm_rto
    nop
    b               no_rearm_rto
    nop
no_rearm_rto:
#endif

    /*
     *
     * if (after(ack, prior_snd_una)) {
     *     md->process_ack_flag |= FLAG_SND_UNA_ADVANCED;
     *  }
     *
     */
    slt             c1, d.snd_una, k.s1_s2s_ack_seq
    tblor.c1.l      d.flag, FLAG_SND_UNA_ADVANCED

    /* ts_recent update must be made after we are sure that the packet
     * is in window.
     *
     *  if (flag & FLAG_UPDATE_TS_RECENT)
     *          tcp_replace_ts_recent(tp, TCP_SKB_CB(skb)->seq);
     *
     */
    smeqb           c2, d.flag, FLAG_UPDATE_TS_RECENT, FLAG_UPDATE_TS_RECENT
    b.!c2           tcp_replace_ts_recent_end
tcp_replace_ts_recent:
    // TODO : implement this
    sne             c4, r7, r0
    add             r7, r0, r0
tcp_replace_ts_recent_end:
    /*
     *
     *  if (!(flag & FLAG_SLOWPATH) && after(ack, prior_snd_una)) {
     *          // Window is constant, pure forward advance.
     *          // No more checks are required.
     *          // Note, we use the fact that SND.UNA>=SND.WL2.
     *
     *          tcp_update_wl(tp, ack_seq);
     *          tcp_snd_una_update(tp, ack);
     *          flag |= FLAG_WIN_UPDATE;
     *
     *          tcp_in_ack_event(sk, CA_ACK_WIN_UPDATE);
     *
     *          NET_INC_STATS(sock_net(sk), LINUX_MIB_TCPHPACKS);
     * } else {
     *         u32 ack_ev_flags = CA_ACK_SLOWPATH;
     *
     *         if (ack_seq != TCP_SKB_CB(skb)->end_seq)
     *                 flag |= FLAG_DATA;
     *         else
     *                 NET_INC_STATS(sock_net(sk), LINUX_MIB_TCPPUREACKS);
     *
     *         flag |= tcp_ack_update_window(sk, skb, ack, ack_seq);
     *
     *         if (TCP_SKB_CB(skb)->sacked)
     *                 flag |= tcp_sacktag_write_queue(sk, skb, prior_snd_una,
     *                                                 &sack_state);
     *
     *         if (tcp_ecn_rcv_ecn_echo(tp, tcp_hdr(skb))) {
     *                 flag |= FLAG_ECE;
     *                 ack_ev_flags |= CA_ACK_ECE;
     *         }
     *
     *         if (flag & FLAG_WIN_UPDATE)
     *                 ack_ev_flags |= CA_ACK_WIN_UPDATE;
     *
     *         tcp_in_ack_event(sk, ack_ev_flags);
     * }
     *
     */

     /*
      * We are already in slow path here, so no need to check that
      */
    sne             c2, k.s1_s2s_payload_len, 0
    tblor.c2.l      d.flag, FLAG_DATA

tcp_ack_update_window:
    /* r2 contains nwin */
    /*
     * nwin = cp->window
     * nwin <<= tp->rx_opt.snd_wscale;
     */
    sll             r2, k.s1_s2s_window, d.snd_wscale
tcp_may_update_window:
    /* after(ack, snd_una) */
    slt             c1, k.s1_s2s_ack_seq, d.snd_una
    /* after(ack_seq, snd_wl1) */
    slt             c2, k.s1_s2s_ack_seq, d.snd_wl1
    /* ack_seq == snd_wl1 */
    slt             c3, k.s1_s2s_ack_seq, d.snd_wl1
    /* nwin > snd_wnd */
    slt             c4, d.snd_wnd, r2
    setcf           c5, [!c3 | !c4]
    setcf           c5, [!c1 & !c2 & c5]
    bcf             [c5], tcp_update_window_bypass
    nop

tcp_update_window:
    /* flag |= FLAG_WIN_UPDATE */
    tblor.l         d.flag, FLAG_WIN_UPDATE
    /* ack_ev_flags |= CA_ACK_WIN_UPDATE */
    ori             r1, r1, CA_ACK_WIN_UPDATE
    /* tcp_update_wl */
    tblwr           d.snd_wl1, k.s1_s2s_ack_seq
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
    sne             c1,d.snd_wnd, r2
    /*   tp->tx.snd_wnd = nwin; */

    tblwr.c1        d.snd_wnd, r2
    /*   tp->rx.pred_flags = 0; */
    tblwr.c1        d.pred_flags, 0
        /*   if (nwin > tp->rx.max_window) { */
    slt.c1          c2, d.max_window, r2
        /*        tp->rx.max_window = nwin; */
    tblwr.c2        d.max_window, r2
    /*        tcp_sync_mss(tp, tp->rx_opt.pmtu);
     *        tcp_sync_mss will be triggered in tx stage based on pending bit
     */
    //phvwr.c2        p.common_phv_pending_sync_mss, 1


tcp_update_window_bypass:
tcp_snd_una_update_slow:
    /* tcp_snd_una_update */
    /* Increment bytes acked by the delta between ack_seq and snd_una */
    sub             r3, k.s1_s2s_ack_seq, d.snd_una
    tbladd          d.bytes_acked, r3
    /* Update snd_una */
    tblwr           d.snd_una, k.s1_s2s_ack_seq

tcp_ecn_rcv_ecn_echo:
    /* ecn_flags & TCP_ECN_OK */
    smeqb           c3, d.ecn_flags, TCP_ECN_OK, TCP_ECN_OK
    /* c4 = !cp->ece */
    seq             c4, k.common_phv_ece, r0
    /* c5 = !cp->syn */
    seq             c5, k.common_phv_syn, r0
    bcf             [c4 | !c5 | !c3], tcp_ece_flag_set_bypass
    nop
    /* md->process_ack_flag |= FLAG_ECE */
    tblor.l         d.flag, FLAG_ECE
    /* ack_ev_flags |= CA_ACK_ECE */
    ori             r1, r1, CA_ACK_ECE
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
     tblwr          d.rcv_tstamp, r6
     seq            c3, k.s1_s2s_packets_out, r0
     bcf            [!c3], no_queue
     nop

tcp_in_ack_event_end:
    phvwrmi         p.common_phv_pending_txdma, TCP_PENDING_TXDMA_SND_UNA_UPDATE, \
                        TCP_PENDING_TXDMA_SND_UNA_UPDATE
    phvwr           p.rx2tx_rx_flag, d.flag
    phvwr           p.rx2tx_snd_una, d.snd_una
    phvwr           p.common_phv_snd_una, d.snd_una
    /*
     * Launch next stage
     */

old_ack:
invalid_ack:
no_queue:
tcp_ack_slow_launch_next_stage:
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN,
                        tcp_rx_rtt_start,
                        k.common_phv_qstate_addr,
                        TCP_TCB_RTT_OFFSET, TABLE_SIZE_512_BITS)
    nop.e
    nop
