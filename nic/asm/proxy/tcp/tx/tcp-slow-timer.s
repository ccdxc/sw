/*
 *	Implements the SLOW TIMER stage of the TxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
	
 /* d is the data returned by lookup result */
struct d_struct {
	/* State needed only by TIMER stage */
	kto_deadline			: TS_WIDTH ;
	kto				: TS_WIDTH ;
	pmtu_to_deadline                : TS_WIDTH ;
	pto_deadline                    : TS_WIDTH ;

	timer_hdl			: ADDRESS_WIDTH	   ;
	mtup_enabled                    : 1		   ;
	pmtu				: MTU_WIDTH	   ;	
	mtup_probe_size			: MTU_WIDTH	   ;
	mtup_search_hi                  : MTU_WIDTH	   ;
	mtup_search_lo                  : MTU_WIDTH	   ;
	net_header_len			: 4                ;
	tcp_header_len			: 4                ;
	mss_clamp			: MTU_WIDTH	   ;
	probe_size			: MTU_WIDTH	   ;
	interval			: TS_WIDTH	   ;
	probe_interval			: TS_WIDTH	   ;	
	size_needed			: MTU_WIDTH	   ;
	probe_threshold			: MTU_WIDTH	   ;
	mtup_probe_ts			: TS_WIDTH	   ;
	probe0s_out			: COUNTER16	   ;
	probe0_backoff			: COUNTER8	   ;
	retries2			: 8		   ;
	curr_ts				: TS_WIDTH	   ;
	
	/* State needed by RX and TX pipelines
	 * This has to be at the beginning.
	 * Each of these fields will be written by Rx only or Tx only
	 */

	pending_tso_keepalive           : 1                     ;\
	pending_tso_pmtu_probe          : 1                     ;\

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
	rcv_tsecr			: TS_WIDTH ;
	window				: WINDOW_WIDTH ;
	process_ack_flag		: 16  ;
	descr_idx			: RING_INDEX_WIDTH ;
	page_idx			: RING_INDEX_WIDTH ;
	descr				: ADDRESS_WIDTH ;
	page				: ADDRESS_WIDTH ;

        write_seq                       : SEQ_NUMBER_WIDTH      ;\
	snd_nxt				: SEQ_NUMBER_WIDTH	;\
	snd_una				: SEQ_NUMBER_WIDTH	;\
	snd_wnd				: SEQ_NUMBER_WIDTH	;\
	snd_up				: SEQ_NUMBER_WIDTH	;\
        snd_cwnd                        : WINDOW_WIDTH          ;\
	ca_state			: 8	                ;\
	dsack			        : 1                     ;\
	reordering			: COUNTER32	        ;\
        rcv_mss                         : 8                     ;\
	rcv_mss_shft			: 4	                ;\
	packets_out			: COUNTER16	        ;\
	sacked_out			: COUNTER16	        ;\
	lost_out			: COUNTER8	        ;\
	retrans_out			: COUNTER8	        ;\
};

struct p_struct p	;
struct k_struct k	;
struct d_struct d	;
	
%%
	
flow_slow_timer_process:
	/* r4 is loaded at the beginning of the stage with current timestamp value */
	tblwr		d.curr_ts, r4
	nop
	/* Check if keep alive deadline expired */
	/* if (tp->sto.kto_deadline < tcp_time_stamp) { */
	sne		c2, d.kto_deadline, r0
	slt		c1, d.kto_deadline, d.curr_ts
	setcf		c3, [c1 & c2]
	jal.c3 		r7, tcp_write_wakeup
	nop
	addi.c1		r1, r0, 1
	tblwr.c1	d.pending_tso_keepalive, r1

	/* Check if PMTU deadline expired */
	/* if (tp->sto.pmtu_to_deadline < tcp_time_stamp) { */
	sne		c2, d.pmtu_to_deadline, r0
	slt		c1, d.pmtu_to_deadline, d.curr_ts
	setcf		c3, [c1 & c2]
	jal.c3 		r7, tcp_mtu_probe
	nop
	add.c1		r1, d.curr_ts, r0
	addi.c1		r1, r1, TCP_PROBE_INTERVAL
	tblwr.c1	d.pmtu_to_deadline, r1
	addi.c1		r1, r0, 1
	tblwr.c1	d.pending_tso_pmtu_probe, r1

	/* Check if zero window probe deadline expired */
	/* if (tp->sto.pto_deadline > tcp_time_stamp) { */
	slt		c1, d.curr_ts, d.pto_deadline
	jal.c1		r7, tcp_send_probe0
	nop

	b 		flow_slow_timer_process_done
	nop

