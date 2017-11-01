/*
 *  Implements the FAST RETX ALERT stage of the RxDMA P4+ pipeline
 *
 * Linux NewReno/SACK/FACK/ECN state machine.
 * --------------------------------------
 *
 * "Open"   Normal state, no dubious events, fast path.
 * "Disorder"   In all the respects it is "Open",
 *      but requires a bit more attention. It is entered when
 *      we see some SACKs or dupacks. It is split of "Open"
 *      mainly to move some processing from fast path to slow one.
 * "CWR"    CWND was reduced due to some Congestion Notification event.
 *      It can be ECN, ICMP source quench, local device congestion.
 * "Recovery"   CWND was reduced, we are fast-retransmitting.
 * "Loss"   CWND was reduced due to RTO timeout or SACK reneging.
 *
 * tcp_fastretrans_alert() is entered:
 * - each incoming ACK, if state is not "Open"
 * - when arrived ACK is unusual, namely:
 *  * SACK
 *  * Duplicate ACK.
 *  * ECN ECE.
 *
 * Counting packets in flight is pretty simple.
 *
 *  in_flight = packets_out - left_out + retrans_out
 *
 *  packets_out is SND.NXT-SND.UNA counted in packets.
 *
 *  retrans_out is number of retransmitted segments.
 *
 *  left_out is number of segments left network, but not ACKed yet.
 *
 *      left_out = sacked_out + lost_out
 *
 *     sacked_out: Packets, which arrived to receiver out of order
 *         and hence not ACKed. With SACKs this number is simply
 *         amount of SACKed data. Even without SACKs
 *         it is easy to give pretty reliable estimate of this number,
 *         counting duplicate ACKs.
 *
 *       lost_out: Packets lost by network. TCP has no explicit
 *         "loss notification" feedback from network (for now).
 *         It means that this number can be only _guessed_.
 *         Actually, it is the heuristics to predict lossage that
 *         distinguishes different algorithms.
 *
 *  F.e. after RTO, when all the queue is considered as lost,
 *  lost_out = packets_out and in_flight = retrans_out.
 *
 *      Essentially, we have now two algorithms counting
 *      lost packets.
 *
 *      FACK: It is the simplest heuristics. As soon as we decided
 *      that something is lost, we decide that _all_ not SACKed
 *      packets until the most forward SACK are lost. I.e.
 *      lost_out = fackets_out - sacked_out and left_out = fackets_out.
 *      It is absolutely correct estimate, if network does not reorder
 *      packets. And it loses any connection to reality when reordering
 *      takes place. We use FACK by default until reordering
 *      is suspected on the path to this destination.
 *
 *      NewReno: when Recovery is entered, we assume that one segment
 *      is lost (classic Reno). While we are in Recovery and
 *      a partial ACK arrives, we assume that one more packet
 *      is lost (NewReno). This heuristics are the same in NewReno
 *      and SACK.
 *
 *  Imagine, that's all! Forget about all this shamanism about CWND inflation
 *  deflation etc. CWND is real congestion window, never inflated, changes
 *  only according to classic VJ rules.
 *
 * Really tricky (and requiring careful tuning) part of algorithm
 * is hidden in functions tcp_time_to_recover() and tcp_xmit_retransmit_queue().
 * The first determines the moment _when_ we should reduce CWND and,
 * hence, slow down forward transmission. In fact, it determines the moment
 * when we decide that hole is caused by loss, rather than by a reorder.
 *
 * tcp_xmit_retransmit_queue() decides, _what_ we should retransmit to fill
 * holes, caused by lost packets.
 *
 * And the most logically complicated part of algorithm is undo
 * heuristics. We detect false retransmits due to both too early
 * fast retransmit (reordering) and underestimated RTO, analyzing
 * timestamps and D-SACKs. When we detect that some segments were
 * retransmitted by mistake and CWND reduction was wrong, we undo
 * window reduction and abort recovery phase. This logic is hidden
 * inside several functions named tcp_try_undo_<something>.
 */


