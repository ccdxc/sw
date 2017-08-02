#include "../include/tcp-constants.h"
#include "../include/tcp-phv.h"
#include "../include/tcp-shared-state.h"
#include "../include/tcp-macros.h"
#include "../include/tcp-table.h"
	
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

k = {
  seq			= 0x10;
  ack_seq 		= 0x1F;
  end_seq 		= 0x20;
  page 			= 0xA0;
  descr			= 0xB0;
  process_ack_flag	= 0x434;
  rcv_tsecr		= 0xE0;

  snd_nxt 		= 0x20;
};

r4 = 0xF0;