#define TCP_WRITE_WAKEUP(_x) 						\
	// if (tcp_send_data_pending(tp) &&  				\
	//   before(tp->tx.snd_nxt, tcp_wnd_end(tp))) {			\
	//								\
	// tcp_send_data_pending					\
	sne		c1, k.write_seq, k.snd_nxt			\
	// tcp_wnd_end 							\
	add		r1, k.snd_una, k.snd_wnd			\
	// before (tp->tx.snd_nxt, tcp_wnd_end(tp)) 			\
	slt		c2, k.snd_nxt, r1				\
	bcf		[c1 && c2], pending_tso_probe_data		\
	nop								\
	addi		r1, k.snd_una, 1				\
	addi		r2, k.snd_una, 0xFFFF				\
	slt		c1, k.snd_up, r2				\
	slt		c2, r1, k.snd_up				\
	bcf		[c1 && c2], _x ## xmit_probe_1			\
	nop								\
_x ## xmit_probe_0:							\
	addi 		r1, 1						\
	sub             r1, k.snd_una, r1                               \
	phvwr		p.tso_seq, r1					\
	phvwr		p.pending_tso_probe, r1				\
	b		_x ## tcp_write_wakeup_done			\
	nop								\
_x ## xmit_probe_1:							\
	addi		r1, r0, 1					\
	phvwwr		p.tso_seq, k.snd_una				\
	phvwr		p.pending_tso_probe, r1				\
	b		_x ## tcp_write_wakeup_done			\
	nop								\
_x ## tso_probe_data:							\
	phvwr		p.pending_tso_probe_data, 1				\
_x ## tcp_write_wakeup_done:						\
	sne		c4, r7, r0					\
	jr.c4		r7						\
	add		r7, r0, r0


//	TCP_WRITE_WAKEUP(zero) BEGIN
	// if (tcp_send_data_pending(tp) &&  				
	//   before(tp->tx.snd_nxt, tcp_wnd_end(tp))) {			
	//								
	// tcp_send_data_pending
tcp_write_wakeup_start:	
	sne		c1, k.write_seq, k.snd_nxt			
	// tcp_wnd_end 							
	add		r1, k.snd_una, k.snd_wnd			
	// before (tp->tx.snd_nxt, tcp_wnd_end(tp)) 			
	slt		c2, k.snd_nxt, r1				
	bcf		[c1 & c2], zero_tso_probe_data			
	nop
	add		r1, k.snd_una, r0
	addi		r1, r1, 1				
	addi		r2, r1, 0xFFFF				
	slt		c1, k.snd_up, r2				
	slt		c2, r1, k.snd_up				
	bcf		[c1 & c2], zero_xmit_probe_1			
	nop								
zero_xmit_probe_0:							
	addi 		r1, r0, 1
	sub		r2, k.snd_una, r1
	phvwr		p.tso_seq, r2
	phvwr		p.pending_tso_probe, r1				
	b		zero_tcp_write_wakeup_done			
	nop								
zero_xmit_probe_1:							
	addi		r1, r0, 1					
	phvwr		p.tso_seq, k.snd_una				
	phvwr		p.pending_tso_probe, r1				
	b		zero_tcp_write_wakeup_done			
	nop								
zero_tso_probe_data:							
	phvwr		p.pending_tso_probe_data, 1				
zero_tcp_write_wakeup_done:						
	sne		c4, r7, r0					
	jr.c4		r7						
	add		r7, r0, r0
//	TCP_WRITE_WAKEUP(zero) END
	
/* Create a new MTU probe if we are ready.
 * MTU probe is regularly attempting to increase the path MTU by
 * deliberately sending larger packets.  This discovers routing
 * changes resulting in larger path MTUs.
 *
 */
tcp_mtu_probe_start:
	/* Not currently probing/verifying,
	 * not in recovery,
	 * have enough cwnd, and
	 * not SACKing (the variable headers throw things off)
	 */
	/* if (likely(!tp->sto.mtup_enabled ||
	     tp->sto.mtup_probe_size ||
	     tp->cc.ca_state != TCP_CA_Open ||
	     tp->cc.snd_cwnd < 11 ||
	     tp->rx_opt.num_sacks || 
	     tp->rx_opt.dsack))
	     return;
	 */
	seq		c1, d.mtup_enabled, r0
	sne		c2, d.mtup_probe_size, r0
	sne		c3, k.ca_state, TCP_CA_Open
	slt		c4, k.snd_cwnd, 11
	sne		c5, k.num_sacks, r0
	sne		c6, k.dsack, r0
	bcf		[c1|c2|c3|c4|c5|c6], tcp_mtu_probe_done
	nop

	/* Use binary search for probe_size between tcp_mss_base,
	 * and current mss_clamp. if (search_high - search_low)
	 * smaller than a threshold, backoff from probing.
	 */
	add		r1, d.mtup_search_hi, d.mtup_search_lo
	srl		r1, r1, 1
	tblwr		d.pmtu, r1
	nop
