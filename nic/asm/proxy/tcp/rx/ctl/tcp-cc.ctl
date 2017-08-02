#include "../include/tcp-constants.h"
#include "../include/tcp-phv.h"
#include "../include/tcp-shared-state.h"
#include "../include/tcp-macros.h"
#include "../include/tcp-table.h"

	
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
	

d = {
  snd_cwnd     		= 0x10;

  max_packets_out	= 0x07;
  is_cwnd_limited 	= 0x00;
  last_max_cwnd		= 0x16;

};

k = {
  seq			= 0x10;
  ack_seq 		= 0x1F;
  end_seq 		= 0x20;
  process_ack_flag	= 0x34;


  snd_ssthresh  	= 0x08;
};

r4 = 0xC0;
