/*
 *    Implements the TX stage of the TxDMA P4+ pipeline
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
struct s5_t0_tcp_tx_xmit_d d;

%%
    .align
    .param          tcp_tso_process_start

    /*
     * r4 = current timestamp. Do not use this register
     */

tcp_xmit_process_start:
    smeqb           c1, k.common_phv_rx_flag, FLAG_SND_UNA_ADVANCED, FLAG_SND_UNA_ADVANCED
    bcf             [c1], tcp_tx_xmit_snd_una_update

    seq             c1, k.common_phv_pending_rto, 1
    bcf             [c1], tcp_tx_retransmit

tcp_tx_enqueue:
    seq             c1, k.common_phv_pending_sesq, 1
    seq.!c1         c1, k.common_phv_pending_asesq, 1

    /* check SESQ for pending data to be transmitted */
    bal.c1          r7, tcp_init_xmit
    nop

    seq             c1, k.common_phv_fin, 1
    bal.c1          r7, tcp_tx_handle_fin
    nop

    /*
     * For RTO case, snd_nxt is snd_una, initialize snd_nxt for other cases
     */
    seq             c1, k.t0_s2s_snd_nxt, r0
    phvwr.c1        p.t0_s2s_snd_nxt, d.snd_nxt

#if 0
    /* Check if there is retx q cleanup needed at head due
     * to ack
     */
    slt             c1, d.retx_snd_una, k.common_phv_snd_una
    bal.c1          r7, tcp_clean_retx_queue
    nop
#endif

    /* Check if cwnd allows transmit of data */
    /* Return value in r6 */
    /* cwnd permits transmit of data if r6 != 0*/
    bal             r7, tcp_cwnd_test
    nop
    sne             c1, r6, r0

    /* Check if peer rcv wnd allows transmit of data
     * Return value in r6
     * Peer rcv wnd allows transmit of data if r6 != 0
     */
    bal.c1          r7, tcp_snd_wnd_test
    nop
    sne             c2, r6, r0

    seq             c3, k.common_phv_pending_ack_send, 1
    bcf             [c3], table_read_TSO
    nop

    bcf             [!c1 | !c2], tcp_tx_end_program
    nop
    /* Inform TSO stage following later to check for any data to
     * send from retx queue
     */
    tblwr           d.pending_tso_data, 1
    phvwri          p.to_s6_pending_tso_data, 1

flow_read_xmit_cursor_start:
    /* Get the point where we are supposed to read from */
    seq             c1, d.xmit_cursor_addr, r0
    bcf             [c1], flow_read_xmit_cursor_done
    add             r1, d.xmit_cursor_addr, r0

    // TODO : r1 needs to be capped by the window size
    add             r1, d.xmit_len, r0
    phvwr           p.to_s6_xmit_cursor_addr, d.xmit_cursor_addr
    phvwr           p.to_s6_xmit_cursor_len, r1
    tbladd          d.snd_nxt, r1

rearm_rto:
    CAPRI_OPERAND_DEBUG(k.t0_s2s_rto)

    /*
     * r1 = rto
     *    = min(rto << backoff, TCP_RTO_MAX)
     *
     * TODO: rto_backoff needs to be reset upon indication from rx
     * pipeline (rx2tx_extra_pending_reset_backoff)
     */
    add             r1, r0, k.t0_s2s_rto
    sll             r1, r1, d.rto_backoff
    slt             c1, r1, TCP_RTO_MAX
    add.!c1         r1, r0, TCP_RTO_MAX

    // TODO : use slow timer just for testing purposes
    // result will be in r3
    CAPRI_TIMER_DATA(0, k.common_phv_fid, TCP_SCHED_RING_RTO, r1)

    // TODO : using slow timer just for testing
    addi            r5, r0, CAPRI_SLOW_TIMER_ADDR(LIF_TCP)
    seq             c1, k.common_phv_debug_dol_dont_start_retx_timer, 1
    memwr.dx.!c1    r5, r3
    tbladd          d.rto_pi, 1

    // TODO : this needs to account for TSO, for now assume one packet
    tbladd          d.packets_out, 1

    /*
     * TODO: init is_cwnd_limited and call tcp_cwnd_validate
     */
     tblwr          d.is_cwnd_limited, 1

    // TODO : Lot of stuff to do here:
    //      if window size is smaller, move xmit_cursor_addr by appropriate amount
    //      if we have more data than one descriptor, adjust next pointer and
    //      schedule ourselves again to send more data
    // TODO : right now code assumes we always send
    //        whatever is posted to sesq, so set cursor_addr back to 0
    tblwr           d.xmit_cursor_addr, r0
    phvwr           p.tx2rx_snd_nxt, d.snd_nxt

