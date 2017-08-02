/*
 *	Implements the CC stage of the RxDMA P4+ pipeline
 */


#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp-constants.h"	
	
/* bic congestion control algorithm state */
struct bictcp {
	cnt				: COUNTER32 	;
	last_max_cwnd			: WINDOW_WIDTH	;
	loss_cwnd			: WINDOW_WIDTH	;
	last_cwnd			: WINDOW_WIDTH	;
	last_time			: TS_WIDTH	;
	epoch_start			: TS_WIDTH	;
	delayed_ack			: TS_WIDTH	;
};

 /* d is the data returned by lookup result */
struct d_struct {
	/* State needed by RX and TX pipelines
	 * This has to be at the beginning.
	 * Each of these fields will be written by Rx only or Tx only
	 */

        snd_cwnd                        : WINDOW_WIDTH          ;\

	snd_wnd_clamp			: WINDOW_WIDTH	;
	snd_cwnd_clamp			: WINDOW_WIDTH	;
	prior_cwnd			: WINDOW_WIDTH	;
	snd_cwnd_cnt			: COUNTER32	;
	prr_delivered			: COUNTER32	;



	is_cwnd_limited			: 8	;
	max_packets_out			: 8	;
	sack_reordering                 : 8     ;
	tune_reordering                 : 8     ;
	curr_ts				: TS_WIDTH	;
//	bic				: struct bictcp;
	cnt				: COUNTER32 	;
	last_max_cwnd			: WINDOW_WIDTH	;

	last_cwnd			: WINDOW_WIDTH	;
	last_time			: TS_WIDTH	;
	epoch_start			: TS_WIDTH	;
	delayed_ack			: TS_WIDTH	;
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
	window				: WINDOW_WIDTH ;
	process_ack_flag		: 16  ;

	ca_state			: 8	                ;\
	packets_out			: COUNTER16	        ;\
	retrans_out			: COUNTER8	        ;\
	sacked_out			: COUNTER16	        ;\
	lost_out			: COUNTER8	        ;\
        snd_ssthresh			: WINDOW_WIDTH	        ;\
        prr_out				: COUNTER32	        ;\

};

struct p_struct p;
struct k_struct k;
struct d_struct d;
	
%%
	
flow_cc_process_start:
	/* Fall Thru */
	/* r4 is loaded at the beginning of the stage with current timestamp value */
	tblwr		d.curr_ts, r4
tcp_cong_control:
	/* Check if we are in congestion window reduction state */
	smeqb 		c1, k.ca_state, TCPF_CA_CWR | TCPF_CA_Recovery, TCPF_CA_CWR | TCPF_CA_Recovery
	bcf 		[c1],tcp_cwnd_reduction
	nop

	/* Decide whether to run the increase function of congestion control. */
	slt  		c2, d.tune_reordering, d.sack_reordering
	add		r5, k.process_ack_flag, r0
	smeqh.c2   	c3, r5, FLAG_FORWARD_PROGRESS, FLAG_FORWARD_PROGRESS
	smeqh.!c2	c3, r5, FLAG_DATA_ACKED, FLAG_DATA_ACKED

	bcf		[c3], tcp_cong_avoid
	nop


tcp_cwnd_reduction:
	/* r1 = newly_acked_sacked */
	add		r1, k.pkts_acked, r0
	/* r2 = flag */
	add		r2, k.process_ack_flag, r0

	add 		r3, k.packets_out, k.retrans_out
	add		r4, k.sacked_out, k.lost_out
	/* r6 = tcp_packets_in_flight = packets_out + retrans_out - (sacked_out + lost_out) */
	sub		r6, r3, r4
	/* r3 = delta = ssthresh - packets_in_flight */
	sub		r3, k.snd_ssthresh, r6
	sle		c1, r1, r0
	bcf		[c1], tcp_cwnd_reduction_done
	nop
	sne		c2, d.prior_cwnd, r0
	bcf		[c2], tcp_cwnd_reduction_done
	nop
	
	tbladd		d.prr_delivered, r1

	slt		c1, r3, 0
	add.c1		r4, k.snd_ssthresh, d.prr_delivered

	addi		r5, r0, 1
	sub.c1		r5, d.prior_cwnd, r5
	
	/* r5 = dividend = (snd_ssthresh + prr_delivered + prior_cwnd - 1) */
	add.c1		r4, r4, r5

	div.c1		r5, r4, d.prior_cwnd
	/* r5 = sndcnt = dividend / prior_cwnd - prr_out */
	bcf		[c1], update_sndcnt
	sub.c1		r5, r5, k.prr_out

	
	add		r4, k.process_ack_flag, r0
	smeqh		c2, r4, FLAG_RETRANS_DATA_ACKED, FLAG_RETRANS_DATA_ACKED
	smeqh		c3, r4, FLAG_LOST_RETRANS, FLAG_LOST_RETRANS
	bcf		[c2 | !c3], sndcnt_retx_data_acked_not_lost_retrans
	sub		r4, d.prr_delivered, k.prr_out
	/* sndcnt = min(delta, newly_acked_sacked); */

	slt		c2, r3, r1
	add.c2		r5, r3, r0
	b 		update_sndcnt
	add.!c2		r5, r1, r0
	/* At this point, r5 = min(delta, newly_acked_sacked) */

