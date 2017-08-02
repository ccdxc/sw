/*
 *	Implements the descr allocation stage of the RxDMA P4+ pipeline
 */

#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
	
 /* d is the data returned by lookup result */
struct d_struct {
	desc				: ADDRESS_WIDTH    ;
};

/* Readonly Parsed packet header info for the current packet */
struct k_struct {
	fid				: 32 ;
	RNMDR_alloc_idx			: RING_INDEX_WIDTH ;
	write_serq			: 1 ;
	
};

struct p_struct p	;
struct k_struct k	;
struct d_struct d	;
%%
	
flow_rdesc_alloc_process_start:
	phvwr		p.descr, d.desc
	phvwr		p.write_serq, k.write_serq
	sne		c1, k.write_serq, r0
	bcf		[c1], flow_rdesc_alloc_process_done
	nop

table_read_SACK4:
	TCP_NEXT_TABLE_READ(k.fid, TABLE_TYPE_RAW, flow_sack4_process,
	                    TCP_TCB_TABLE_BASE, TCP_TCB_TABLE_ENTRY_SIZE_SHFT,
	                    TCP_TCB_SACK_OFFSET, TCP_TCB_TABLE_ENTRY_SIZE)

flow_rdesc_alloc_process_done:	

