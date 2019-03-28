/*
 *  Implements the cc cubic algorithm in  RxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s4_t0_tcp_rx_k.h"

struct phv_ p;
struct s4_t0_tcp_rx_k_ k;
struct s4_t0_tcp_rx_tcp_cc_cubic_d d;


%%
    .param          tcp_rx_cc_stage_end
    .align

tcp_cc_cubic:
    srl             r4, r4, 13   //r4 now in 10us ticks
   /* Handle RTO from TX pipeline*/
    seq             c1, k.s1_s2s_cc_rto_signal, 1
    bal.c1            r7, tcp_cc_cubic_rto_event
    nop


   /*
    * Regular ack received when not in recovery
    * todo : allow every ack and recovery acks for srtt recording
    */
    seq             c1, k.to_s4_cc_flags, 0
    seq             c2, k.to_s4_cc_ack_signal, TCP_CC_ACK_SIGNAL
    seq             c3, d.cc_flags, 0
    bcf             [c1 & c2 & c3], tcp_cc_cubic_ack_recvd
    nop
        /*
     * Exit fast_recovery
     */
    bcf             [c1 & c2 & !c3], tcp_cc_cubic_post_recovery

    /*
     * Entering recovery due to fast retransmit
     */
    smeqb           c4, d.cc_flags, TCP_CCF_FAST_RECOVERY, TCP_CCF_FAST_RECOVERY
    seq             c6, k.to_s4_cc_ack_signal, TCP_CC_DUPACK_SIGNAL
    bcf             [c6 & !c4], tcp_cc_enter_cubic_fast_recovery

    /*
     * dup_ack/partial_ack in fast retransmit
     */
    seq             c6, k.to_s4_cc_ack_signal, TCP_CC_DUPACK_SIGNAL
    seq.!c6         c6, k.to_s4_cc_ack_signal, TCP_CC_PARTIAL_ACK_SIGNAL
    bcf             [c6 & c4], tcp_cc_dupack_fast_recovery
    nop

    /*
     * Enter cong recovery
     */
    bbeq            k.to_s4_cc_ack_signal[TCP_CC_ECE_SIGNAL_BIT], 1, tcp_cc_enter_cong_recovery
    nop

    j               tcp_rx_cc_stage_end
    nop

tcp_cc_cubic_ack_recvd:

cubic_record_rtt:
    //check if less than 8 rtt samples are taken
    /*
     * Record the current SRTT as our minrtt if it's the smallest
     * we've seen or minrtt is currently equal to its initialised
     * value.
     *
     * XXXLAS: Should there be some hysteresis for minrtt?
     */

    seq            c1, k.to_s4_srtt_valid, r0
//todo allow the branch below for srtt convergance issues 
//    b.c1           cubic_ack_rec

    //record rtt
    //(t_srtt_ticks = srtt/32)
    srl            r1, k.to_s4_t_srtt, 5
    slt            c1, r1, d.min_rtt_ticks
    //compare to TCPTV_SRTTBASE (0)
    seq            c2, d.min_rtt_ticks, r0
    setcf          c3,   [ c1 | c2 ] 
    b.!c3          cubic_record_rtt_end
    nop
    slt            c1, r1, 1
    tblwr.!c1      d.min_rtt_ticks, r1
    tblwr.c1       d.min_rtt_ticks, 1
    
    /*
     * If the connection is within its first congestion
     * epoch, ensure we prime mean_rtt_ticks with a
     * reasonable value until the epoch average RTT is
     * calculated in cubic_post_recovery().
     */
    sle           c1, d.min_rtt_ticks, d.mean_rtt_ticks

    tblwr.!c1     d.mean_rtt_ticks, d.min_rtt_ticks        

    /* Sum samples for epoch average RTT calculation. */
    tbladd         d.sum_rtt_ticks, r1
    add            r2, d.epoch_ack_count, 1
    tblwr          d.epoch_ack_count, r2

cubic_record_rtt_end:

cubic_ack_rec:
    /*
     * Regular ACK and we're not in cong/fast recovery and we're cwnd
     * limited and we're either not doing ABC or are slow starting or are
     * doing ABC and we've sent a cwnd's worth of bytes.
     */
