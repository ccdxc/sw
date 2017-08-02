/*
 *	Implements the page alloc stage of the RxDMA P4+ tls  pipeline
 */

#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"

 /* d is the data returned by lookup result */
struct d_struct {
	opage				: ADDRESS_WIDTH    ;
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
	phvwr		p.opage, d.opage
	nop

table_read_queue_brq:
	TLS_NEXT_TABLE_READ(k.fid, TABLE_TYPE_RAW, tls_queue_brq_process,
	                    TLS_TCB_TABLE_BASE, TLS_TCB_TABLE_ENTRY_SIZE_SHFT,
	                    TLS_TCB_OFFSET, TLS_TCB_TABLE_ENTRY_SIZE)
	nop.e
	nop
