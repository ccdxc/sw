#include "../include/tcp-constants.h"
#include "../include/tcp-phv.h"
#include "../include/tcp-shared-state.h"
#include "../include/tcp-macros.h"
#include "../include/tcp-table.h"

 /* d is the data returned by lookup result */
struct d_struct {
	retx_snd_una			: SEQ_NUMBER_WIDTH ;
	retx_snd_nxt			: SEQ_NUMBER_WIDTH ;
	retx_head_desc			: ADDRESS_WIDTH ;
	retx_snd_una_cursor		: ADDRESS_WIDTH ;
	retx_tail_desc			: ADDRESS_WIDTH ;
	retx_snd_nxt_cursor		: ADDRESS_WIDTH ;
	retx_xmit_cursor		: ADDRESS_WIDTH ;
	retx_xmit_seq			: SEQ_NUMBER_WIDTH ;

	rcv_wup				: WINDOW_WIDTH ;

	xmit_cursor_flags		: 8  ;
	xmit_cursor_addr		: ADDRESS_WIDTH ;
	xmit_cursor_offset		: OFFSET_WIDTH ;
	xmit_cursor_len			: LEN_WIDTH ;
	
	/* ethhdr fields from hdr template */
//	h_dest				: 48 ;
//	h_source			: 48 ;

	/* iphdr fields from hdr template page */
//	saddr				: 32 ;
//	daddr				: 32 ;
	/* tcphdr fields from hdr template page */
	fid                             : 32 ;
	source				: 16 ;
	dest				: 16 ;
	pending_ack_tx			: 1  ;
	pending_delayed_ack_tx		: 1  ;

	/* State needed by RX and TX pipelines
	 * This has to be at the end.
	 * Each of these fields will be written by Rx only or Tx only
	 */
	
        prr_out				: COUNTER32	        ;\

};

/* Readonly Parsed packet header info for the current packet */
struct k_struct {
	fid				: 32 ;
	xmit_cursor_flags		: 8  ;
	xmit_cursor_addr		: ADDRESS_WIDTH ;
	xmit_cursor_offset		: OFFSET_WIDTH ;
	xmit_cursor_len			: LEN_WIDTH ;


	pending_challenge_ack_send	: 1	                ;\
	pending_ack_send		: 1	                ;\
	pending_sync_mss		: 1	                ;\
	pending_tso_keepalive           : 1                     ;\
	pending_tso_pmtu_probe          : 1                     ;\
	pending_tso_data		: 1                     ;\
	pending_tso_probe_data		: 1                     ;\
	pending_tso_probe		: 1                     ;\
	pending_ooo_se_recv		: 1                     ;\
	pending_tso_retx	        : 1                     ;\
	pending_rexmit		        : 2                     ;\

	rcv_nxt				: SEQ_NUMBER_WIDTH	;\
	rcv_wnd				: WINDOW_WIDTH          ;\
        rcv_mss                         : 8                     ;\
	ca_state			: 8	                ;\

};

struct p_struct p	;
struct k_struct k	;
struct d_struct d	;
	

d = {
  retx_snd_una		= 0x16;

  retx_tail_desc	= 0xD0;
  retx_snd_nxt_cursor   = 0x298;
  retx_snd_una_cursor   = 0x298;

  fid			= 0xAA;
  source		= 0xA5;
  dest			= 0x5A;
  retx_xmit_seq		= 0x20;
};

k = {
  fid			= 0xAA;
  xmit_cursor_flags	= 0;
  xmit_cursor_addr	= 0xA0;
  xmit_cursor_offset	= 0x40;
  xmit_cursor_len	= 0xB0;


  rcv_nxt		= 0x10;

  pending_tso_data	= 1;


  rcv_wnd		= 0x10;
  rcv_mss		= 0xFF;

};

r4 = 0xC0;