//if (type == CC_ACK && !IN_RECOVERY(CCV(ccv, t_flags)) &&
//    (ccv->flags & CCF_CWND_LIMITED) && (!V_tcp_do_rfc3465 ||
//        CCV(ccv, snd_cwnd) <= CCV(ccv, snd_ssthresh) ||
//        (V_tcp_do_rfc3465 && ccv->flags & CCF_ABC_SENTAWND))) {`

   //todo..remove comments below 
    seq             c1, d.snd_cwnd, k.to_s4_snd_wnd //c1 contains CCF_CWND_LIMITED check
   // seq             c2, k.to_s4_cc_ack_signal, TCP_CC_ACK_SIGNAL
    //or              r2, TCP_CCF_CONG_RECOVERY,TCP_CCF_FAST_RECOVERY
    //or              r2, d.cc_flags, r2
    //seq             c3, r2, r0
    seq             c4, d.abc_l_var, 0
    sle             c5, d.snd_cwnd,d.snd_ssthresh
    //slt             c6, d.snd_cwnd, k.to_s4_bytes_acked  //todo..is this CCF_ABC_SENTAWND

    //setcf           c7, [!c4 & c6]
    //setcf           c6, [c7 | c5 | c4]
    //b.!c1           cubic_ack_rec_end
    b.c5            cubic_do_reno_ss
    sub             r2, r4, d.t_last_cong
    /* r2 contains t_last_cong */
    
    /*
     * The mean RTT is used to best reflect the equations in
     * the I-D. Using min_rtt in the tf_cwnd calculation
     * causes w_tf to grow much faster than it should if the
     * RTT is dominated by network buffering rather than
     * propagation delay.
     */
//        w_tf = tf_cwnd(ticks_since_cong,
//                cubic_data->mean_rtt_ticks, cubic_data->max_cwnd,
//                CCV(ccv, t_maxseg));

 /*
 * Compute an approximation of the "TCP friendly" cwnd some number of ticks
 * after a congestion event that is designed to yield the same average cwnd as
 * NewReno while using CUBIC's beta of 0.8. RTT should be the average RTT
 * estimate for the path measured over the previous congestion epoch and wmax is
 * the value of cwnd at the last congestion event.
 */
//static __inline unsigned long
//tf_cwnd(int ticks_since_cong, int rtt_ticks, unsigned long wmax,
//    uint32_t smss)
//{
//
//    /* Equation 4 of I-D. */
//    return (((wmax * CUBIC_BETA) + (((THREE_X_PT2 * ticks_since_cong *
//        smss) << CUBIC_SHIFT) / TWO_SUB_PT2 / rtt_ticks)) >> CUBIC_SHIFT);
//}


    /* r2 contains t_last_cong */
    mul            r3, r2, THREE_X_PT2
    mul            r1, CUBIC_BETA, d.max_cwnd
    mul            r3, d.smss,r3  

    /* begin calc of cubic_cwnd */

    mul            r6, d.cubic_K, CUBIC_HZ 
    sll            r5, r2, CUBIC_SHIFT
    sub            r2, r5, r6

    /* continue newreno cwnd */

    sll            r3, r3, CUBIC_SHIFT
    div            r3, r3, TWO_SUB_PT2


    /* continue cubic_cwnd */
    
    srl            r5, r2, 17
    srl            r6, r2, 19
    add            r2, r5, r6
    mul            r5, r2, r2

    /* continue newreno cwnd */

    div            r3,r3,d.mean_rtt_ticks

    /*continue cubic_cwnd */

    mul            r7, r5,r2
    mul            r2, CUBIC_C_FACTOR, d.smss

    /* continue new reno */

    add            r3, r1, r3
    /* continue cubic_cwnd */
    mul            r2, r2, r7
    /* new_reno */
    srl            r3, r3, CUBIC_SHIFT

    /*new reno cwnd result in r3 */

    
    /*
     * C(t - K)^3 + wmax
     * The down shift by CUBIC_SHIFT_4 is because cwnd has 4 lots of
     * CUBIC_SHIFT included in the value. 3 from the cubing of cwnd above,
     * and an extra from multiplying through by CUBIC_C_FACTOR.
     *  cwnd = ((cwnd * CUBIC_C_FACTOR * smss) >> CUBIC_SHIFT_4) + wmax;
     */
    srl           r2, r2,CUBIC_SHIFT_4
    add           r2, r2, d.max_cwnd

    /* cubic cwnd in r2 */
    /* newreno cwnd in r5 */

    /* cubic cwnd less than tcp reno */
    sle           c1, r2,r5
    /* tcp_friendly region */
    b.c1          tcp_friendly_cwnd

    /* snd_cwnd less than cubic cwnd */

    sle           c2, d.snd_cwnd, r2
    /* concave or convex , follow cubic cwnd */
    b.c2          tcp_cubic_cwnd