#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp-constants.h"
#include "ingress.h"
#include "INGRESS_p.h"
    
struct phv_ p;
struct tcp_rx_tcp_fra_k k;
struct tcp_rx_tcp_fra_tcp_fra_d d;

%%
    .param          tcp_rx_cc_stage4_start
    .align
tcp_rx_fra_stage3_start:
    CAPRI_SET_DEBUG_STAGE0_3(p.s6_s2s_debug_stage0_3_thread, CAPRI_MPU_STAGE_3, CAPRI_MPU_TABLE_0)
    /* r4 is loaded at the beginning of the stage with current timestamp value */
    sne         c1, k.common_phv_write_arq, r0
    bcf         [c1], flow_fra_process_done
    nop
    tblwr       d.curr_ts, r4

    tblwr       d.ack_flag, k.common_phv_process_ack_flag
    tblwr       d.pkts_acked, k.common_phv_pkts_acked

    

    /*   tcp_fastretrans_alert(tp, md->pkts_acked, md->is_dupack, &flag, &rexmit); */
tcp_fastretrans_alert:
    /* Process an event, which can update packets-in-flight not trivially.
     * Main goal of this function is to calculate new estimate for left_out,
     * taking into account both packets sitting in receiver's buffer and
     * packets lost by network.
     *
     * Besides that it updates the congestion state when packet loss or ECN
     * is detected. But it does not reduce the cwnd, it is done by the
     * congestion control later.
     *
     * It does _not_ decide what to send, it is made in function
         * tcp_xmit_retransmit_queue().
         */
    /* int fast_rexmit = 0 */
    tblwr       d.fast_rexmit, r0
    /* bool do_lost = is_dupack || ((flag & FLAG_DATA_SACKED) &&
                    (tcp_fackets_out(tp) > tp->sack.reordering));
     */
    /* c1 = !(flag & FLAG_DATA_SACKED) */
    add         r1, d.ack_flag, r0
    smneh       c1, r1, FLAG_DATA_SACKED, FLAG_DATA_SACKED
    /* c2 = !(tcp_fackets_out(tp) > tp->sack.reordering) */
    slt         c2, d.fackets_out, d.reordering
    /* c3 = !is_dupack */
    seq         c3, k.common_phv_is_dupack, r0
    tblwr       d.do_lost, r0
    setcf       c4, [c1 | c2]
    andcf       c4, [c3 & c4]
    bcf         [c4], no_do_lost
    nop
    tblwr       d.do_lost,1 
no_do_lost:
    /* c1 = tp->tx.packets_out */
    sne         c1, k.to_s3_packets_out, r0
    /* c2 = !tp->sack.sacked_out */
    seq         c2, k.to_s3_sacked_out, r0
    /* if (tp->tx.packets_out || !tp->sack.sacked_out) */
    bcf         [c1 | c2], no_clear_sacked_out
    nop
    /* tp->sack.sacked_out = 0 */
     phvwr      p.s4_s2s_sacked_out, r0
no_clear_sacked_out:
    /* c1 = tp->tx.sacked_out */
    sne         c1, k.to_s3_sacked_out, r0
    /* c2 = !tp->sack.fackets_out */
    seq         c2, d.fackets_out, r0
    /* if (tp->tx.sacked_out || !tp->sack.fackets_out) */
    bcf         [c1 | c2], no_clear_fackets_out
    nop
    /* tp->sack.fackets_out = 0 */
    tblwr       d.fackets_out, r0
no_clear_fackets_out:
    /* Now state machine starts. */

    /*
     * A. ECE, hence prohibit cwnd undoing, the reduction is required.
     *
     * if (flag & FLAG_ECE)
        tp->cc.prior_ssthresh = 0;
     */
    add         r1, d.ack_flag, r0
    smeqh       c1, r1, FLAG_ECE, FLAG_ECE
    tblwr.c1    d.prior_ssthresh, r0

    /* B. In all the states check for reneging SACKs.
     *
     * if (tcp_check_sack_reneging(tp, flag))
        return;
     */