sndcnt_retx_data_acked_not_lost_retrans:
	/*    sndcnt = min(delta,
		           max(tp->cc.prr_delivered - tp->cc.prr_out,
		               newly_acked_sacked) + 1);
	*/
	/* r4 = prr_delivered - prr_out
	 * r1 = newly_acked_sacked
	 * c2 = (r4 < r1)
	 */
	slt		c2, r4, r1
	
	addi.c2		r5, r1, 1
	addi.!c2	r5, r4, 1

	/* At this point, r5 = max (prr_delivered - prr_out, newly_acked_sacked) + 1*/
	slt		c2, r5, r3
	addi.!c2	r5, r3, 0
	/* At this point, r5 = min (delta, max(prr_delivered - prr_out, newly_acked_sacked) + 1) */
	
	/* Force a fast retransmit upon entering fast recovery */
update_sndcnt:
	sne		c2, k.prr_out, r0
	add.c2		r4, r0, r0
	addi.!c2	r4, r0, 1
	/* r4 = (prr_out ? 0 : 1) */
	slt		c2, r5, r4
	add.!c2		r5, r4, 0
	/* r5 = sndcnt = max(sndcnt, (prr_out ? 0 : 1)) */
	/* r6 = packets in flight */
	/* snd_cwnd = packets_in_flight + sndcnt */
	add		r5, r5, r6
	tbladd		d.snd_cwnd, r5
tcp_cwnd_reduction_done:
	nop.e
	/* u32 ack in r1, u32 acked in r2 */
tcp_cong_avoid:

	/* Check if cwnd is limited */
tcp_is_cwnd_limited:
	/* Check in slow start */
	slt		c1, d.snd_cwnd,k.snd_ssthresh

	/* In slow start check */
	add		r2, d.max_packets_out, r0
	muli		r3, r2, 2
	slt.c1		c2, d.snd_cwnd, r3
	sne.!c1		c2, d.is_cwnd_limited, r0

	/* c2 == true means tcp_is_cwnd_limited is true */
	/* c1 == true means tcp in slow start */
	b.c1		tcp_slow_start
	nop
bictcp_update:
	seq		c1, d.last_cwnd, d.snd_cwnd
	sub		r3, d.curr_ts, d.last_time
	slt		c2, r3, HZ/32
	bcf		[c2], bictcp_update_done
	nop
	tblwr		d.last_cwnd, d.snd_cwnd
	tblwr		d.last_time, d.curr_ts

	/* Record the beginning of epoch */
	slt		c1, d.epoch_start, r0
	tblwr.c1	d.epoch_start, d.curr_ts

	/* Start off normal */
	slt		c1, d.snd_cwnd, LOW_WINDOW
	bcf		[c1], bictcp_update_done
	tblwr		d.cnt, d.snd_cwnd
	/* binary increase */
	/* c1 = (snd_cwnd < last_max_cwnd) */
	slt		c1, d.snd_cwnd, d.last_max_cwnd
	bcf		[!c1],bictcp_aimd_incr
	nop
bictcp_bin_incr:
	/* dist in r3 */
	sub.c1		r3, d.last_max_cwnd, d.snd_cwnd
	/* r3 = (last_max_cwnd - cwnd) / BICTCP_B */
	srl.c1		r3, r3,BICTCP_B_SHIFT

	/* c2 = (dist < max_increment) */
	slt		c2, r3, MAX_INCREMENT
	srl.!c2		r4, d.snd_cwnd, MAX_INCREMENT_SHIFT

	bcf		[!c2],bictcp_cnt_update_done
	tblwr.!c2	d.cnt, r4

	/* c3 = (dist <= 1U) */
	sle.c2		c3, r3, 1
	/* r4 = cwnd * smooth_part */

	add		r6, d.snd_cwnd, r0
	muli.c3		r4, r6, 20
	/* r4 = (cwnd * smooth_part) / BICTCP_B */
	srl.c3		r4, r4, BICTCP_B_SHIFT

	/* cnt = (cwnd * smooth_part) / BICTCP_B */
	bcf		[c3],bictcp_cnt_update_done
	tblwr.c3	d.cnt, r4

	/* cnt = cwnd / dist */
	add		r6, d.snd_cwnd, r0
	div.!c3		r5, r6, r3
	bcf		[!c3],bictcp_cnt_update_done
	tblwr.!c3	d.cnt, r5

