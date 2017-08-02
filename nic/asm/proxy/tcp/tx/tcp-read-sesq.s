/*
 * 	Implements the reading of SESQ entry to queue the desc to TCP tx q
 */

#include "tcp-constants.h"
#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
	
/* d is the data returned by lookup result */
struct d_struct {
        seqe_fid                         : 16;
        seqe_desc                        : ADDRESS_WIDTH;
};

/* SERQ consumer index */
struct k_struct {
	sesq_ci				: RING_INDEX_WIDTH ;
};

struct p_struct p	;
struct k_struct k	;
struct d_struct d	;
	
%%
	
flow_sesq_read_process_start:
	phvwr		p.seqe_fid, d.seqe_fid
	phvwr		p.seqe_desc, d.seqe_desc

	TCP_READ_IDX(SESQ_CONSUMER_IDX, TABLE_TYPE_RAW, flow_sesq_consume_process)

	TCP_NEXT_TABLE_READ(d.seqe_fid, TABLE_TYPE_RAW, flow_queue_sched_process,
	                    TCP_TCB_TABLE_BASE, TCP_TCB_TABLE_ENTRY_SIZE_SHFT,
	                    TCP_TCB_TX_OFFSET, TCP_TCB_TABLE_ENTRY_SIZE)
	nop.e
	nop
