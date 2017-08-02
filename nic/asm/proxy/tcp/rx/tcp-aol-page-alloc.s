/*
 *	Implements the SACK stage of the RxDMA P4+ pipeline
 */

#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"

 /* d is the data returned by lookup result */
struct d_struct {
	aol_page			: ADDRESS_WIDTH    ;
};

/* Readonly Parsed packet header info for the current packet */
struct k_struct {
	fid				: 32 ;
	RNMPR_alloc_idx			: RING_INDEX_WIDTH ;
	write_serq			: 1		   ;
	desc				: ADDRESS_WIDTH	   ;
};
struct p_struct p;
struct k_struct k;
struct d_struct d;

%%
	
flow_aol_page_alloc_process_start:
	phvwr		p.page, d.aol_page
	phvwri		p.aol_page_alloc, 1
	sne		c1, k.write_serq, r0
	bcf		[c1], table_read_WRITE_SERQ
	nop

table_read_SACK2:
	TCP_NEXT_TABLE_READ(k.fid, TABLE_TYPE_RAW, flow_sack2_process,
	                    TCP_TCB_TABLE_BASE, TCP_TCB_TABLE_ENTRY_SIZE_SHFT,
	                    TCP_TCB_SACK_OFFSET, TCP_TCB_TABLE_ENTRY_SIZE)

table_read_WRITE_SERQ:	
	TCP_NEXT_TABLE_READ(k.fid, TABLE_TYPE_RAW, flow_write_serq_process,
	                    TCP_TCB_TABLE_BASE, TCP_TCB_TABLE_ENTRY_SIZE_SHFT,
	                    TCP_TCB_WRITE_SERQ_OFFSET, TCP_TCB_TABLE_ENTRY_SIZE)

