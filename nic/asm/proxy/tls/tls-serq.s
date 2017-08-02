/*
 * 	Implements the reading of SERQ to pick up the new descriptor to process
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
	
/* d is the data returned by lookup result */
struct d_struct {
        seqe_fid                         : 16;
        seqe_desc                        : ADDRESS_WIDTH;
};

/* SERQ consumer index */
struct k_struct {
	serq_consumer_idx		: RING_INDEX_WIDTH ;
};

struct p_struct p	;
struct k_struct k	;
struct d_struct d	;
	
%%
	
tls_serq_read_process_start:
	add		r1, d.seqe_fid, r0
	smeqh		c1, r1, ENC_FLOW_ID_MASK, ENC_FLOW_ID_MASK
	phvwri.c1	p.enc_flow, 1
	phvwri.!c1      p.enc_flow, 0
	
	phvwr		p.seqe_fid, d.seqe_fid
	phvwr		p.seqe_desc, d.seqe_desc
	phvwri		p.pending_rx_serq, 1

	TLS_READ_IDX(SERQ_CONSUMER_IDX, TABLE_TYPE_RAW, tls_serq_consume_process)
	
table_read_DESC:
	add		r2, d.seqe_desc, r0
	TLS_READ_ADDR(r2, TABLE_TYPE_RAW, tls_read_desc_process)
	nop.e
	nop
