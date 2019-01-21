/*
 *  TCP ACK processing (ack received from peer)
 */


#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp-constants.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s2_t0_tcp_rx_k.h"

struct phv_ p;
struct s2_t0_tcp_rx_k_ k;
struct s2_t0_tcp_rx_tcp_ack_d d;

%%
    .align
    .param          tcp_rx_rtt_start

#define c_est c6
#define c_snd_una_advanced c4

tcp_ack_slow:
#ifdef CAPRI_IGNORE_TIMESTAMP
    add             r4, r0, r0
    add             r6, r0, r0
#endif

    /*
     * if RTO event occurred in Tx pipeline. Handle that first
     */
    seq             c1, k.s1_s2s_cc_rto_signal, 1
    bal.c1          r7, tcp_ack_rto_event
    nop

    /*
     * check for dupack
     */
    sne             c_snd_una_advanced, k.s1_s2s_ack_seq, d.snd_una
    smneb           c2, k.to_s2_flag, FLAG_DATA, FLAG_DATA
    seq             c5, d.snd_wnd, k.to_s2_window
    /*
     * RFC 5681: consider it as a dup_ack only if window
     * has not changed
     */

    bcf             [c_est & c5 & !c_snd_una_advanced & c2], tcp_dup_ack
    nop

    bcf             [c_est & c5 & !c_snd_una_advanced], dup_ack
    nop

    phvwr           p.to_s4_cc_ack_signal, TCP_CC_ACK_SIGNAL

    /*
     * Check for ECE bit
     */
    smeqb           c1, k.to_s2_flag, FLAG_ECE, FLAG_ECE
    bal.c1          r7, tcp_ack_ece
    nop

    bcf             [c_est], tcp_ack_established_slow

tcp_ack_not_established:
    sne             c2, k.s1_s2s_snd_nxt, k.s1_s2s_ack_seq
    b.c2            tcp_ack_slow_launch_next_stage
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
     */
    scwlt           c1, k.s1_s2s_ack_seq, d.snd_una
    b.c1            old_ack
    nop

    /* If the ack includes data we haven't sent yet, discard
     * this segment (RFC793 Section 3.9).
     *
     *  if (after(ack, tp->snd_nxt))
     *          goto invalid_ack;
     *
     */
    scwlt           c1, k.s1_s2s_snd_nxt, k.s1_s2s_ack_seq
    bcf             [c1], invalid_ack
    nop

    /* ts_recent update must be made after we are sure that the packet
     * is in window.
     *
     *  if (flag & FLAG_UPDATE_TS_RECENT)
     *          tcp_replace_ts_recent(tp, TCP_SKB_CB(skb)->seq);
     *
     */
    smeqb           c2, k.to_s2_flag, FLAG_UPDATE_TS_RECENT, FLAG_UPDATE_TS_RECENT
    b.!c2           tcp_replace_ts_recent_end
tcp_replace_ts_recent:
    // TODO : implement this
    //sne             c1, r7, r0
    add             r7, r0, r0
tcp_replace_ts_recent_end:
    b.!c_snd_una_advanced tcp_snd_una_update_end
tcp_update_window_bypass:
tcp_snd_una_update_slow:
    /* tcp_snd_una_update */
    /* Increment bytes acked by the delta between ack_seq and snd_una */
    sub             r3, k.s1_s2s_ack_seq, d.snd_una
bytes_acked_stats_update_start:
    CAPRI_STATS_INC(bytes_acked, r3[31:0], d.bytes_acked, p.to_s7_bytes_acked)
