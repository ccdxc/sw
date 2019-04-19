/*
 *  Implements the RTT stage of the RxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s3_t0_tcp_rx_k.h"
    
struct phv_ p;
struct s3_t0_tcp_rx_k_ k;
struct s3_t0_tcp_rx_tcp_rtt_d d;

    
%%
    .param          tcp_rx_cc_stage_start
    .align
tcp_rx_rtt_start:

    CAPRI_CLEAR_TABLE0_VALID
    /*
        Verify if TSopt is enabled for this flow:
            Enabled: process the TS received in the incoming segment
            Disabled: process the current ts, the ts of the segment being ACKed
        For now, support only TSopt based RTT estimation
    */
    sne         c1, k.common_phv_write_arq, r0
    bcf         [c1], flow_rtt_process_done
    
    seq         c1, k.common_phv_tsopt_enabled, 1
    seq         c2, k.common_phv_tsopt_available, 1
    seq         c3, k.common_phv_ooq_tx2rx_pkt, 1 //to indicate 2nd pass of the o-o-o packet

    bcf         [c3], flow_rtt_process_done
    
    setcf       c2, [!c1 | !c2]
    
    // add.c2      r5, k.to_s3_rtt_time, r0
    seq.c2      c3, k.to_s3_rtt_time, r0
    seq.c2      c4, d.rtt_time, r0 
    bcf.c2      [c3 & c4], flow_rtt_process_done
    nop
    tblwr.!c3   d.rtt_time, k.to_s3_rtt_time
    seq.c2      c3, k.to_s3_rtt_seq, r0
    tblwr.!c3   d.rtt_seq, k.to_s3_rtt_seq
    sle.c2      c3, k.to_s3_rtt_seq, k.to_s3_snd_nxt 
    bcf.c2      [!c3], flow_rtt_process_done
    nop
    /* r4 is loaded at the beginning of the stage with current timestamp value */
    tblwr       d.curr_ts, r4
    /* Prefer RTT measured from ACK's timing to TS-ECR. This is because
     * broken middle-boxes or peers may corrupt TS-ECR fields. But
     * Karn's algorithm forbids taking RTT if some retransmitted data
     * is acked (RFC6298).
     */

    /* RTTM Rule: A TSecr value received in a segment is used to
     * update the averaged RTT measurement only if the segment
     * acknowledges some new data, i.e., only if it advances the
     * left edge of the send window.
     * See draft-ietf-tcplw-high-performance-00, section 3.3.

     * if (seq_rtt_us < 0 &&  tp->rx_opt.rcv_tsecr &&
     *    flag & FLAG_ACKED)
     *   seq_rtt_us = ca_rtt_us = jiffies_to_usecs(tcp_time_stamp -
     *                    tp->rx_opt.rcv_tsecr);
     */
    seq         c1, k.common_phv_process_ack_flag, 1
    bcf         [!c1], flow_rtt_process_done /* cannot use the received TS */
    nop

    /* Capri @ 833MHz : bits 13-44 track at 10us (really 9.83us) granularity */
    /* TSval format: Bits 13-44 of the Capri timestamp (i.e. 10us granularity) */
    /* TODO: other clock rates (333MHz?) */
#ifndef HW
    addi        r4, r0, 0xFEEEFED0
#endif
    srl         r2, r4, 13
    andi        r2, r2, 0x1FFFFFFF
    //srl         r2, r4, d.ts_shift  //ts_shift setting
    //sub.c2      r1, r2, k.to_s3_rcv_tsecr
    sub.c3      r1, r2, d.rtt_time 

    /* Early execution of mul due to single cycle stall */
    //mul         r3, r1, d.ts_ganularity_us   /* r3 now is in tick units */
    /*
     * if (seq_rtt_us < 0)
     *   return false;
     */
    /* Wrap around */
    slt.s       c1, r1, r0   //todo test wrap
    bcf         [c1], flow_rtt_process_done
    nop

    /* srtt = tp->rtt.srtt_us */
    add         r2, d.srtt_us, r0
    /* r1 is srtt */
    seq         c1, r2, r0
    bcf         [c1], first_rtt_measure
    nop

    /* srtt != 0 */

     /* RFC 6298 2.3 */
     /* (2.3) When a subsequent RTT measurement R' is made, a host MUST set

            RTTVAR <- (1 - beta) * RTTVAR + beta * |SRTT - R'|
            SRTT <- (1 - alpha) * SRTT + alpha * R'

         The value of SRTT used in the update to RTTVAR is its value
         before updating SRTT itself using the second assignment.  That
         is, updating RTTVAR and SRTT MUST be computed in the above
         order.
         The above SHOULD be computed using alpha=1/8 and beta=1/4 (as
         suggested in [JK88]).
    */ 
    
    add         r2, d.rttvar_us, 0
    srl         r3, r2, 1
    srl         r4, r2, 2
    add         r2, r3, r4     // ((1-beta) * rttvar) 
    add         r3, d.srtt_us, r0
    sub         r4, r3, r1
    slt         c1, r4, r0
    sub.c1      r4, r0, r4    //r4 is abs(srtt-rtt)
    /* r1 is rtt, r2 rtt_var, r3 srtt */
    srl         r4, r4, 2
    add         r2, r2, r4
    tblwr       d.rttvar_us, r2
    
    srl         r2, r3, 3  // r2 = 1/8 srtt
    srl         r1, r1, 3  // 1/8 rtt
    sub         r3, r3, r2 // r3 = 7/8 srtt
    add         r3, r3, r1 // 7/8 srtt + 1/8 rtt
    phvwr       p.to_s4_t_srtt, r3

    b           tcp_set_rto
    tblwr       d.srtt_us, r3
    
first_rtt_measure: 
    /* rfc6298 paragraph 2.2
       (2.2) When the first RTT measurement R is made, the host MUST set

       SRTT <- R
       RTTVAR <- R/2
       RTO <- SRTT + max (G, K*RTTVAR)

       where K = 4
     */
    tblwr       d.srtt_us, r1
    phvwr       p.to_s4_t_srtt, r1
    srl         r1, r1, 1
    tblwr       d.rttvar_us, r1

tcp_set_rto:
    sll         r1, d.rttvar_us, 2
    slt         c1, r1, 1
    add.c1      r2, d.srtt_us, 1
    add.!c1     r2, d.srtt_us, r1
    slt         c1, r2, TCP_RTO_MIN_TICK
    add.c1      r2, TCP_RTO_MIN_TICK, r0
    slt         c1, TCP_RTO_MAX_TICK, r2
    add.c1      r2, TCP_RTO_MAX_TICK, r0
//todo put rto into timer ticks
    tblwr       d.rto, r2 
    phvwr       p.rx2tx_extra_rto, r2
    add         r2, d.rtt_updated, r0
    addi        r2, r2, 1
    sle         c2, r2, 7
    phvwr.c2    p.to_s4_srtt_valid, 1
    tblwr       d.rtt_updated, r2
    phvwr       p.rx2tx_extra_rtt_seq_req_, 1
    
    tblwr       d.backoff, r0

    /*
     * TODO - we need to indicate to tx pipeline when backoff is reset
     * via rx2tx_extra_pending_reset_backoff
     */
flow_rtt_process_done:
    
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN,
                        tcp_rx_cc_stage_start,
                        k.common_phv_qstate_addr,
                        TCP_TCB_CC_OFFSET, TABLE_SIZE_512_BITS)
    nop.e
    nop