#define TCP_MTU_TO_MSS() 								\
	add		r1, d.pmtu, r0							\
	// r1 = pmtu									\
	// Calculate base mss without TCP options:					\
	//   It is MMS_S - sizeof(tcphdr) of rfc1122					\
	// mss_now = pmtu - tp->rx_opt.net_header_len  - sizeof(struct tcphdr);		\
	addi		r6, r0, TCP_HDR_SIZE						\
	add		r3, d.net_header_len, r0					\
	sll		r3, r3, 2							\
	sub		r2, r1, r3							\
	sub		r2, r2, r6							\
	// r2 = mss_now 								\
	// Clamp it (mss_clamp does not include tcp options) 				\
	// if (mss_now > tp->rx_opt.mss_clamp) 						\
	//     mss_now = tp->rx_opt.mss_clamp; 						\
	slt		c1, d.mss_clamp, r2						\
	add.c1		r2, d.mss_clamp, r0						\
	// Then reserve room for full set of TCP options and 8 bytes of data 		\
	// if (mss_now < 48) 								\
	//   mss_now = 48;								\
	slti		c1, r2, 48							\
	li.c1		r2, 48


//	TCP_MTU_TO_MSS BEGIN
	add		r1, d.pmtu, r0							
	// r1 = pmtu									
	// Calculate base mss without TCP options:					
	//   It is MMS_S - sizeof(tcphdr) of rfc1122					
	// mss_now = pmtu - tp->rx_opt.net_header_len  - sizeof(struct tcphdr);		
	addi		r6, r0, TCP_HDR_SIZE
	add		r3, d.net_header_len, r0
	sll		r3, r3, 2
	sub		r2, r1, r3
	sub		r2, r2, r6							
	// r2 = mss_now 								
	// Clamp it (mss_clamp does not include tcp options) 				
	// if (mss_now > tp->rx_opt.mss_clamp) 						
	//     mss_now = tp->rx_opt.mss_clamp; 						
	slt		c1, d.mss_clamp, r2						
	add.c1		r2, d.mss_clamp, r0						
	// Then reserve room for full set of TCP options and 8 bytes of data 		
	// if (mss_now < 48) 								
	//   mss_now = 48;								
	slt		c1, r2, 48							
	addi.c1		r2, r0, 48
//	TCP_MTU_TO_MSS END


	/* Subtract TCP options size, not including SACKs */
	/* return __tcp_mtu_to_mss(tp, pmtu) -
	         (tp->rx_opt.tcp_header_len - sizeof(struct tcphdr));
	 */
	addi		r5, r0, TCP_HDR_SIZE
	add		r3, d.tcp_header_len, r0
	sll		r3, r3, 2
	sub		r6, r3, r5
	sub		r6, r2, r6
	tblwr		d.probe_size, r6
	
	/* probe_size = tcp_mtu_to_mss(tp, (tp->sto.mtup_search_high +
				   tp->sto.mtup_search_low) >> 1);
	 */

	/* size_needed = probe_size + (tp->sack.reordering + 1) * tp->tso.mss_cache; */
	add		r6, k.reordering, r0
	addi		r6, r6, 1
	mul		r6, r6, k.rcv_mss
	add		r6, r6, r2
	tblwr		d.size_needed, r6

	sub		r6, d.mtup_search_hi, d.mtup_search_lo
	tblsub		d.interval, r6
	/* When misfortune happens, we are reprobing actively,
	 * and then reprobe timer has expired. We stick with current
	 * probing process by not resetting search range to its orignal.
	 */
	/* if (probe_size > tcp_mtu_to_mss(tp, tp->sto.mtup_search_high) ||
	       interval < tp->tune.probe_threshold) {
	 */

	add		r1, d.mtup_search_hi, r0
	

