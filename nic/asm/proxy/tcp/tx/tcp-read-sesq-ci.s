/*
 *      This stage will be used to get
 *	      - SESQ consumer index
 *      Cannot auto increment it here till we have read the
 *	SESQ entry contents at this index which is in the next stage
 */

#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"

 /* d is the data returned by lookup result */
struct d_struct {
	SESQ_cidx_value			: 8    ;
};

/* Readonly Parsed packet header info for the current packet */
struct k_struct {
	semaphore_addr			: ADDRESS_WIDTH ;
};

struct p_struct p	;
struct k_struct k	;
struct d_struct d	;
	
%%
	
flow_read_sesq_ci:
	phvwr		p.SESQ_cidx, d.SESQ_cidx_value

table_read_SESQ_ENTRY:
	TCP_NEXT_TABLE_READ(d.SESQ_cidx_value, TABLE_TYPE_RAW, flow_sesq_read_process,
	                    SESQ_BASE, SESQ_ENTRY_SIZE_SHFT,
	                    0, SESQ_ENTRY_SIZE)
	nop.e
	nop
