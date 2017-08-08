/*
 *	Implements the TX stage of the TxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp-sched.h"
	
 /* d is the data returned by lookup result */
struct d_struct {
	TCB_TX_SHARED_STATE
};

/* Readonly Parsed packet header info for the current packet */
struct k_struct {
	fid				: 32 ;
	flags				: 8  ;
	addr				: ADDRESS_WIDTH ;
	offset				: OFFSET_WIDTH ;
	len				: LEN_WIDTH ;
	snd_una				: SEQ_NUMBER_WIDTH	;\
	rcv_nxt				: SEQ_NUMBER_WIDTH	;\
	rcv_mss_shft			: 4	                ;\
	pingpong			: 1                     ;\
	quick				: 4	                ;\
	ooo_datalen			: COUNTER16 	        ;\

        write_seq                       : SEQ_NUMBER_WIDTH      ;\
	snd_nxt				: SEQ_NUMBER_WIDTH	;\
	snd_wnd				: SEQ_NUMBER_WIDTH	;\
	snd_up				: SEQ_NUMBER_WIDTH	;\
        snd_cwnd                        : WINDOW_WIDTH          ;\
	packets_out			: COUNTER16	        ;\
	sacked_out			: COUNTER16	        ;\
	lost_out			: COUNTER8	        ;\
	retrans_out			: COUNTER8	        ;\
};

struct p_struct p	;
struct k_struct k	;
struct d_struct d	;
	
%%

flow_tx_process_start:
	/* check SESQ for pending data to be transmitted */
	smeqb		c1,d.sched_flag, TCP_SCHED_FLAG_PENDING_TX, TCP_SCHED_FLAG_PENDING_TX

	jal.c1		r7, tcp_retxq_consume
	nop
	jal.c1		r7, tcp_retx_enqueue
	nop


	/* Check if there is retx q cleanup needed at head due
	 * to ack
	 */
	slt		c1, d.retx_snd_una, k.snd_una
	jal.c1		r7, tcp_clean_retx_queue
	nop

	/* Check if cwnd allows transmit of data */
	/* Return value in r6 */
	jal		r7, tcp_cwnd_test
	nop
	/* cwnd permits transmit of data if r6 != 0*/
	sne		c1, r6, r0
	/* Check if peer rcv wnd allows transmit of data 
	 * Return value in r6
	 */
	jal.c1		r7, tcp_snd_wnd_test
	nop
	/* Peer rcv wnd allows transmit of data if r6 != 0 */
	sne		c2, r6, r0
	bcf		[!c1 | !c2], tcp_ack_snd_check
	nop
	/* Inform TSO stage following later to check for any data to
	 * send from retx queue
	 */
	tblwr		d.pending_tso_data, 1

flow_read_xmit_cursor_start:
	/* Get the point where we are supposed to read from */
	add		r2, d.retx_xmit_cursor, r0
	add		r1, r2, r0
	seq		c1, r1, r0
	/* If the retx was all cleaned up , then reinit the xmit
	 * cursor to snd_una cursor which is the head of data that
         * can be sent
	 */
	add		r2, d.retx_snd_una_cursor, r0
	tblwr.c1	d.retx_xmit_cursor, r2
	/* This nop is needed to make the above table data
	 * write visibile for the next instruction below
	 */
	nop
	/* Even after all this retx_xmit_cursor has no data, then
	 * there is no data to send
	 */
	seq		c1, d.retx_xmit_cursor, r0
	bcf		[c1], flow_read_xmit_cursor_done
	nop

	seq		c1, d.xmit_cursor_addr, r0
	bcf		[c1], table_read_xmit_cursor
	nop
table_read_TSO:	
	TCP_NEXT_TABLE_READ(k.fid, TABLE_TYPE_RAW, flow_tx_process,
	                    TCP_TCB_TABLE_BASE, TCP_TCB_TABLE_ENTRY_SIZE_SHFT,
	                    TCP_TCB_TX_OFFSET, TCP_TCB_TABLE_ENTRY_SIZE)

table_read_xmit_cursor:
	/* Read the xmit cursor if we have zero xmit cursor addr */
	add.c1		r1, d.retx_xmit_cursor, r0	

	phvwr		p.table_sel, TABLE_TYPE_RAW
	phvwr.c1	p.table_mpu_entry_raw, flow_read_xmit_cursor
	phvwr.!c1	p.table_mpu_entry_raw, flow_tso_process
	phvwr		p.table_addr, r1

flow_read_xmit_cursor_done:

	
tcp_ack_snd_check:
	/* r1 = rcv_nxt - rcv_wup */
	sub		r1, k.rcv_nxt , d.rcv_wup
	
	addi		r2, r0, 1
	add		r3, k.rcv_mss_shft, r0
	sllv		r2, r2, r3
	/* r2 = rcv_mss */
	/* c1 = ((rcv_nxt - rcv_wup) > rcv_mss) */
	/* c1 = more than one full frame received */
	slt		c1, r2, r1

	addi		r1, r0, 1
	sub		r1, r1, k.pingpong
	/* r1 = 1 - pingpong = !pingpong */
	sne		c2, r0, r0
	/* xxx: c2 = (new rcv window >= rcv_wnd) */
	/* r2 = quick && !pingpong */
	and		r2, k.quick, r1
	sne		c3, r2, r0
	/* c3 = in quick ack mode */
	/* c4 = we have out of order data */
	sne		c4, k.ooo_datalen, r0
	addi 		r1, r0, 1
	bcf		[c1 | c2 | c3 | c4], pending_ack_tx
	nop
