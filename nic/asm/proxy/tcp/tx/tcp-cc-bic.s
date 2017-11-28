/*
 *  TCP CC (BIC)
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
struct s5_t0_tcp_tx_k k;
struct s5_t0_tcp_tx_cc_and_xmit_d d;

%%

bictcp_cong_avoid:

tcp_is_cwnd_limited:
    /*
     * c1 = tcp_in_slow_start = snd_cwnd < snd_ssthresh
     */
    slt             c1, d.snd_cwnd, k.t0_s2s_snd_ssthresh

    /*
     *  c2 = tcp_is_cwnd_limited()
     *
     *    if (tcp_in_slow_start(tp))
     *            return tp->snd_cwnd < 2 * tp->max_packets_out;
     *
     *    return tp->is_cwnd_limited;
     */
    add             r2, r0, d.max_packets_out, 2
    slt             c2, d.snd_cwnd, r2

    sne.!c1         c2, d.is_cwnd_limited, r0

    /*
     *   if (!tcp_is_cwnd_limited(sk))
     *           return;
     */
    jr.!c2          r7
    nop

    b.c1            tcp_slow_start
    nop

bictcp_update:

    /*
     * if (ca->last_cwnd == cwnd &&
     *     (s32)(tcp_time_stamp - ca->last_time) <= HZ / 32)
     *         return;
     */
    seq             c1, d.last_cwnd, d.snd_cwnd
    sub             r3, r4, d.last_time
    slt             c2, r3, HZ/32
    bcf             [c1 & c2], bictcp_update_done
    nop

    /*
     * ca->last_cwnd = cwnd;
     * ca->last_time = tcp_time_stamp;
     */
    tblwr           d.last_cwnd, d.snd_cwnd
    tblwr           d.last_time, r4

    /*
     * if (ca->epoch_start == 0) // record the beginning of an epoch
     *         ca->epoch_start = tcp_time_stamp;
     */
    seq             c1, d.epoch_start, r0
    tblwr.c1        d.epoch_start, r4

    /*
     * // start off normal
     * if (cwnd <= low_window) {
     *         ca->cnt = cwnd;
     *         return;
     * }
     */
    sle             c1, d.snd_cwnd, LOW_WINDOW
    tblwr.c1        d.cnt, d.snd_cwnd
    b.c1            bictcp_update_done

    /*
     * // binary increase
     * if (cwnd < ca->last_max_cwnd) {
     *         __u32   dist = (ca->last_max_cwnd - cwnd)
     *                 / BICTCP_B;
     *
     *         if (dist > max_increment)
     *                 // linear increase
     *                 ca->cnt = cwnd / max_increment;
     *         else if (dist <= 1U)
     *                 // binary search increase
     *                 ca->cnt = (cwnd * smooth_part) / BICTCP_B;
     *         else
     *                 // binary search increase
     *                 ca->cnt = cwnd / dist;
     */
    slt             c1, d.snd_cwnd, d.last_max_cwnd
    bcf             [!c1], bictcp_aimd_incr
    nop

bictcp_bin_incr:
    /*
     * r3 = dist
     */
    sub             r3, d.last_max_cwnd, d.snd_cwnd
    srl             r3, r3, BICTCP_B_SHIFT

    /*
     * if (dist > max_increment)
     *      // linear increase
     *      ca->cnt = cwnd / max_increment;
     */
    sle             c1, r3, MAX_INCREMENT
    srl.!c1         r5, d.snd_cwnd, MAX_INCREMENT_SHIFT
    tblwr.!c1       d.cnt, r4
    b.!c1           bictcp_cnt_update_done

    /*
     * else if (dist <= 1)
     *      // binary search increase
     *      (ca->cnt = cwnd * smooth_part) / BICTCP_B
     */
    sle             c1, r3, 1

    mul.c1          r5, d.snd_cwnd, SMOOTH_PART
    srl.c1          r5, r5, BICTCP_B_SHIFT
    tblwr.c1        d.cnt, r5
    b.c1            bictcp_cnt_update_done

    /*
     * else
     *      // binary search increase
     *      ca->cnt = cwnd / dist
     */
    div             r5, d.snd_cwnd, r3
    tblwr           d.cnt, r5
    b               bictcp_cnt_update_done
    nop

bictcp_aimd_incr:
    /*
     * } else {
     *         // slow start AMD linear increase 
     *         if (cwnd < ca->last_max_cwnd + BICTCP_B)
     *                 // slow start
     *                 ca->cnt = (cwnd * smooth_part) / BICTCP_B;
     *         else if (cwnd < ca->last_max_cwnd + max_increment*(BICTCP_B-1))
     *                 // slow start
     *                 ca->cnt = (cwnd * (BICTCP_B-1))
     *                         / (cwnd - ca->last_max_cwnd);
     *         else
     *                 // linear increase
     *                 ca->cnt = cwnd / max_increment;
     * }
     */
    add             r3, d.last_max_cwnd, BICTCP_B
    slt             c1, d.snd_cwnd, r3
    mul.c1          r5, d.snd_cwnd, SMOOTH_PART, BICTCP_B_SHIFT
    tblwr.c1        d.cnt, r5
    b.c1            bictcp_cnt_update_done

    add             r3, d.last_max_cwnd, MAX_INCREMENT * (BICTCP_B-1)
    slt             c1, d.snd_cwnd, r3
    mul.c1          r5, d.snd_cwnd, (BICTCP_B-1)
    sub.c1          r6, d.snd_cwnd, d.last_max_cwnd
    div.c1          r5, r5, r6
    tblwr.c1        d.cnt, r5
    b.c1            bictcp_cnt_update_done

    srl             r5, d.snd_cwnd, MAX_INCREMENT_SHIFT
    tblwr           d.cnt, r5

