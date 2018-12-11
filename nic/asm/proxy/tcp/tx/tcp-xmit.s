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
#include "INGRESS_s5_t0_tcp_tx_k.h"

struct phv_ p;
struct s5_t0_tcp_tx_k_ k;
struct s5_t0_tcp_tx_xmit_d d;

%%
    .align
    .param          tcp_tso_process_start

    /*
     * r4 = current timestamp. Do not use this register
     */

#define c_sesq c5
#define c_snd_una c6
#define c_retval c7

#define r_linkaddr r7

tcp_xmit_process_start:
    seq             c_sesq, k.common_phv_pending_sesq, 1
    seq.!c_sesq     c_sesq, k.common_phv_pending_asesq, 1
    sne             c_snd_una, k.common_phv_pending_retx_cleanup, 0

    seq             c1, k.t0_s2s_state, TCP_RST
    bcf             [c1 & !c_snd_una], tcp_tx_end_program_and_drop

    bcf             [c_snd_una], tcp_tx_xmit_snd_una_update

    bbeq            k.common_phv_pending_rto, 1, tcp_tx_retransmit

    bbeq            k.common_phv_pending_fast_retx, 1, rearm_rto

tcp_tx_enqueue:
    /*
     * If we have no window, bail out
     */
    seq             c1, d.no_window, 1
    seq             c2, k.to_s5_window_open, 1
    bcf             [c1 & !c2], tcp_tx_end_program_and_drop
    nop
    tblwr           d.no_window, 0

    phvwr           p.t0_s2s_snd_nxt, d.snd_nxt

    seq             c1, k.common_phv_fin, 1
    bal.c1          r_linkaddr, tcp_tx_handle_fin
    nop

    /* Check if cwnd allows transmit of data
     * Return value in c_retval
     */
    bal             r_linkaddr, tcp_cwnd_test
    nop
    b.!c_retval     tcp_tx_no_window
    nop

    /* 
     * Check if peer rcv wnd allows transmit of data.
     * Return value in c_retval
     */
    bal             r_linkaddr, tcp_snd_wnd_test
    nop
    b.!c_retval     tcp_tx_no_window
    nop

    /* Inform TSO stage following later to check for any data to
     * send from retx queue
     */
    phvwri          p.to_s6_pending_tso_data, 1

flow_read_xmit_cursor_start:
    /* Get the point where we are supposed to read from */
    seq             c1, k.t0_s2s_addr, r0
    bcf             [c1], flow_read_xmit_cursor_done

    // TODO : r1 needs to be capped by the window size
    add             r1, k.t0_s2s_len, r0

    tbladd          d.snd_nxt, r1

rearm_rto:
#ifndef HW
    bbeq            k.common_phv_debug_dol_dont_start_retx_timer, 1, rearm_rto_done
#endif

    CAPRI_OPERAND_DEBUG(k.to_s5_rto)

    /*
     * r2 = rto
     *    = min(rto << backoff, TCP_RTO_MAX)
     */
    sll             r2, k.to_s5_rto, d.rto_backoff
    slt             c1, r2, TCP_RTO_MAX_TICK
    add.!c1         r2, r0, TCP_RTO_MAX_TICK

    add             r1, k.common_phv_qstate_addr, TCP_TCB_RX2TX_RTO_OFFSET
    memwr.h         r1, r2

rearm_rto_done:

    tbladd.c_sesq   d.packets_out, 1

    /*
     * TODO: init is_cwnd_limited and call tcp_cwnd_validate
     */
    tblwr           d.is_cwnd_limited, 1

    phvwr           p.tx2rx_snd_nxt, d.snd_nxt

table_read_TSO:
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN,
                        tcp_tso_process_start, k.common_phv_qstate_addr,
                        TCP_TCB_TSO_OFFSET, TABLE_SIZE_512_BITS)
    nop.e
    nop

flow_read_xmit_cursor_done:
    phvwri          p.p4_intr_global_drop, 1
    CAPRI_CLEAR_TABLE_VALID(0)
    nop.e
    nop



tcp_cwnd_test:
    // TODO 
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
    // TODO
    // for now unconditionally return cwnd available
    setcf           c_retval, [c0]
    jr              r_linkaddr
    nop

/*
 * Return snd_wnd available
 *
 * r1 = snd_nxt + len
 * r2 = snd_una + snd_wnd
 * return (r1 <= r2)
 */
tcp_snd_wnd_test:
    add             r1, d.snd_nxt, k.t0_s2s_len
    sll             r2, k.t0_s2s_snd_wnd, d.snd_wscale
    add             r2, r2, k.common_phv_snd_una
    scwle           c_retval, r1, r2
    jr              r_linkaddr
    nop


tcp_tx_handle_fin:
    seq             c1, k.t0_s2s_state, TCP_ESTABLISHED
    seq             c2, k.t0_s2s_state, TCP_CLOSE_WAIT
    setcf           c1, [c1 | c2]
    tbladd.c1       d.snd_nxt, 1
    phvwr           p.tx2rx_fin_sent, 1
    jr              r_linkaddr
    nop

tcp_tx_xmit_snd_una_update:
    /*
     * reset rto_backoff if snd_una advanced
     * TODO : We should reset backoff to 0 only if we receive an 
     * ACK for a packet that was not retransmitted. We can piggyback
     * this logic to RTT when that is implemented. For now reset it 
     * for all acks that advance snd_una
     */
    tblwr           d.rto_backoff, 0

    tblssub         d.packets_out, k.t0_s2s_packets_out_decr
    seq             c1, d.packets_out, 0
    // cancel retx timer if packets_out == 0
    add.c1          r1, k.common_phv_qstate_addr, TCP_TCB_RX2TX_RTO_OFFSET
    memwr.h.c1      r1, 0

tcp_tx_end_program:
    // We have no window, wait till window opens up
    // or we are only running congestion control algorithm,
    // so no more stages
    CAPRI_CLEAR_TABLE_VALID(0)
    nop.e
    nop

tcp_tx_end_program_and_drop:
    phvwri          p.p4_intr_global_drop, 1
    CAPRI_CLEAR_TABLE_VALID(0)
    nop.e
    nop

tcp_tx_retransmit:
    b               rearm_rto
    tbladd          d.rto_backoff, 1

tcp_tx_no_window:
    add             r1, k.common_phv_qstate_addr, TCP_TCB_RX2TX_TX_CI_OFFSET
    // Bail out, but remember the current ci in stage 0 CB
    tblwr           d.no_window, 1
    b               tcp_tx_end_program_and_drop
    memwr.h         r1, k.to_s5_sesq_tx_ci
