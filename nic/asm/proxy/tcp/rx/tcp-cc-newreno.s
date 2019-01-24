/*
 *  Implements the RTT stage of the RxDMA P4+ pipeline
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
struct s4_t0_tcp_rx_tcp_cc_d d;


%%
    .param          tcp_rx_cc_stage_end
    .align
tcp_cc_new_reno:
    /*
     * RTO event occurred in Tx pipeline. Handle that first
     */
    seq             c1, k.s1_s2s_cc_rto_signal, 1
    bal.c1          r7, tcp_cc_new_reno_rto_event
    nop

    /*
     * Regular ack received when not in recovery
     */
    seq             c1, k.to_s4_cc_flags, 0
    seq             c2, k.to_s4_cc_ack_signal, TCP_CC_ACK_SIGNAL
    seq             c3, d.cc_flags, 0
    bcf             [c1 & c2 & c3], tcp_cc_new_reno_ack_recvd

    /*
     * Exit fast_recovery
     */
    bcf             [c1 & c2 & !c3], tcp_cc_exit_fast_recovery

    /*
     * Entering recovery due to fast retransmit
     */
    smeqb           c4, d.cc_flags, TCP_CCF_FAST_RECOVERY, TCP_CCF_FAST_RECOVERY
    seq             c6, k.to_s4_cc_ack_signal, TCP_CC_DUPACK_SIGNAL
    bcf             [c6 & !c4], tcp_cc_enter_fast_recovery

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

/*
 * Handle regular ack received
 * (slow start or congestion avoidance)
 */
tcp_cc_new_reno_ack_recvd:
    slt             c1, d.snd_ssthresh, d.snd_cwnd
    b.c1            tcp_cc_new_reno_cong_avoid
    nop

/*
 * Slow Start
 */
tcp_cc_new_reno_slow_start:
    // Without ABC, increment cwnd by 1 mss for every ack
    seq             c1, d.abc_l_var, 0
    b.c1            tcp_cc_new_reno_slow_start_incr_cwnd
    add.c1          r1, r0, d.smss

    // ABC  Appropriate Byte Counting
    // incr (r1) = min(bytes_this_ack, l * smss)
    // TODO : should use l=1, immediately after timeout
    add             r1, r0, k.to_s4_bytes_acked
    slt             c1, d.smss_times_abc_l, r1
    add.c1          r1, r0, d.smss_times_abc_l
    tblwr           d.abc_bytes_acked, 0

tcp_cc_new_reno_slow_start_incr_cwnd:
    // cwnd (r1) = min(cwnd + incr, max_win)
    add             r1, d.snd_cwnd, r1
    slt             c1, d.max_win, r1
    add.c1          r1, r0, d.max_win
    j               tcp_rx_cc_stage_end
    tblwr           d.snd_cwnd, r1

/*
 * Congestion Avoidance
 */
tcp_cc_new_reno_cong_avoid:
    seq             c1, d.abc_l_var, 0
    b.c1            tcp_cc_new_reno_cong_avoid_skip_abc
tcp_cc_new_reno_cong_abc:
    // r1 = min(bytes_this_ack, l * mss)
    add             r1, r0, k.to_s4_bytes_acked
    slt             c1, d.smss_times_abc_l, r1
    add.c1          r1, r0, d.smss_times_abc_l
    tbladd          d.abc_bytes_acked, r1

    // if abc_bytes_acked > snd_cwnd, incr = smss
    // else incr = 0 and exit
    slt             c1, d.snd_cwnd, d.abc_bytes_acked
    j.!c1           tcp_rx_cc_stage_end
    add.c1          r1, r0, d.smss
    b               tcp_cc_new_reno_cong_avoid_incr_cwnd
    tblsub          d.abc_bytes_acked, d.snd_cwnd
    
tcp_cc_new_reno_cong_avoid_skip_abc:
    // incr (r1) = max((smss * smss / snd_cwnd), 1);
    div             r1, d.smss_squared, d.snd_cwnd
    slt             c1, r1, 1
    add.c1          r1, r0, 1

tcp_cc_new_reno_cong_avoid_incr_cwnd:
    // cwnd (r1) = min(cwnd + incr, max_win)
    add             r1, r1, d.snd_cwnd
    slt             c1, d.max_win, r1
    add.c1          r1, r0, d.max_win
    j               tcp_rx_cc_stage_end
    tblwr           d.snd_cwnd, r1

/*
 * Enter fast recovery
 */
tcp_cc_enter_fast_recovery:
    sne             c1, d.cc_flags, 0
    b.c1            tcp_cc_enter_fast_recovery_done
    // snd_cwnd = max(cwnd/2, 2*SMSS)
    srl             r1, d.snd_cwnd, 1
    sll             r2, d.smss, 1
    slt             c1, r1, r2
    add.c1          r1, r0, r2
    tblwr           d.snd_ssthresh, r1
    // inflate snd_cwnd
    mul             r2, d.smss, TCP_FASTRETRANS_THRESH
    add             r1, r1, r2
    tblwr           d.snd_cwnd, r1
    tblor           d.t_flags, TCPHDR_CWR
tcp_cc_enter_fast_recovery_done:
    j               tcp_rx_cc_stage_end
    tblor           d.cc_flags, TCP_CCF_FAST_RECOVERY

/*
 * Enter cong recovery
 */
tcp_cc_enter_cong_recovery:
    sne             c1, d.cc_flags, 0
    b.c1            tcp_cc_enter_cong_recovery_done
    // snd_cwnd = max(cwnd/2, 2*SMSS)
    srl             r1, d.snd_cwnd, 1
    sll             r2, d.smss, 1
    slt             c1, r1, r2
    add.c1          r1, r0, r2
    tblwr           d.snd_ssthresh, r1
    tblwr           d.snd_cwnd, r1
    tblor           d.t_flags, TCPHDR_CWR
tcp_cc_enter_cong_recovery_done:
    j               tcp_rx_cc_stage_end
    tblor           d.cc_flags, TCP_CCF_CONG_RECOVERY

/*
 * Exit fast recovery
 */
tcp_cc_exit_fast_recovery:
    tblwr           d.snd_cwnd, d.snd_ssthresh
    tbland          d.t_flags, ~TCPHDR_CWR
    j               tcp_rx_cc_stage_end
    tblwr           d.cc_flags, 0

/*
 * dupack in fast recovery
 */
tcp_cc_dupack_fast_recovery:
    // inflate cwnd by 1 mss
    j               tcp_rx_cc_stage_end
    tbladd          d.snd_cwnd, d.smss

tcp_cc_new_reno_rto_event:
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

    jr              r7
    // exit recovery
    tblwr           d.cc_flags, 0
