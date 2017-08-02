/*
 *	Implements the RTT stage of the RxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
	
	
 /* d is the data returned by lookup result */
struct d_struct {
	/* State needed by RX and TX pipelines
	 * This has to be at the beginning.
	 * Each of these fields will be written by Rx only or Tx only
	 */

	srtt_us				: TS_WIDTH              ;\
	rto				: 8	                ;\
	backoff				: 4                     ;\
	
	/* State needed only by RTT stage */
	seq_rtt_us			: TS_WIDTH ;
	ca_rtt_us			: TS_WIDTH ;
	curr_ts				: TS_WIDTH ;
	rtt_min				: TS_WIDTH ;
	rttvar_us			: TS_WIDTH ;
	mdev_us				: TS_WIDTH ;
	mdev_max_us			: TS_WIDTH ;
	rtt_seq				: SEQ_NUMBER_WIDTH ;
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

	snd_una				: SEQ_NUMBER_WIDTH	;\
	snd_nxt				: SEQ_NUMBER_WIDTH	;\

};

struct p_struct p	;
struct k_struct k	;
struct d_struct d	;

	
%%
	
flow_rtt_process_start:
	/* r4 is loaded at the beginning of the stage with current timestamp value */
	tblwr		d.curr_ts, r4
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
	 *				      tp->rx_opt.rcv_tsecr);
	 */
	add		r1, k.process_ack_flag, r0
	smeqh		c1, r1, FLAG_SND_UNA_ADVANCED, FLAG_SND_UNA_ADVANCED

	sub		r1, d.curr_ts, k.rcv_tsecr
	tblwr.c1	d.seq_rtt_us, r1
	tblwr.c1	d.ca_rtt_us, r1

	/*
	 * if (seq_rtt_us < 0)
	 *   return false;
	 */
	slt		c1, d.seq_rtt_us, r0
	bcf		[c1], flow_rtt_process_done
	nop

tcp_update_rtt_min:
	slt		c1, d.ca_rtt_us, d.rtt_min
	add.c1		r1, d.ca_rtt_us, r0
	tblwr.c1	d.rtt_min, r1
	seq		c1, d.rtt_min, r0
	tblwr.c1	d.rtt_min, 1

tcp_rtt_estimator:
	/* Called to compute a smoothed rtt estimate. The data fed to this
	 * routine either comes from timestamps, or from segments that were
	 * known _not_ to have been retransmitted .see Karn/Partridge
	 * Proceedings SIGCOMM 87. The algorithm is from the SIGCOMM 88
	 * piece by Van Jacobson.
	 * NOTE: the next three routines used to be one big routine.
	 * To save cycles in the RFC 1323 implementation it was better to break
	 * it up into three procedures. -- erics
	 */
	/*      The following amusing code comes from Jacobson's
	 *      article in SIGCOMM '88.  Note that rtt and mdev
	 *      are scaled versions of rtt and mean deviation.
	 *      This is designed to be as fast as possible
	 *      m stands for "measurement".
	 *
	 *      On a 1990 paper the rto value is changed to:
	 *      RTO = rtt + 4 * mdev
	 *
	 * Funny. This algorithm seems to be very broken.
	 * These formulae increase RTO, when it should be decreased, increase
	 * too slowly, when it should be increased quickly, decrease too quickly
	 * etc. I guess in BSD RTO takes ONE value, so that it is absolutely
	 * does not matter how to _calculate_ it. Seems, it was trap
	 * that VJ failed to avoid. 8)
	 */
	/* srtt = tp->rtt.srtt_us */
	add		r1, d.srtt_us, r0
	/* r1 is srtt */
	add		r2, d.seq_rtt_us, r0
	/* r2 is m */
	seq		c1, r1, r0
	
	bcf		[c1], first_rtt_measure
	nop

	/* srtt != 0 */
	/* srtt >> 3 */ 
	srl		r3, r1, 3
	/* m -= (srtt >> 3) */
	/* m is now error in rtt est */
	sub		r2, r2, r3
	/* rtt = 7/8 rtt + 1/8 * new */
	/* srtt += m */
	add		r1, r1, r2
	/* if (m < 0) { */
	slt		c1, r2, r0
	bcf		[!c1], m_ge_0
	nop
	/* m = -m, m is now abs(error)  */
	sub		r2, r0, r2
	/* m -= (tp->rtt.mdev_us >> 2); */
	add		r4, d.mdev_us, r0
	srl		r4, r4, 2
	sub		r2, r2, r4
	/* This is similar to one of Eifel findings.
	 * Eifel blocks mdev updates when rtt decreases.
	 * This solution is a bit different: we use finer gain
	 * for mdev in this case (alpha*beta).
	 * Like Eifel it also prevents growth of rto,
	 * but also it limits too fast rto decreases,
	 * happening in pure Eifel.
	 */
	slt		c1, r0, r2
	/* c1 = (m > 0) */
	srl.c1		r2, r2, 3
	
	b		m_ge_0_done
	nop