//	TCP_MTU_TO_MSS BEGIN
	add		r1, d.pmtu, r0							
	// r1 = pmtu									
	// Calculate base mss without TCP options:					
	//   It is MMS_S - sizeof(tcphdr) of rfc1122					
	// mss_now = pmtu - tp->rx_opt.net_header_len  - sizeof(struct tcphdr);		
	addi		r6, r0, TCP_HDR_SIZE						
	add		r3, d.net_header_len, r0
	sll		r3, r3, 2
	sub		r2, r1, r3
	sub		r2, r2, r6							
	// r2 = mss_now 								
	// Clamp it (mss_clamp does not include tcp options) 				
	// if (mss_now > tp->rx_opt.mss_clamp) 						
	//     mss_now = tp->rx_opt.mss_clamp; 						
	slt		c1, d.mss_clamp, r2						
	add.c1		r2, d.mss_clamp, r0						
	// Then reserve room for full set of TCP options and 8 bytes of data 		
	// if (mss_now < 48) 								
	//   mss_now = 48;								
	slt		c1, r2, 48							
	addi.c1		r2, r0, 48
//	TCP_MTU_TO_MSS END

	slt		c1, r2, d.probe_size
	slt		c2, d.interval, d.probe_threshold
	/* Check whether enough time has elaplased for
	 * another round of probing.
	 */
	bcf		[!c1 & !c2], tcp_mtu_check_reprobe_done
	nop
tcp_mtu_check_reprobe:

	/*   interval = tp->tune.probe_interval; */
	tblwr		d.interval, d.probe_interval
	/*  delta = tcp_time_stamp - tp->sto.mtup_probe_timestamp; */
	sub		r1, d.curr_ts, d.mtup_probe_ts
	/* if (delta >= interval * HZ) { */
	sle		c1, d.interval, r1
	bcf		[!c1], tcp_mtu_check_reprobe_done
	nop
	/* Update current search range */
	/* tp->sto.mtup_probe_size = 0; */
	tblwr		d.mtup_probe_size, r0

	/*     tp->sto.mtup_search_high = tp->rx_opt.mss_clamp +
		  sizeof(struct tcphdr) +
		  tp->rx_opt.net_header_len;
         */
	add		r1, d.mss_clamp, r0
	addi		r1, r1, TCP_HDR_SIZE
	add		r3, d.net_header_len, r0
	sll		r3, r3, 2
	add		r1, r1, r3
	tblwr		d.mtup_search_hi, r1

	/* int mss = tcp_current_mss(tp); */
	add		r1, k.rcv_mss, r0

	/* 	tp->sto.mtup_search_low = tcp_mss_to_mtu(tp, mss);*/
	
//	TCP_MTU_TO_MSS BEGIN
	add		r1, d.pmtu, r0							
	// r1 = pmtu									
	// Calculate base mss without TCP options:					
	//   It is MMS_S - sizeof(tcphdr) of rfc1122					
	// mss_now = pmtu - tp->rx_opt.net_header_len  - sizeof(struct tcphdr);		
	addi		r6, r0, TCP_HDR_SIZE						
	add		r3, d.net_header_len, r0
	sll		r3, r3, 2
	sub		r2, r1, r3
	sub		r2, r2, r6							
	// r2 = mss_now 								
	// Clamp it (mss_clamp does not include tcp options) 				
	// if (mss_now > tp->rx_opt.mss_clamp) 						
	//     mss_now = tp->rx_opt.mss_clamp; 						
	slt		c1, d.mss_clamp, r2						
	add.c1		r2, d.mss_clamp, r0						
	// Then reserve room for full set of TCP options and 8 bytes of data 		
	// if (mss_now < 48) 								
	//   mss_now = 48;								
	slt		c1, r2, 48							
	addi.c1		r2, r0, 48
//	TCP_MTU_TO_MSS END

	tblwr		d.mtup_search_lo, r2

	
	/* Update probe time stamp */
	/* tp->sto.mtup_probe_timestamp = tcp_time_stamp; */
	tblwr		d.mtup_probe_ts, d.curr_ts

	

	b		tcp_mtu_probe_done
	nop

tcp_mtu_check_reprobe_done:	
	/* Have enough data in the send queue to probe? */
	/* if (tp->tx.write_seq - tp->tx.snd_nxt < size_needed)
	    return;
	 */
	sub		r1, k.write_seq, k.snd_nxt
	slt		c1, r1, d.size_needed
	bcf		[c1], tcp_mtu_probe_done
	nop

	/* if (tp->tx.snd_wnd < size_needed)
	      return;
	 */
	slt		c1, k.snd_wnd, d.size_needed
	bcf		[c1], tcp_mtu_probe_done
	nop

	/*
	   if (after(tp->tx.snd_nxt + size_needed, tcp_wnd_end(tp)))
              return;
	 */
	add		r1, k.snd_una, k.snd_wnd
	add		r2, k.snd_nxt, d.size_needed
	slt		c1, r1, r2
	bcf		[c1], tcp_mtu_probe_done
	nop

	/* Do we need to wait to drain cwnd? With none in flight, don't stall */
	/* if (tcp_packets_in_flight(tp) + 2 > tp->cc.snd_cwnd) {
	     return;
	 */
	add		r1, k.packets_out, k.retrans_out
	add		r2, k.sacked_out, k.lost_out
	sub		r1, r1, r2
	addi		r1, r1,2
	slt		c1, k.snd_cwnd, r1
	bcf		[c1], tcp_mtu_probe_done
	nop
	
	/* tp->sto.mtup_probe_size = probe_size; */
	tblwr		d.mtup_probe_size, d.probe_size

	/* We're ready to send.  If this fails, the probe will
	 * be resegmented into mss-sized pieces by tcp_write_xmit().
	 */
	/* tp->pending.tso_probe_data = 1; */
	
	phvwr		p.pending_tso_probe_data, 1

