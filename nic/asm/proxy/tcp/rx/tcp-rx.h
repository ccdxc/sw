/*
 *	Implements the RX stage of the RxDMA P4+ pipeline
 */

#ifndef __TCP_RX_H__
#define __TCP_RX_H__

#include "tcp-constants.h"
#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp-sched.h"
	
 /* d is the data returned by lookup result */
struct d_struct {
	/* State needed by RX and TX pipelines
	 * This has to be at the beginning.
	 * Each of these fields will be written by Rx only or Tx only
	 */
	rcv_nxt				: SEQ_NUMBER_WIDTH	;\
	quick				: 4	                ;\
	rto				: 8	                ;\
	snd_una				: SEQ_NUMBER_WIDTH	;\
        pending		                : 3                     ;\
	pending_txdma                   : 1			;\
	snd_wl1				: SEQ_NUMBER_WIDTH	;\
	snd_wscale			: 4	                ;\
	snd_wnd				: SEQ_NUMBER_WIDTH	;\
	ecn_flags			: 8	                ;\
	pingpong			: 1                     ;\
	
	retx_head_ts			: TS_WIDTH	        ;\
	rto_deadline			: TS_WIDTH	        ;\


	/* State needed only by RX stage */

	ato				: 4	;

	dup_ack_cnt			: COUNTER8	;
	pred_flags			: 8	;

	ca_flags			: 2	;
	fastopen_rsk			: 1	;
	delivered			: COUNTER8	;
	lost				: COUNTER8	;

	max_window			: WINDOW_WIDTH	;
	rcv_tstamp			: TS_WIDTH	;
	lrcv_time			: TS_WIDTH	;
//	curr_ts				: TS_WIDTH	;
	rcv_tsval			: TS_WIDTH	;
	ts_recent			: TS_WIDTH	;
//	ts_recent_tstamp		: TS_WIDTH	;
	bytes_rcvd			: COUNTER16	;
	bytes_acked			: COUNTER16	;
	write_serq                      : 1		;
	nde_free_pending		: 1	;
	nde_valid			: 1	;
	nde_addr			: ADDRESS_WIDTH	;
	nde_offset			: OFFSET_WIDTH	;
	nde_len				: LEN_WIDTH	;
	
};

/* Readonly Parsed packet header info for the current packet */
struct k_struct {
	fid				: 32 ;
	syn				: 1 ;
	ece				: 1 ;
	cwr				: 1 ;
	ooo_rcv				: 1 ;
	write_serq			: 1 ;
	rsvd				: 3 ;
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
	descr_idx			: RING_INDEX_WIDTH ;
	page_idx			: RING_INDEX_WIDTH ;
	descr				: ADDRESS_WIDTH ;
	page				: ADDRESS_WIDTH ;
	serq_pidx			: 8		;

	snd_nxt				: SEQ_NUMBER_WIDTH	;\
	rcv_mss_shft			: 4	                ;\
	rcv_wnd				: WINDOW_WIDTH          ;\
	packets_out			: COUNTER16	        ;\
};

struct p_struct p	;
struct k_struct k	;
struct d_struct d	;

#endif
