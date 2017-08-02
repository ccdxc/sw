/*
 *	Get an index and auto increment it.
 *      This stage will be used to get
	      - RNMDR alloc idx
	      - RNMPR alloc idx
	      - TNMDR alloc idx
	      - SERQ producer idx
 */

#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"

 /* d is the data returned by lookup result */
struct d_struct {
	scratch				: 64    ;
};

/* Readonly Parsed packet header info for the current packet */
struct k_struct {
	serq_pidx_get			: 1 ;
	rnmdr_aidx_get			: 1 ;
	rnmpr_aidx_get			: 1 ;
	tnmdr_aidx_get			: 1 ;
};

struct p_struct p	;
struct k_struct k	;
struct d_struct d	;
	
%%
	
flow_read_idx_start:
	sne		c1, k.serq_pidx_get, r0
	sne		c2, k.rnmdr_aidx_get, r0
	sne		c3, k.rnmpr_aidx_get, r0
	sne		c4, k.tnmdr_aidx_get, r0
	
serq_pidx_get:
	TCP_READ_IDX(SERQ_PRODUCER_IDX, TABLE_TYPE_RAW, flow_read_serq_pidx_process)

rnmdr_aidx_get:
	TCP_READ_IDX(RNMDR_ALLOC_IDX, TABLE_TYPE_RAW, flow_read_rnmdr_aidx_process)

rnmpr_aidx_get:
	TCP_READ_IDX(RNMPR_ALLOC_IDX, TABLE_TYPE_RAW, flow_read_rnmpr_aidx_process)
	
tnmdr_aidx_get:
	TCP_READ_IDX(TNMDR_ALLOC_IDX, TABLE_TYPE_RAW, flow_read_tnmdr_aidx_process)
	
