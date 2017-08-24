/*
 * 	Implements the reading of SERQ to pick up the new descriptor to process
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "ingress.h"
#include "INGRESS_p.h"
	
/* d is the data returned by lookup result */

/* SERQ consumer index */
struct k_struct {
	serq_consumer_idx		: RING_INDEX_WIDTH ;
};

struct phv_ p	;
struct k_struct k	;
struct tx_table_s0_t0_d d	;
	
%%

	.param		tls_serq_consume_process
	.param		tls_read_desc_process_start
	
tls_stage0:
	phvwr	    p.tls_global_phv_dec_flow, d.u.read_tls_stg0_d.dec_flow
	
	phvwr		p.tls_global_phv_fid, d.u.read_tls_stg0_d.fid
    add         r3, r0, d.u.read_tls_stg0_d.ci_0
    sll         r3, r3, NIC_SERQ_ENTRY_SIZE_SHIFT
    add         r3, r3, d.u.read_tls_stg0_d.serq_base

	phvwr		p.tls_global_phv_desc, r3
	phvwri		p.tls_global_phv_pending_rx_serq, 1

#	TLS_READ_IDX(SERQ_CONSUMER_IDX, TABLE_TYPE_RAW, tls_serq_consume_process)
	
table_read_DESC:
    CAPRI_NEXT_IDX0_READ(TABLE_LOCK_DIS, tls_read_desc_process_start, r3, TABLE_SIZE_64_BITS)
	nop.e
	nop
