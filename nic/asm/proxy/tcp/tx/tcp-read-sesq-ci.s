/*
 *      This stage will be used to get
 *	      - SESQ consumer index
 *      Cannot auto increment it here till we have read the
 *	SESQ entry contents at this index which is in the next stage
 */

#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct phv_ p;
struct tcp_tx_read_sesq_ci_k k;
struct tcp_tx_read_sesq_ci_read_sesq_ci_d d;
	
%%
	
flow_read_sesq_ci:
	//TODO: phvwr		p.SESQ_cidx, d.SESQ_cidx_value

table_read_SESQ_ENTRY:
	CAPRI_NEXT_TABLE0_READ(d.sesq_cidx, TABLE_LOCK_EN, flow_sesq_read_process,
	                    SESQ_BASE, SESQ_ENTRY_SIZE_SHFT,
	                    0, TABLE_SIZE_16_BITS)
	nop.e
	nop
