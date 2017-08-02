/*
 *	Implements the getting tickets for object allocations
 */

#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"	

/* d is the data returned by lookup result */
struct d_struct {
	descr_idx			: RING_INDEX_WIDTH ;
	page_idx			: RING_INDEX_WIDTH ;
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
	
flow_get_tkt_start:
	phvwr		p.page_idx, d.page_idx
	phvwr		p.descr_idx, d.descr_idx
rnmdr_read:	
	addi		r1, r0, RNMDR_TABLE_BASE
	add 		r2, d.descr_idx, r0
	sll		r2, r2, RNMDR_TABLE_ENTRY_SIZE_SHFT
	add		r1, r1, r2


	phvwr		p.table_sel, TABLE_TYPE_RAW
	phvwr		p.table_addr, r1

rnmpr_read:	
	addi		r1, r0, RNMPR_TABLE_BASE
	add 		r2, d.page_idx, r0
	sll		r2, r2, RNMPR_TABLE_ENTRY_SIZE_SHFT
	add		r1, r1, r2


	phvwri		p.table_sel, TABLE_TYPE_RAW
	phvwr		p.table_addr, r1
	phvwri.e	p.table_mpu_entry_raw, flow_get_obj