table_read_TSO:
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN,
                        tcp_tso_process_start, k.common_phv_qstate_addr,
                        TCP_TCB_TSO_OFFSET, TABLE_SIZE_512_BITS)
    nop.e
    nop

flow_read_xmit_cursor_done:
    CAPRI_CLEAR_TABLE_VALID(0)
    nop.e
    nop



tcp_cwnd_test:
    /* in_flight = packets_out - (sacked_out + lost_out) + retrans_out
    */
    /* r1 = packets_out + retrans_out */
    add             r1, d.packets_out, d.retrans_out
    /* r2 = left_out = sacked_out + lost_out */
    add             r2, d.sacked_out, d.lost_out
    /* r1 = in_flight = (packets_out + retrans_out) - (sacked_out + lost_out)*/
    sub             r1, r1, r2
    /* c1 = (in_flight >= cwnd) */
    sle             c1, k.to_s5_snd_cwnd, r1
    bcf             [c1], tcp_cwnd_test_done
    /* no cwnd remaining */
    addi.c1         r6, r0, 0
    /* r6 = snd_cwnd >> 1 */
    add             r6, k.to_s5_snd_cwnd, r0
    srl             r6, r6, 1

    addi            r5, r0, 1
    slt             c2, r6,r5
    add.c2          r6, r5, r0
    /* at this point r6 = max(cwnd >> 1, 1) */
    /* r5 = cwnd - in_flight */
    sub             r5, k.to_s5_snd_cwnd, r1
    slt             c2, r5, r6
    add.c2          r6, r5, r0
    /* At this point r6 = min((max(cwnd >>1, 1) , (cwnd - in_flight)) */
tcp_cwnd_test_done:
    sne             c4, r7, r0
    jr.c4           r7
    add             r7, r0, r0

tcp_snd_wnd_test:
    /* r1 = bytes_sent = snd_nxt - snd_una */
    sub             r1, d.snd_nxt, k.common_phv_snd_una
    /* r1 = snd_wnd - bytes_sent */
    sub             r1, k.t0_s2s_snd_wnd, r1
    /* r1 = (snd_wnd - bytes_sent) / mss_cache */
    add             r5, k.to_s5_rcv_mss_shft, r0
    srlv            r6, r1, r5
    sne             c4, r7, r0
    jr.c4           r7
    add             r7, r0, r0



tcp_init_xmit:
    seq             c1, d.xmit_cursor_addr, r0
    jr.!c1          r7
    nop
    tblwr           d.xmit_cursor_addr, k.to_s5_addr
    tblwr           d.xmit_offset, k.to_s5_offset
    tblwr           d.xmit_len, k.to_s5_len
    jr              r7
    nop

tcp_tx_handle_fin:
    seq             c1, k.to_s5_state, TCP_ESTABLISHED
    seq             c2, k.to_s5_state, TCP_CLOSE_WAIT
    setcf           c1, [c1 | c2]
    tbladd.c1       d.snd_nxt, 1
    phvwr           p.tx2rx_fin_sent, 1
    jr              r7
    nop

tcp_clean_retx_queue:
    // TODO: schedule ourselves to clean retx
    jr              r7
    nop

tcp_tx_xmit_snd_una_update:
    tblsub          d.packets_out, k.t0_s2s_packets_out_decr
    tbladd          d.rto_pi, k.t0_s2s_rto_pi_incr

tcp_tx_end_program:
    // We have no window, wait till window opens up
    // or we are only running congestion control algorithm,
    // so no more stages
    CAPRI_CLEAR_TABLE_VALID(0)
    nop.e
    nop

tcp_tx_retransmit:
    seq             c1, d.rto_pi, k.t0_s2s_rto_pi
    b.!c1           tcp_tx_end_program // old timer, ignore it
    b               rearm_rto
    tbladd          d.rto_backoff, 1