tcp_friendly_cwnd:
    b             tcp_store_max_cwnd
    tblwr         d.snd_cwnd, r5

tcp_cubic_cwnd:
    add.c4        r1,r0,r2
    add.!c4       r6,r0,d.snd_cwnd
    sub.!c4       r2,r2,r6
    mul.!c4       r3,r2,d.smss
    nop
    div.!c4       r5, r3,r6
    nop
    add.!c4       r1,r5,r6
    tblwr         d.snd_cwnd, r1

tcp_store_max_cwnd:
    seq           c1, d.num_cong_events, r0
    slt           c2, d.max_cwnd, d.snd_cwnd
    setcf         c3,[c1 & c2]
cubic_ack_rec_end:
    j             tcp_rx_cc_stage_end
    tblwr.c3      d.max_cwnd, r1         


/* tcp cubic congest signal */
tcp_cc_enter_cubic_fast_recovery:

    sne             c1, d.cc_flags, 0
    b.c1            tcp_cc_enter_fast_recovery_done

    seq             c2,d.num_cong_events, r0
    srl.c2          r1,d.snd_cwnd,1
    mul.!c2         r2,d.snd_cwnd,CUBIC_BETA
    srl.!c2         r1,r2,CUBIC_SHIFT
    tblwr           d.snd_ssthresh,r1
    add             r3, d.num_cong_events,1
    tblwr           d.num_cong_events,r3
    add             r2, d.max_cwnd, r0
    tblwr           d.prev_max_cwnd, r2
    tblwr           d.max_cwnd, d.snd_cwnd
tcp_cc_enter_fast_recovery_done:
    j               tcp_rx_cc_stage_end
    tblor           d.cc_flags, TCP_CCF_FAST_RECOVERY

tcp_cc_enter_cong_recovery:
    sne             c1, d.cc_flags, 0
    b.c1            tcp_cc_enter_cong_recovery_done

    seq             c2,d.num_cong_events, r0
    srl.c2          r1,d.snd_cwnd,1
    mul.!c2         r2,d.snd_cwnd,CUBIC_BETA
    srl.!c2         r1,r2,CUBIC_SHIFT
    tblwr           d.snd_ssthresh,r1
    add             r3, d.num_cong_events,1
    tblwr        d.num_cong_events,r3
    add             r2, d.max_cwnd, r0
    tblwr           d.prev_max_cwnd, r2
    tblwr           d.max_cwnd, d.snd_cwnd
    tblwr           d.snd_cwnd, r1
    tblwr           d.t_last_cong, r4
tcp_cc_enter_cong_recovery_done:
    j               tcp_rx_cc_stage_end
    tblor           d.cc_flags, TCP_CCF_CONG_RECOVERY


