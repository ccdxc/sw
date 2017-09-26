/*
 *    Implements the TX stage of the TxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp-sched.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct phv_ p;
struct tcp_tx_tcp_tx_k k;
struct tcp_tx_tcp_tx_tcp_tx_d d;

%%
    .align
    .param          tcp_tso_process_start

tcp_tx_process_stage3_start:
    /* check SESQ for pending data to be transmitted */
    seq             c1, k.common_phv_pending_sesq, 1

    //bal.c1        r7, tcp_retxq_consume
    //nop
    bal.c1          r7, tcp_retx_enqueue
    nop

    seq             c1, k.common_phv_pending_ack_send, 1
    bcf             [c1], table_read_TSO
    nop


    /* Check if there is retx q cleanup needed at head due
     * to ack
     */
    slt             c1, d.retx_snd_una, k.common_phv_snd_una
    //bal.c1        r7, tcp_clean_retx_queue
    nop

    /* Check if cwnd allows transmit of data */
    /* Return value in r6 */
    bal             r7, tcp_cwnd_test
    nop
    /* cwnd permits transmit of data if r6 != 0*/
    sne             c1, r6, r0
    /* Check if peer rcv wnd allows transmit of data
     * Return value in r6
     */
    bal.c1          r7, tcp_snd_wnd_test
    nop
    /* Peer rcv wnd allows transmit of data if r6 != 0 */
    sne             c2, r6, r0
    bcf             [!c1 | !c2], tcp_ack_snd_check
    nop
    /* Inform TSO stage following later to check for any data to
     * send from retx queue
     */
    tblwr           d.pending_tso_data, 1
    phvwri          p.to_s4_pending_tso_data, 1

flow_read_xmit_cursor_start:
    /* Get the point where we are supposed to read from */
    add             r2, d.retx_xmit_cursor, r0
    add             r1, r2, r0
    seq             c1, r1, r0
    /* If the retx was all cleaned up , then reinit the xmit
     * cursor to snd_una cursor which is the head of data that
         * can be sent
     */
    add        r2, d.retx_snd_una_cursor, r0
    tblwr.c1    d.retx_xmit_cursor, r2
    /* This nop is needed to make the above table data
     * write visibile for the next instruction below
     */
    nop
    /* Even after all this retx_xmit_cursor has no data, then
     * there is no data to send
     */
    seq        c1, d.retx_xmit_cursor, r0
    bcf        [c1], flow_read_xmit_cursor_done
    nop

    seq        c1, d.xmit_cursor_addr, r0
    bcf        [c1], table_read_xmit_cursor
    nop
table_read_TSO:
        CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN,
                        tcp_tso_process_start, k.common_phv_qstate_addr,
                        TCP_TCB_TX_OFFSET, TABLE_SIZE_512_BITS)
        nop.e
        nop

table_read_xmit_cursor:
    /* Read the xmit cursor if we have zero xmit cursor addr */
    add.c1        r1, d.retx_xmit_cursor, r0

#if 0
    phvwr        p.table_sel, TABLE_TYPE_RAW
    phvwr.c1    p.table_mpu_entry_raw, flow_read_xmit_cursor
    phvwr.!c1    p.table_mpu_entry_raw, flow_tso_process
    phvwr        p.table_addr, r1
#endif

flow_read_xmit_cursor_done:


tcp_ack_snd_check:
    /* r1 = rcv_nxt - rcv_wup */
    sub        r1, k.common_phv_rcv_nxt , d.rcv_wup

    addi        r2, r0, 1
    add        r3, k.to_s3_rcv_mss_shft, r0
    sllv        r2, r2, r3
    /* r2 = rcv_mss */
    /* c1 = ((rcv_nxt - rcv_wup) > rcv_mss) */
    /* c1 = more than one full frame received */
    slt        c1, r2, r1

    addi        r1, r0, 1
    sub        r1, r1, k.to_s3_pingpong
    /* r1 = 1 - pingpong = !pingpong */
    sne        c2, r0, r0
    /* xxx: c2 = (new rcv window >= rcv_wnd) */
    /* r2 = quick && !pingpong */
    and        r2, k.to_s3_quick, r1
    sne        c3, r2, r0
    /* c3 = in quick ack mode */
    /* c4 = we have out of order data */
    sne        c4, k.to_s3_ooo_datalen, r0
    addi         r1, r0, 1
    bcf        [c1 | c2 | c3 | c4], pending_ack_tx
    nop
flow_tx_process_done:
    tblwr.e        d.pending_delayed_ack_tx,r1
    nop.e
pending_ack_tx:
    tblwr.e        d.pending_ack_tx, r1
    nop.e



tcp_cwnd_test:
    /* in_flight = packets_out - (sacked_out + lost_out) + retrans_out
    */
    /* r1 = packets_out + retrans_out */
    add        r1, k.t0_s2s_packets_out, k.t0_s2s_retrans_out
    /* r2 = left_out = sacked_out + lost_out */
    add         r2, k.t0_s2s_sacked_out, k.t0_s2s_lost_out
    /* r1 = in_flight = (packets_out + retrans_out) - (sacked_out + lost_out)*/
    sub         r1, r1, r2
    /* c1 = (in_flight >= cwnd) */
    sle        c1, k.t0_s2s_snd_cwnd, r1
    bcf        [c1], tcp_cwnd_test_done
    /* no cwnd remaining */
    addi.c1        r6, r0, 0
    /* r6 = snd_cwnd >> 1 */
    add        r6, k.t0_s2s_snd_cwnd, r0
    srl        r6, r6, 1

    addi        r4, r0, 1
    slt        c2, r6,r4
    add.c2        r6, r4, r0
    /* at this point r6 = max(cwnd >> 1, 1) */
    /* r5 = cwnd - in_flight */
    sub        r5, k.t0_s2s_snd_cwnd, r1
    slt        c2, r5, r6
    add.c2        r6, r5, r0
    /* At this point r6 = min((max(cwnd >>1, 1) , (cwnd - in_flight)) */