tcp_mtu_probe_done:	
	sne		c4, r7, r0
	jr.c4		r7
	add		r7, r0, r0


	/* A window probe timeout has occurred.  If window is not closed send
	 * a partial packet else a zero probe.
	 */
tcp_send_probe0_start:
//	TCP_WRITE_WAKEUP(one) BEGIN
	// if (tcp_send_data_pending(tp) &&  				
	//   before(tp->tx.snd_nxt, tcp_wnd_end(tp))) {			
	//								
	// tcp_send_data_pending					
	sne		c1, k.write_seq, k.snd_nxt			
	// tcp_wnd_end 							
	add		r1, k.snd_una, k.snd_wnd			
	// before (tp->tx.snd_nxt, tcp_wnd_end(tp)) 			
	slt		c2, k.snd_nxt, r1				
	bcf		[c1 & c2], one_tso_probe_data			
	nop
	add		r1, k.snd_una, r0
	addi		r1, r1, 1				
	addi		r2, r1, 0xFFFF				
	slt		c1, k.snd_up, r2				
	slt		c2, r1, k.snd_up				
	bcf		[c1 & c2], one_xmit_probe_1			
	nop								
one_xmit_probe_0:							
	addi 		r1, r0, 1
	sub		r2, k.snd_una, r1
	phvwr		p.tso_seq, r2
	phvwr		p.pending_tso_probe, r1				
	b		one_tcp_write_wakeup_done			
	nop								
one_xmit_probe_1:							
	addi		r1, r0, 1					
	phvwr		p.tso_seq, k.snd_una				
	phvwr		p.pending_tso_probe, r1				
	b		one_tcp_write_wakeup_done			
	nop								
one_tso_probe_data:							
	phvwr		p.pending_tso_probe_data, 1				
one_tcp_write_wakeup_done:						
	sne		c4, r7, r0					
	jr.c4		r7						
	add		r7, r0, r0

//	TCP_WRITE_WAKEUP(one) END


	/*   if (tp->tx.packets_out || !tcp_send_data_pending(tp)) { */
	sne		c1, k.packets_out, r0
	seq		c2, k.write_seq, k.snd_nxt
	bcf		[c1|c2], cancel_probe0

	/* if (tp->sto.probe0_backoff < tp->tune.retries2)
              tp->sto.probe0_backoff++;
	 */

	slt		c1, d.probe0_backoff, d.retries2
	tbladd.c1	d.probe0_backoff, 1

	/* tp->sto.probe0s_out++; */
	tbladd		d.probe0s_out,  1

	/* probe_max = TCP_RTO_MAX; */
	addi		r1, r0, TCP_RTO_MAX

		
	/* tp->sto.pto_deadline = probe_max << tp->sto.probe0_backoff + tcp_time_stamp; */
	add		r2, d.probe0_backoff, r0
	sllv		r1, r1, r2
	add		r1, r1, d.curr_ts
	tblwr		d.pto_deadline, r1

	
	b		tcp_send_probe0_done
	nop
cancel_probe0:
	/* Cancel probe timer, if it is not required. */
	/* tp->sto.probe0s_out = 0; */
	tblwr		d.probe0s_out, r0
	/* tp->sto.probe0_backoff = 0; */
	tblwr		d.probe0_backoff, r0
	
tcp_send_probe0_done:	
	sne		c4, r7, r0
	jr.c4		r7
	add		r7, r0, r0

	
flow_slow_timer_process_done:
	
table_read_TX:
	TCP_NEXT_TABLE_READ(k.fid, TABLE_TYPE_RAW, flow_tx_process,
	                    TCP_TCB_TABLE_BASE, TCP_TCB_TABLE_ENTRY_SIZE_SHFT,
	                    TCP_TCB_TX_OFFSET, TCP_TCB_TABLE_ENTRY_SIZE)
	nop.e
	nop