/* post-recovery ie exit fast-recovery */
//todo compute the pipe
//todo can be done for all cong types, currently only fast retrans
tcp_cc_cubic_post_recovery:
    /* Fast convergence heuristic */
    slt             c1, d.max_cwnd,d.prev_max_cwnd
    add.c1          r1, d.max_cwnd, r0
    mul.c1          r2, r1, CUBIC_FC_FACTOR
    srl.c1          r3, r2, CUBIC_SHIFT
    tblwr.c1        d.max_cwnd, r3
    /*  fastrecovery post recovery needs pipe*/
    tblwr           d.snd_cwnd, d.snd_ssthresh
    tbland          d.t_flags, ~TCPHDR_CWR
    j               tcp_rx_cc_stage_end
    tblwr           d.cc_flags, 0
    /* all cong types */
    tblwr           d.t_last_cong, r4
    /* calc avg RTT between cong epochs */
    add             r1, d.epoch_ack_count,r0
    sle             c1, r0, r1
    sle             c2, r1, d.sum_rtt_ticks
    setcf           c3, [c1 & c2]
    div.c3          r2, d.sum_rtt_ticks, d.epoch_ack_count
    tblwr.c3        d.mean_rtt_ticks, r2
    
    div             r1, d.max_cwnd,d.smss  //for K
    tblwr           d.epoch_ack_count,0
    tblwr           d.sum_rtt_ticks,0

    /* calc K */

    /* (wmax * beta)/C with CUBIC_SHIFT worth of precision. */
    mul             r2, r1, ONE_SUB_CUBIC_BETA, CUBIC_SHIFT
    srl             r3, r2,7
    srl             r2, r2,9
    add             r1, r3,r2 //s == r1

    /* rebase s to be between 1 and 1/8 with shift of CUBIC_SHIFT */

    srl             r7, r1,8
    clz             r2, r7
    sub             r3, 64, r2  //r3 count of bits to be shifted
    mod             r7, r3, 8   
    seq             c2, r7, r0
    div             r7, r3, 8
    xor             r2, r0, r0 //init p
    add.c2          r2, r7,1
    add.!c2         r2, r7,r0  //r2 is p
    div             r3, r1,r2,3  //r3 contains rebased s
cubic_K_calc:
    /*
     * Some magic constants taken from the Apple TR with appropriate
     * shifts: 275 == 1.072302 << CUBIC_SHIFT, 98 == 0.3812513 <<
     * CUBIC_SHIFT, 120 == 0.46946116 << CUBIC_SHIFT.
     */
    mul             r1, r3, 275
    mul             r4, r3, r3
    srl             r5, r1, CUBIC_SHIFT
    mul             r6, r5, 120
    add             r1, r5, 98
    srl             r4, r6, CUBIC_SHIFT
    sub             r6, r1, r4
    srl             r1, r6, CUBIC_SHIFT
    sll             r3, r1,r2
    tblwr           d.cubic_K, r3
    j               tcp_rx_cc_stage_end
    tblwr           d.cc_flags, 0

/* enter dupack fast recovery */
tcp_cc_dupack_fast_recovery:
    // inflate cwnd by 1 mss
    j               tcp_rx_cc_stage_end
    tbladd          d.snd_cwnd, d.smss

    
tcp_cc_cubic_rto_event:
    // r3 = min(cwnd, awnd)
    sll             r3, k.to_s4_snd_wnd, d.snd_wscale
    slt             c1, d.snd_cwnd, r3
    add.c1          r3, r0, d.snd_cwnd

    // snd_ssthresh = max(min(cwnd, awnd)/2, 2*SMSS)
    srl             r1, r3, 1
    sll             r2, d.smss, 1
    slt             c1, r1, r2
    add.c1          r1, r0, r2
    tblwr           d.snd_ssthresh, r1

    // snd_cwnd = 1 MSS
    tblwr           d.snd_cwnd, d.smss
    tblwr           d.t_last_cong, r4

    jr              r7
    // exit recovery
    tblwr           d.cc_flags, 0

/*
 * Slow Start
 */
cubic_do_reno_ss:
    // Without ABC, increment cwnd by 1 mss for every ack
    seq             c1, d.abc_l_var, 0
    b.c1            tcp_cc_new_reno_slow_start_incr_cwnd
    add.c1          r1, r0, d.smss
    mul             r2, d.smss, d.abc_l_var
   

    // ABC  Appropriate Byte Counting
    // incr (r1) = min(bytes_this_ack, l * smss)
    // TODO : should use l=1, immediately after timeout
    add             r1, r0, k.to_s4_bytes_acked
    slt             c1, r2, r1
    add.c1          r1, r0, r2
    tblwr           d.abc_bytes_acked, 0

tcp_cc_new_reno_slow_start_incr_cwnd:
    // cwnd (r1) = min(cwnd + incr, max_win)
    add             r1, d.snd_cwnd, r1
    slt             c1, d.max_win, r1
    add.c1          r1, r0, d.max_win
    j               tcp_rx_cc_stage_end
    tblwr           d.snd_cwnd, r1