tcp_cwnd_test_done:
    sne        c4, r7, r0
    jr.c4        r7
    add        r7, r0, r0

tcp_snd_wnd_test:
    /* r1 = bytes_sent = snd_nxt - snd_una */
    sub        r1, k.t0_s2s_snd_nxt, k.common_phv_snd_una
    /* r1 = snd_wnd - bytes_sent */
    sub        r1, k.t0_s2s_snd_wnd, r1
    /* r1 = (snd_wnd - bytes_sent) / mss_cache */
    add        r5, k.to_s3_rcv_mss_shft, r0
    srlv        r6, r1, r5
    sne        c4, r7, r0
    jr.c4        r7
    add        r7, r0, r0



tcp_retx_enqueue:
    /* All the previous packets were acked or first packet
         *   retxq tail descriptor is NULL
     * Wait for a new descriptor to be allocated from TNMDR
     */
    seq        c1, d.retx_tail_desc, r0
        sne             c2, k.to_s3_sesq_desc_addr, r0
    bcf        [c1 & c2], queue_to_empty_retx // TODO: what if queue is not empty
        nop
    bcf        [c1], table_read_TNMDR
    nop

queue_to_empty_retx:
        tblwr.c2        d.retx_head_desc, k.to_s3_sesq_desc_addr
        tblwr.c2        d.retx_tail_desc, k.to_s3_sesq_desc_addr
        add             r2, k.to_s3_addr, k.to_s3_offset
        tblwr.c2        d.retx_xmit_cursor, r2
        tblwr.c2        d.xmit_cursor_addr, r2
        phvwr           p.to_s4_xmit_cursor_addr, k.to_s3_addr
        phvwr           p.to_s4_xmit_cursor_offset, k.to_s3_offset
        phvwr           p.to_s4_xmit_cursor_len, k.to_s3_len
    sne        c4, r7, r0
    jr.c4        r7

    /* retxq tail descriptor is not NULL
     * check if the retxq tail descriptor entries are all filled
     * up.
      * Wait for a new descriptor to be allocated from TNMDR
     */
    /* r1 = &retx_tail_desc->entry.0 */
    add        r1, d.retx_tail_desc, NIC_DESC_ENTRY_0_OFFSET
    /* r1 = &retx_tail_desc->entry.nxt_free_idx - &retx_tail_desc->entry.0 */
    sub        r1, d.retx_snd_nxt_cursor, r1
    /* r1 = nxt_free_idx */
    srl        r1, r1, NIC_DESC_ENTRY_SIZE_SHIFT
    /* nxt_free_idx >= MAX_ENTRIES_PER_DESC = descriptor full ? */
    sle        c1, MAX_ENTRIES_PER_DESC, r1
    /* Wait for a new descriptor to be allocaed from TNMDR */
    bcf        [c1], table_read_TNMDR
    nop
    /* This is the fast path, we have a empty slot in the
     * tail descriptor
     */
nic_desc_entry_write:
    /* Write A */
    addi        r2, r0, NIC_DESC_ENTRY_ADDR_OFFSET
    add        r1, d.retx_snd_nxt_cursor, r2
    memwr.w        r1, k.to_s3_addr
    phvwr        p.to_s4_xmit_cursor_addr, k.to_s3_addr
    /* Write O */
    addi        r2, r0,  NIC_DESC_ENTRY_OFF_OFFSET
    add        r1, d.retx_snd_nxt_cursor, r2
    memwr.h        r1, k.to_s3_offset
    phvwr        p.to_s4_xmit_cursor_offset, k.to_s3_offset
    /* Write L */
    addi         r2, r0, NIC_DESC_ENTRY_LEN_OFFSET
    add        r1, d.retx_snd_nxt_cursor, r2
    memwr.h        r1, k.to_s3_len
    phvwr        p.to_s4_xmit_cursor_len, k.to_s3_len
    /* Update retx_snd_nxt_cursor */
    tbladd        d.retx_snd_nxt_cursor, NIC_DESC_ENTRY_SIZE
    sne        c4, r7, r0
    jr.c4        r7
    add        r7, r0, r0

        CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN,
                        tcp_tso_process_start, k.common_phv_qstate_addr,
                        TCP_TCB_TX_OFFSET, TABLE_SIZE_512_BITS)


table_read_TNMDR:
#if 0
    addi        r1,r0,TNMDR_TABLE_BASE
    addi        r2,r0,TNMDR_ALLOC_IDX
    mincr         r2,1,TNMDR_TABLE_SIZE_SHFT
    sll        r2,r2,TNMDR_TABLE_ENTRY_SIZE_SHFT
    add        r1,r1,r2

    phvwri        p.table_sel, TABLE_TYPE_RAW
    phvwri        p.table_mpu_entry_raw, flow_tdesc_alloc_process
    phvwr.e        p.table_addr, r1
    nop.e
#endif
        nop.e
        nop


