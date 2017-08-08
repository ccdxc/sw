/*
 * 	Implements the reading of SESQ entry to queue the desc to TCP tx q
 */

#include "tcp-constants.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "ingress.h"
#include "INGRESS_p.h"
	
struct phv_ p;
struct tcp_tx_read_sesq_k k;
struct tcp_tx_read_sesq_read_sesq_d d;
	
	
%%
	
flow_sesq_read_process_start:
    phvwr               p.common_phv_fid, d.sesq_fid
	//TODO: phvwr		p.seqe_fid, d.seqe_fid
	//TODO: phvwr		p.seqe_desc, d.seqe_desc

	CAPRI_NEXT_IDX1_READ(TABLE_LOCK_DIS, flow_sesq_consume_process,
	                    SESQ_CONSUMER_IDX, TABLE_SIZE_16_BITS)

	// TODO: CAPRI_NEXT_TABLE0_READ(d.sesq_fid, TABLE_LOCK_EN, flow_queue_sched_process,
	CAPRI_NEXT_TABLE0_READ(d.sesq_fid, TABLE_LOCK_EN, flow_tx_process,
	                    TCP_TCB_TABLE_BASE, TCP_TCB_TABLE_ENTRY_SIZE_SHFT,
	                    TCP_TCB_TX_OFFSET, TABLE_SIZE_512_BITS)
	nop.e
	nop