tcp_check_sack_reneging:
    /* If ACK arrived pointing to a remembered SACK, it means that our
     * remembered SACKs do not reflect real state of receiver i.e.
     * receiver _host_ is heavily congested (or buggy).
     *
     * To avoid big spurious retransmission bursts due to transient SACK
     * scoreboard oddities that look like reneging, we give the receiver a
     * little time (max(RTT/2, 10ms)) to send us some more ACKs that will
     * restore sanity to the SACK scoreboard. If the apparent reneging
     * persists until this RTO then we'll clear the SACK scoreboard.
     */
    add         r1, d.ack_flag, r0
    smneh       c1, r1, FLAG_SACK_RENEGING, FLAG_SACK_RENEGING
    bcf         [c1], tcp_check_sack_reneging_done
    nop
    /* unsigned long delay = max(usecs_to_jiffies(tp->rtt.srtt_us >> 4),
                  usecs_to_jiffies(TCP_DELACK_MAX));
     */

    add         r1, k.to_s3_srtt_us, r0
    srl         r1, r1, 4
    addi        r2, r0, TCP_DELACK_MAX
    slt         c1, r1, r2
    add.c1      r1, r2, r0
    /* r1 = delay */
    addi        r2, r0, TCP_RTO_MAX
    slt         c1, r1, r2
    add.c1      r1, r2, r0
    /* r1 = max(delay, TCP_RTO_MAX) */
    add         r2, d.curr_ts, r1
    phvwr       p.rx2tx_rto_deadline, r2
    bcf         [c1], flow_fra_process_done
    nop
tcp_check_sack_reneging_done:   

    /* C. Check consistency of the current state. */
    /* tcp_verify_left_out(tp);*/
tcp_verify_left_out:
    add         r1, k.to_s3_sacked_out, k.to_s3_lost_out
    slt         c1, k.to_s3_packets_out, r1
    /* c1 = k.packets_out < (k.sacked_out + k.lost_out)
     * c1 should be false
     */

    /* D. Check state exit conditions. State can be terminated
     *    when high_seq is ACKed.
     */

    /* if (tp->cc.ca_state == TCP_CA_Open) {
        tp->tx.retx_head_ts = 0;
     * }
     */
    seq         c1, d.ca_state, TCP_CA_Open
    tblwr       d.retx_head_ts, r0
    bcf         [c1], check_E
    nop
    /*
     * if (!before(tp->tx.snd_una, tp->cc.high_seq)) {
        switch (tp->cc.ca_state) {
        case TCP_CA_CWR:
            /* CWR is to be held something *above* high_seq
             * is ACKed for CWR bit to reach receiver. *
            if (tp->tx.snd_una != tp->cc.high_seq) {
                tcp_end_cwnd_reduction(tp);
                tcp_set_ca_state(tp, TCP_CA_Open);
            }
            break;

        case TCP_CA_Recovery:
                if (tcp_try_undo_recovery(tp))
              return;
            tcp_end_cwnd_reduction(tp);
            break;
        }
       }
    */
    slt         c1, k.common_phv_snd_una, d.high_seq
    bcf         [c1], check_E
    nop
    sne         c1, d.ca_state, TCP_CA_CWR
    bcf         [c1], tcp_ca_recovery_case
tcp_ca_cwr_case:    
    /* CWR is to be held something *above* high_seq
     * is ACKed for CWR bit to reach receiver.
     */
    /* if (tp->tx.snd_una != tp->cc.high_seq) {
          tcp_end_cwnd_reduction(tp);
          tcp_set_ca_state(tp, TCP_CA_Open);
        }
      */
    seq         c1, k.common_phv_snd_una, d.high_seq
    bcf         [c1],check_E
    nop

    TCP_END_CWND_REDUCTION
    
    /* tcp_set_ca_state(tp, TCP_CA_Open); */
    tblwr       d.ca_state, TCP_CA_Open