bictcp_aimd_incr:
	add		r3, d.last_max_cwnd, r0
	addi		r3, r3, BICTCP_B
	/* c2 = (cwnd < (last_max_cwnd + BICTCP_B)) */
	slt		c2, d.snd_cwnd, r3
	/* r4 still contains (cwnd * smooth_part)/BICTCP_B */
	bcf		[c2],bictcp_cnt_update_done
	tblwr.c2	d.cnt, r4

	add		r3, d.last_max_cwnd, r0
	addi		r3, r3, MAX_INCREMENT * (BICTCP_B-1)

	/* c2 = (cwnd < last_max_cwnd + max_increment * (BICTCP_B-1)) */
	slt		c2, d.snd_cwnd, r3
	add		r5, d.snd_cwnd, r0
	muli.c2		r5, r5, (BICTCP_B-1)
	sub.c2 		r6, d.snd_cwnd, d.last_max_cwnd

	div		r5, r5, r6	
	bcf		[c2],bictcp_cnt_update_done
	tblwr		d.cnt, r5

	add.!c2		r4, d.snd_cwnd, r0
	srl.!c2		r4, r4, MAX_INCREMENT_SHIFT
	bcf		[!c2],bictcp_cnt_update_done
	tblwr.!c2	d.cnt, r4
	
bictcp_cnt_update_done:	
	/* if in slow start or link utilization is very low */
	slt		c2, d.last_max_cwnd, r0
	addi		r5, r0, 20
	sle.c2		c3, d.cnt, r5
	tblwr.!c2	d.cnt,r5

	add		r5, d.cnt, r0
	sll 		r5, r5, ACK_RATIO_SHIFT
	div		r5, r5, d.delayed_ack
	tblwr		d.cnt, r5
	slt		c2, d.cnt, r0
	addi 		r5,r0,1
	tblwr.c2	d.cnt,r5
	
	/* Fall thru to tcp_cong_avoid_ai */


bictcp_update_done:	
	/* u32 w in r1, u32 acked in r2 */
tcp_cong_avoid_ai:

	/* If credits accumulated at a higher w, apply them gently now. */
	sle		c1, r1, r2
	tblwr.c1	d.snd_cwnd_cnt, r0
	tbladd.c1	d.snd_cwnd,1

	tbladd 		d.snd_cwnd_cnt, r2

	div.c1		r3, d.snd_cwnd_cnt, r1
	tbladd.c1	d.snd_cwnd, r3
	mul.c1		r3, r3, r1
	tblsub.c1	d.snd_cwnd_cnt, r3

	slt		c2, d.snd_cwnd_clamp, d.snd_cwnd
	tblwr.c2	d.snd_cwnd, d.snd_cwnd_clamp
table_read_FC:
	TCP_NEXT_TABLE_READ(k.fid, TABLE_TYPE_RAW, flow_fc_process,
	                    TCP_TCB_TABLE_BASE, TCP_TCB_TABLE_ENTRY_SIZE_SHFT,
	                    TCP_TCB_FC_OFFSET, TCP_TCB_TABLE_ENTRY_SIZE)
	nop.e
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
	/* u32 tcp_slow_start(tcb_t *tp, u32 acked)
	 {
	  u32 cwnd = min(tp->cc.snd_cwnd + acked, tp->cc.snd_ssthresh) ;

	  acked -= cwnd - tp->cc.snd_cwnd ;
	  tp->cc.snd_cwnd = min(cwnd, tp->cc.snd_cwnd_clamp) ;

	  return acked		;
	 }
	*/
tcp_slow_start:
	/* r2 = acked */
	/* r3 = tp->cc.snd_cwnd + acked */
	add		r3, d.snd_cwnd, r2
	/* r4 = tp->cc.snd_ssthresh */
	add		r4, k.snd_ssthresh, r0
	slt		c1, r4, r3
	add.!c1		r4, r3, r0
	/* r4 = cwnd = min(r3, r4) */
	/* r4 = cwnd - tp->cc.snd_cwnd */
	sub		r4, r4, d.snd_cwnd
	/* acked -= (cwnd - tp->cc.snd_cwnd) */
	b		table_read_FC
	sub		r2, r2, r4

	
	
