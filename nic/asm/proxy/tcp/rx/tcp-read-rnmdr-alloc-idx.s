/*
 *	Get an index and auto increment it.
 *      This stage will be used to get
	      - RNMDR alloc idx
 */

#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"

 /* d is the data returned by lookup result */
struct d_struct {
	RNMDR_alloc_idx			: 8    ;
};

/* Readonly Parsed packet header info for the current packet */
struct k_struct {
	semaphore_addr			: ADDRESS_WIDTH ;
};

struct p_struct p	;
struct k_struct k	;
struct d_struct d	;
	
%%
	
flow_read_rnmdr_alloc_idx:
	phvwr		p.RNMDR_alloc_idx, d.RNMDR_alloc_idx

table_read_RNMDR_DESC:
	TCP_NEXT_TABLE_READ(d.RNMDR_alloc_idx, TABLE_TYPE_RAW, flow_rdesc_alloc_process,
	                    RNMDR_TABLE_BASE, RNMDR_TABLE_ENTRY_SIZE_SHFT,
	                    0, RNMDR_TABLE_ENTRY_SIZE)
	nop.e
	nop