m_ge_0:	
	/* m -= (tp->rtt.mdev_us >> 2); */
	add		r4, d.mdev_us, r0
	srl		r4, r4, 2
	sub		r2, r2, r4

m_ge_0_done:
	/* tp->rtt.mdev_us += m */
	add		r5, d.mdev_us, r2
	tblwr		d.mdev_us, r5

	/* if (tp->rtt.mdev_us > tp->rtt.mdev_max_us)
	 *	tp->rtt.mdev_max_us = tp->rtt.mdev_us ;
	 */
	slt		c1, d.mdev_max_us, d.mdev_us
	tblwr.c1	d.mdev_max_us, d.mdev_us
	
	/* if (tp->rtt.mdev_max_us > tp->rtt.rttvar_us)
	 *	tp->rtt.rttvar_us = tp->rtt.mdev_max_us ;
	 */
	slt		c1, d.rttvar_us, d.mdev_max_us
	tblwr.c1	d.rttvar_us, d.mdev_max_us
	/*
	 * if (after(tp->tx.snd_una, tp->rtt.rtt_seq)) {
	 */
	slt		c1, k.snd_una, d.rtt_seq
	slt.c1		c2, d.mdev_max_us, d.rttvar_us
	sub.c2		r4, d.rttvar_us, d.mdev_max_us
	srl.c2		r4, r4,2
	sub.c2		r5, d.rttvar_us, r4
	tblwr		d.rttvar_us, r5
	tblwr.c1	d.rtt_seq, k.snd_nxt
	addi.c1		r4, r0, TCP_RTO_MIN
	tblwr.c1	d.mdev_max_us, r4
	
first_rtt_measure:
	/* srtt = m << 3;          /* take the measured time to be rtt */
	add		r1, r2, r0
	sll		r1, r1, 3

	/* tp->rtt.mdev_us = m << 1;   /* make sure rto = 3*rtt */
	add		r3, r2, r0
	sll		r3, r3, 1
	tblwr		d.mdev_us, r3
	
	/* tp->rtt.rttvar_us = max(tp->rtt.mdev_us, tcp_rto_min_us(tp));*/
	addi		r3, r0, TCP_RTO_MIN
	slt		c1, d.mdev_us, r3
	tblwr.c1	d.rttvar_us, r3
	tblwr.!c1	d.rttvar_us, d.mdev_us
	
	/* tp->rtt.mdev_max_us = tp->rtt.rttvar_us; */
	tblwr		d.mdev_max_us, d.rttvar_us
	
	/* tp->rtt.rtt_seq = tp->tx.snd_nxt; */
	tblwr		d.rtt_seq, k.snd_nxt
first_rtt_measure_done:
	/* tp->rtt.srtt_us = max(1U, srtt) */
	addi		r5,r0,1
	slt		c1, r1, r5
	tblwr.c1	d.srtt_us, r5
	tblwr.!c1	d.srtt_us, r1
	nop
	
	
tcp_set_rto:
	/*   return usecs_to_jiffies((tp->rtt.srtt_us >> 3) + tp->rtt.rttvar_us); */
	add		r1, d.srtt_us, r0
	srl		r1, r1, 3
	add		r1, r1, d.rttvar_us

	/*
	 *  if (tp->rtt.rto > TCP_RTO_MAX) 
	 *     tp->rtt.rto = TCP_RTO_MAX;
	 */
	addi		r2, r0, TCP_RTO_MAX
	slt		c1, r2, r1
	add.c1		r1, r2, r0

	tblwr		d.rto, r1
	
	tblwr		d.backoff, r0
flow_rtt_process_done:
	
table_read_FC:
	TCP_NEXT_TABLE_READ(k.fid, TABLE_TYPE_RAW, flow_fra_process,
	                    TCP_TCB_TABLE_BASE, TCP_TCB_TABLE_ENTRY_SIZE_SHFT,
	                    TCP_TCB_FRA_OFFSET, TCP_TCB_TABLE_ENTRY_SIZE)
	nop.e
	nop


