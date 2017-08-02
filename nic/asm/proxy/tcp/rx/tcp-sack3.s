/*
 *	Implements the SACK stage of the RxDMA P4+ pipeline
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

	/* State needed only by SACK stage */

	ooo_head_cursor			: ADDRESS_WIDTH    ;
	ooo_tail_cursor			: ADDRESS_WIDTH	;
	delivered			: COUNTER32	;
	lost				: COUNTER32	;


	l_cell				: 8	;
	r_cell				: 8	;
	l_aol_start_seq			: SEQ_NUMBER_WIDTH	;
	aol_idx				: 8	;
	l_chop				: 8	;
	r_chop				: 8	;
	aol_page                        : ADDRESS_WIDTH    ;
//	aol_scratch			: 64    ;
	aol_free_pending		: 1	;
	aol_valid			: 1	;
	aol_addr			: ADDRESS_WIDTH	;
	aol_offset			: OFFSET_WIDTH	;
	aol_len				: LEN_WIDTH	;

//	rcv_aol_scratch			: 64    ;
	rcv_aol_free_pending		: 1	;
	rcv_aol_valid			: 1	;
	rcv_aol_addr			: ADDRESS_WIDTH	;
	rcv_aol_offset			: OFFSET_WIDTH	;
	rcv_aol_len			: LEN_WIDTH	;

	rcv_cursor_addr			: ADDRESS_WIDTH ;
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
	window				: WINDOW_WIDTH ;
	process_ack_flag		: 16  ;
	descr_idx			: RING_INDEX_WIDTH ;
	page_idx			: RING_INDEX_WIDTH ;
	descr				: ADDRESS_WIDTH ;
	page				: ADDRESS_WIDTH ;
	aol_zero_scratch                : 1  ;
	aol_page_alloc                  : 1  ;
	aol_scratch                     : 64 ;
	aol_free_pending		: 1;
	aol_valid			: 1;
	aol_addr			: ADDRESS_WIDTH;
	aol_offset			: OFFSET_WIDTH;
	aol_len				: LEN_WIDTH;

	rcv_aol_scratch                 : 64 ;
	rcv_aol_free_pending		: 1;
	rcv_aol_valid			: 1;
	rcv_aol_addr			: ADDRESS_WIDTH;
	rcv_aol_offset			: OFFSET_WIDTH;
	rcv_aol_len			: LEN_WIDTH;

	pending_ooo_se_recv		: 1                     ;\
	
	
};

struct p_struct p	;
struct k_struct k	;
struct d_struct d	;

%%
	
flow_sack3_process_start:

	tblwr		d.rcv_aol_free_pending, k.rcv_aol_free_pending
	tblwr		d.rcv_aol_valid, k.rcv_aol_valid
	tblwr		d.rcv_aol_addr, k.rcv_aol_addr
	tblwr           d.rcv_aol_offset, k.rcv_aol_offset
	tblwr		d.rcv_aol_len, k.rcv_aol_len

	sne		c1, k.pending_ooo_se_recv, r0
	bcf		[!c1], table_read_CC

tcp_ooo_se_recv:
	sne		c1, d.rcv_cursor_addr, r0
	bcf		[!c1], read_aol_entry
	nop
read_aol_entry:	
	sne		c1, d.ooo_head_cursor, r0
	bcf		[!c1], table_read_CC
	nop
	phvwr		p.table_sel, TABLE_TYPE_RAW
	phvwr		p.table_mpu_entry_raw, flow_read_se_rcv_aol_entry_process
	phvwr.e		p.table_addr, d.ooo_head_cursor
	
flow_sack3_process_done:
table_read_CC:
	TCP_NEXT_TABLE_READ(k.fid, TABLE_TYPE_RAW, flow_cc_process,
	                    TCP_TCB_TABLE_BASE, TCP_TCB_TABLE_ENTRY_SIZE_SHFT,
	                    TCP_TCB_CC_OFFSET, TCP_TCB_TABLE_ENTRY_SIZE)



