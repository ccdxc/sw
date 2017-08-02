/*
 *	Implements the descriptor stage of the TxDMA P4+ pipeline
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
	TNMDR_alloc_idx			: RING_INDEX_WIDTH ;
};

struct p_struct p	;
struct k_struct k	;
struct d_struct d	;
%%
	
flow_tdesc_alloc_process_start:
	phvwr		p.descr, d.desc

table_read_tx2:
	TCP_NEXT_TABLE_READ(k.fid, TABLE_TYPE_RAW, flow_tx2_process,
	                    TCP_TCB_TABLE_BASE, TCP_TCB_TABLE_ENTRY_SIZE_SHFT,
	                    TCP_TCB_TX_OFFSET, TCP_TCB_TABLE_ENTRY_SIZE)


flow_desc_alloc_process_done:	

