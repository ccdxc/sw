/*
 *	Implements the FAST TIMER stage of the TxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
	
 /* d is the data returned by lookup result */
struct d_struct {
	/* State needed only by TIMER stage */
	curr_ts				: TS_WIDTH	        ;
	ato				: TS_WIDTH              ;
	timer_hdl			: ADDRESS_WIDTH	        ;
	/* State for later TSO stage */
	pending_ack_tx			: 1                     ;
	pending_delayed_ack_tx		: 1                     ;

	/* State needed by RX and TX pipelines
	 * This has to be at the beginning.
	 * Each of these fields will be written by Rx only or Tx only
	 */

	pending_tso_retx	        : 1                     ;\

};

/* Readonly Parsed packet header info for the current packet */
struct k_struct {
	fid				: 32 ;
	syn				: 1 ;
	ece				: 1 ;
	cwr				: 1 ;
	ooo_rcv				: 1 ;
	rsvd				: 4 ;
	ca_event			: 4 ;
	num_sacks			: 8 ;
	sack_off			: 8 ;
	d_off				: 8 ;
	ts_off				: 8 ;
	ip_dsfield			: 8 ;
	pkts_acked			: 8  ;
	seq				: SEQ_NUMBER_WIDTH ;
	end_seq				: SEQ_NUMBER_WIDTH ;
	ack_seq				: SEQ_NUMBER_WIDTH ;
	rcv_tsecr			: TS_WIDTH ;
	window				: WINDOW_WIDTH ;
	process_ack_flag		: 16  ;
	descr_idx			: RING_INDEX_WIDTH ;
	page_idx			: RING_INDEX_WIDTH ;
	descr				: ADDRESS_WIDTH ;
	page				: ADDRESS_WIDTH ;

	rto_deadline			: TS_WIDTH	        ;\
	ato_deadline			: TS_WIDTH	        ;\
	pingpong			: 1                     ;\
	ack_pending			: 3	                ;\
	srtt_us				: TS_WIDTH              ;\
	ack_blocked			: 1	                ;\
	pending_ack_send		: 1	                ;\
};

struct p_struct p	;
struct k_struct k	;
struct d_struct d	;
	
%%
	
flow_fast_timer_process:
	/* r4 is loaded at the beginning of the stage with current timestamp value */
	tblwr		d.curr_ts, r4
	nop
	slt		c1, d.curr_ts, k.rto_deadline
	addi		r1, r0, 1
	tblwr.c1	d.pending_tso_retx, r1
	phvwr.c1	p.pending_tso_retx, r1

	slt		c1, d.curr_ts, k.ato_deadline
	bcf		[!c1], flow_fast_timer_process_done
	nop
tcp_send_delayed_ack:
	/* int ato = tp->fto.ato; */
	add		r1, d.ato, r0
	/*   tcp_ca_event(tp, CA_EVENT_DELAYED_ACK); */
	addi		r2, r0, CA_EVENT_DELAYED_ACK
	phvwr		p.ca_event, r2
	/*   if (ato > TCP_DELACK_MIN) { */
	addi		r2, r0, TCP_DELACK_MIN
	slt		c1,r2, d.ato
	bcf		[!c1], ato_le_tcp_delack_min
	nop
	/* max_ato = HZ/2 ; */
	addi		r4, r0, HZ/2
	sne		c1, k.pingpong, r0
	smeqb		c2, k.ack_pending, ICSK_ACK_PUSHED, ICSK_ACK_PUSHED
	bcf		[!c1 & !c2], check_srtt_us
	nop
	/* max_ato = TCP_DELACK_MAX */
	addi 		r4, r0, TCP_DELACK_MAX
check_srtt_us:
	/* Slow path, intersegment interval is "high". */

	/* If some rtt estimate is known, use it to bound delayed ack.
	 * Do not use inet_csk(sk)->icsk_rto here, use results of rtt measurements
	 * directly.
	 */
	/* if (tp->rtt.srtt_us) {*/
	sne		c1, k.srtt_us, r0
	bcf		[!c1], set_ato
	nop

	/*       int rtt = max(usecs_to_jiffies(tp->rtt.srtt_us >> 3),
		    TCP_DELACK_MIN);
	 */


	add		r5, k.srtt_us, r0
	srl		r5, r5, 3
	addi		r6, r0, TCP_DELACK_MIN
	slt		c1, r5, r6
	add.c1		r3, r6, r0
	add.!c1		r3, r5, r0
	/* rtt is r3, max_ato is r4 */
	/* if (rtt < max_ato)
	 *    max_ato = rtt	;
	 */
	slt		c1, r3, r4
	add		r4, r3, r0


set_ato:
	/* ato = min(ato, max_ato) */
	slt		c1, r4, d.ato
	tblwr		d.ato, r4
	
ato_le_tcp_delack_min:
	/* Stay within the limit we were given */
	/* timeout = tcp_time_stamp + ato; */
	add		r5, d.curr_ts, d.ato

	/* Use new timeout only if there wasn't a older one earlier. */
	/* if (tp->tx.ack_pending & ICSK_ACK_TIMER) { */
	smeqb		c1, k.ack_pending, ICSK_ACK_TIMER, ICSK_ACK_TIMER
	bcf		[!c1], set_ato_deadline
	nop
	/* If delack timer was blocked or is about to expire,
	 * send ACK now.
	*/
	/* if (tp->tx.ack_blocked ||
	 * time_before_eq(tp->fto.ato_deadline, tcp_time_stamp + (ato >> 2))) {
	 *    tp->pending.ack_tx = 1;
	 *    return;
	 * }
	 */
	sne		c1, k.ack_blocked, r0
	add		r6, d.ato, r0
	srl		r6, r6, 2
	add		r6, d.curr_ts, r6
	slt		c2, k.ato_deadline,r6
	bcf		[c1 | c2], pending_ack_tx
	nop

	/*
	 * if (!time_before(timeout, tp->fto.ato_deadline))
	 *   timeout = tp->fto.ato_deadline;
	 */
	/* timeout is r5 */
	slt		c1, k.ato_deadline, r5
	add.c1		r5, k.ato_deadline, r0

set_ato_deadline:
	 /* tp->tx.ack_pending |= ICSK_ACK_SCHED | ICSK_ACK_TIMER; */
	ori		r6, r0,  (ICSK_ACK_SCHED | ICSK_ACK_TIMER)
	phvwr		p.ack_pending, r6
	/* tp->fto.ato_deadline = timeout;*/
	phvwr		p.ato_deadline, r5
	/* SCHEDULE_FAST_TIMER */
	slt		c1, k.ato_deadline, k.rto_deadline

	/*
	 * TBD
	memrd.c1	d.timer_hdl, d.ato_deadline
	memrd.!c1	d.timer_hdl, d.rto_deadline
	 */

pending_ack_tx:
	addi		r6,r0, 1
	phvwr		p.pending_ack_send, r6
	
flow_fast_timer_process_done:
	
table_read_TX:
	TCP_NEXT_TABLE_READ(k.fid, TABLE_TYPE_RAW, flow_tx_process,
	                    TCP_TCB_TABLE_BASE, TCP_TCB_TABLE_ENTRY_SIZE_SHFT,
	                    TCP_TCB_TX_OFFSET, TCP_TCB_TABLE_ENTRY_SIZE)
	nop.e
	nop


