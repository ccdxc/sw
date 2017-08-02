/*
 *	Get an index and auto increment it.
 *      This stage will be used to get
	      - RNMPR alloc idx
 */

#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"

 /* d is the data returned by lookup result */
struct d_struct {
	RNMPR_alloc_idx			: 8    ;
};

/* Readonly Parsed packet header info for the current packet */
struct k_struct {
	semaphore_addr			: ADDRESS_WIDTH ;
};

struct p_struct p	;
struct k_struct k	;
struct d_struct d	;
	
%%
	
flow_read_rnmpr_alloc_idx:
	phvwr.e		p.RNMPR_alloc_idx, d.RNMPR_alloc_idx
table_read_RNMPR_PAGE:
	TCP_NEXT_TABLE_READ(d.RNMPR_alloc_idx, TABLE_TYPE_RAW, flow_rpage_alloc_process,
	                    RNMPR_TABLE_BASE, RNMPR_TABLE_ENTRY_SIZE_SHFT,
	                    0, RNMPR_TABLE_ENTRY_SIZE)
	nop.e
	nop

