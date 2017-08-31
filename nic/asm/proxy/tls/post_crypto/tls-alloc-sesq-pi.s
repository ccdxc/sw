/*
 *	Get an index and auto increment it.
 *      This stage will be used to get
	      - SESQ producer idx
 */

#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"

 /* d is the data returned by lookup result */
struct d_struct {
	sesq_pidx			: 8    ;
};

/* Readonly Parsed packet header info for the current packet */
struct k_struct {
	semaphore_addr			: ADDRESS_WIDTH ;
	fid                             : 16		;
};

struct p_struct p	;
struct k_struct k	;
struct d_struct d	;
	
%%
	
flow_alloc_sesq_pi_process:
	phvwr		p.sesq_pidx, d.sesq_pidx

table_read_queue_sesq:
	TLS_NEXT_TABLE_READ(k.fid, TABLE_TYPE_RAW, tls_queue_sesq_process,
	                    TLS_TCB_TABLE_BASE, TLS_TCB_TABLE_ENTRY_SIZE_SHFT,
	                    TLS_TCB_OFFSET, TLS_TCB_TABLE_ENTRY_SIZE)
	nop.e
	nop
