/*
 *	Implements the getting tickets for object allocations
 */
#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"	

/* d is the data returned by lookup result */
struct d_struct {
	descr				: ADDRESS_WIDTH ;
	page				: ADDRESS_WIDTH ;
};

/* Readonly Parsed packet header info for the current packet */
struct k_struct {
	fid				: 32 ;
	syn				: 1 ;
	ece				: 1 ;
	cwr				: 1 ;
	ooo_rcv				: 1 ;
	rsvd				: 4 ;
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
};

struct p_struct p	;
struct k_struct k	;
struct d_struct d	;
	
%%

flow_get_obj_start:
	phvwr		p.page, d.page
	phvwr		p.descr, d.descr
table_read_RX:	
	TCP_NEXT_TABLE_READ(k.fid, TABLE_TYPE_RAW, flow_rx_process,
	                    TCP_TCB_TABLE_BASE, TCP_TCB_TABLE_ENTRY_SIZE_SHFT,
	                    TCP_TCB_RX_OFFSET, TCP_TCB_TABLE_ENTRY_SIZE)
	