tcp_ca_recovery_case:   
    sne         c1, d.ca_state, TCP_CA_Recovery
    bcf         [c1], switch_done_1

tcp_try_undo_recovery:
tcp_may_undo:
    /* static inline bool tcp_may_undo(tcb_t *tp)
     * {
     * return tp->sack.undo_marker && (!tp->sack.undo_retrans || tcp_packet_delayed(tp));
}
     */
    seq         c1, d.undo_marker, r0
    sne         c2, k.to_s3_undo_retrans, r0
    /* Nothing was retransmitted or returned timestamp is
     * less than timestamp of the first retransmission.
     *
     * static inline bool tcp_packet_delayed(tcb_t *tp)
     * {
            return !tp->tx.retx_head_ts ||
              tcp_tsopt_ecr_before(tp, tp->tx.retx_head_ts) ;
    }
    */
    sne         c3, d.retx_head_ts, r0
    /*
    static bool tcp_tsopt_ecr_before(tcb_t *tp, u32 when)
    {
            return tp->rx_opt.saw_tstamp && tp->rx_opt.rcv_tsecr &&
              before(tp->rx_opt.rcv_tsecr, when) ;
    }
    */
    //slt       c4, d.retx_head_ts, k.common_phv_rcv_tsecr
    setcf       c5, [c2 & c3 & c4]
    setcf       c5, [c1 | c5]
    bcf         [c5], tcp_undo_cwnd_reduction
    nop
    /* tcp_undo_cwnd_reduction(tp, false) */
tcp_undo_cwnd_reduction:
    /*
      if (tp->cc.prior_ssthresh) {
     */
    seq         c1, d.prior_ssthresh, r0
    bcf         [c1], clear_undo_marker
    nop
    /* tp->cc.snd_cwnd = bictcp_undo_cwnd(tp) ; */
    /*
    u32 bictcp_undo_cwnd(tcb_t *tp)
    {
      const struct bictcp *ca = &tp->cc.bic    ;

      return max(tp->cc.snd_cwnd, ca->loss_cwnd) ;
    }

    */
    slt         c1, d.snd_cwnd, d.loss_cwnd
    add.c1      r1, d.loss_cwnd, r0
    phvwr.c1    p.to_s4_snd_cwnd, r1

    /* if (tp->cc.prior_ssthresh > tp->cc.snd_ssthresh) { */
    slt         c1, d.prior_ssthresh, d.snd_ssthresh
    /*        tp->cc.snd_ssthresh = tp->cc.prior_ssthresh ; */
    add.c1      r1, d.prior_ssthresh, r0
    tblwr.c1    d.snd_ssthresh, r1

    /* tcp_ecn_withdraw_cwr(tp)           ; */
    /*
     * static void tcp_ecn_withdraw_cwr(tcb_t *tp)
       {
         tp->rx_opt.ecn_flags &= ~TCP_ECN_DEMAND_CWR ;
           }
     */
    add         r1, k.common_phv_ecn_flags, r0
    andi        r1, r1, ~TCP_ECN_DEMAND_CWR
    phvwr       p.common_phv_ecn_flags, r1
clear_undo_marker:  
    /* tp->sack.undo_marker = 0 ; */
    tblwr       d.undo_marker, r0
    
tcp_undo_cwnd_reduction_done:   
    /* tcp_set_ca_state(tp, TCP_CA_Open) */
    tblwr       d.ca_state, TCP_CA_Open
    
    
    TCP_END_CWND_REDUCTION
    
    
switch_done_1:  
check_E:    
    
flow_fra_process_done:
    phvwr       p.to_s4_snd_ssthresh, d.snd_ssthresh
    
table_read_CC:
    phvwr       p.s4_s2s_debug_stage4_7_thread, 0
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN,
                    tcp_rx_cc_stage4_start, k.common_phv_qstate_addr,
                    TCP_TCB_CC_OFFSET, TABLE_SIZE_512_BITS)
    nop.e
    nop

