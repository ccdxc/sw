/*
 *	Implements the tnmdr ring desc of the TxDMA P4+ tls  pipeline
 */

#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"

 /* d is the data returned by lookup result */
struct d_struct {
	odesc			 : ADDRESS_WIDTH ;
};

/* Readonly Parsed packet header info for the current packet */
struct k_struct {
	fid				: 32 ;
	qstate_addr			: ADDRESS_WIDTH ;
};

struct p_struct p;
struct k_struct k;
struct d_struct d;

%%
	
        .param          tls_queue_brq_start
        .param          TNMDR_TABLE_BASE
	.align
tls_tdesc_alloc_start:

        CAPRI_CLEAR_TABLE0_VALID

	phvwr		p.odesc, d.odesc

table_read_QUEUE_BRQ:
	CAPRI_NEXT_TABLE0_READ(k.fid, TABLE_LOCK_EN, tls_queue_brq_start,
	                    k.qstate_addr, TLS_TCB_TABLE_ENTRY_SIZE_SHFT,
	                    TLS_TCB_OFFSET, TABLE_SIZE_512_BITS)
	nop.e
	nop

