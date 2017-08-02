#include "../include/tcp-constants.h"
#include "../include/tcp-phv.h"
#include "../include/tcp-shared-state.h"
#include "../include/tcp-macros.h"
#include "../include/tcp-table.h"
	
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

d = {
  kto_deadline		= 0xC0;
};

k = {
  page 			= 0xA0;
  descr			= 0xB0;
  process_ack_flag	= 0x434;
  rcv_tsecr		= 0xE0;

  snd_una		= 0x16;

  snd_nxt 		= 0x26;
  snd_cwnd     		= 0x10;
  snd_wnd		= 0x10;

};

r4 = 0xD0;
