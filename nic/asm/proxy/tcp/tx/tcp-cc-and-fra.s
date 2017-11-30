/*
 *	Implements the TX stage of the TxDMA P4+ pipeline
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
struct s4_t0_tcp_tx_k k;
struct s4_t0_tcp_tx_cc_and_fra_d d;

%%
    .align
    .param          tcp_xmit_process_start
    .param          bictcp_cong_avoid

tcp_cc_and_fra_process_start:
    seq             c1, k.common_phv_pending_snd_una_update, 1
    bcf             [c1], tcp_cong_control

    phvwr           p.to_s5_snd_cwnd, d.snd_cwnd
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN,
                        tcp_xmit_process_start,
                        k.common_phv_qstate_addr,
                        TCP_TCB_XMIT_OFFSET, TABLE_SIZE_512_BITS)

    nop.e
    nop

tcp_cong_control:
    /*
     * Check if we are in congestion window reduction state
     */
    smeqb           c1, d.ca_state, TCPF_CA_CWR | TCPF_CA_Recovery, TCPF_CA_CWR | TCPF_CA_Recovery
    bcf             [c1], tcp_cwnd_reduction
    nop

    /*
     * Decide whether to run the increase function of congestion control
     * TODO : tcp_may_raise_cwnd() - until then set c1 to TRUE unconditionally
     */
    seq             c1, r0, r0
    jal.c1          r7, bictcp_cong_avoid
    nop

tcp_cwnd_reduction_done:
    nop.e
    nop

/* The cwnd reduction in CWR and Recovery uses the PRR algorithm in RFC 6937.
 * It computes the number of packets to send (sndcnt) based on packets newly
 * delivered:
 *   1) If the packets in flight is larger than ssthresh, PRR spreads the
 *      cwnd reductions across a full RTT.
 *   2) Otherwise PRR uses packet conservation to send as much as delivered.
 *      But when the retransmits are acked without further losses, PRR
 *      slow starts cwnd up to ssthresh to speed up the recovery.
 */
tcp_cwnd_reduction:
    /*
     * if (newly_acked_sacked <= 0 || WARN_ON_ONCE(!tp->prior_cwnd))
     *      return;
     */
     sle            c1, k.t0_s2s_pkts_acked, r0
     sne            c2, d.prior_cwnd, r0
     bcf            [c1 | !c2], tcp_cwnd_reduction_done
     nop

    /*
     * r6 = tcp_packets_in_flight = packets_out + retrans_out - (sacked_out + lost_out)
     */
     add            r6, k.to_s4_packets_out, k.to_s4_retrans_out
     add            r5, k.to_s4_sacked_out, k.to_s4_lost_out
     sub            r6, r6, r5

     /*
      * r3 = delta = snd_ssthresh - tcp_packets_in_flight
      */
      sub           r3, k.t0_s2s_snd_ssthresh, r6

    /*
     * tp->prr_delivered += newly_acked_sacked
     */
     tbladd         d.prr_delivered, k.t0_s2s_pkts_acked

    /*
     * r5 = sndcnt
     */

    /*
     *  if (delta < 0) {
     *          u64 dividend = (u64)tp->snd_ssthresh * tp->prr_delivered +
     *                         tp->prior_cwnd - 1;
     *          sndcnt = div_u64(dividend, tp->prior_cwnd) - tp->prr_out;
     *  } else if ((flag & FLAG_RETRANS_DATA_ACKED) &&
     *             !(flag & FLAG_LOST_RETRANS)) {
     *          sndcnt = min_t(int, delta,
     *                         max_t(int, tp->prr_delivered - tp->prr_out,
     *                               newly_acked_sacked) + 1);
     *  } else {
     *          sndcnt = min(delta, newly_acked_sacked);
     *  }
     *  // Force a fast retransmit upon entering fast recovery
     *  sndcnt = max(sndcnt, (tp->prr_out ? 0 : 1));
     *  tp->snd_cwnd = tcp_packets_in_flight(tp) + sndcnt;
     */

    slt             c1, r3, r0
    b.!c1           packets_in_flight_less_than_ssthresh

    //      u64 dividend = (u64)tp->snd_ssthresh * tp->prr_delivered +
    //                     tp->prior_cwnd - 1;
    //      sndcnt = div_u64(dividend, tp->prior_cwnd) - tp->prr_out;
    mul             r5, k.t0_s2s_snd_ssthresh, d.prr_delivered
    add             r5, r5, d.prior_cwnd
    sub             r5, r5, 1

    div             r5, r5, d.prior_cwnd
    sub             r5, r5, d.prr_out
    b               update_sndcnt

packets_in_flight_less_than_ssthresh:
    add             r1, r0, k.common_phv_process_ack_flag
    smeqh           c1, r1, FLAG_RETRANS_DATA_ACKED, FLAG_RETRANS_DATA_ACKED
    smeqh           c2, r1, FLAG_LOST_RETRANS, FLAG_LOST_RETRANS
    bcf             [c1 & !c2], sndcnt_retx_data_acked_not_lost_retrans

    // else {
    //      sndcnt = min(delta, newly_acked_sacked);

    add             r5, r0, k.t0_s2s_pkts_acked
    slt             c1, r3, r5
    add.c1          r5, r0, r3
    b               update_sndcnt

sndcnt_retx_data_acked_not_lost_retrans:
    // else if flag & FLAG_RETRANS_DATA_ACKED && !(flag & FLAG_LOST_RETRANS)
    //      sndcnt = min_t(int, delta,
    //                     max_t(int, tp->prr_delivered - tp->prr_out,
    //                           newly_acked_sacked) + 1);
    sub             r2, d.prr_delivered, d.prr_out
    slt             c1, r2, k.t0_s2s_pkts_acked
    add.!c1         r2, r0, k.t0_s2s_pkts_acked
    add             r5, r2, 1

    slt             c1, r3, r5
    add.c4          r5, r0, r3

update_sndcnt:
    // sndcnt = max(sndcnt, (tp->prr_out ? 0 : 1));
    sne             c1, d.prr_out, r0
    add.c1          r1, r0, r0
    add.!c1         r1, r0, 1
    slt             c1, r5, r1
    add.c1          r5, r0, r1

    // tp->snd_cwnd = tcp_packets_in_flight(tp) + sndcnt;
    add             r5, r6, r5
    tbladd          d.snd_cwnd, r5

    b               tcp_cwnd_reduction_done