flow_tx_process_done:
	tblwr.e		d.pending_delayed_ack_tx,r1
	nop.e
pending_ack_tx:
	tblwr.e		d.pending_ack_tx, r1
	nop.e
	


tcp_cwnd_test_start:
	/* in_flight = packets_out - (sacked_out + lost_out) + retrans_out
	*/
	/* r1 = packets_out + retrans_out */
	add		r1, k.packets_out, k.retrans_out
	/* r2 = left_out = sacked_out + lost_out */
	add 		r2, k.sacked_out, k.lost_out
	/* r1 = in_flight = (packets_out + retrans_out) - (sacked_out + lost_out)*/
	sub 		r1, r1, r2
	/* c1 = (in_flight >= cwnd) */
	sle		c1, k.snd_cwnd, r1
	bcf		[c1], tcp_cwnd_test_done
	/* no cwnd remaining */
	addi.c1		r6, r0, 0
	/* r6 = snd_cwnd >> 1 */
	add		r6, k.snd_cwnd, r0
	srl		r6, r6, 1
	
	addi		r4, r0, 1
	slt		c2, r6,r4
	add.c2		r6, r4, r0
	/* at this point r6 = max(cwnd >> 1, 1) */
	/* r5 = cwnd - in_flight */
	sub		r5, k.snd_cwnd, r1
	slt		c2, r5, r6
	add.c2		r6, r5, r0
	/* At this point r6 = min((max(cwnd >>1, 1) , (cwnd - in_flight)) */
tcp_cwnd_test_done:
	sne		c4, r7, r0
	jr.c4		r7
	add		r7, r0, r0

tcp_snd_wnd_test_start:
	/* r1 = bytes_sent = snd_nxt - snd_una */
	sub		r1, k.snd_nxt, k.snd_una
	/* r1 = snd_wnd - bytes_sent */
	sub		r1, k.snd_wnd, r1
	/* r1 = (snd_wnd - bytes_sent) / mss_cache */
	add		r5, k.rcv_mss_shft, r0
	srlv		r6, r1, r5
	sne		c4, r7, r0
	jr.c4		r7
	add		r7, r0, r0

	
	
tcp_retx_enqueue_start:
	/* All the previous packets were acked or first packet
         *   retxq tail descriptor is NULL
	 * Wait for a new descriptor to be allocated from TNMDR 
	 */
	seq		c1, d.retx_tail_desc, r0
	bcf		[c1], table_read_TNMDR
	nop

	/* retxq tail descriptor is not NULL
	 * check if the retxq tail descriptor entries are all filled
	 * up.
 	 * Wait for a new descriptor to be allocated from TNMDR
	 */
	/* r1 = &retx_tail_desc->entry.0 */
	add		r1, d.retx_tail_desc, NIC_DESC_ENTRY_0_OFFSET
	/* r1 = &retx_tail_desc->entry.nxt_free_idx - &retx_tail_desc->entry.0 */
	sub		r1, d.retx_snd_nxt_cursor, r1
	/* r1 = nxt_free_idx */
	srl		r1, r1, NIC_DESC_ENTRY_SIZE_SHIFT
	/* nxt_free_idx >= MAX_ENTRIES_PER_DESC = descriptor full ? */
	sle		c1, MAX_ENTRIES_PER_DESC, r1
	/* Wait for a new descriptor to be allocaed from TNMDR */
	bcf		[c1], table_read_TNMDR
	nop
	/* This is the fast path, we have a empty slot in the
	 * tail descriptor
	 */
nic_desc_entry_write:
	/* Write A */
	addi		r2, r0, NIC_DESC_ENTRY_ADDR_OFFSET
	add		r1, d.retx_snd_nxt_cursor, r2
	memwr.w		r1, k.addr
	phvwr		p.xmit_cursor_addr, k.addr
	/* Write O */
	addi		r2, r0,  NIC_DESC_ENTRY_OFF_OFFSET	
	add		r1, d.retx_snd_nxt_cursor, r2
	memwr.h		r1, k.offset
	phvwr		p.xmit_cursor_offset, k.offset
	/* Write L */
	addi 		r2, r0, NIC_DESC_ENTRY_LEN_OFFSET
	add		r1, d.retx_snd_nxt_cursor, r2
	memwr.h		r1, k.len
	phvwr		p.xmit_cursor_len, k.len
	/* Update retx_snd_nxt_cursor */
	tbladd		d.retx_snd_nxt_cursor, NIC_DESC_ENTRY_SIZE
	sne		c4, r7, r0
	jr.c4		r7
	add		r7, r0, r0


table_read_TNMDR:
	addi		r1,r0,TNMDR_TABLE_BASE
	addi		r2,r0,TNMDR_ALLOC_IDX
	mincr 		r2,1,TNMDR_TABLE_SIZE_SHFT
	sll		r2,r2,TNMDR_TABLE_ENTRY_SIZE_SHFT
	add		r1,r1,r2

	phvwri		p.table_sel, TABLE_TYPE_RAW
	phvwri		p.table_mpu_entry_raw, flow_tdesc_alloc_process
	phvwr.e		p.table_addr, r1
	nop.e

tcp_retxq_consume_start:
	tbladd		d.retx_ci, 1
	/* address will be in r4 */
	CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_CIDX_SET, DB_SCHED_UPD_EVAL, 0, LIF_TCP)
	/* data will be in r3 */
	CAPRI_RING_DOORBELL_DATA(0, k.fid, TCP_SCHED_RING_DATA, d.retx_ci)
	memwr.d		r4, r3
	sne		c4, r7, r0
	jr.c4		r7
	add		r7, r0, r0
	