bytes_acked_stats_update_end:
    /* Update snd_una */
    tblwr           d.snd_una, k.s1_s2s_ack_seq

    phvwr           p.to_s4_bytes_acked, r3[31:0]
    /*
     * If we are in recovery, check if we can come out of it
     */
    seq             c1, d.cc_flags, TCP_CCF_FAST_RECOVERY
    sle             c2, d.snd_recover, k.s1_s2s_ack_seq
    setcf           c3, [c1 & c2]
    tblwr.c3        d.snd_recover, 0
    tblwr.c3        d.num_dup_acks, 0
    tblwr.c3        d.cc_flags, 0
    /*
     * Check for partial ack (ack_seq < snd_recover)
     * Force fast retransmission for partial acks
     */
    setcf           c3, [c1 & !c2]
    phvwrmi.c3      p.common_phv_pending_txdma, TCP_PENDING_TXDMA_FAST_RETRANS, \
                        TCP_PENDING_TXDMA_FAST_RETRANS
    phvwrpair.c3    p.to_s4_cc_ack_signal, TCP_CC_PARTIAL_ACK_SIGNAL, \
                        p.to_s4_cc_flags, d.cc_flags
    phvwr.!c3       p.to_s4_cc_flags, d.cc_flags

tcp_snd_una_update_end:
    /*
     * tell txdma we have work to do
     */
    phvwrmi         p.common_phv_pending_txdma, TCP_PENDING_TXDMA_SND_UNA_UPDATE, \
                        TCP_PENDING_TXDMA_SND_UNA_UPDATE

tcp_in_ack_event_end:
    tblwr           d.snd_wnd, k.to_s2_window
    phvwr           p.to_s4_snd_wnd, k.to_s2_window
    phvwr           p.common_phv_snd_una, d.snd_una
    /*
     * Launch next stage
     */

dup_ack:
old_ack:
invalid_ack:
no_queue:
tcp_ack_slow_launch_next_stage:
    phvwrpair       p.rx2tx_extra_snd_wnd, k.to_s2_window, \
                        p.rx2tx_extra_snd_una, d.snd_una
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN,
                        tcp_rx_rtt_start,
                        k.common_phv_qstate_addr,
                        TCP_TCB_RTT_OFFSET, TABLE_SIZE_512_BITS)
    nop.e
    nop

tcp_dup_ack:
    tbladd          d.num_dup_acks, 1

    slt             c1, d.num_dup_acks, TCP_FASTRETRANS_THRESH
    b.c1            tcp_dup_ack_done

    slt             c1, TCP_FASTRETRANS_THRESH, d.num_dup_acks
    b.c1            tcp_dup_ack_gt_thresh
    nop
tcp_dup_ack_eq_thresh:
    tblor           d.cc_flags, TCP_CCF_FAST_RECOVERY
    tblwr           d.snd_recover, k.s1_s2s_snd_nxt
    phvwrpair       p.to_s4_cc_ack_signal, TCP_CC_DUPACK_SIGNAL, \
                        p.to_s4_cc_flags, d.cc_flags
    b               tcp_dup_ack_done
    /*
     * tell txdma we have work to do
     */
    phvwrmi         p.common_phv_pending_txdma, TCP_PENDING_TXDMA_FAST_RETRANS, \
                        TCP_PENDING_TXDMA_FAST_RETRANS
tcp_dup_ack_gt_thresh:
    b               tcp_dup_ack_done
    phvwrpair       p.to_s4_cc_ack_signal, TCP_CC_DUPACK_SIGNAL, \
                        p.to_s4_cc_flags, d.cc_flags

tcp_dup_ack_done:
    b               tcp_in_ack_event_end
    nop

/******************************************************************************
 * Functions
 *****************************************************************************/
tcp_ack_rto_event:
    // exit recovery
    tblwr           d.cc_flags, 0
    jr              r7
    tblwr           d.num_dup_acks, 0

tcp_ack_ece:
    // If already in CONG_RECOVERY, exit
    smeqb           c1, d.cc_flags, TCP_CCF_CONG_RECOVERY, TCP_CCF_CONG_RECOVERY
    jr.c1           r7
    nop

    tblor           d.cc_flags, TCP_CCF_CONG_RECOVERY
    tblwr           d.snd_recover, k.s1_s2s_snd_nxt
    jr              r7
    phvwr           p.to_s4_cc_ack_signal, TCP_CC_ECE_SIGNAL

