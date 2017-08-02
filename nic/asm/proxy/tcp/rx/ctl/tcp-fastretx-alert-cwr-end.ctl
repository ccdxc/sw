#include "../include/tcp-constants.h"
#include "../include/tcp-phv.h"
#include "../include/tcp-shared-state.h"
#include "../include/tcp-macros.h"
#include "../include/tcp-table.h"

 /* d is the data returned by lookup result */
struct d_struct {
	/* State needed only by FAST RETX ALERT stage */
	do_lost				: 1  ;
	fast_rexmit			: 2  ;
	ack_flag			: 16 ;
	pkts_acked			: 8  ;
	curr_ts				: TS_WIDTH ;
	/* State needed by RX and TX pipelines
	 * This has to be at the beginning.
	 * Each of these fields will be written by Rx only or Tx only
	 */
	
        snd_ssthresh			: WINDOW_WIDTH	        ;\
	fackets_out			: COUNTER16	        ;\
	reordering			: COUNTER32	        ;\
	prior_ssthresh			: WINDOW_WIDTH	        ;\
	ca_state			: 8	                ;\
	retx_head_ts			: TS_WIDTH	        ;\
	high_seq			: SEQ_NUMBER_WIDTH	;\
	undo_marker			: SEQ_NUMBER_WIDTH	;\
};

/* Readonly Parsed packet header info for the current packet */
struct k_struct {
	fid				: 32 ;
	syn				: 1 ;
	ece				: 1 ;
	cwr				: 1 ;
	ooo_rcv				: 1 ;
	is_dupack			: 1 ;
	rsvd				: 3 ;
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

	packets_out			: COUNTER16	        ;\
	sacked_out			: COUNTER16	        ;\
	srtt_us				: TS_WIDTH              ;\
	rto_deadline			: TS_WIDTH	        ;\
	lost_out			: COUNTER8	        ;\
	snd_una				: SEQ_NUMBER_WIDTH	;\
	snd_wnd				: SEQ_NUMBER_WIDTH	;\
	undo_retrans			: SEQ_NUMBER_WIDTH	;\
        snd_cwnd                        : WINDOW_WIDTH          ;\
	loss_cwnd			: WINDOW_WIDTH	        ;\
	ecn_flags			: 8	                ;\


};

struct p_struct p	;
struct k_struct k	;
struct d_struct d	;

d = {
  ca_state		= 0x2;




  high_seq		= 0x10;
};

k = {
  page 			= 0xA0;
  descr			= 0xB0;
  process_ack_flag	= 0x434;
  rcv_tsecr		= 0xE0;

  snd_una		= 0x16;


  snd_cwnd     		= 0x50;
  snd_wnd		= 0x50;




};

r4 = 0xD0;