bictcp_cnt_update_done:
    /*
     * // if in slow start or link utilization is very low
     * if (ca->last_max_cwnd == 0) {
     *         if (ca->cnt > 20) // increase cwnd 5% per RTT
     *                 ca->cnt = 20;
     * }
     *
     * ca->cnt = (ca->cnt << ACK_RATIO_SHIFT) / ca->delayed_ack;
     * if (ca->cnt == 0)                       // cannot be zero
     *         ca->cnt = 1;
     */
    seq             c1, d.last_max_cwnd, r0
    sle             c2, d.cnt, 20
    setcf           c3, [c1 & c2]
    tblwr.c3        d.cnt, 20

    add             r5, d.cnt, r0, ACK_RATIO_SHIFT
    div             r5, r5, d.delayed_ack
    tblwr           d.cnt, r5
    seq             c1, r5, r0
    tbladd.c1       d.cnt, 1
    nop             // delay slot so d.cnt is visible to the next instruction

    /* Fall thru to tcp_cong_avoid_ai */
bictcp_update_done:
tcp_cong_avoid_ai:
    /*
     * // If credits accumulated at a higher w, apply them gently now.
     * if (tp->snd_cwnd_cnt >= w) {
     *         tp->snd_cwnd_cnt = 0;
     *         tp->snd_cwnd++;
     * }
     *
     * tp->snd_cwnd_cnt += acked;
     * if (tp->snd_cwnd_cnt >= w) {
     *         u32 delta = tp->snd_cwnd_cnt / w;
     *
     *         tp->snd_cwnd_cnt -= delta * w;
     *         tp->snd_cwnd += delta;
     * }
     * tp->snd_cwnd = min(tp->snd_cwnd, tp->snd_cwnd_clamp);
     */

    sle             c1, d.cnt, d.snd_cwnd_cnt
    tblwr.c1        d.snd_cwnd_cnt, r0
    tbladd.c1       d.snd_cwnd, 1
    tbladd          d.snd_cwnd_cnt, 1 // acked = 1 passed to tcp_cong_avoid_ai
    nop // delay slot to make the table write visible below

    /*
     * r1 = delta
     * r2 = delta * d.cnt
     *
     * TBD : Why do we need div/mul here, it seems snd_cwnd_cnt is always
     * incremented by 1, so when snd_cwnd_cnt >= w, div operation will always
     * yield 1. So below logic could be simplified as
     * tblwr.c1     d.snd_cwnd_cnt, r0
     * tbladd.c1    d.snd_cwnd, 1
     * FIX later. For now we will use this to verify div/mul operations are
     * working as expected
     */
    sle             c1, d.cnt, d.snd_cwnd_cnt
    div.c1          r1, d.snd_cwnd_cnt, d.cnt
    mul.c1          r2, r1, d.cnt
    tblsub.c1       d.snd_cwnd_cnt, r2
    tbladd.c1       d.snd_cwnd, r1

    slt             c1, d.snd_cwnd_clamp, d.snd_cwnd
    tblwr.c1        d.snd_cwnd, d.snd_cwnd_clamp

flow_cc_process_done:
    jr              r7
    nop

/* Slow start is used when congestion window is no greater than the slow start
 * threshold. We base on RFC2581 and also handle stretch ACKs properly.
 * We do not implement RFC3465 Appropriate Byte Counting (ABC) per se but
 * something bettertcp_slow_start: ;) a packet is only considered (s)acked in its entirety to
 * defend the ACK attacks described in the RFC. Slow start processes a stretch
 * ACK of degree N as if N acks of degree 1 are received back to back except
 * ABC caps N to 2. Slow start exits when cwnd grows over ssthresh and
 * returns the leftover acks to adjust cwnd in congestion avoidance mode.
 */
tcp_slow_start:
    /* u32 tcp_slow_start(tcb_t *tp, u32 acked)
     * {
     *    u32 cwnd = min(tp->cc.snd_cwnd + acked, tp->cc.snd_ssthresh) ;
     *
     *    acked -= cwnd - tp->cc.snd_cwnd ;
     *    tp->cc.snd_cwnd = min(cwnd, tp->cc.snd_cwnd_clamp) ;
     *
     *    return acked      ;
     * }
     */

    /*
     * r3 = cwnd = min(tp->cc.snd_cwnd + acked, tp->cc.snd_ssthresh)
     */
    add         r3, d.snd_cwnd, k.t0_s2s_pkts_acked
    slt         c1, k.t0_s2s_snd_ssthresh, r3
    add.c1      r3, r0, k.t0_s2s_snd_ssthresh

    /*
     * snd_cwnd = min(cwnd, tp->cc.snd_cwnd_clamp)
     */
    slt         c1, d.snd_cwnd_clamp, r3
    add.c1      r3, r0, d.snd_cwnd_clamp

    tblwr       d.snd_cwnd, r3
    jr          r7
    nop


